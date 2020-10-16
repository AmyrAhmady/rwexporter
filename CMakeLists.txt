# CMakeList.txt : CMake project for rwexporter, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 11)
project ("rwexporter")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
	include_directories(
		deps/json/single_include/nlohmann
		deps/rwtools/include
		deps/lodepng
		deps/dirent/include
	)
else()
	include_directories(
		deps/json/single_include/nlohmann
		deps/rwtools/include
		deps/lodepng
	)
endif()

set(LODEPNG
	"deps/lodepng/lodepng.cpp"
	"deps/lodepng/lodepng.h"
)

file(GLOB LODEPNGFILES ${LODEPNG})

set(RWFILES
	"deps/rwtools/include/renderware.h"
	"deps/rwtools/src/renderware.cpp"
	"deps/rwtools/src/dffread.cpp"
	"deps/rwtools/src/txdread.cpp"
	"deps/rwtools/src/ps2native.cpp"
	"deps/rwtools/src/xboxnative.cpp"
	"deps/rwtools/src/oglnative.cpp"
)

set(JSONFILES
	"deps/json/single_include/nlohmann/json.hpp"
)

if(WIN32)
	set(DIRENTFILES
		"deps/dirent/include/dirent.hpp"
	)

	source_group("dirent" FILES ${DIRENTFILES})
endif()

source_group("renderware" FILES ${RWFILES})
source_group("json" FILES ${JSONFILES})

add_library(liblodepng STATIC ${LODEPNGFILES})
target_include_directories(liblodepng PUBLIC "deps/lodepng")

# Add source to this project's executable.
file(GLOB sources ${RWFILES} ${JSONFILES} "main.cpp" "src/*cpp" "src/*hpp")

add_executable (rwexporter ${sources})
target_link_libraries(rwexporter PUBLIC liblodepng)
