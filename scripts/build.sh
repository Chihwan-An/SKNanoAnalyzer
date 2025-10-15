#!/bin/bash
set -euo pipefail

use_ninja=0
clean_build=0
jobs="${BUILD_JOBS:-$(nproc)}"

for arg in "$@"; do
    case "$arg" in
        use_ninja|--use-ninja|-n)
            use_ninja=1
            ;;
        clean|--clean|-c)
            clean_build=1
            ;;
        *)
            echo "Usage: $0 [use_ninja|--use-ninja|-n] [clean|--clean|-c]" >&2
            exit 1
            ;;
    esac
done

echo "@@@@ Prepare to build SKNanoAnalyzer in ${SKNANO_BUILDDIR}"
if [ "${clean_build}" -eq 1 ]; then
    echo "@@@@ Clean build requested – removing ${SKNANO_BUILDDIR} and ${SKNANO_LIB}"
    rm -rf "${SKNANO_BUILDDIR}" "${SKNANO_LIB}"
else
    echo "@@@@ Incremental build – reusing existing artifacts"
fi

mkdir -p "${SKNANO_BUILDDIR}"
cd "${SKNANO_BUILDDIR}"

export CORRECTION_CMAKE_PREFIX
CORRECTION_CMAKE_PREFIX="$(correction config --cmake)"
read -r -a CORRECTION_ARGS <<< "${CORRECTION_CMAKE_PREFIX}"

cmake_args=(
    -DCMAKE_INSTALL_PREFIX="${SKNANO_INSTALLDIR}"
    "${CORRECTION_ARGS[@]}"
    -DCMAKE_PREFIX_PATH="${LIBTORCH_INSTALL_DIR}"
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

cmake_cmd=(cmake)
if [ "${use_ninja}" -eq 1 ]; then
    cmake_cmd+=(-GNinja)
fi
cmake_cmd+=("${cmake_args[@]}" "${SKNANO_HOME}")

printf '@@@@ %s\n' "${cmake_cmd[*]}"
"${cmake_cmd[@]}"

if [ "${use_ninja}" -eq 1 ]; then
    build_cmd=(ninja -j "${jobs}")
    install_cmd=(ninja -j "${jobs}" install)
else
    build_cmd=(make -j"${jobs}")
    install_cmd=(make -j"${jobs}" install)
fi

printf '@@@@ %s\n' "${build_cmd[*]}"
"${build_cmd[@]}"

echo "@@@@ install to ${SKNANO_LIB}"
"${install_cmd[@]}"
