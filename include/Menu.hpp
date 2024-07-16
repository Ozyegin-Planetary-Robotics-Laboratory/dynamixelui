#ifndef MENU_HPP
#define MENU_HPP

#include <thread>
#include "Component.hpp"
#include "Button.hpp"
#include "Input.hpp"
#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk/dynamixel_stats.h"

class Menu : public Component {
  dynamixel::PortHandler *m_portHandler;
  dynamixel::PacketHandler *m_packetHandler;
  typedef std::shared_ptr<Component> ComponentPtr;
  std::vector<std::vector <ComponentPtr>> m_buttons;
  int m_cursor_index[2];
  int m_active_index[2];
  bool *m_shutdown_ptr;
  bool m_locked;
  std::thread m_command_thread;
  
  ComponentPtr _get_curs_button() {
    return m_buttons[m_cursor_index[0]][m_cursor_index[1]];
  }

  ComponentPtr _get_active_button() {
    if (m_active_index[0] == -1) return nullptr;
    return m_buttons[m_active_index[0]][m_active_index[1]];
  }

  inline void _delegate_command() {
    float vel = static_cast<InputBuffer <float> *> (m_buttons[0][1].get())->m_value;
    vel = vel > 114*6 ? 114*6 : vel;
    vel = vel < 0 ? 0 : vel;
    uint16_t vel_int = static_cast<uint16_t>(vel*1023/(116.62*6));
    uint8_t err1;    
    int dxl_comm_result = m_packetHandler->write2ByteTxRx(m_portHandler, DXL_ID, ADDR_MX_MOVING_SPEED, vel_int, &err1);
    float pos = static_cast<InputBuffer <float> *> (m_buttons[0][0].get())->m_value;
    pos = pos > 2523 ? 2523 : pos;
    pos = pos < -2523 ? -2523 : pos;
    uint16_t pos_int = static_cast<uint16_t>(pos/0.088);
    uint8_t err2;
    dxl_comm_result = m_packetHandler->write2ByteTxRx(m_portHandler, DXL_ID, ADDR_MX_GOAL_POSITION, pos_int, &err2);
  }

public:
  Menu(int x, int y, int w, int h, bool *shutdown_ptr, dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler) :
    Component(x, y, w, h),
    m_buttons{
      {
      std::make_shared <InputBuffer <float>>     ("Pos", x+(w*1)/7, y+1, w/7, 3),
      std::make_shared <InputBuffer <float>>     ("Vel", x+(w*3)/7, y+1, w/7, 3),
      },
      {
      std::make_shared <Button>                  (x+(w*1)/7, y+5, w/7, 3, "Send")
      }
     },
    m_cursor_index{0, 0},
    m_active_index{-1, -1},
    m_shutdown_ptr(shutdown_ptr),
    m_locked(false),
    m_portHandler(portHandler),
    m_packetHandler(packetHandler)
  {}

  void focus() override {
    m_focused = true;
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->focus();
      }
    }
  }

  void unfocus() override {
    m_focused = false;
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->unfocus();
      }
    }
  }

  void mount() override {
    box(m_win, 0, 0);
    mvwprintw(m_win, 2, w/7-16, "Setpoint:");
    wrefresh(m_win);
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->mount();
      }
    }
    ButtonUpdate hover_state(ButtonState::HOVER);
    m_buttons[m_cursor_index[0]][m_cursor_index[1]]->update(&hover_state);
  }

  void update(UpdatePacket *packet) override {
    static ButtonUpdate button_normal(ButtonState::NORMAL);
    static ButtonUpdate button_hover(ButtonState::HOVER);
    static ButtonUpdate button_active(ButtonState::ACTIVE);

    if (packet->type != UpdatePacket::INPUT) return;
    InputUpdate *update = static_cast<InputUpdate *>(packet);

    int key_in = update->key_in;
    #ifdef LOCK_ENABLED
    if (m_locked && key_in != '\n') {
      return;
    }
    #endif
    switch (key_in) {
      case KEY_RIGHT:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[1] = (m_cursor_index[1] + 1) % (m_buttons[m_cursor_index[0]].size());
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_LEFT:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[1]--;
        if (m_cursor_index[1] == -1) { // Modulos doesn't map -1.
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        } else {
          m_cursor_index[1] %= m_buttons[m_cursor_index[0]].size();
        }
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_UP:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[0]--;
        if (m_cursor_index[0] == -1) { // Modulos doesn't map -1.
          m_cursor_index[0] = m_buttons.size()-1;
        } else {
          m_cursor_index[0] %= m_buttons.size();
        }

        // Second index safeguard
        if (m_cursor_index[1] > m_buttons[m_cursor_index[0]].size()-1) {
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        }
        if (m_cursor_index[1] < 0) {
          m_cursor_index[1] = 0;
        }
        _get_curs_button()->update(&button_hover);
        break;
      case KEY_DOWN:
        _get_curs_button()->update(&button_normal);
        m_cursor_index[0]++;
        if (m_cursor_index[0] == -1) { // Modulos doesn't map -1.
          m_cursor_index[0] = m_buttons.size()-1;
        } else {
          m_cursor_index[0] %= m_buttons.size();
        }

        // Second index safeguard
        if (m_cursor_index[1] > m_buttons[m_cursor_index[0]].size()-1) {
          m_cursor_index[1] = m_buttons[m_cursor_index[0]].size()-1;
        }
        if (m_cursor_index[1] < 0) {
          m_cursor_index[1] = 0;
        }
        _get_curs_button()->update(&button_hover);
        break;
      case '\n':
        if (m_cursor_index[0] == 0) {             // Input Buffers
          _get_curs_button()->update(packet);
        } else {                                  // Buttons
          _get_curs_button()->update(&button_active);
          if (m_cursor_index[0] == 1) {           // Send
            if (m_locked) {                       // Deactivate send
              m_locked = false;
              m_command_thread.join();
              _get_curs_button()->update(&button_hover);
            } else {                              // Activate send
              m_locked = true;
              m_command_thread = std::thread([this] {
                while (m_locked) {
                  _delegate_command();
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
              });
              _get_curs_button()->update(&button_active);
            }
          }          
        }
        break;
      default:
        break;
    }
    wrefresh(m_win);
  }

  void unmount() override {
    m_locked = false;
    if (m_command_thread.joinable()) m_command_thread.join();
    werase(m_win);
    for (int row = 0; row < m_buttons.size(); row++) {
      for (int col = 0; col < m_buttons[row].size(); col++) {
        m_buttons[row][col]->unmount();
      }
    }
    wrefresh(m_win);
  }
};

#endif // MENU_HPP