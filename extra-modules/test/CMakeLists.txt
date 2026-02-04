add_library(stc-testutil INTERFACE)
target_sources(stc-testutil INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stc/test/catch2/vendor/ConsoleReporter.cpp
)
target_include_directories(stc-testutil INTERFACE .)
