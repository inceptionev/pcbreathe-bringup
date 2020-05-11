//bringup test code for RespiraWorks Ventilator Mainboard Rev 1.0
//use STM32duino and select appropriate board and COM port and use SWD programming
//analog outputs can be plotted use Arduino Serial Plotter

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

int pressure = 0;
int flow_inh = 0;
int flow_exh = 0;
int vsense = 0;
int inh_flow =0;
int exh_flow = 0;
int state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(PIN_LED_R,OUTPUT);
  pinMode(PIN_LED_Y,OUTPUT);
  pinMode(PIN_LED_G,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_BLOWER,OUTPUT);
  pinMode(PIN_SOLENOID,OUTPUT);
  pinMode(PIN_HEATER,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(state) {
    case 0:
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_Y, HIGH);
      digitalWrite(PIN_LED_G, LOW);
      analogWrite(PIN_BUZZER, 2);
      analogWrite(PIN_BLOWER, 100);
      state = 1;
      break;

    case 1:
      digitalWrite(LED_BUILTIN,LOW);
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_Y, LOW);
      digitalWrite(PIN_LED_G, HIGH);
      analogWrite(PIN_BUZZER, 0);
      analogWrite(PIN_BLOWER, 50);
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
  Serial.print(pressure);
  Serial.print("\t");
  Serial.print(flow_inh);
  Serial.print("\t");
  Serial.print(flow_exh);
  Serial.print("\t");
  Serial.println(vsense);
  delay(200);
}
