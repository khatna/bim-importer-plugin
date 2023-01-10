cd /D "%~dp0"
cd assimp
echo HELLO
cmake CMakeLists.txt -DLIBRARY_SUFFIX:STRING= 
cmake --build . --config release -j4