add_definitions(-DDATA_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Data/\")

add_executable(ConsoleTest ConsoleTest/main.cpp ConsoleTest/ConsoleTestManager.cpp)
target_link_libraries(ConsoleTest AmphixCore_test)

add_executable(CameraTest CameraTest/main.cpp CameraTest/CameraTestManager.cpp)
target_link_libraries(CameraTest AmphixCore_test)

add_executable(FallingTest FallingTest/main.cpp FallingTest/FallingTestApp.cpp FallingTest/FallingTestManager.cpp)
target_link_libraries(FallingTest AmphixCore_test)

add_executable(FloatingTest FloatingTest/main.cpp FloatingTest/FloatingTestManager.cpp)
target_link_libraries(FloatingTest AmphixCore_test)

add_executable(FlyingTest FlyingTest/main.cpp FlyingTest/FlyingTestManager.cpp)
target_link_libraries(FlyingTest AmphixCore_test)

add_executable(JointsTest JointsTest/main.cpp JointsTest/JointsTestManager.cpp JointsTest/JointsTestApp.cpp)
target_link_libraries(JointsTest AmphixCore_test)

add_executable(SlidingTest SlidingTest/main.cpp SlidingTest/SlidingTestApp.cpp SlidingTest/SlidingTestManager.cpp)
target_link_libraries(SlidingTest AmphixCore_test)

add_executable(UnderwaterTest UnderwaterTest/main.cpp UnderwaterTest/UnderwaterTestApp.cpp UnderwaterTest/UnderwaterTestManager.cpp)
target_link_libraries(UnderwaterTest AmphixCore_test)

add_executable(FluidDynamicsTest FluidDynamicsTest/main.cpp FluidDynamicsTest/FluidDynamicsTestManager.cpp)
target_link_libraries(FluidDynamicsTest AmphixCore_test)

add_executable(LearningTest LearningTest/main.cpp LearningTest/LearningTestManager.cpp)
target_link_libraries(LearningTest AmphixCore_test)

add_executable(CableTest CableTest/main.cpp CableTest/CableTestApp.cpp CableTest/CableTestManager.cpp)
target_link_libraries(CableTest AmphixCore_test)

add_executable(ParserTest ParserTest/main.cpp ParserTest/ParserTestManager.cpp)
target_link_libraries(ParserTest AmphixCore_test)

add_library(LinkSensorPlugin SHARED ParserTest/LinkSensorPlugin.cpp)
target_link_libraries(LinkSensorPlugin PRIVATE AmphixCore_test)

add_library(LinkActuatorPlugin SHARED ParserTest/LinkActuatorPlugin.cpp)
target_link_libraries(LinkActuatorPlugin PRIVATE AmphixCore_test)