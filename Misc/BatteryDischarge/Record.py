from ssl import PROTOCOL_TLS_SERVER
import serial
import sys, os #allow keyboard interrupt to exit
import pandas as pd
import time

Port = "COM14" # For the M5

Arduino = serial.Serial(port=Port, baudrate=115200, timeout=.1) #the com port depends on the computer

start_time = time.time()

time_rec = []
data_rcv = []

def get_curr_mins():
    return ( time.time() - start_time )

def read_data():
    try:
        data = Arduino.readline()
        number = data.decode().strip()
        return number
    except:
        print("Unable to decode data, returning 0")
        return 0



while True:
    try:
        mydata = read_data()
        if mydata != "":
            print(mydata)
            time_rec.append(int(get_curr_mins()))
            data_rcv.append(mydata)
        
        time.sleep(1)

    except KeyboardInterrupt:
        Arduino.close()
        print("Serial port COM24 closed")

        data = list(zip(time_rec,data_rcv))
        df = pd.DataFrame(data, columns = ['Time', 'Data'])
        df.to_csv("Data.csv",index=False)

        sys.exit()
    
    

