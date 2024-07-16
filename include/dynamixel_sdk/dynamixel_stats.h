#ifndef DYNAMIXEL_STATS_H
#define DYNAMIXEL_STATS_H

#define STDIN_FILENO 0
#define PROTOCOL_VERSION                1.0
#define DXL_ID                          1
#define BAUDRATE                        1000000

#define ADDR_MX_CW_ANGLE_LIMIT          6
#define ADDR_MX_CCW_ANGLE_LIMIT         8
#define ADDR_MX_TORQUE_ENABLE           24
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_MOVING_SPEED            32
#define ADDR_MX_PRESENT_POSITION        36
#define ADDR_MX_PRESENT_SPEED           38
#define ADDR_MX_PRESENT_LOAD            40
#define ADDR_MX_PRESENT_VOLTAGE         42
#define ADDR_MX_PRESENT_TEMPERATURE     43
#define ADDR_MX_MOVING                  46

#define LEN_MX_GOAL_POSITION            2
#define LEN_MX_PRESENT_POSITION         2
#define LEN_MX_MOVING                   1

#define TORQUE_ENABLE                   1
#define TORQUE_DISABLE                  0 
#define DXL_MOVING_STATUS_THRESHOLD     10

#endif // DYNAMIXEL_STATS_H