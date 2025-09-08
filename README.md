# BLE_con_ARDUINO
Here is the BLE_con.ino file for Nicla Sense Me "hiking app" project 

## Installation
Download [Arduino IDE](https://www.arduino.cc/en/software/).


In Arduino IDE do:


Download Board:
1. ``Boards Manager``->``ArduinoMbed OS Nicla Boards``

Download Libraries:
1. ``Library Manager``->``Arduino_BHY2``
2. ``Library Manager``->``Arduino_BLE``
3. (only needed for printFreeRAM())``Sketch``->``Include Library``->``Add .Zip Library``, then select the downloaded Arduino-MemoryFree.zip from this git

Arduino-MemoryFree Example :

``File``->``Examples``->``Arduino-MemoryFree``


Upload Code:
1. ``Tools``->``Board``->``ArduinoMbed OS Nicla Boards``->``Arduino Nicla Sense ME``
2. ``Tools``->``Port``, select port where nicla sense me is connected to
3. Click ``Upload``

