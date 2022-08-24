# M5AngleReadWireless

This folder contains the a master and slave code where a M5 Atom Matrix with the inbuilt MPU6886 to read the 6 axis IMU readings. These readings are processed through the AHRS algorithm provided by M5 docs and then sent via ESP-NOW to a master M5. The master M5 reads this data and prints them onto a serial monitor.

Note for use with another MCU, you need to change the mac address

The current mac address that recieves the angles is: {0x4C,0x75,0x25,0xC4,0xFA,0xF4}
