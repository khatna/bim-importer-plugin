#!/bin/bash
cd "${0%/*}/assimp"

ANDROID_NDK="/Users/developer/Library/Android/sdk/ndk/21.4.7075529"

PATH=$PATH:/opt/homebrew/bin

cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK \
      -DANDROID_PLATFORM=android-29 \
      -DANDROID_ABI=arm64-v8a \
      -DASSIMP_ANDROID_JNIIOSYSTEM=ON \
      -DCMAKE_BUILD_TYPE=Release \
      .
cmake --build . -j4