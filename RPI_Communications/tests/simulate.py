import sys
import os
sys.path.append(os.getcwd())
import serial, serial.tools.list_ports #reading COM ports
import sys, os #allow keyboard interrupt to exit
import json #editing of files
import time 
from random import random
import socketio
import bluetooth
import constants
import time
import json
from threading import Thread
from models import Reading

reading_attr = []
freq = 1
def read_data(mcu):
    data = mcu.readline()
    numbers = data.decode().strip() #decode data \n etc from serial data
    return numbers

def find_MCU_port():
    ports = list(serial.tools.list_ports.comports())
    
    for p in ports:
        print(p.description)
        if "M5" in p.description: #i am NOT SURE if this will apply to all MCU, im worried it might apply to usb mouse
            print(p.device + " is the port" )
            return p.device
    
    print("Unable to find valid port, COM0 returned")
    return 'COM0'

def simulate(**kwargs):
    sio = socketio.Client()

    try:
        sio.connect(f'{address}:{port}', namespaces=['/'])
    except socketio.exceptions.ConnectionError:
        print("Unable to connect to socket.")
        return()

    reading_attr = json.loads(os.getenv(constants.READING_ATTR_ENV_VAR, constants.DEFAULT_ATTR))[constants.READING_ATTR_VAR_NAME]
    start = time.time()
    freq = float(os.getenv(constants.FREQ_ENV_VAR, constants.DEFAULT_FREQ))
    id = "null"
    duration = float(os.getenv('SIM_DURATION', 3600))
    num_of_sensors = int(os.getenv(constants.NUM_OF_SENSORS_ENV_VAR, constants.DEFAULT_NUM_OF_SENSORS))
    mode = os.getenv(constants.SENSOR_MODE_ENV_VAR, constants.DEFAULT_SENSOR_MODE)
    
    if kwargs:
        id_name = list(kwargs.keys())[0] #First argument ALWAYS ID
        id = kwargs[id_name]
    angle=59
    while duration > time.time() - start:
        if mode == 'INDIVIDUAL':
            reading = {id_name: id}
            reading = reading | {attr: random() * 90 for attr in reading_attr}
            # print(reading)
            reading = Reading(**reading)
            
            try:
                sio.emit(constants.DEFAULT_READING_EMIT_EVENT, reading.to_json())
            except Exception as e:
                print(e)
                sio.disconnect()
                return()

        elif mode == 'MASTER':
            try:
                mydata = ""
                serial_mcu.reset_input_buffer() #remove all the data that was left behind
                
                

                while(mydata == ""):
                    mydata = read_data(serial_mcu)
                    lis = mydata.split(" ")
                    if(lis[0] != 'x') and (lis[-1]!='x'):
                        mydata = ""
                        
                        continue
                
                
                lis = mydata.split(" ")
                angle1 = lis[1]
                angle2 = lis[2]
                angle3 = lis[3]
            
                
            except KeyboardInterrupt:
                serial_mcu.close()
                print("Serial port",PORT,"closed by user")
                sys.exit()

            except:
                serial_mcu.close()
                print("We have an error, closing serial port", PORT)
                sys.exit()
            
            
            reading = {"Sensor1": angle1,"Sensor2": angle2,"Sensor3": angle3}
            
            # print(reading)
            try:
                sio.emit(constants.DEFAULT_READING_EMIT_EVENT, json.dumps(reading))
            except Exception as e:
                print(e)
                sio.disconnect()
                return()
            
        else:
            sio.disconnect()
            raise Exception('Mode is either INDIVIDUAL or MASTER')
           
        time.sleep(1/freq)

    sio.disconnect()

def create_threads():
    threads = []
    num_of_sensors = int(os.getenv(constants.NUM_OF_SENSORS_ENV_VAR, constants.DEFAULT_NUM_OF_SENSORS))
    for i in range(num_of_sensors):
        t = Thread(target=simulate, args=[], kwargs={'id': f'thread{i}'})
        threads.append(t)
    return threads


if __name__ == '__main__':
    PORT = find_MCU_port()
    if(PORT == 'COM0'):
        print("No valid usb device connected")
        sys.exit() #exit

    BAUDRATE = 115200
    TIMEOUT = 0.1
    

    serial_mcu = serial.Serial(port = PORT, baudrate=BAUDRATE, timeout=TIMEOUT)
    address = os.getenv(constants.ADDRESS_ENV_VAR, constants.DEFAULT_ADDRESS)
    port = os.getenv(constants.PORT_ENV_VAR, constants.DEFAULT_PORT)
    mode = os.getenv(constants.SENSOR_MODE_ENV_VAR, constants.DEFAULT_SENSOR_MODE)
    reading_attr = json.loads(os.getenv(constants.READING_ATTR_ENV_VAR, constants.DEFAULT_ATTR))[constants.READING_ATTR_VAR_NAME]
    freq = float(os.getenv(constants.FREQ_ENV_VAR, constants.DEFAULT_FREQ))
    duration = float(os.getenv('SIM_DURATION', 3600))
    num_of_sensors = int(os.getenv(constants.NUM_OF_SENSORS_ENV_VAR, constants.DEFAULT_NUM_OF_SENSORS))
    mode = os.getenv(constants.SENSOR_MODE_ENV_VAR, constants.DEFAULT_SENSOR_MODE)
    
    print(f'Address: {address}')
    print(f'Port: {port}')
    print(f'Number of sensors: {num_of_sensors}')
    print(f'Attributes: {reading_attr}')
    print(f'Emit frequency: {freq}')
    print(f'Duration: {duration}')
    print(f'Mode: {mode}')

    if mode == "MASTER":
        simulate()
        exit()
    elif mode == "INDIVIDUAL":
        threads = create_threads()
    else:
        raise Exception('Mode is either INDIVIDUAL or MASTER')

    if threads:
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()



    
    

