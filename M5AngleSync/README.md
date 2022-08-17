# M5 Angle Sync

This folder contains the master and slave codes, for 2 slave M5s to connect to 1 M5. This M5 is connected to my computer and outputs the rolls of each M5 and the joint angle. This angle is:

Joint Angle = 180 - abs(Roll_1 - Roll_2)

The communication relied on ESP-NOW. When the slave M5s have turned on, they start a calibration sequence. As it is gradient descent, i give the condition that i need to do calibration for at least 3s, and that the difference between the max and min must be less that 1 degrees. This ensures that the reading has stablised. This offset is added to the roll calculated by the AHRS algorithm provided by [M5 docs](https://docs.m5stack.com/en/api/atom/mpu).

Once the calibration has been done, the slave M5s send a message every 1s via esp-now to the master device. This message consists of the slave MacAddress. It only starts sampling when it has recieved the order from the master M5 to start sampling. The reason for the MacAddress is that this allows the M5 master to differentiate M5s. Also, the message to start sampling is sent to both slave M5s, once 2 **different** esp now messages with different Mac addresses are recieved. The sending is sent to the slave M5s, with the message containing sampling rate and also a different roll angle to another device. This allows that device to start sampling sligtly(50ms) later which prevents ESP-NOW conflicts.

From there both M5s sample the roll and send it via ESP-NOW to the master M5. The sampling rate is decided by the Master. 

The isssue is that the plan needs to be parallel to the z axis, meaning that the top of the M5 must point to the sky vertically. If not the roll will not be vertical. I shall look into calibrating of the axises.
