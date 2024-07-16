#include <signal.h>
#include <ncurses.h>
#include <sys/stat.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>

#include "../include/Component.hpp"
#include "../include/Dashboard.hpp"
#include "../include/Button.hpp"
#include "../include/Input.hpp"
#include "../include/Menu.hpp"
#include "../include/tmotor.hpp"
#include "../include/dynamixel_sdk/dynamixel_stats.h"
#include "../include/dynamixel_sdk/dynamixel_sdk.h"

bool initialize_dynamixel(dynamixel::PortHandler *handler, dynamixel::PacketHandler *packetHandler);

int main(int argc, char **argv) {
  std::string usb_device;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <usb_device>" << std::endl;
    return 1;
  }
  usb_device = argv[1];
  struct stat buffer;
  if (stat (usb_device.c_str(), &buffer) != 0)
  {
    std::cerr << "Device " << usb_device << " does not exist." << std::endl;
    return 1;
  }
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(usb_device.c_str());
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
  if (!initialize_dynamixel(portHandler, packetHandler))
  {
    return 1;
  }

  initscr();
  atexit((void (*)()) endwin);

  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  noecho();
  refresh();

  init_colors();
  paint_scr();

  { // Main loop

    bool shutdown = false;
    Menu menu(1+COLS/5, 0, 4*COLS/5, (COLS)/(5*2), &shutdown, portHandler, packetHandler);
    Dashboard dashboard(0, 0, COLS/5, (COLS)/(5*2), DXL_ID);
    
    menu.mount();
    dashboard.mount();
    menu.focus();
    dashboard.focus();

    std::thread dashboard_updater([&shutdown, &dashboard, &portHandler, &packetHandler] {
      auto uint16_to_int16 = [](uint16_t val) {
        return (val & 0x8000) ? -((~val & 0xFFFF) + 1) : val;
      };   
      while (!shutdown) {
        DynamixelPacket motor_packet;
        motor_packet.gear_ratio = 1.0f;
        uint8_t err = 0;
        int result = packetHandler->read2ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_POSITION, &motor_packet.position, &err);
        if (result != COMM_SUCCESS) {
          continue;
        }
        result = packetHandler->read2ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_SPEED, &motor_packet.velocity, &err);
        if (result != COMM_SUCCESS) {
          continue;
        }
        result = packetHandler->read2ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_LOAD, &motor_packet.load, &err);
        if (result != COMM_SUCCESS) {
          continue;
        }
        result = packetHandler->read1ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_VOLTAGE, &motor_packet.voltage, &err);
        if (result != COMM_SUCCESS) {
          continue;
        }
        result = packetHandler->read1ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_TEMPERATURE, &motor_packet.temperature, &err);
        if (result != COMM_SUCCESS) {
          continue;
        }
        dashboard.update(&motor_packet);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
    
    InputUpdate packet('0');
    while (((packet.key_in = getch()) != 'q') && (!shutdown)) {
      menu.update(&packet);
    }

    shutdown = true;
    dashboard.unmount();
    menu.unmount();
    dashboard_updater.join();
    portHandler->clearPort();
    portHandler->closePort();
  }

  return 0;
}

bool initialize_dynamixel(dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler)
{
  if (!portHandler->openPort())
  {
    std::cerr << "Failed to open the port!" << std::endl;
    return false;
  }
  if (!portHandler->setBaudRate(BAUDRATE))
  {
    std::cerr << "Failed to set the baud rate!" << std::endl;
    return false;
  }
  uint8_t err;
  int result = packetHandler->ping(portHandler, DXL_ID, &err);
  if (result != COMM_SUCCESS)
  {
    std::cerr << "Failed to ping the motor!" << std::endl;
    return false;
  }
  result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_CW_ANGLE_LIMIT, 4095, &err);
  if (result != COMM_SUCCESS)
  {
    std::cerr << "Failed to set the CW angle limit!" << std::endl;
    return false;
  }

  result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_CCW_ANGLE_LIMIT, 4095, &err);
  if (result != COMM_SUCCESS)
  {
    std::cerr << "Failed to set the CCW angle limit!" << std::endl;
    return false;
  }
  return true;
}