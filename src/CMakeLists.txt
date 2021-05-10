add_subdirectory(syscalls)
add_subdirectory(analysis)

add_executable(system-call-replayer
    ReplayerResourcesManager.cpp
    SystemCallTraceReplayLogger.cpp
    SystemCallTraceReplayModule.cpp
    SystemCallTraceReplayer.cpp
    VirtualAddressSpace.cpp    
)

# Replayer Subdirectories
target_link_libraries(system-call-replayer
    analysis
    syscalls
)

# External Libraries
target_link_libraries(system-call-replayer
	Lintel
	aio DataSeries
	boost_program_options
	profiler
	pthread
	tbb
	tcmalloc
)