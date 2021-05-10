add_subdirectory(syscalls)
add_subdirectory(analysis)

add_executable(system-call-replayer
    ReplayerResourcesManager.cpp
    SystemCallTraceReplayer.cpp
    SystemCallTraceReplayLogger.cpp
    SystemCallTraceReplayModule.cpp
    VirtualAddressSpace.cpp    
)

target_link_libraries(system-call-replayer
    syscalls
    analysis
	tbb
	pthread
	boost_program_options
	aio DataSeries
	Lintel
	profiler
	tcmalloc
)