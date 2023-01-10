#!/bin/bash
cd "${0%/*}/assimp"
ANDROID_ABI=arm64-v8a
ANDROID_NDK="/Users/developer/Library/Android/sdk/ndk/21.4.7075529"
ANDROID_PLATFORM=android-28

cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=$ANDROID_ABI \
      -DANDROID_PLATFORM=$ANDROID_PLATFORM \
      -DASSIMP_ANDROID_JNIIOSYSTEM=ON \
      .
cmake --build . --config release -j4