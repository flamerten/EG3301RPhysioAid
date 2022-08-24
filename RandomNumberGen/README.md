# Random Number Gen

This is a python file that reads data coming through the Serial port. In practice, we will connect the master M5 to the RPI, and the RPI runs this code to read data coming through.

The data is read using pyserial, and then saved to a json file

The arduino sketch is for testing, but basically prints data in a similar format, and then prints in through serial. However, this data is in integer format while the actual master outputs data through degrees.