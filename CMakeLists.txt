cmake_minimum_required(VERSION 3.21)
project(SleepManager)

# Add the main program
set(SOURCES src/main.cpp src/discovery.cpp src/interface.cpp src/participant.cpp src/manager.cpp src/monitoring.cpp src/management.cpp)
add_executable(SleepManagerMain ${SOURCES})

# Include the tests subdirectory
add_subdirectory(tests)
