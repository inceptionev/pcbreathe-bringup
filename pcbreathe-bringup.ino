//bringup test code for RespiraWorks Ventilator Mainboard Rev 1.0
//use STM32duino and select appropriate board and COM port and use SWD programming
//analog outputs can be plotted use Arduino Serial Plotter
//
//## HOW TO USE THIS TEST:
//* Follow the instructions on the https://github.com/inceptionev/pcbreathe-bringup readme


#include <SPI.h>
#include <SD.h>

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

//test parameters
#define BUZZER_VOL 3 //buzzer volume
#define CYCLE_PERIOD 200 //actuation cycle timing in ms
#define BLOWER_HIGH 100 //blower high throttle command
#define BLOWER_LOW 50 //blower low throttle command

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
HardwareSerial Serial3((int)PA3, (int)PA2);

void setup() {
  // put your setup code here, to run once:
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial3.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //set hw pin modes
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(PIN_LED_R,OUTPUT);
  pinMode(PIN_LED_Y,OUTPUT);
  pinMode(PIN_LED_G,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_BLOWER,OUTPUT);
  pinMode(PIN_SOLENOID,OUTPUT);
  pinMode(PIN_HEATER,OUTPUT);


  //TEST: SD CARD
  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
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
    while (1);
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

  //output the same thing to the Pi
  Serial3.print(pressure);
  Serial3.print("\t");
  Serial3.print(flow_inh);
  Serial3.print("\t");
  Serial3.print(flow_exh);
  Serial3.print("\t");
  Serial3.println(vsense);

  delay(CYCLE_PERIOD);
}
