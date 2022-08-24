#Code to read serial data from M5
import serial
import sys #allow keyboard interrupt to exit
import json

import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.animation as animation

m5 = serial.Serial(port='COM14', baudrate=115200, timeout=.1)

#Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
time = []
anlge = []

try:
    with open('json_data.json','r') as json_file:
        data = json.load(json_file)
except:
    print('No JSON file currently') #Or the file is empty, nothing there
    with open('json_data.json','w') as json_file:
        print("Json file json_data created")
        data = {'physio_data':[
            {'name': 'Patient1',
             'joint_angle': '0'}
            ]
                }
        json.dump(data,json_file)

def animate(
        

def read_data():
    data = m5.readline()
    number = data.decode().strip()
    return number

def update_json_file(angle):
    #hmm its index?
    data['physio_data'][0]['joint_angle'] = str(angle)
    with open('json_data.json','w') as outfile:
        json.dump(data,outfile)
    print("Json File Updated")



while True:
    try:
        mydata = read_data()
        if mydata != "":
            print(mydata)
            lis = mydata.split(" ")
            update_json_file(lis[2]) #last number is the angle
            
    except KeyboardInterrupt:
        m5.close()
        print("Serial port COM14 closed");
        sys.exit()

    

    
    
