// #include "wstp.h"

// #ifdef _WIN32
// #define EXPORT __declspec(dllexport)
// #else
// #define EXPORT
// #endif

// extern "C" {
//     EXPORT int WSTPCreateInterpolator(WSLINK link);
//     EXPORT int WSTPInterpolatePoint(WSLINK link);
//     EXPORT int WSTPDeleteInterpolator(WSLINK link);
//     int WSMain(int argc, char* argv[]);
// }

// int main(int argc, char* argv[]) {
//     return WSMain(argc, argv);
// }

#include "wstp.h"

// Remove these external declarations as they conflict with the generated ones
// extern "C" {
//     EXPORT int WSTPCreateInterpolator(WSLINK link);
//     EXPORT int WSTPInterpolatePoint(WSLINK link);
//     EXPORT int WSTPDeleteInterpolator(WSLINK link);
//     int WSMain(int argc, char* argv[]);
// }

// Keep only the WSMain declaration
// extern "C" {
//     int WSMain(int argc, char* argv[]);
// }

// int main(int argc, char* argv[]) {
//     return WSMain(argc, argv);
// }

int main(int argc, char* argv[]) {
    return WSMain(argc, argv);
}