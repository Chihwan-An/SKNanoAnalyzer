#!/bin/bash
# Rebuild the project without clean
# check if the Makefile exists (created by cmake)
cd $SKNANO_BUILDDIR
if [ -f "Makefile" ]; then
    make && make install
else
    echo "Makefile does not exist. Run build.sh first."
fi
