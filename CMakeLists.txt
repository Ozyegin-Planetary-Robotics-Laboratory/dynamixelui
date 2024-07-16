cmake_minimum_required(VERSION 3.0.0)
project(dynamixelui)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include include/dynamixel_sdk)
include_directories(include include)

# Comment the below line to disable the lock feature when sending commandss
# add_definitions(-DLOCK_ENABLED)

add_executable(dynamixelui
  src/main.cpp
  src/dynamixel_sdk/group_bulk_read.cpp
  src/dynamixel_sdk/group_bulk_write.cpp
  src/dynamixel_sdk/group_sync_read.cpp
  src/dynamixel_sdk/group_sync_write.cpp
  src/dynamixel_sdk/packet_handler.cpp
  src/dynamixel_sdk/port_handler_arduino.cpp
  src/dynamixel_sdk/port_handler_linux.cpp
  src/dynamixel_sdk/port_handler_mac.cpp
  src/dynamixel_sdk/port_handler_windows.cpp
  src/dynamixel_sdk/port_handler.cpp
  src/dynamixel_sdk/protocol1_packet_handler.cpp
  src/dynamixel_sdk/protocol2_packet_handler.cpp
)

target_link_libraries(dynamixelui PUBLIC ncurses pthread)

install(TARGETS dynamixelui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)