cmake_minimum_required(VERSION 3.0.0)
project(system-call-replayer)
set(CMAKE_CXX_STANDARD 11)
find_package(LibXml2 REQUIRED)

find_library(STRACE2DS_LIBRARY
    NAMES strace2ds libstrace2ds
    PATHS ${CMAKE_INSTALL_PREFIX}/strace2ds/lib
    REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include
	${CMAKE_INSTALL_PREFIX}/dataseries/include
	${LIBXML2_INCLUDE_DIR}
	${CMAKE_INSTALL_PREFIX}/strace2ds/include)

set(COMMON_FLAGS "-Wall -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
set(CMAKE_CXX_FLAGS "-g -O0 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O4 ${COMMON_FLAGS}")

set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

add_subdirectory(src)

install(FILES ${OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
