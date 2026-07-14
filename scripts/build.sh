#!/bin/bash
set -euo pipefail

use_ninja=0
clean_build=0
jobs="${BUILD_JOBS:-$(nproc)}"
build_type="${SKNANO_BUILD_TYPE:-Release}"
use_asan=0

usage() {
    cat >&2 <<'EOF'
Usage: build.sh [options]

Options:
  -n, --use-ninja        Configure CMake with the Ninja generator
  -c, --clean            Remove the build directory and previously installed library
  -t, --build-type TYPE  Set CMAKE_BUILD_TYPE (default: Release)
      --debug            Shortcut for --build-type Debug
      --relwithdebinfo   Shortcut for --build-type RelWithDebInfo
      --asan             Build with AddressSanitizer
  -h, --help             Show this help message and exit
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        use_ninja|--use-ninja|-n)
            use_ninja=1
            ;;
        clean|--clean|-c)
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
        --relwithdebinfo)
            build_type="RelWithDebInfo"
            ;;
        --asan)
            use_asan=1
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

echo "@@@@ Prepare to build SKNanoAnalyzer in ${SKNANO_BUILDDIR}"
if [ "${clean_build}" -eq 1 ]; then
    echo "@@@@ Clean build requested – removing ${SKNANO_BUILDDIR} and ${SKNANO_INSTALLDIR}"
    rm -rf "${SKNANO_BUILDDIR}" "${SKNANO_INSTALLDIR}"
else
    echo "@@@@ Incremental build – reusing existing artifacts"
fi
echo "@@@@ CMAKE_BUILD_TYPE set to ${build_type}"

mkdir -p "${SKNANO_BUILDDIR}"
cd "${SKNANO_BUILDDIR}"

export CORRECTION_CMAKE_PREFIX
CORRECTION_CMAKE_PREFIX="$(correction config --cmake)"
read -r -a CORRECTION_ARGS <<< "${CORRECTION_CMAKE_PREFIX}"

opt_info_dir="$(pwd)/opt-info"
mkdir -p "${opt_info_dir}"

base_cxxflags="${CXXFLAGS:-}"
base_cflags="${CFLAGS:-}"
base_cxxflags="${base_cxxflags//-fopt-info[^ ]*/}"
base_cflags="${base_cflags//-fopt-info[^ ]*/}"
base_cxxflags="${base_cxxflags//-fopt-info-vec[^ ]*/}"
base_cflags="${base_cflags//-fopt-info-vec[^ ]*/}"
base_cxxflags="${base_cxxflags//-fopt-info-vec-missed[^ ]*/}"
base_cflags="${base_cflags//-fopt-info-vec-missed[^ ]*/}"
base_cxxflags="${base_cxxflags//-fopt-info-vec-optimized[^ ]*/}"
base_cflags="${base_cflags//-fopt-info-vec-optimized[^ ]*/}"
export CXXFLAGS="${base_cxxflags} -fopt-info-vec-all=${opt_info_dir}/cxx-vectorization.txt"
export CFLAGS="${base_cflags} -fopt-info-vec-all=${opt_info_dir}/c-vectorization.txt"

if [ "${use_asan}" -eq 1 ]; then
    asan_flags="-fsanitize=address -fno-omit-frame-pointer"
    export CXXFLAGS="${CXXFLAGS} ${asan_flags}"
    export CFLAGS="${CFLAGS} ${asan_flags}"
    export LDFLAGS="${LDFLAGS:-} ${asan_flags}"
    echo "@@@@ AddressSanitizer enabled"
fi

cmake_args=(
    -DCMAKE_INSTALL_PREFIX="${SKNANO_INSTALLDIR}"
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}"
    -DCMAKE_C_FLAGS="${CFLAGS}"
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS:-}"
    -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS:-}"
    "${CORRECTION_ARGS[@]}"
    -DCMAKE_PREFIX_PATH="${LIBTORCH_INSTALL_DIR}"
    -DCMAKE_BUILD_TYPE="${build_type}"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

cmake_cmd=(cmake)
if [ "${use_ninja}" -eq 1 ]; then
    cmake_cmd+=(-GNinja)
fi
cmake_cmd+=("${cmake_args[@]}" "${SKNANO_HOME}")

printf '@@@@ %s\n' "${cmake_cmd[*]}"
"${cmake_cmd[@]}"

build_cmd=(cmake --build . --parallel "${jobs}")
install_cmd=(cmake --build . --target install --parallel "${jobs}")

printf '@@@@ %s\n' "${build_cmd[*]}"
"${build_cmd[@]}"

echo "@@@@ install to ${SKNANO_LIB}"
"${install_cmd[@]}"
