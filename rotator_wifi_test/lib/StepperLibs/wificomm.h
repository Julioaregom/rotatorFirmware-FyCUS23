
#ifndef __WIFICOMM_H__
#define __WIFICOMM_H__

#include <Arduino.h>
#include <globals.h>
#include <Wifi.h>

#define BUFFER_SIZE   256   // Set the size of serial buffer
#define BAUDRATE      9600 /// Set Baudrate

const char *ssid = "FyCUS23";
const char *password = "Parihuela1notfound";
const int serverPort = 2024;

WiFiServer server(serverPort);
WiFiClient client;
// IPAddress ip(192, 168, 1, 239); 
// IPAddress gateway(192, 168, 1, 1); 
// IPAddress subnet(255, 255, 255, 0); 

class wificomm {
public:

    void wificomm_init() {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {  
        Serial.println("Connecting to WiFi..");
        delay(1000);
        }
        Serial.println("Connected to the WiFi network");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }


    // Read from Serial Port. Expected message structure is $header,value1,value2(if exists),@ Example: $PO,(AZ),(EL),@
    void wificomm_receive() {
        char buffer[BUFFER_SIZE];
        char incomingByte;
        char *Data = buffer;
        char *rawData;
        static bool dataFlag=0;
        static int BufferCnt=0;
        char data[100];

        incomingByte = client.read();
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
                client.flush();
                dataFlag=false;

            }else{
                buffer[BufferCnt]=incomingByte;
                BufferCnt++;
            }
    }

    // Write Serial Port. Sending message structure: #SD,(AZ_status),(EL_status),(rotor_status),&
    void wificomm_send(){

        String str1, str2, str3,str;
        str1=String(control_az.dirStatus,2);
        str2=String(control_el.dirStatus,2);            
        str3=String(rotor.rotor_status);
        client.print("#SD," + str1 + "," + str2 + "," + str3 + "," + "&");

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

#endif // __WIFICOMM_H__