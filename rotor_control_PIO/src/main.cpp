

#define RATIO              1    ///< Gear ratio of rotator gear box    108original                       default 54
#define MICROSTEP          2     ///< Set Microstep poner a 8
#define MAX_SPEED          3200  ///< In steps/s, consider the microstep
#define MAX_ACCELERATION   1600  ///< In steps/s^2, consider the microstep
#define SPR                400 ///< Step Per Revolution, consider the microstep 1600L 200*^Microstep
#define MIN_M1_ANGLE       0     ///< Minimum angle of azimuth
#define MAX_M1_ANGLE       360   ///< Maximum angle of azimuth
#define MIN_M2_ANGLE       0     ///< Minimum angle of elevation
#define MAX_M2_ANGLE       180   ///< Maximum angle of elevation
#define DEFAULT_HOME_STATE LOW  ///< Change to LOW according to Home sensor

#include <AccelStepper.h>
#include <globals.h>
#include <serialcomm.h>
#include <rotator_pins.h>
#include <endstop.h>

serialcomm serialport;  
AccelStepper stepper_az(1, M1IN1, M1IN2); //X pin in CNC shield
AccelStepper stepper_el(1, M2IN1, M2IN2); //Y pin
endstop switch_el(SW2, DEFAULT_HOME_STATE);
int32_t deg2step(float deg);
float step2deg(int32_t step);
float minTurn(float prev, float point); //Calculates minimum turn
float recalculateToRange(float current); //Recalculates Az position in between 0-360 deg (in case it exceeds)
void homing(int32_t seek_el);

void setup() {
    // Homing switch
    switch_el.init();

    // Serial Communication initialization
    serialport.serialcomm_init();

    // Stepper Motor setup
    stepper_az.setEnablePin(MOTOR_EN);
    stepper_az.setPinsInverted(false, false, true);
    stepper_az.setMaxSpeed(MAX_SPEED);
    stepper_az.setAcceleration(MAX_ACCELERATION);
    stepper_el.setPinsInverted(true, false, true); 
    stepper_el.setMaxSpeed(MAX_SPEED);
    stepper_el.setAcceleration(MAX_ACCELERATION);
}

void loop() {

    // Run serialcomm packet receiver
    serialport.serialcomm_receive();

    // Get position of both axis
    control_az.dirStatus = recalculateToRange(step2deg(stepper_az.currentPosition()));
    control_el.dirStatus = step2deg(stepper_el.currentPosition());

    //Setting axis limits
    if ((control_az.pointer>MAX_M1_ANGLE || control_az.pointer<MIN_M1_ANGLE)||(control_el.pointer>MAX_M2_ANGLE || control_el.pointer<MIN_M2_ANGLE))
    {
        rotor.rotor_error=true;
    }else{
        rotor.rotor_error=false;
    }
    

    // Rotator movement
    if (!rotor.rotor_error && ONOFF && !rotor.home_error) {

        if(rotor.home_flag){ //if elevation already homed

            // States moveTo position
            if(rotor.msg_flag==true){
                // Azimuth axis uses relative coordinates, Elevation axis uses absolute coordinates
                stepper_az.move(deg2step(minTurn(control_az.prev_pointer,control_az.pointer)));
                stepper_el.moveTo(deg2step(control_el.pointer));
                rotor.rotor_status=moving;
                rotor.msg_flag=false;
            }

            if (rotor.rotor_status==moving){

                // Move azimuth and elevation motors
                stepper_az.enableOutputs();
                stepper_el.enableOutputs();
                stepper_az.run();
                stepper_el.run();

                // Idle rotator
                if (stepper_az.distanceToGo() == 0 && stepper_el.distanceToGo() == 0) {
                    rotor.rotor_status = idle;
                    control_az.prev_pointer=recalculateToRange(step2deg(stepper_az.currentPosition())); // Sets previous pointer in absolute position
                }
            }else{
                // Stop motors and disable driver for lower consumption 
                stepper_az.stop();
                stepper_az.disableOutputs();
                stepper_el.stop();
                stepper_el.disableOutputs();
            }
        }else{
            homing(deg2step(-MAX_M2_ANGLE)); // Starts Elevation Homing
        }
    } else {
        // Stop motors and disable driver 
        stepper_az.stop();
        stepper_az.disableOutputs();
        stepper_el.stop();
        stepper_el.disableOutputs();
    }

    // Run Serialcomm packet sender
    serialport.serialcomm_send();
} 

// Homing elevation axis, stops when reaches switch
void homing(int32_t seek_el) {
    
    rotor.rotor_status=homing_el;
    serialport.serialcomm_send();

    bool isHome_el=false;

    stepper_el.enableOutputs();
    stepper_az.enableOutputs();
    stepper_el.setSpeed(500); //Constant speed for homing
    stepper_el.moveTo(seek_el);


    while(isHome_el==false){

        if (switch_el.get_state()==true && isHome_el==false)
        {
            isHome_el=true;
            stepper_el.moveTo(stepper_el.currentPosition()); 
            stepper_el.stop();
        }
        if (stepper_el.distanceToGo()==0 && !isHome_el)
        {
            rotor.home_error=true;
            rotor.rotor_status=idle;
            return;
        }
        if (isHome_el==false)
        {
            stepper_el.runSpeed();
        }
    }

    stepper_el.setCurrentPosition(0);
    stepper_az.setCurrentPosition(deg2step(control_az.prev_pointer));      

    rotor.home_flag=true;
    rotor.rotor_status=idle;
    return;
}


int32_t deg2step(float deg) {
    return (RATIO * SPR * deg / 360);
}

float step2deg(int32_t step) {
    return (360.00 * step / (SPR * RATIO));
}

// Calculates wheter the rotor passes through 0 position, then returns the shorterst turn (Only Azimuth stepper)
float minTurn(float _prev, float _point){
    float prev=_prev;
    float point=_point;
    if (abs(prev-point)>180){

        if (prev<point){
            return(point-prev-360);
        }else if (prev>point){
            return(point-prev+360);
        }else{
            return 0;}

    }else{
        return(point-prev);
    }
}

// Recalculates current position in case position exceeds limits [0-360] (Only Azimuth stepper)
float recalculateToRange(float current){
    if (current>360)
    {
        return(current-360*trunc(current/360));
    }else if(current<0){
        return(360*(trunc(abs(current)/360)+1)+current);
    }else{
        return(current);
    }
}
