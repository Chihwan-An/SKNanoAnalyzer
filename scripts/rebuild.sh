#!/bin/bash
set -euo pipefail
# Rebuild the project without clean
# check if the build directory exists
if [ -d "build/${SYSTEM}" ]; then
    cd build/${SYSTEM}
    cmake --build . --parallel "${BUILD_JOBS:-$(nproc)}"
    cmake --build . --target install --parallel "${BUILD_JOBS:-$(nproc)}"
    cd -
else
    echo "Build directory does not exist. Run build.sh first."
fi
