#!/bin/bash
set -euo pipefail

# --- 기본 설정 변수 ---
use_ninja=0
clean_build=0
jobs="${BUILD_JOBS:-$(nproc)}"       # 코어 수만큼 병렬 빌드 (기본값)
build_type="Release"                 # 기본은 Release
use_asan=0                           # AddressSanitizer (메모리 오류 검출용)

# --- 사용법 출력 함수 ---
usage() {
    cat >&2 <<'EOF'
Usage: build.sh [options]

Options:
  -n, --use-ninja        Use Ninja generator (faster)
  -c, --clean            Clean build directory before building
  -t, --build-type TYPE  Set CMAKE_BUILD_TYPE (Debug, Release, RelWithDebInfo)
      --debug            Shortcut for --build-type Debug
      --asan             Build with AddressSanitizer (Debug memory issues)
  -h, --help             Show this help message
EOF
}

# --- 인자 파싱 (Argument Parsing) ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        -n|--use-ninja)
            use_ninja=1
            ;;
        -c|--clean)
            clean_build=1
            ;;
        -t|--build-type)
            shift
            if [[ $# -eq 0 ]]; then
                echo "Error: --build-type requires an argument" >&2
                usage
                exit 1
            fi
            build_type="$1"
            ;;
        --debug)
            build_type="Debug"
            ;;
        --asan)
            use_asan=1
            build_type="Debug" # ASan은 보통 Debug 모드와 함께 사용
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Error: unknown option '$1'" >&2
            usage
            exit 1
            ;;
    esac
    shift
done

# --- 환경 변수 확인 (기존 스크립트 로직 유지) ---
echo "@@@@ SKNANO_BUILDDIR=${SKNANO_BUILDDIR}"
echo "@@@@ SKNANO_LIB=${SKNANO_LIB}"
echo "@@@@ SKNANO_INSTALLDIR=${SKNANO_INSTALLDIR}"
echo "@@@@ SKNANO_HOME=${SKNANO_HOME}"

# --- Clean Build 처리 ---
if [ "${clean_build}" -eq 1 ]; then
    echo "@@@@ Clean build requested: Removing ${SKNANO_BUILDDIR} and ${SKNANO_LIB}"
    rm -rf "${SKNANO_BUILDDIR}" "${SKNANO_LIB}"
else
    echo "@@@@ Incremental build: Reusing existing artifacts"
fi

mkdir -p "${SKNANO_BUILDDIR}"
cd "${SKNANO_BUILDDIR}"

# --- CorrectionLib 설정 ---
# python 모듈 실행 결과를 캡처합니다.
export CORRECTION_CMAKE_PREFIX
CORRECTION_CMAKE_PREFIX="$(python -m correctionlib.cli config --cmake)"
# 결과를 배열로 변환 (공백 처리 안전하게)
read -r -a CORRECTION_ARGS <<< "${CORRECTION_CMAKE_PREFIX}"

# --- Compiler Flags 설정 ---
# 기존 환경 변수 가져오기
export CXXFLAGS="${CXXFLAGS:-}"
export CFLAGS="${CFLAGS:-}"

# 디버그/ASan 플래그 추가
if [ "${use_asan}" -eq 1 ]; then
    asan_flags="-fsanitize=address -fno-omit-frame-pointer"
    export CXXFLAGS="${CXXFLAGS} ${asan_flags}"
    export CFLAGS="${CFLAGS} ${asan_flags}"
    export LDFLAGS="${LDFLAGS:-} ${asan_flags}"
    echo "@@@@ AddressSanitizer Enabled"
fi

# 색상 강제 출력 (CI/CD 로그 등에서 유용)
export CXXFLAGS="${CXXFLAGS} -fdiagnostics-color=always"

# --- CMake 명령어 구성 (배열 사용) ---
cmake_args=(
    -DCMAKE_INSTALL_PREFIX="${SKNANO_INSTALLDIR}"
    "${CORRECTION_ARGS[@]}"
    -DCMAKE_PREFIX_PATH="${LIBTORCH_INSTALL_DIR}"
    -DCMAKE_BUILD_TYPE="${build_type}"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON  # IDE용 컴파일 DB 생성 (매우 중요)
)

cmake_cmd=(cmake)

if [ "${use_ninja}" -eq 1 ]; then
    cmake_cmd+=(-GNinja)
fi

# CMake 실행
cmake_cmd+=("${cmake_args[@]}" "${SKNANO_HOME}")
printf '@@@@ Configuring: %s\n' "${cmake_cmd[*]}"
"${cmake_cmd[@]}"

# --- 빌드 및 설치 명령어 구성 ---
if [ "${use_ninja}" -eq 1 ]; then
    # ninja -d stats 옵션은 필요하면 추가
    build_cmd=(ninja -j "${jobs}")
    install_cmd=(ninja install)
else
    build_cmd=(make -j"${jobs}")
    install_cmd=(make install)
fi

# 빌드 실행
printf '@@@@ Building: %s\n' "${build_cmd[*]}"
"${build_cmd[@]}"

# 설치 실행
echo "@@@@ Installing to ${SKNANO_INSTALLDIR} (and/or ${SKNANO_LIB})"
# 기존 스크립트에서 cd $SKNANO_BUILDDIR && make install 했으므로 여기서 실행
"${install_cmd[@]}"

echo "@@@@ Build Complete (Type: ${build_type})"