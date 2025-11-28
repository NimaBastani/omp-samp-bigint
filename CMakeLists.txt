cmake_minimum_required(VERSION 3.19)
project(omp-bigint LANGUAGES C CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

# Option to build SA:MP plugin instead of open.mp component
option(BUILD_SAMP_PLUGIN "Build as SA:MP plugin instead of open.mp component" OFF)

if(BUILD_SAMP_PLUGIN)
	# SA:MP Plugin Build
	project(samp-bigint LANGUAGES C CXX VERSION 0.0.1)

	include_directories(deps/samp-plugin-sdk)
	include_directories(
		deps
		deps/pawn/source
		deps/pawn/source/linux
	)
	
	add_definitions(
		-DHAVE_STDINT_H=1
		-DPAWN_CELL_SIZE=32
		-DBUILD_SAMP_PLUGIN=1
	)
	
	add_library(${PROJECT_NAME} SHARED
	deps/samp-plugin-sdk/amxplugin.cpp
	deps/samp-plugin-sdk/amxplugin2.cpp
	deps/samp-plugin-sdk/amx/getch.c
	main.cpp bigint_common.hpp samp_plugin.h 
	plugin.def)
	
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	
	# On Windows, SA:MP plugins are .dll files
	# On Linux, they are .so files (default)
	if(WIN32)
		set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll")
	endif()
else()
	# open.mp Component Build (default)
add_subdirectory(deps/omp-sdk)

include_directories(
	deps
	deps/pawn/source
	deps/pawn/source/linux
	deps/pawn-natives
)

add_definitions(
	-DHAVE_STDINT_H=1
	-DPAWN_CELL_SIZE=32
)

	add_library(${PROJECT_NAME} SHARED main.cpp bigint_common.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE OMP-SDK)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif()
