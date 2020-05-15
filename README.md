# pcbreathe-bringup
bringup code for RespiraWorks Ventilator Mainboard Rev 1.0

make sure to check the pcbreathe repo for quick start quide and board hardware errata: [https://github.com/RespiraWorks/pcbreathe](https://github.com/RespiraWorks/pcbreathe)

for more test code with a state machine for running a ventilator setup in closed-loop control mode, see: [https://github.com/inceptionev/FMLtest](https://github.com/inceptionev/FMLtest)

## How to use this test:
* Follow the instructions on the https://github.com/RespiraWorks/pcbreathe readme if you need help getting the hardware set up, where to plug things in, etc.
* Follow the instructions on the https://github.com/inceptionev/pcbreathe-bringup readme to get setup using STM32duino to program the nucleo.  Remember to switch JP5 to the U5V position.
* Insert a FAT32-formatted micro SD card into the cycle controller SD card slot on the PCB.
* Connect representative loads to the heater and solenoid switch outputs on the PCB.  Pick something like a solenoid that will respond visually or audibly)
* Program the board.
* Open the serial monitor and set it to 9600 baud (or whatever you've in the code below)
* Press reset to run the SD card test at the beginning, followed by the hardware cycling test
* The following behaviors are expected on a functioning board:
    * The serial monitor should display valid information about the SD card.
    * Red light on the bottom blinks (Green and Yellow LEDs do not work on Rev 1.0 due to a pin conflict)
    * Buzzer will beep softly on every cycle.  If you can't hear it over whatever loads (blower, solenoid) you have connected, increase the volume in the defines below.
    * The blower will turn on and cycle between the two power levels set at the top of the code.
    * The loads you plugged in will alternate activating 
    * The serial console will stream the three pressure sensor readings of the dP sensors, the vsense, and the last chactacter received from Rpi via the UART connection - in the order of patient pressure, inhalation, exhalation, vsense, Rpi character.
    * Gently press your finger over the positive port of each of the dP sensors and you should see each reading go up slightly in turn.
    * The expected value for vsense is about 814 for 12.0V input (expect 780-850 for 11.5-12.5V)
    * Open up a serial terminal on the Rpi such as minicom (you may have to disable serial console and enable serial port in Rpi Config)
    * Set the terminal to 9600 8N1, no flow control.
    * Observe that you see the same data here (3x dP readings, vsense, last char received)
    * Hold down a key on the Pi to transmit characters and see that the last char received echoes it back.

## STM32duino setup
To use this code, you will need to setup the Arduino IDE to talk to STM32.  Use this link for instructions: [http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/](http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/)

note: You may have better luck using the SWD update mode than that of Mass Storage.

## Getting data out
The code outputs data on the serial port attached to the ST-LINK USB serial port emulator interface at 9600 baud.

The code has the ability to output in either Arduino Plotter format or Cypress Bridge Control Panel formal, just uncomment the correspoinding lines of code at the bottom of the sketch.
