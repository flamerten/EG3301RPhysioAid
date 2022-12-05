# Ian Tkinter

This folder shows Ian's work on using Tkinter to display data coming from the M5 with the connection through serial port

`ReadSerialData.py` is a python file that reads data coming from the serial port of the M5. It assumes that the data is coming in the format >  `angle angle angle`, with COM14 as the port. The data is read using pyserial and saved to a json file, `angle_json_data.json`

The JSON file is read continuously by the file `guibuild.py` it also uses Tkinter and Matplotlib to build a GUI to show patient knee angle (`angle_json_data`) and SEMG data (`emg_json_data.json`).

The arduino folder `RandomNumberGen` is used to generate random numbers for testing the use of Serial Data

#
## Updates

### 31/08/2022
Had some succeess in reading raw data, combining the data read from `M5AngleSync`. However, there was some issues in writing to the json file. This was solved with `try except`. The reason could be `guibuild.py` reads the json file at the exact same time that `ReadSerialData.py` writes to the file. Might need to reduce the frequency of updating the JSON file. 

Had some issues with speed as well, there are 6 graphs and each are updating at its own time. This can be pretty slow because its 6 seperate instances.
