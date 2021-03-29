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

set(CMAKE_CXX_FLAGS "-g -O4 -Wall -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")

link_directories(${CMAKE_INSTALL_PREFIX}/lib)

add_executable(system-call-replayer
	src/ReplayerResourcesManager.cpp
	src/SystemCallTraceReplayer.cpp
	src/SystemCallTraceReplayLogger.cpp
	src/SystemCallTraceReplayModule.cpp
	src/VirtualAddressSpace.cpp
	src/OpenSystemCallTraceReplayModule.cpp
	src/CloseSystemCallTraceReplayModule.cpp
	src/ReadSystemCallTraceReplayModule.cpp
	src/WriteSystemCallTraceReplayModule.cpp
	src/LSeekSystemCallTraceReplayModule.cpp
	src/TruncateSystemCallTraceReplayModule.cpp
	src/AccessSystemCallTraceReplayModule.cpp
	src/UnlinkSystemCallTraceReplayModule.cpp
	src/RmdirSystemCallTraceReplayModule.cpp
	src/LinkSystemCallTraceReplayModule.cpp
	src/CreatSystemCallTraceReplayModule.cpp
	src/ChdirSystemCallTraceReplayModule.cpp
	src/FChdirSystemCallTraceReplayModule.cpp
	src/ChrootSystemCallTraceReplayModule.cpp
	src/SymlinkSystemCallTraceReplayModule.cpp
	src/MkdirSystemCallTraceReplayModule.cpp
	src/BasicStatSystemCallTraceReplayModule.cpp
	src/BasicStatfsSystemCallTraceReplayModule.cpp
	src/ReadlinkSystemCallTraceReplayModule.cpp
	src/UtimeSystemCallTraceReplayModule.cpp
	src/ChmodSystemCallTraceReplayModule.cpp
	src/ChownSystemCallTraceReplayModule.cpp
	src/ReadvSystemCallTraceReplayModule.cpp
	src/WritevSystemCallTraceReplayModule.cpp
	src/RenameSystemCallTraceReplayModule.cpp
	src/FsyncSystemCallTraceReplayModule.cpp
	src/FdatasyncSystemCallTraceReplayModule.cpp
	src/FallocateSystemCallTraceReplayModule.cpp
	src/ReadaheadSystemCallTraceReplayModule.cpp
	src/MknodSystemCallTraceReplayModule.cpp
	src/PipeSystemCallTraceReplayModule.cpp
	src/DupSystemCallTraceReplayModule.cpp
	src/Dup2SystemCallTraceReplayModule.cpp
	src/Dup3SystemCallTraceReplayModule.cpp
	src/ExitSystemCallTraceReplayModule.cpp
	src/ExecveSystemCallTraceReplayModule.cpp
	src/MmapSystemCallTraceReplayModule.cpp
	src/MunmapSystemCallTraceReplayModule.cpp
	src/FcntlSystemCallTraceReplayModule.cpp
	src/ExitSystemCallTraceReplayModule.cpp
	src/GetdentsSystemCallTraceReplayModule.cpp
	src/IoctlSystemCallTraceReplayModule.cpp
	src/FChmodSystemCallTraceReplayModule.cpp
	src/CloneSystemCallTraceReplayModule.cpp
	src/VForkSystemCallTraceReplayModule.cpp
	src/UmaskSystemCallTraceReplayModule.cpp
	src/FTruncateSystemCallTraceReplayModule.cpp
	src/SetxattrSystemCallTraceReplayModule.cpp
	src/SocketSystemCallTraceReplayModule.cpp
	src/SocketPairSystemCallTraceReplayModule.cpp
	src/AcceptSystemCallTraceReplayModule.cpp
	src/Accept4SystemCallTraceReplayModule.cpp
	src/EPollCreateSystemCallTraceReplayModule.cpp
	src/ReadSystemCallAnalysisModule)

target_link_libraries(system-call-replayer
	tbb
	pthread
	boost_program_options
	aio DataSeries
	Lintel
	profiler
	tcmalloc)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
