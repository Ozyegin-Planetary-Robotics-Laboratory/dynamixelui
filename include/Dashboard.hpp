#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include <map>
#include "Component.hpp"

struct DynamixelPacket : public UpdatePacket {
  uint16_t load;
  uint16_t position;
  uint16_t velocity;
  uint8_t voltage;
  float gear_ratio;
  uint8_t temperature;

  DynamixelPacket(uint16_t load_in, uint16_t position_in, uint16_t velocity_in, uint8_t voltage_in, float gear_ratio_in, uint8_t temperature_in) :
    UpdatePacket(UpdatePacket::DYNAMIXEL),
    load(load_in),
    position(position_in),
    velocity(velocity_in),
    voltage(voltage_in),
    gear_ratio(gear_ratio_in),
    temperature(temperature_in)
  {}

  DynamixelPacket() :
    UpdatePacket(UpdatePacket::DYNAMIXEL),
    load(0),
    position(0),
    velocity(0),
    voltage(0),
    gear_ratio(1),
    temperature(0)
  {}
};

class Dashboard : public Component {
  bool m_shutdown;
  bool m_mounted;
  int m_motor_id;

  std::string _get_empty(size_t len) {
    std::string empty("");
    for (size_t i = 0; i < len; i++) {
      empty += " ";
    }
    return empty;
  }

  void _clear() {
    mvwprintw(m_win, 2, 2+11, "%s", _get_empty(w-3-11).c_str());
    mvwprintw(m_win, 3, 2+11, "%s", _get_empty(w-3-11).c_str());
    mvwprintw(m_win, 4, 2+10, "%s", _get_empty(w-3-10).c_str());
    mvwprintw(m_win, 5, 2+13, "%s", _get_empty(w-3-13).c_str());
    mvwprintw(m_win, 6, 2+14, "%s", _get_empty(w-3-14).c_str());
    mvwprintw(m_win, 7, 2+14, "%s", _get_empty(w-3-14).c_str());
    mvwprintw(m_win, 8, 2+14, "%s", _get_empty(w-3-14).c_str());
  }

  void _draw() {
    mvwprintw(m_win, 2, 2+11, "%.2f", position/gear_ratio);
    mvwprintw(m_win, 3, 2+11, "%.2f", velocity/gear_ratio);
    mvwprintw(m_win, 4, 2+10, "%.2f", load);
    mvwprintw(m_win, 5, 2+13, "%.2f", voltage);
    mvwprintw(m_win, 6, 2+14, "%.2f", gear_ratio);
    mvwprintw(m_win, 7, 2+15, "%.2i", temperature);
  }

public:
  float load;
  float position;
  float velocity;
  float voltage;
  float gear_ratio;
  uint8_t temperature;

  Dashboard (int x, int y, int w, int h, int id) :
    Component(x, y, w, h),
    m_shutdown(false),
    m_motor_id(id)
  {}
 
  void focus() override {}

  void unfocus() override {}

  void mount() override {
    box(m_win, 0, 0);
    std::string title = std::string("DYNAMIXEL ID: ") + std::to_string(m_motor_id);
    mvwprintw(m_win, 1, (w-title.size())/2, "%s", title.c_str());
    mvwprintw(m_win, 2, 2, "Position: ");
    mvwprintw(m_win, 3, 2, "Velocity: ");
    mvwprintw(m_win, 4, 2, "Load: ");
    mvwprintw(m_win, 5, 2, "Voltage: ");
    mvwprintw(m_win, 6, 2, "Gear Ratio: ");
    mvwprintw(m_win, 7, 2, "Temperature: ");
    wrefresh(m_win);
  }

  void update(UpdatePacket *packet) override {
    if (packet->type != UpdatePacket::DYNAMIXEL) return;
    DynamixelPacket *update = static_cast<DynamixelPacket *>(packet);
    _clear();
    bool ccw_load = (update->load & 0x400) == 0;
    float load_f = (update->load & 0x3FF) * (ccw_load ? 1 : -1) * 100.0f / 1023.0f;
    load = load_f;
    union {uint16_t unsigned_val; int16_t signed_val;} converter;
    converter.unsigned_val = update->position;
    position = converter.signed_val * 0.088f;
    bool ccw_vel = (update->velocity & 0x400) == 0;
    velocity = (update->velocity & 0x3FF) * (ccw_vel ? 1 : -1) * 0.11f * 6.0f;
    voltage = update->voltage / 10.0f;
    gear_ratio = update->gear_ratio;
    temperature = update->temperature;
    _draw();
    wrefresh(m_win);
  }

  void unmount() override {
    werase(m_win);
    wrefresh(m_win);
  }

};

#endif // DASHBOARD_HPP