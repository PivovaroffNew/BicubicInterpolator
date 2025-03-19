cmake -G "Visual Studio 17 2022" -A x64 -S ../src -B .  -DMATHEMATICA_DIR="путь/к/директории/Wolfram Mathematica/версия"

cmake --build . --config Release
