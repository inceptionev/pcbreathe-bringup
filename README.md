# pcbreathe-bringup
bringup code for RespiraWorks Ventilator Mainboard Rev 1.0

# STM32duino setup
To use this code, you will need to setup the Arduino IDE to talk to STM32.  Use this link for instructions: [http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/](http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/)
You may have better luck using the SWD update mode than that of Mass Storage.

# Getting data out
The code outputs data on the serial port attached to the ST-LINK USB serial port emulator interface at 9600 baud.
The code has the ability to output in either Arduino Plotter format or Cypress Bridge Control Panel formal, just uncomment the correspoinding lines of code at the bottom of the sketch.
