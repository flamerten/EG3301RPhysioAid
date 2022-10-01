from time import sleep
import socketio

# standard Python
sio = socketio.Client()

sio.connect('http://127.0.0.1:5000')

while(True):
    try:
        sio.emit("log", {'foo': 'bar'})
    except:
        print("Error emmiting")
        exit()
    print("Emmited")
    sleep(0.5)