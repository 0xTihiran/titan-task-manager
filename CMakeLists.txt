cmake_minimum_required(VERSION 3.16)
project(titan-task-manager VERSION 0.7.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network)
add_executable(titan-task-manager
    src/main.cpp
    src/ipcserver.h src/ipcserver.cpp
    src/thmclient.h src/thmclient.cpp
    src/thmstateparser.h src/thmstateparser.cpp
    src/systemmodel.h src/systemmodel.cpp
    src/procparser.h src/procparser.cpp
    src/procidentity.h src/procidentity.cpp
    src/protectionfilter.h src/protectionfilter.cpp
    src/actionpreflight.h src/actionpreflight.cpp
    src/actioncontroller.h src/actioncontroller.cpp
    src/procmodel.h src/procmodel.cpp
    src/workloadtreecore.h src/workloadtreecore.cpp
    src/workloadtree.h src/workloadtree.cpp
    src/slicemodel.h src/slicemodel.cpp
    src/decisionsmodel.h src/decisionsmodel.cpp
    src/hyprlandmonitor.h src/hyprlandmonitor.cpp
    resources.qrc
)
target_link_libraries(titan-task-manager PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network
)
target_include_directories(titan-task-manager PRIVATE src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(titan-task-manager PRIVATE -Wall -Wextra -Wpedantic)
endif()
# No global install, airootfs modification, or shared config edits without lead approval.

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Test)
    add_executable(test_thmstateparser tests/test_thmstateparser.cpp src/thmstateparser.cpp)
    target_include_directories(test_thmstateparser PRIVATE src)
    target_link_libraries(test_thmstateparser PRIVATE Qt6::Core Qt6::Test)
    target_compile_definitions(test_thmstateparser PRIVATE
        THM_FIXTURE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/demo-thm-state.json")
    add_test(NAME thmstateparser COMMAND test_thmstateparser)

    add_executable(test_thmclient tests/test_thmclient.cpp src/thmclient.cpp src/thmstateparser.cpp)
    target_include_directories(test_thmclient PRIVATE src)
    target_link_libraries(test_thmclient PRIVATE Qt6::Core Qt6::Test)
    target_compile_definitions(test_thmclient PRIVATE
        THM_FIXTURE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/demo-thm-state.json")
    add_test(NAME thmclient COMMAND test_thmclient)

    add_executable(test_procparser tests/test_procparser.cpp src/procparser.cpp)
    target_include_directories(test_procparser PRIVATE src)
    target_link_libraries(test_procparser PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME procparser COMMAND test_procparser)

    add_executable(test_workloadtree tests/test_workloadtree.cpp src/workloadtreecore.cpp)
    target_include_directories(test_workloadtree PRIVATE src)
    target_link_libraries(test_workloadtree PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME workloadtree COMMAND test_workloadtree)

    add_executable(test_protectionfilter
        tests/test_protectionfilter.cpp
        src/protectionfilter.cpp src/procidentity.cpp)
    target_include_directories(test_protectionfilter PRIVATE src)
    target_link_libraries(test_protectionfilter PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME protectionfilter COMMAND test_protectionfilter)

    add_executable(test_actionpreflight
        tests/test_actionpreflight.cpp src/actionpreflight.cpp src/protectionfilter.cpp)
    target_include_directories(test_actionpreflight PRIVATE src)
    target_link_libraries(test_actionpreflight PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME actionpreflight COMMAND test_actionpreflight)

    add_executable(test_decisions tests/test_decisions.cpp src/decisionsmodel.cpp)
    target_include_directories(test_decisions PRIVATE src)
    target_link_libraries(test_decisions PRIVATE Qt6::Core Qt6::Network Qt6::Test)
    add_test(NAME decisions COMMAND test_decisions)

    add_executable(test_hyprland tests/test_hyprland.cpp src/hyprlandmonitor.cpp)
    target_include_directories(test_hyprland PRIVATE src)
    target_link_libraries(test_hyprland PRIVATE Qt6::Core Qt6::Network Qt6::Test)
    add_test(NAME hyprland COMMAND test_hyprland)
endif()
