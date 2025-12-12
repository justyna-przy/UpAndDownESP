# ESP-IDF component registration for UpAndDownESP

# Get application sources (C and C++)
FILE(GLOB_RECURSE app_sources_c ${CMAKE_SOURCE_DIR}/src/*.c)
FILE(GLOB_RECURSE app_sources_cpp ${CMAKE_SOURCE_DIR}/src/*.cpp)
FILE(GLOB_RECURSE tinyframe_sources ${CMAKE_SOURCE_DIR}/lib/TinyFrame/src/*.c)
set(app_sources ${app_sources_c} ${app_sources_cpp} ${tinyframe_sources})

# Register component
idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/lib/TinyFrame/include"
)
