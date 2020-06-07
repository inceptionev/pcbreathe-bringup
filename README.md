# pcbreathe-bringup
# bringup test code for RespiraWorks Ventilator Mainboard Rev 1.0
use STM32duino and select appropriate board and COM port and use SWD programming

this branch of pcbreathe-bringup, pcbreathe-PinchValveCharacterization, was created to characterize the proportional stepper valves designed by Marc Auger and the venturi flow sensors designed by Ethan Chaleff for RespiraWorks.

This code was writted by Edwin Chiu for RespiraWorks.

## WHAT IT DOES:
* It sets the blower to a fixed power level and ramps through a series of position in the inhale limb pinch valve.
* For characterizing the pinch valves, it can be used with a low-dP flow sensors such as the AD Pneumotachograph downstream of the valve.  Note that you may want to remove the venturi for this test.
* It can also be used to provide varying flow rates for venturi sensor characterization.  In this case, put the venturi downstream of the valve and a reference flow sensor downstream of that.
* It supports an AMS5915_0100_D i2c differential pressure sensor on port I2C SENSOR D on the PCB.  Refer to the ICD if you need a pinout. (But the setup may have an adapter cable provided to plug directly into I2C SENSOR D.)

## SETTINGS:
* In the #defines below, most of the settings for this test can be edited
* STATE_PERIOD is the dwell time at each valve position setting
* CONTROL_PERIOD is the sampling rate of the data output
* The test will cycle from OPENPOS with increment STEPSIZE until it reaches CLOSEDSIZE.
* Then it will make a one-STATE_PERIOD blip to position BLIPPOS, this creates a clear signal to assists in aligninging the data with a reference data acquision setup.
* The test will then cycle back from CLOSEDPOS with increment STEPSIZE until it reaches OPENPOS.

## GETTING DATA OUT:
Outputs can be plotted and exported with Cypress PSoC Programmer (Bridge Control Panel Tool)
* Download and install, connect serial
* Tools > Protocol Configuration > serial 115200:8n1 > hit OK
* In the editor tab, use this command:

    ```RX8 [h=43] @1Key1 @0Key1 @1Key2 @0Key2 @1Key3 @0Key3 @1Key4 @0Key4 @1Key5 @0Key5 @1Key6 @0Key6 @1Key7 @0Key7```
* In order, each of these outputs is: Time(ms), Valve Position, Pressure dP, Inhale dP, Exhale dP, AMS5915
* The MPXV5004DPs come out as 10bit values and the scaling is ```kPa = 5*Value/1023-1```.  The AMS5915 is 14bit and the scaling is kPa = ```10*(Value-1638)/(14745-1638)```.
* The MPXV5004DP sensors are assigned to Pressure, Inhale, and Exhale on this board, but of course you can connect them to anything you want to measure
* The hypodermic needles can be useful for picking off pressures anywhere you have rubber tubing.
* Be careful not to poke yourself.
* Watch out for dynamic pressure effects when using the needles.  I have found better results by inserting the needle at an acute angle to the flow, pointing downstream, and retracting the needle such that the opening is near the sidewall of the hose.
* Chart > Variable Settings
* Tick Key1 through Key7, configure as int, and choose colors > hit OK
* Press >|< icon to connect to com port if necessary
* Click REPEAT button, go to Chart tab  
* both traces should now be plotting
* Click STOP button to stop recording.
* Chart > Export Collected Data in the format of your choice.  Note that this method captures a maximum of 10,000 samples.  It will clip the beginning of your experiment if it is longer than 10k samples.
* If you need more than 10k samples use a different logger (or use the TO FILE button instead of REPEAT, it will output hex data that you can copy-and-paste, so be prepared to do some post-processing)

## HOW TO USE THIS TEST (generic to pcbreathe-bringup):
* Follow the instructions on the https://github.com/inceptionev/pcbreathe-bringup readme to set up the hardware and the IDE.
    

## STM32duino setup
* To use this code, you will need to setup the Arduino IDE to talk to STM32.  Use this link for instructions: [http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/](http://www.emcu.eu/2017/03/13/how-to-use-stm32-and-arduino-ide/) 
* To program this board, you will need to go to Tools... and select the following
    * Board: Nucleo-64
    * Board part number: L452RE
    * Upload method: Mass Storage
    * Port: [select the com port where you computer assigned the Nucleo]
    * other settings can be left on their defaults.
* Further dependencies.  You will also need to install the following packages to run this code (install by going to Tools...Manage Libraries... in your Arduino IDE and seaching for the below):
  * Adafruit SDD1306
  * Adafruit GFX
  * [powerSTEP01 arduino library](https://github.com/Megunolink/powerSTEP01_Arduino_Library) (may require manual install: follow the directions in the readme of the library.) 
  * some folks have reported issues with the Mass Storage programming method (not enough space error).  Here are a couple things to try:
  * make sure you have the latest version of the Arduino IDE and the STM32duino board package (first link in this section up abobe).
  * If that doesn't work the "STM32CubeProgrammer (SWD)" programming option will almost certainly work.  However, to use the SWD mode you will need to download and install the [STM32CubeProgrammer](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stm32cubeprog.html#overview)


## Getting data out
The code outputs data on the serial port attached to the ST-LINK USB serial port emulator interface at 9600 baud.

The code has the ability to output in either Arduino Plotter format or Cypress Bridge Control Panel formal, just uncomment the correspoinding lines of code at the bottom of the sketch.
