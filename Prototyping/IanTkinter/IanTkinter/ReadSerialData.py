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

def update_json_file(angle):
    #hmm its index?
    data['physio_data'][0]['Patient1'] = str((angle))
    with open(file_name,'w') as outfile:
        json.dump(data,outfile) #save data
    print("Json File Updated")


if __name__ == "__main__": #Only if file is run
    
    file_name = os.path.join(sys.path[0],"AppSource","angle_json_data.json")

    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    

    serial_mcu = serial.Serial(port = PORT, baudrate=BAUDRATE, timeout=TIMEOUT) #defined above

    

    try:
        with open(file_name,'r') as json_file:
            data = json.load(json_file)
    except:
        print('No JSON file currently') #Or the file is empty, nothing there, so create a new file and save it
        with open(file_name,'w') as json_file:
            print("Json file json_data created")
            data = {"physio_data": 
            [{"Patient1": "60"},
            {"Patient2": "20"},
            {"Patient3": "100"}]}
            json.dump(data,json_file) #save json file


    while True:
        try:
            mydata = ""
            serial_mcu.reset_input_buffer() #remove all the data that was left behind

            while(mydata == ""):
                mydata = read_data(serial_mcu)
            
            print(mydata)
            lis = mydata.split(" ")
            update_json_file(lis[2]) #last number is the angle, so only update that angle
            
                
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


    

    
    
