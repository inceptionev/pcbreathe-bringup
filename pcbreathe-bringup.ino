//bringup test code for RespiraWorks Ventilator Mainboard Rev 1.0
//use STM32duino and select appropriate board and COM port and use SWD programming
//analog outputs can be plotted use Arduino Serial Plotter
//
//## HOW TO USE THIS TEST:
//* Follow the instructions on the https://github.com/inceptionev/pcbreathe-bringup readme


#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <powerSTEP01ArduinoLibrary.h>


//hardware definitions
//PCB pins
#define PIN_PRES PA1
#define PIN_INH PA4
#define PIN_EXH PB0
#define PIN_VSENSE PA0
#define PIN_BUZZER PB4
#define PIN_BLOWER PB3
#define PIN_SOLENOID PA11
#define PIN_HEATER PA8
#define PIN_LED_R PC13
#define PIN_LED_Y PC14
#define PIN_LED_G PC15
const int chipSelect = PA15;  //SD card chip select
// Pin definitions for the X-NUCLEO-IHM03A1 (stepper driver)
#define nCS_PIN PB6
#define STCK_PIN PC7
#define nSTBY_nRESET_PIN PA9
#define nBUSY_PIN PB5

//Pinch valve motion settings
#define STARTSTROKE 7000
#define OPENPOS 200
#define CLOSEDPOS 6200

//i2c test device definitions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  //instantiate display

//instantiate stepper driver
powerSTEP driver(0, nCS_PIN, nSTBY_nRESET_PIN);

//test parameters
#define BUZZER_VOL 5 //buzzer volume
#define CYCLE_PERIOD 1000 //actuation cycle timing in ms
#define BLOWER_HIGH 150 //blower high throttle command
#define BLOWER_LOW 100 //blower low throttle command

//create state machine variables
int pressure = 0;
int flow_inh = 0;
int flow_exh = 0;
int vsense = 0;
int inh_flow =0;
int exh_flow = 0;
int state = 0;
unsigned int now = 0;

//create objects for SD card test
Sd2Card card;
SdVolume volume;
SdFile root;

//instantiate USART3
HardwareSerial Serial3(PB11,PB10);

void setup() {
  // put your setup code here, to run once:
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial3.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  //set hw pin modes
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(PIN_LED_R,OUTPUT);
  pinMode(PIN_LED_Y,OUTPUT);
  pinMode(PIN_LED_G,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_BLOWER,OUTPUT);
  pinMode(PIN_SOLENOID,OUTPUT);
  pinMode(PIN_HEATER,OUTPUT);
  //Stepper driver pins
  pinMode(nSTBY_nRESET_PIN, OUTPUT);
  pinMode(nCS_PIN, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SCK, OUTPUT);

  // Reset powerSTEP and set CS
  digitalWrite(nSTBY_nRESET_PIN, HIGH);
  digitalWrite(nSTBY_nRESET_PIN, LOW);
  digitalWrite(nSTBY_nRESET_PIN, HIGH);
  digitalWrite(nCS_PIN, HIGH);

  // Start SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);

  // Configure powerSTEP
  driver.SPIPortConnect(&SPI); // give library the SPI port
  
  driver.configSyncPin(BUSY_PIN, 0); // use SYNC/nBUSY pin as nBUSY, 
                                     // thus syncSteps (2nd paramater) does nothing
                                     
  driver.configStepMode(STEP_FS_128); // 1/128 microstepping, full steps = STEP_FS,
                                // options: 1, 1/2, 1/4, 1/8, 1/16, 1/32, 1/64, 1/128
                                
  driver.setMaxSpeed(1000); // max speed in units of full steps/s 
  driver.setFullSpeed(2000); // full steps/s threshold for disabling microstepping
  driver.setAcc(2000); // full steps/s^2 acceleration
  driver.setDec(2000); // full steps/s^2 deceleration
  
  driver.setSlewRate(SR_520V_us); // faster may give more torque (but also EM noise),
                                  // options are: 114, 220, 400, 520, 790, 980(V/us)
                                  
  driver.setOCThreshold(8); // over-current threshold for the 2.8A NEMA23 motor
                            // used in testing. If your motor stops working for
                            // no apparent reason, it's probably this. Start low
                            // and increase until it doesn't trip, then maybe
                            // add one to avoid misfires. Can prevent catastrophic
                            // failures caused by shorts
  driver.setOCShutdown(OC_SD_ENABLE); // shutdown motor bridge on over-current event
                                      // to protect against permanant damage
  
  driver.setPWMFreq(PWM_DIV_1, PWM_MUL_0_75); // 16MHz*0.75/(512*1) = 23.4375kHz 
                            // power is supplied to stepper phases as a sin wave,  
                            // frequency is set by two PWM modulators,
                            // Fpwm = Fosc*m/(512*N), N and m are set by DIV and MUL,
                            // options: DIV: 1, 2, 3, 4, 5, 6, 7, 
                            // MUL: 0.625, 0.75, 0.875, 1, 1.25, 1.5, 1.75, 2
                            
  driver.setVoltageComp(VS_COMP_DISABLE); // no compensation for variation in Vs as
                                          // ADC voltage divider is not populated
                                          
  driver.setSwitchMode(SW_USER); // switch doesn't trigger stop, status can be read.
                                 // SW_HARD_STOP: TP1 causes hard stop on connection 
                                 // to GND, you get stuck on switch after homing
                                      
  driver.setOscMode(INT_16MHZ); // 16MHz internal oscillator as clock source

  // KVAL registers set the power to the motor by adjusting the PWM duty cycle,
  // use a value between 0-255 where 0 = no power, 255 = full power.
  // Start low and monitor the motor temperature until you find a safe balance
  // between power and temperature. Only use what you need
  driver.setRunKVAL(60); //2.8V in voltage mode for 2A max on 1.4ohm coils
  driver.setAccKVAL(60);
  driver.setDecKVAL(60);
  driver.setHoldKVAL(32);

  driver.setParam(ALARM_EN, 0x8F); // disable ADC UVLO (divider not populated),
                                   // disable stall detection (not configured),
                                   // disable switch (not using as hard stop)

  driver.getStatus(); // clears error flags

  //home the actuator
  driver.move(REV, STARTSTROKE); // move into the stop
  while(driver.busyCheck()); // wait fo the move to finish
  driver.resetPos(); //establish home

  //Setup display (i2c test)
  Wire.begin();
  for (int k=0;k<4;k++) {
    Wire.beginTransmission(0x70); //address the i2c switch
    Wire.write(4+k); //select i2c port, base address 4, cycle thru 5-7
    Wire.endTransmission(); //send and stop
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
  }


  //TEST: SD CARD
  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);


}

void loop() {
  // put your main code here, to run repeatedly:

  //TEST: Blower control, buzzer, LEDs, heater switch, solenoid switch
  switch(state) {
    case 0:
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_Y, HIGH);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_SOLENOID, HIGH);
      digitalWrite(PIN_HEATER, LOW);
      analogWrite(PIN_BUZZER, BUZZER_VOL);
      analogWrite(PIN_BLOWER, BLOWER_HIGH);
      driver.goTo(OPENPOS);
      
      state = 1;
      break;

    case 1:
      digitalWrite(LED_BUILTIN,LOW);
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_Y, LOW);
      digitalWrite(PIN_LED_G, HIGH);
      digitalWrite(PIN_SOLENOID, LOW);
      digitalWrite(PIN_HEATER, HIGH);
      analogWrite(PIN_BUZZER, 0);
      analogWrite(PIN_BLOWER, BLOWER_LOW);
      driver.goTo(CLOSEDPOS);
      
      state = 0;
      break;

    default:
      state = 0;
      break;
  }
  pressure = analogRead(PIN_PRES);
  flow_inh = analogRead(PIN_INH);
  flow_exh = analogRead(PIN_EXH);
  vsense = analogRead(PIN_VSENSE);
  now = (unsigned int)millis();
  
  //Output serial data in Cypress Bridge Control Panel format
  //Serial.print("C"); //output to monitor
  //Serial.write(now>>8);
  //Serial.write(now&0xff);
  //Serial.write(int(pressure)>>8); //output to monitor
  //Serial.write(int(pressure)&0xff); //output to monitor
  //Serial.write(int(flow_inh)>>8); //output to monitor
  //Serial.write(int(flow_inh)&0xff); //output to monitor
  //Serial.write(int(flow_exh)>>8); //output to monitor
  //Serial.write(int(flow_exh)&0xff); //output to monitor
  
  //Output serial data in Arduino Serial Plotter Format
  Serial.print(pressure);
  Serial.print("\t");
  Serial.print(flow_inh);
  Serial.print("\t");
  Serial.print(flow_exh);
  Serial.print("\t");
  Serial.println(vsense);
  Serial.write(Serial3.read()); //prints the last character received from pi
  Serial.print("\r\n");
  
  //output the same thing to the Pi
  Serial3.print(pressure);
  Serial3.print("\t");
  Serial3.print(flow_inh);
  Serial3.print("\t");
  Serial3.print(flow_exh);
  Serial3.print("\t");
  Serial3.print(vsense);
  Serial3.print("\t");
  Serial3.write(Serial3.read()); //prints the last character received from pi
  Serial3.print("\r\n");

  //Print to display using the i2c switch
  for (int k=0;k<4;k++) {
    Wire.beginTransmission(0x70); //address the i2c switch
    Wire.write(4+k); //select i2c port, base address 4, cycle thru 5-7
    Wire.endTransmission(); //send and stop
    display.setCursor(0, 0);     // Start at top-left corner
    display.clearDisplay();
    display.print(F("Port "));
    display.println(k);
    display.println(" ");
    display.print(pressure);
    display.print(" ");
    display.print(flow_inh);
    display.print(" ");
    display.print(flow_exh);
    display.print(" ");
    display.print(vsense);
    display.print(" ");
    display.write(Serial3.read()); //prints the last character received from pi
    display.display(); //send the buffer
  }
  
  

  delay(CYCLE_PERIOD);
}
