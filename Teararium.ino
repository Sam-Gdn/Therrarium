///imports///
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <AccelStepper.h>
#include "max6675.h"
#include <HX711_ADC.h> // https://github.com/olkal/HX711_ADC
#include <Wire.h>
    
int Tea1switchPin = 35;
int Tea2switchPin = 27;
int Tea3switchPin = 23;
int smallCupSwitchPin = 34;
int largeCupSwitchPin = 36;
int teapotSwitchPin = 38;  
int craneSwitchPin = 31;  
int craneSwitchPinValue;
int wagonSwitchPin = 33;  
int wagonSwitchPinValue;
int TeaBallUpSwitchPin = 25;  
int TeaBallUpSwitchValue;
int TeaBallDownSwitchPin = 29;  
int TeaBallDownSwitchValue;
int WagonHomePosition;
int WagonDestination = 0;
int CraneHomePosition;
int CraneDestination = 0;
int arm_up_angle = 70;   
int arm_down_angle = 15; 
int closed_teaball_angle = 180; 
int open_teaball_angle = 0;
int pos=0;
boolean teaball_open = false;
///PIN LED///
int switchPinled1 = 24;              //led thé2
int switchPinled2 = 32;              //led petite tasse
int switchPinled3 = 30;              //led thé3
int switchPinled4 = 28;              //led thé1
int switchPinled5 = 22;              // 
int switchPinled6 = 26;   
int ledPin=17;
int firefliesPin=54;
boolean leds_enabled = true;
///SPEAKER PIN
int speakerPin = 6;
///INIT DC MOTOR////
#define Pin1 37  // Pump Motor A pins
#define Pin2 39
///INIT STEPPER///
#include <TMCStepper.h>

#define EN_PIN_craneStepper           45 // Enable
#define DIR_PIN_craneStepper         60 // Direction
#define STEP_PIN_craneStepper         59 // Step
#define SERIAL_PORT_craneStepper Serial1 // TMC2208/TMC2224 HardwareSerial port


#define EN_PIN_wagonStepper           48 // Enable
#define DIR_PIN_wagonStepper         56 // Direction
#define STEP_PIN_wagonStepper         55 // Step
#define SERIAL_PORT_wagonStepper Serial1 // TMC2208/TMC2224 HardwareSerial port

#define R_SENSE 0.11f // Match to your driver
                     // SilentStepStick series use 0.11
                     // UltiMachine Einsy and Archim2 boards use 0.2
                     // Panucatt BSD2660 uses 0.1
                     // Watterott TMC5160 uses 0.075

TMC2208Stepper craneDriver = TMC2208Stepper(&SERIAL_PORT_craneStepper, R_SENSE); // Hardware Serial0
TMC2208Stepper wagonDriver = TMC2208Stepper(&SERIAL_PORT_wagonStepper, R_SENSE); // Hardware Serial0

constexpr uint32_t steps_per_mm = 80;

#include <AccelStepper.h>
AccelStepper craneStepper = AccelStepper(craneStepper.DRIVER, STEP_PIN_craneStepper, DIR_PIN_craneStepper);
AccelStepper wagonStepper = AccelStepper(wagonStepper.DRIVER, STEP_PIN_wagonStepper, DIR_PIN_wagonStepper);

// Create the motor shield object with its I2C address
Adafruit_MotorShield AFMS1 = Adafruit_MotorShield(0x60); 
//Adafruit_MotorShield AFMS3 = Adafruit_MotorShield(0x60); 
///INIT MP3 AMP///
#include <SoftwareSerial.h>

#define ARDUINO_RX 14// Inverser RX et TX !!!!!
#define ARDUINO_TX 15// Inverser RX et TX !!!!!
SoftwareSerial myMP3(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*basic commands*/
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F//stop interruptting with a song, just stop the interlude

/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
#define DEV_TF 0X01
#define CMD_IC_MODE 0X35
#define CMD_SLEEP   0X03
#define CMD_WAKE_UP 0X02
#define CMD_RESET   0X05

/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43

/*Special commands*/
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31

#define CMD_SET_PLAY_MODE 0X33
#define ALL_CYCLE 0X00
#define SINGLE_CYCLE 0X01

#define CMD_PLAY_COMBINE 0X45//can play combination up to 15 songs
void sendCommand(int8_t command, int16_t dat );
///INIT SERVOS///
Servo shovel_servo;  
Servo arm_servo;
Servo teaball_servo;

//INIT Scale
const int HX711_dout = 41; //mcu > HX711 dout pin
const int HX711_sck = 43; //mcu > HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck);
unsigned long t = 0;
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_DCMotor *Silo3Motor = AFMS1.getMotor(1);
Adafruit_DCMotor *Silo2Motor = AFMS1.getMotor(4);
Adafruit_DCMotor *Silo1Motor = AFMS1.getMotor(3);
Adafruit_DCMotor *CraneMotor = AFMS1.getMotor(2);

int PumpMotorState = 0;
int StirrerMotorState = 0;
//INIT CAPTEUR TEMPERATURE
//int thermoDO = 38;
//int thermoCS = 36;
//int thermoCLK = 34;
int thermoDO = 26;
int thermoCS = 24;
int thermoCLK = 22;
int desired_temp = 20;
MAX6675 thermocouple1(thermoCLK, thermoCS, thermoDO);
//thermocouple1.setOffset(-10);
//INIT RELAY
int ThermoblockRelayPin1 = 42;
int ThermoblockRelayPin2 = 46;
int solenoidRelayPin = 44;
int solenoid2RelayPin = 40;
//INIT FLOW SENSOR
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int L_per_hour; // Computed litres/hour
unsigned int mL_per_sec; // Computed mL/sec
unsigned char flowsensor = 2; //pin 18 is an interruptable pin, raises event to interrupt(5) : cf doc interrupt Arduino Mega 2560
float pouring_time;
float volume_poured;
float step_volume_poured;
float flowrate;
//Sensor Input
unsigned long currentTime;
unsigned long cloopTime;
//Crane inputs
float up_to_down_time = 3200;
//power input
const byte powerPin = 18;
boolean powerButtonState = LOW; 
boolean powered = false; 
//tea_selection
int TeaSize = 0;
int teaChoice = 0;
boolean power = false;
boolean teararium_initialized = false;
boolean turn_off = false;
//default preperation settings
int temp1 = 80;
int temp2 = 80;
int temp3 = 80;
int time1 = 3;
int time2 = 3;
int time3 = 3;
float infusingFactor[4] = {0, 1, 1, 1.5};
float waterVolume[4] = {0, 135, 270, 500};
float teaWeight[4] = {0, 0.85, 1.5, 2.25};
int infusingTemp[4] = {80,temp1,temp2,temp3};
float infusingTime[4] = {180, time1*60, time2*60, time3*60};
int previous_tea_index=0;

void setup() {
  pinMode(powerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(firefliesPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  Serial.begin(9600);
  //thermoblock setup
  turn_thermoblock_off();
  ///Led Setup///
  turn_buttons_leds_off();
  //motors setup
  AFMS1.begin();
  stop_motor(1);
  stop_motor(2);
  stop_motor(3);
  stop_teaball();
  stop_pump();
  Serial.println("Teararium turned ON");
  setupMenu();
  Serial.println("Menu OK");
  //set tea switches as inputs
  pinMode(Tea1switchPin, INPUT);
  pinMode(Tea2switchPin, INPUT);
  pinMode(Tea3switchPin, INPUT);
  pinMode(smallCupSwitchPin, INPUT);
  pinMode(largeCupSwitchPin, INPUT);
  pinMode(teapotSwitchPin, INPUT);
  pinMode(craneSwitchPin, INPUT);    
  pinMode(TeaBallDownSwitchPin, INPUT);
  pinMode(TeaBallUpSwitchPin, INPUT);
  ///Crane setup///
  Serial.println("Initializing motor Feather 1");
  Serial.println("motor Feather 1 initialized");
  ///Servos setup///
  //SETUP RELAY
  pinMode(solenoidRelayPin, OUTPUT);
  digitalWrite(solenoidRelayPin, HIGH);
  pinMode(solenoid2RelayPin, OUTPUT);
  digitalWrite(solenoid2RelayPin, HIGH);
  pinMode(ThermoblockRelayPin1, OUTPUT);
  digitalWrite(ThermoblockRelayPin1, HIGH);
  pinMode(ThermoblockRelayPin2, OUTPUT);
  digitalWrite(ThermoblockRelayPin2, HIGH);
  //SETUP SCALE
  float calibrationValue; // calibration value
  calibrationValue = -1710; // tested value 
  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue);
    Serial.println("Load cell startup is complete");
  }
  while (!LoadCell.update());
  Serial.println(LoadCell.getCalFactor());
  Serial.println(LoadCell.getConversionTime());
  Serial.println(LoadCell.getSPS());
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  }
  else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }
  //SETUP PUMP DC MOTOR
  pinMode(Pin1, OUTPUT);
  pinMode(Pin2, OUTPUT);
  digitalWrite(Pin1, LOW);
  digitalWrite(Pin2, LOW);
  //SETUP FLOW SENSOR
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, RISING); // Setup Interrupt // interrupt(5) corresponds to pin 18 raising event for arduino Mega 2560
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;
  //SETUP MP3 AMP
  digitalWrite(49, HIGH);
  delay(2500);
  myMP3.begin(9600);
  delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card  
  delay(200);//wait for 200ms
  //SETUP INTERRUPT POWER UP
  pinMode(18, INPUT);
  digitalWrite(18, LOW);
//  load_all_settings();
  init_close_teaball();
  Serial.println("Arduino ready !!!");  
}

void loop() {
  is_powered();
  if(powered==true){
    getTeaSize();
    getTeaChoice();
  }
  if (TeaSize==0){
    displayMenu();
  }
  else {
    display_volume_choice();
  }
//  monitor_thermoblock(true);

}

void test_interrupt(){
  Serial.println("Interruption !");
  is_powered();
  
//    pwrUp();
//  Serial.println("interrupt");
//  Silo1Motor->setSpeed(0);
//  Silo2Motor->setSpeed(0);
//  Silo3Motor->setSpeed(0);
//  stop_motor(1);
//  stop_motor(2);
//  stop_motor(3);
//  stop_teaball();
//  stop_pump();
}


boolean is_powered(){
  powerButtonState = digitalRead(powerPin);
  if (powerButtonState == HIGH){
    if(powered==false){
      Serial.println("Power up");
      powered = true;
      pwrUp();
      return true;
    }
  }
  else{
    if(powered==true){
      Serial.println("Power down");
      pwrDwn();
      powered = false;
      return false;
    }
  }
}

void pwrUp(){
//  playWithVolume(0X0F04);//play the 9th (09) song with volume 20(0x14) class
  turn_speaker_on();
//  playWithVolume(0X2702);//play the 9th (09) song with volume 20(0x14) class
  //http://cactus.io/resources/toolbox/decimal-binary-octal-hexadecimal-conversion
  turn_buttons_leds_on_style();
  turn_lights_on();
  turn_fireflies_on();
  delay(1200);
//  turn_speaker_off();
  initialize_teararium();
//  load_all_settings();
  wagonStepper.disableOutputs();
  craneStepper.disableOutputs();
  Serial.println("POWERED ON");  
}

void pwrDwn(){
  stop_teaball();
  stop_motor(1);
  stop_motor(2);
  stop_motor(3);
  wagonStepper.disableOutputs();
  craneStepper.disableOutputs();
  stop_pump();
  turn_thermoblock_off();
  close_teaball();
  pull_teaball_up(true);
//  playWithVolume(0X2505);//play the 9th (09) song with volume 20(0x14) class
  delay(500);
  turn_buttons_leds_off();
  turn_lights_off();
  turn_fireflies_off();
  turn_speaker_off();
  Serial.println("POWERED OFF");
}

boolean debouncePowerButton(boolean state)
{
  boolean stateNow = digitalRead(powerPin);
  if(state!=stateNow)
  {
    delay(10);
    stateNow = digitalRead(powerPin);
  }
  return stateNow;
}

void initialize_teararium(){

   //playWithVolume(0X0F09);//play the 9th (09) song with volume 20(0x14) class
  turn_buttons_leds_on();
  stop_pump();
  initialize_steppers();
  initialize_arm();
  initialize_wagon();
  initialize_crane(false);
  TeaSize = 0;
  teaChoice = 0;
  teararium_initialized = true;
  log_info("Teararium initialized !", 1, 0 , 10);
}
