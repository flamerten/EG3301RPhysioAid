#Code to read serial data from M5
import serial, serial.tools.list_ports #reading COM ports
import sys, os #allow keyboard interrupt to exit

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



if __name__ == "__main__": #Only if file is run
    

    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    

    serial_mcu = serial.Serial(port = PORT, baudrate=BAUDRATE, timeout=TIMEOUT) #defined above



    while True:
        try:
            mydata = ""
            serial_mcu.reset_input_buffer() #remove all the data that was left behind

            while(mydata == ""):
                mydata = read_data(serial_mcu)
            
            print(mydata) #just for viewing

            
                
        except KeyboardInterrupt:
            serial_mcu.close()
            print("Serial port",PORT,"closed by user")
            sys.exit()

        except:
            serial_mcu.close()
            print("We have an error, closing serial port", PORT)
            sys.exit()

        time.sleep(0.1)
        #sleep for 1s (not too sure if i need this, but it might help with frequency issues
        #remember to flush the input buffer


    

    
    
