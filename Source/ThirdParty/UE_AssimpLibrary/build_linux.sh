#!/bin/bash
cd "${0%/*}/assimp"

cmake CMakeLists.txt -DLIBRARY_SUFFIX:STRING= 
cmake --build . -j4