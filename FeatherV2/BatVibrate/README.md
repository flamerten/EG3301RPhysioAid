# BatVibrate

This folder contains a full set of wearables. Both feathers will have the battery charging indicator, the pogo pin, the IMU, with one having vibration capabilities. A larger button willl be used which is easier to press. The master will communicate to the M5. We also use **PITCH** instead to decrease size

Vibration is done through the M5, the RPI sends a serial message to the M5 which in return sends a message over ESPnow to the correct feather.

Vibration is done on the slave, because the master feather is already recieving data from the slave, so we want to avoid too much data being sent/recieved from eat feather


## Features
    #define PATIENT_NO 0 //Patient 0 for this case
    uint8_t master[] = {0x4C,0x75,0x25,0xA1,0x84,0x20}; //USB M5 Matrix
    #define VIBRATE 26 //Transistor, with power to the USB port, and 26 acting as a switch
    #define SLEEP_BUTTON 38 


## Feather MacAddress
    uint8_t MasterFeather = {0xE8,0x9F,0x6D,0x28,0x0C,0x98}; 
    uint8_t SlaveFeather  = {0xE8,0x9F,0x6D,0x25,0x3C,0x24}; //With vibration



    