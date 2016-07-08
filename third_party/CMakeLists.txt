cmake_minimum_required (VERSION 2.8)

cmake_policy(SET CMP0015 NEW)

# Ignore all warnings
if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++1y -w")
	set(CMAKE_C_FLAGS "-w")
endif()
   
if(MSVC)
	string(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W")
	string(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}") 
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W") 
endif()

if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warn-absolute-paths")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths")
endif()

add_subdirectory(glm)
add_subdirectory(imgui)