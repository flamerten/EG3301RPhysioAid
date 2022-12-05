# M5Stack - Initial Prototyping

This folder contains the master and slave codes, for 2 slave M5s to connect to 1 M5. This M5 is connected to my computer and outputs the rolls of each M5 and the joint angle. The angle is calculated by this way, with rolls_rsv containing the rolls of the 2 IMUs.

    angle_knee = int(abs(rolls_rcv[0] - rolls_rcv[1]) );
    if(angle_knee > 180) angle_knee = 360 - angle_knee;

The communication relied on [ESP-NOW](https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/). The roll is calculated by the AHRS algorithm provided by [M5 docs](https://docs.m5stack.com/en/api/atom/mpu).

When the slave M5s have turned on, they start a calibration sequence. As it is gradient descent, i give the condition that i need to do calibration for at least 3s, and that the difference between the max and min must be less that 1 degrees. This ensures that the reading has stablised. This offset is added to the future rolls added.

Once the calibration has been done, the slave M5s send a message every 1s via esp-now to the master device, in order to start the syncing. This allows the M5s to send the angles at the same period and similiar timings. Similar because if its exactly the same there might be some conflict when ESP-NOW sends its messages. So i add a small delay of 50ms.

This broadcast/callibration message consists of the slave MacAddress. It only starts sampling when it has recieved the order from the master M5 to start sampling. The reason for the MacAddress is that this allows the M5 master to differentiate M5s. Also, the message to start sampling is sent to both slave M5s, once 2 **different** esp now messages with different Mac addresses are recieved. The sending is sent to the slave M5s, with the message containing sampling rate and also a different roll angle to another device. 

From there both M5s sample the roll and send it via ESP-NOW to the master M5. The sampling rate is decided by the Master. 

The isssue is that the plan needs to be parallel to the z axis, meaning that the top of the M5 must point to the sky vertically.

#
## Updates
### 31/08/2022
Testing was generally good, the issue of calibration still not solved. However, the angles are relatively accurate when the devices are put on properly. An indicator is used to show which M5 recieves which message. I noticed there were some times where one M5 stop transmitting. The Master M5 indicator only showed one M5 sending, so not too sure why, will test and see the reason.
