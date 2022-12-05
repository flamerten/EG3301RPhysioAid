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

def change_angle(mcu,patient_no, angle):
    """Code not implemeted yet"""
    number_to_be_sent = str(patient_no * 1000 + angle)


    """
    Note that the angle ranges from 0 to 180.
    Hence we give a 4 digit number, with the first number being the patient number
    and the other 3 digits representing the angle to beep the motor at

    Note that we might need to implement another algo for 2 angles, angle low and angle high :(
    """

    mcu.write(number_to_be_sent.encode())



if __name__ == "__main__": #Only if file is run

    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    

    serial_mcu = serial.Serial(port = PORT, baudrate=BAUDRATE, timeout=TIMEOUT) #defined above

    time.sleep(2)
    
    while True:
        try:
            mydata = ""
            serial_mcu.reset_input_buffer() #remove all the data that was left behind

            while(mydata == ""):
                mydata = read_data(serial_mcu).split(" ")
                if(mydata[0] != mydata[-1] != "x"  or len(mydata)!= 5):
                    mydata = "" #reset flag, as data is corrupted
            
            numbers = list(map(int,mydata[1:-1])) #change all the numbers to int
            
            print("Angle1:%3d Angle2:%3d Angle3:%3d" % (numbers[0],numbers[1],numbers[2] ))

            
                
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
