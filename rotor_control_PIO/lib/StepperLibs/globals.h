
#ifndef LIBRARIES_GLOBALS_H_
#define LIBRARIES_GLOBALS_H_

#include <Arduino.h>

// Rotator status
enum _rotator_status {
    idle = 1, moving = 2, homing_el=4
};

struct _control{
    double dirStatus;     /// Motor direction current status (position)
    double pointer;       /// Motor pointing position (moveTo position)
    double prev_pointer;  /// Previous pointer (to calculate minTurn)
};

struct _rotator{
    volatile enum _rotator_status rotor_status; /// Rotator status
    bool msg_flag;                              /// Check if message is received
    bool home_flag;                             /// Homing flag
    bool rotor_error;                           /// Rotor error flag
    bool home_error;                            /// Homing error flag
};

float OFFSET;// Azimuth Offset from North

_control control_az = { .dirStatus = 0, .pointer=0, .prev_pointer=OFFSET};
_control control_el = { .dirStatus = 0, .pointer=0, .prev_pointer=0};
_rotator rotor = { .rotor_status = idle, .msg_flag=false, .home_flag=false,.rotor_error=false,.home_error=false};

bool ONOFF=false;



#endif /* LIBRARIES_GLOBALS_H_ */
