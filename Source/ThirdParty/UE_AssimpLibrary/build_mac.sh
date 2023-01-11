#!/bin/bash
cd "${0%/*}/assimp"

cmake CMakeLists.txt -DLIBRARY_SUFFIX:STRING= -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
cmake --build . -j4