
#ifndef __SERIALCOMM_H__
#define __SERIALCOMM_H__

#include <Arduino.h>
#include <globals.h>

#define BUFFER_SIZE   256   // Set the size of serial buffer
#define BAUDRATE      9600 /// Set Baudrate

class serialcomm {
public:

    void serialcomm_init() {
	    Serial.begin(BAUDRATE);
    }

    // Read from Serial Port. Expected message structure is $header,value1,value2(if exists),@ Example: $PO,(AZ),(EL),@
    void serialcomm_receive() {
        char buffer[BUFFER_SIZE];
        char incomingByte;
        char *Data = buffer;
        char *rawData;
        static bool dataFlag=0;
        static int BufferCnt=0;
        char data[100];

        while (Serial.available() > 0) {
            incomingByte = Serial.read();
            if(incomingByte=='$'){
                BufferCnt=0;
                dataFlag=true;
            }

            if (incomingByte=='@' && dataFlag){
                if (buffer[1] == 'P' && buffer[2] == 'O') {
                    // Get position in deg for azimuth
                    rawData = strtok_r(Data, ",", &Data);
                    strncpy(data, rawData + 3, 10);
                    if (isNumber(data)) {
                        control_az.pointer=atof(data);
                        rotor.msg_flag=true;
                    }                                 
                    // Get position in deg for elevation
                    rawData = strtok_r(Data, ",", &Data);
                    strncpy(data, rawData, 10);
                    if (isNumber(data)) {
                        control_el.pointer=atof(data);
                        rotor.msg_flag=true;
                    }
                // Get ON or OFF flag    
                }else if (buffer[1] == 'O' && buffer[2] == 'N'){
                    rawData = strtok_r(Data, ",", &Data);
                    strncpy(data, rawData + 3, 10);
                    if (isNumber(data)){
                        if (atoi(data)==1)
                        {
                            ONOFF=true;
                        }else if (atoi(data)==0)
                        {
                            ONOFF=false;
                        }       
                    }
                // Get initial OFFSET    
                }else if (buffer[1] == 'O' && buffer[2] == 'F'){
                    rawData = strtok_r(Data, ",", &Data);
                    strncpy(data, rawData + 3, 10);
                    if (isNumber(data)){
                        OFFSET=atof(data);
                        control_az.prev_pointer=OFFSET;
                    }
                // Get Homing command
                }else if (buffer[1] == 'H' && buffer[2] == 'M'){
                    rawData = strtok_r(Data, ",", &Data);
                    strncpy(data, rawData + 3, 10);
                    if (isNumber(data)){
                        if (atoi(data)==0){
                            rotor.home_flag=false;
                        }
                    }
                }
                
                // Reset the buffer an clean the serial buffer
                Serial.flush();
                dataFlag=false;

            }else{
                buffer[BufferCnt]=incomingByte;
                BufferCnt++;
            }
        }
    }

    // Write Serial Port. Sending message structure: #SD,(AZ_status),(EL_status),(rotor_status),&
    void serialcomm_send(){
        if (Serial.availableForWrite() > 0)
        {
            //int cont=0;
            //String str1, str2, str3;
            //char str1=char(control_az.dirStatus);
            //char str2=char(control_el.dirStatus);            
            //char str3=char(rotor.rotor_status);
            //char hola=char(control_az.dirStatus);
            //Serial.print('#'+'S'+'D'+',' + str1 + ',' + str2 + ',' + str3 + ',' + '&');
            //Serial.print("#SD," + str3 + "," + byte(control_el.dirStatus) + "," + byte(rotor.rotor_status) + "," + "&");
            //Serial.print("#SD," + str1 + ',' + str2 + ',' + str3 + ',' + '&');
            //Serial.print(cont);
            //cont++;
            String str1, str2, str3,str;
            str1=String(control_az.dirStatus,2);
            str2=String(control_el.dirStatus,2);            
            str3=String(rotor.rotor_status);
            str="#SD," + str1 + "," + str2 + "," + str3 + "," + "&";
            Serial.print(str);
        }
    }



private:
    bool isNumber(char *input) {
        for (uint16_t i = 0; input[i] != '\0'; i++) {
            if (isalpha(input[i]))
                return false;
        }
        return true;
    }
};

#endif // __SERIALCOMM_H__