# Библиотека с алгоритмами
add_library(interior_math STATIC
    BicubicInterpolator.cpp
)
target_include_directories(interior_math PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# WSTP исполняемый файл
add_executable(BicubicWSTP
    BicubicWSTP.cpp
)
target_include_directories(BicubicWSTP PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WolframLanguage_WSTP_INCLUDE_DIRS}
)
target_link_libraries(BicubicWSTP
    PRIVATE interior_math
    PRIVATE WSTP::WSTP
    PRIVATE WSTP64i4
)

set(WSTP_LIB_DIR "W:/wolfram/13.0/SystemFiles/Links/WSTP/DeveloperKit/Windows-x86-64/CompilerAdditions")
target_link_directories(BicubicWSTP PRIVATE ${WSTP_LIB_DIR})

# Установка выходного файла
install(TARGETS BicubicWSTP
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)