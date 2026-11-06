link_libraries(stc::stc)

if (NOT WIN32)
    add_executable(file-watcher src/FileWatcher.cpp)
endif()

