#Code to read serial data from M5


import serial, serial.tools.list_ports #reading COM ports
import sys, os #allow keyboard interrupt to exit
import json #editing of files
import time 

def find_MCU_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if "USB" in p.description: #i am NOT SURE if this will apply to all MCU, im worried it might apply to usb mouse
            print(p.device + " is the port" )
            return p.device
    
    print("Unable to find valid port, COM0 returned")
    return 'COM0'

def read_data(mcu):
    data = mcu.readline()
    numbers = data.decode().strip() #decode data \n etc from serial data
    return numbers

def vibrate_patient(mcu,patient_no):
    number_to_be_sent = (patient_no)
    mcu.write(bytearray([number_to_be_sent]))

if __name__ == "__main__": #Only if file is run

    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    

    serial_mcu = serial.Serial(port = PORT, baudrate=BAUDRATE, timeout=TIMEOUT) #defined above

    time.sleep(2)


    print(vibrate_patient(serial_mcu,0))


    time.sleep(1)

    serial_mcu.close()
