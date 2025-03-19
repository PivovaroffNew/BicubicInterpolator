cmake_minimum_required(VERSION 3.12)
project(BicubicInterpolatorWSTP CXX)

# Определяем переменные для путей к Mathematica
set(MATHEMATICA_DIR "C:/Program Files/Wolfram Research/Mathematica/13.2" CACHE PATH "Путь к директории Mathematica")
set(WSTP_DIR "${MATHEMATICA_DIR}/SystemFiles/Links/WSTP/DeveloperKit" CACHE PATH "Путь к директории DeveloperKit WSTP")

# Определяем пути в зависимости от платформы
if(WIN32)
    set(WSTP_SYSTEM_DIR "${WSTP_DIR}/Windows-x86-64")
    set(WSTP_COMPILER_DIR "${WSTP_SYSTEM_DIR}/CompilerAdditions")
    set(WSTP_SYSTEM_ADDITIONS_DIR "${WSTP_SYSTEM_DIR}/SystemAdditions")  # Добавлено для DLL
    set(WSTP_LIB_NAME_I "wstp64i4")  # Исправлено: имя .lib файла без расширения
    set(WSTP_LIB_NAME_M "wstp64i4m")  # Исправлено: имя .lib файла без расширения
    set(WSTP_LIB_NAME_S "wstp64i4s")  # Исправлено: имя .lib файла без расширения
    set(WSTP_TEMPLATE "${WSTP_COMPILER_DIR}/wsprep.exe")
elseif(APPLE)
    set(WSTP_SYSTEM_DIR "${WSTP_DIR}/MacOSX-x86-64")
    set(WSTP_COMPILER_DIR "${WSTP_SYSTEM_DIR}/CompilerAdditions")
    set(WSTP_LINK_DIR "${WSTP_SYSTEM_DIR}/SystemAdditions")
    set(WSTP_LIB_NAME "WSTP64i4")
    set(WSTP_TEMPLATE "${WSTP_COMPILER_DIR}/wsprep")
else()
    set(WSTP_SYSTEM_DIR "${WSTP_DIR}/Linux-x86-64")
    set(WSTP_COMPILER_DIR "${WSTP_SYSTEM_DIR}/CompilerAdditions")
    set(WSTP_LINK_DIR "${WSTP_SYSTEM_DIR}/SystemAdditions")
    set(WSTP_LIB_NAME "wstp64i4")
    set(WSTP_TEMPLATE "${WSTP_COMPILER_DIR}/wsprep")
endif()

# Проверка существования директории CompilerAdditions
if(NOT EXISTS "${WSTP_COMPILER_DIR}")
    message(FATAL_ERROR "WSTP директория не найдена: ${WSTP_COMPILER_DIR}")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WSTP_COMPILER_DIR}
)

# Пути для линковки (CompilerAdditions содержит .lib/.a файлы)
link_directories("${WSTP_COMPILER_DIR}")

# Генерация файла из шаблона WSTP (оставить без изменений)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/InterpolatorWSTP.cpp
    COMMAND ${WSTP_TEMPLATE} ${CMAKE_CURRENT_SOURCE_DIR}/InterpolatorWSTP.tm -o ${CMAKE_CURRENT_BINARY_DIR}/InterpolatorWSTP.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/InterpolatorWSTP.tm
    COMMENT "Обработка WSTP шаблона"
)

set(SOURCES
    BicubicInterpolator.cpp
    WSTPFunctions.cpp
    InterpolatorWSTPMain.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/InterpolatorWSTP.cpp
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/InterpolatorWSTP.c 
    PROPERTIES GENERATED TRUE
)

add_executable(BicubicInterpolator ${SOURCES})
if(WIN32)
    target_compile_definitions(BicubicInterpolator PRIVATE -D_WIN32)
endif()

# Поиск библиотеки в CompilerAdditions
find_library(WSTP_LIB_I
    NAMES ${WSTP_LIB_NAME_I}
    PATHS "${WSTP_COMPILER_DIR}"
    NO_DEFAULT_PATH
)

if(NOT WSTP_LIB_I)
    message(FATAL_ERROR "WSTP_I library not found in: ${WSTP_COMPILER_DIR}")
endif()

find_library(WSTP_LIB_M
    NAMES ${WSTP_LIB_NAME_M}
    PATHS "${WSTP_COMPILER_DIR}"
    NO_DEFAULT_PATH
)

if(NOT WSTP_LIB_M)
    message(FATAL_ERROR "WSTP_M library not found in: ${WSTP_COMPILER_DIR}")
endif()

find_library(WSTP_LIB_S
    NAMES ${WSTP_LIB_NAME_S}
    PATHS "${WSTP_COMPILER_DIR}"
    NO_DEFAULT_PATH
)

if(NOT WSTP_LIB_S)
    message(FATAL_ERROR "WSTP_S library not found in: ${WSTP_COMPILER_DIR}")
endif()

# Связывание с библиотекой
target_link_libraries(BicubicInterpolator ${WSTP_LIB_I})
target_link_libraries(BicubicInterpolator ${WSTP_LIB_M})
target_link_libraries(BicubicInterpolator ${WSTP_LIB_S})

# Копирование DLL на Windows
if(WIN32)
    add_custom_command(TARGET BicubicInterpolator PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WSTP_SYSTEM_ADDITIONS_DIR}/wstp64i4.dll"
            $<TARGET_FILE_DIR:BicubicInterpolator>
        COMMENT "Копирование WSTP DLL в выходную директорию"
    )
endif()


message(STATUS "WSTP Directory: ${WSTP_SYSTEM_DIR}")
message(STATUS "WSTP Library: ${WSTP_LIB_NAME_I}")