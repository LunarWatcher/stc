add_library(stc-testutil INTERFACE)
target_sources(stc-testutil INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stc/test/catch2/vendor/ConsoleReporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stc/test/catch2/ExplicitStreamTestReporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stc/test/catch2/AugmentedXMLReporter.cpp
)
target_include_directories(stc-testutil INTERFACE .)
target_link_libraries(stc-testutil INTERFACE stc::stc)
