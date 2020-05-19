# pcbreathe-bringup
bringup code for RespiraWorks Ventilator Mainboard Rev 1.0

make sure to check the pcbreathe repo for quick start quide and board hardware errata: [https://github.com/RespiraWorks/pcbreathe](https://github.com/RespiraWorks/pcbreathe)

for unofficial (proceed at your own risk!) test code with a state machine for running a ventilator setup in closed-loop control mode, see: [https://github.com/inceptionev/FMLtest](https://github.com/inceptionev/FMLtest)

Quick notes:
* If you want to use this for capturing serial data out, you may need to comment or uncomment some of the sections at the bottom of the code for Cypress or Arduino serial formatting.  It indicates in code which blocks apply.  
* Also, the i2c display block (also labeled) slows down the cycle rate quite a bit, so comment out that block too if you want to speed it up.  
* You will need to install some dependencies to use the code, which is also documented in the readme on git, in the section titled "STM32duino setup".  

## How to use this test:
* Follow the instructions on the https://github.com/RespiraWorks/pcbreathe readme if you need help getting the hardware set up, where to plug things in, etc.
* Follow the instructions below under the STM32duino setup section to get program the Nucleo.  Remember to switch JP5 to the E5V position.
* Insert a FAT32-formatted micro SD card into the cycle controller SD card slot on the PCB.
* Connect representative loads to the heater and solenoid switch outputs on the PCB.  Pick something like a solenoid that will respond visually or audibly)
* To test i2c, connect a representative i2c device (the code is written for an [SSD1306 OLED display](https://www.amazon.com/gp/product/B07RKPSHRK)) to one or all of the i2c sensor ports.
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
    * If you have i2c oled displays hooked up, they will also display this data, along with their port number.  (note that you cannot pull out the OLED display and move it between ports while the program is live to test all the ports.  To do this you must reset the STM32 each time so that the display initializes correctly)
    * Gently press your finger over the positive port of each of the dP sensors and you should see each reading go up slightly in turn.
    * The expected value for vsense is about 814 for 12.0V input (expect 780-850 for 11.5-12.5V)
    * If you have an Rpi connected, open up a serial terminal on the Rpi such as minicom (you may have to disable serial console and enable serial port in Rpi Config)
    * Set the terminal to 9600 8N1, no flow control.
    * Observe that you see the same data here (3x dP readings, vsense, last char received)
    * Hold down a key on the Pi to transmit characters and see that the last char received echoes it back.
    

## STM32duino setup
* To use this code, you will need to setup the Arduino IDE to talk to STM32.  Use this link for instructions: [http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/](http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/)
* note: You may have better luck using the SWD update mode than that of Mass Storage.  However, to use the SWD mode you will need to download and install the [STM32CubeProgrammer](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stm32cubeprog.html#overview)
* To program this board, you will need to go to Tools... and select the following
    * Board: Nucleo-64
    * Board part number: L452RE
    * Upload method: STM32CubeProgrammer (SWD)
    * Port: [select the com port where you computer assigned the Nucleo]
    * other settings can be left on their defaults.
* Further dependencies.  You will also need to install the following packages to run this code (install by going to Tools...Manage Libraries... in your Arduino IDE and seaching for the below):
  * Adafruit SDD1306
  * Adafruit GFX



## Getting data out
The code outputs data on the serial port attached to the ST-LINK USB serial port emulator interface at 9600 baud.

The code has the ability to output in either Arduino Plotter format or Cypress Bridge Control Panel formal, just uncomment the correspoinding lines of code at the bottom of the sketch.
