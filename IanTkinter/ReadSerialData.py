#Code to read serial data from M5
import serial
import sys, os #allow keyboard interrupt to exit
import json

#File that is imported by GUIBuild to read Serial Data

def take_from_home(file_name):
    return os.path.join(sys.path[0],file_name) #look for file in the current directory

m5 = serial.Serial(port='COM14', baudrate=115200, timeout=.1) #the com port depends on the computer
file_name = take_from_home('angle_json_data.json')

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
        json.dump(data,json_file)
        

def read_data():
    data = m5.readline()
    number = data.decode().strip() #decode data \n etc from serial data
    return number

def update_json_file(angle):
    #hmm its index?
    data['physio_data'][0]['Patient1'] = str((angle))
    with open(file_name,'w') as outfile:
        json.dump(data,outfile) #save data
    print("Json File Updated")



while True:
    try:
        mydata = read_data()
        if mydata != "":
            print(mydata)
            lis = mydata.split(" ")
            update_json_file(lis[2]) #last number is the angle, so only update that angle
            
    except KeyboardInterrupt:
        m5.close()
        print("Serial port COM14 closed")
        sys.exit()

    

    
    
