cmake -G "Visual Studio 17 2022" -A x64 -S ../src -B .  -DMATHEMATICA_DIR=W:/wolfram/13.0 -DWOLFRAM_VERSION="13.0"

cmake --build . --config Release
