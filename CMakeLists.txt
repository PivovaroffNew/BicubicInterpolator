cmake_minimum_required(VERSION 3.10)
project(BicubicInterpolatorWSTP)

set(WOLFRAM_VERSION "13.2" CACHE STRING "Wolfram Mathematica version (e.g., 13.2)")

# Проверка, что версия поддерживается
if (NOT WOLFRAM_VERSION MATCHES "^13\\.[0-2]$")
    message(FATAL_ERROR "Unsupported Wolfram Mathematica version: ${WOLFRAM_VERSION}. Supported versions are 13.0, 13.1, and 13.2.")
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Настройка стандартов C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Поиск компонентов Wolfram
find_package(WolframLanguage ${WOLFRAM_VERSION} REQUIRED COMPONENTS WSTP MODULE)

# Настройка для MinGW
if(MINGW)
    add_link_options(-static-libgcc -static-libstdc++)
endif()

add_subdirectory(src)

if(EXISTS ${CMAKE_SOURCE_DIR}/tests AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()