cmake_minimum_required(VERSION 2.8)
project(engine)
cmake_policy(SET CMP0015 NEW)

file(GLOB_RECURSE
     SRC_LIST
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
     *.cpp *.h*)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

if(EMSCRIPTEN)
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    #
    #if(${BUILD_TYPE} STREQUAL "debug")
    #    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0 -s DEMANGLE_SUPPORT=1")
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -s DEMANGLE_SUPPORT=1")
    #else()
    #    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s TOTAL_MEMORY=52428800")
    #endif()
    set(GLEW_LIBRARIES "")
    set(OPENGL_LIBRARIES "")
    set(SDL2_LIBRARY "")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_libraries(${PROJECT_NAME} imgui ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
else()
    find_package(SDL2 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)
    add_definitions(${OpenGL_DEFINITIONS})
    include_directories(${SDL2_INCLUDE_DIR} ${OpenGL_INCLUDE_DIRS})
    link_directories(${OpenGL_LIBRARY_DIRS})
    
    target_link_libraries(${PROJECT_NAME} imgui ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
endif()

# Create source groups for Visual Studio filters
# Get all directories first:
set(dir_list "")
foreach(file_path ${SRC_LIST})
        get_filename_component(dir_path ${file_path} PATH)
        set(dir_list ${dir_list} ${dir_path})
endforeach()
list(REMOVE_DUPLICATES dir_list)

file(GLOB sources *.cpp *.h*)
source_group(\\ FILES ${sources})

foreach(dir ${dir_list})
        set(abs_dir "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
        file(GLOB sources ${abs_dir}/*.cpp ${abs_dir}/*.h*)
        string(REPLACE "/" "\\\\" Group ${dir})
        source_group(${Group} FILES ${sources})
endforeach()
