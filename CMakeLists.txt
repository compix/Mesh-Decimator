cmake_minimum_required(VERSION 2.8)
project(MeshDecimator)

cmake_policy(SET CMP0015 NEW)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build: ${CMAKE_BUILD_TYPE}")

if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wextra -Wall -pedantic -Werror -Wno-unused-parameter -Wno-nested-anon-types")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules" ${CMAKE_MODULE_PATH})

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

if(EMSCRIPTEN)
    if(${BUILD_TYPE} STREQUAL "debug")
        set(SHELL_PATH "${CMAKE_SOURCE_DIR}/shells/debug.html")
    else()
        set(SHELL_PATH "${CMAKE_SOURCE_DIR}/shells/release.html")
    endif()
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --shell-file ${SHELL_PATH}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

file(GLOB_RECURSE
     SRC_LIST
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     source/main.cpp source/app/*.cpp source/app/*.h*)

file(GLOB_RECURSE
     ASSET_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     assets/*.frag* assets/*.vert* assets/*.glsl assets/*.cl assets/*.comp* assets/*.geom*)

set(ROOT_DIR ${CMAKE_SOURCE_DIR})

# Set output directories and copy assets to binary directory
set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
if(MSVC)
	message(STATUS "***** Using MSVC compiler *****")
			
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        file(GLOB DLLS DLLs/x64/*)
    else()
        file(GLOB DLLS DLLs/x86/*)
    endif()
	
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /wd4310 /wd4127 /wd4100 /wd4201")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX /wd4310 /wd4127 /wd4100 /wd4201")
	
    # Handle Visual Studio Debug/Release... config folders
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/lib)
        set(PDB_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/Temp)
    endforeach()

    set(PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Temp)

	# Create source groups for Visual Studio filters
	# Get all directories first:
	set(dir_list "")
	foreach(file_path ${SRC_LIST} ${ASSET_FILES})
		get_filename_component(dir_path ${file_path} PATH)
		set(dir_list ${dir_list} ${dir_path})
	endforeach()
	list(REMOVE_DUPLICATES dir_list)

	file(GLOB sources *.cpp *.h*)
	source_group(\\ FILES ${sources})

	foreach(dir ${dir_list})
		set(abs_dir "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
		file(GLOB sources ${abs_dir}/*.cpp ${abs_dir}/*.h* ${abs_dir}/*.vert* ${abs_dir}/*.frag* ${abs_dir}/*.glsl ${abs_dir}/*.cl ${abs_dir}/*.comp* ${abs_dir}/*.geom*)
		string(REPLACE "/" "\\\\" Group ${dir})
		source_group(${Group} FILES ${sources})
	endforeach()

    # Copy DLLs and assets    
    file(COPY ${DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${ASSET_DIR} DESTINATION ${CMAKE_BINARY_DIR})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    # Copy assets to the binary directory
    file(COPY ${ASSET_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file assets")
endif()
    
add_subdirectory(third_party)
INCLUDE_DIRECTORIES(source)
INCLUDE_DIRECTORIES(third_party/glm)
INCLUDE_DIRECTORIES(third_party/imgui)

if(EMSCRIPTEN)
    message(STATUS "Using emscripten.")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    
    if(${BUILD_TYPE} STREQUAL "debug")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0 -s DEMANGLE_SUPPORT=1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -s DEMANGLE_SUPPORT=1")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s TOTAL_MEMORY=52428800")
    endif()
    
    set(GLEW_LIBRARIES "")
    set(OPENGL_LIBRARIES "")
    set(SDL2_LIBRARY "")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    set(SDL2 "third_party/SDL2-2.0.4")
    set(GLEW "third_party/glew-1.13.0")
    find_package(SDL2 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    add_definitions(${OpenGL_DEFINITIONS})
    include_directories(${SDL2_INCLUDE_DIR} ${OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR})
    link_directories(${OpenGL_LIBRARY_DIRS})
endif()

add_subdirectory(source/engine)

add_executable(${PROJECT_NAME} ${ASSET_FILES} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} imgui engine ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
