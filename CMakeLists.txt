cmake_minimum_required (VERSION 3.10)
project (ecv)

set(ECV_BRIEF "Exact cover problem solver")
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME} )
set (CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "This application requires an out of source build.
        Please create a separate build directory")
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_DIR}/include>)

target_compile_options    (${PROJECT_NAME} PRIVATE -O3 -Werror -Wall -Wextra -pedantic)
target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "include/ecv.hpp")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".a")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")

install (TARGETS ${PROJECT_NAME} 
         ARCHIVE DESTINATION "${INSTALL_DIR}/lib"
         COMPONENT library
         PUBLIC_HEADER DESTINATION "${INSTALL_DIR}/include")
