# Demo Instructions

## Requirements
    matplotlib==3.5.2
    pyserial==3.5


This files details the steps for using the M5 IMUs with guibuild file, to show the joint angle in real time. This demo is the same as how we demoed with Mark at Active Global.

1) Using a type c cable, plug the Master M5 Atom into the USB port of a computer. Check that the cable is a data cable, by opening `Device Manager > Ports ` If you see the M5 atom there then you are good to go. Note that the Master M5 Atom has a grey wire on it, while the slaves do not have any wires at the GPIO ports

2) Attach 2 `Tailbat`s to the slave atoms. It should turn on immediately and show the green battery bar. If not press once. If there is no red light, the `Tailbat` has run out of battery, use another one. 

3) Note that there is no more callibration. After 2s, the master M5 should have 2 flashing RGB lights. This signifies that it is recieving data from the 2 slave atoms

4) Run the guibuild.py file. A Window should show the real time data of joint angle between the 2 M5 IMUs.