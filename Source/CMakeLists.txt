# Need 3.12 as it has VS_SHADER_OBJECT_FILE_NAME
cmake_minimum_required (VERSION 3.12)
# Molly Game Engine Toolset
project (MoGET)

if( ${CMAKE_VS_PLATFORM_NAME} MATCHES WIN32 )
	set( MOGET_PLATFORM "Win32" CACHE STRING "The current platform, platform independent" FORCE )
elseif( ${CMAKE_VS_PLATFORM_NAME} MATCHES x64 )
	set( MOGET_PLATFORM "Win64" CACHE STRING "The current platform, platform independent" FORCE )
endif()

set( MOGET_INTERMEDIATE_DIR "${CMAKE_SOURCE_DIR}/../Intermediate" CACHE PATH "Directory where we put intermediate products" FORCE )
set( MOGET_INTERMEDIATE_BUILD_DIR "${MOGET_INTERMEDIATE_DIR}/Build/" CACHE PATH "Directory where our build products end up" FORCE )

# @todo: set this dynamically
add_definitions(-DMOGET_WINDOWS=1)
# Default defines
add_definitions(-DUNICODE=1)

# @todo: iterate over all directories in the folder and automatically add them as projects
# Add all projects to the project
add_subdirectory (Common)
add_subdirectory (Example1)
add_subdirectory (Example2)
add_subdirectory (Example3)

