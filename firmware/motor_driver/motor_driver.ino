#include <Wire.h>
#include "variables.h"

enum class DrivingMode{
  Stop=0,Brake=1,Drive=2,ReverseDrive=3,
};

const int INPUT_PINS_D[]={
  10,11,12,9,8,13
};
// num : 0 ~ 5
bool system_input_is_on(int num){
  if(num<0||num>=6){
    return false;
  }
  return digitalRead(INPUT_PINS_D[num]) == HIGH;
}
void system_setup_input(int num){
  if(num<0||num>=6){
    return;
  }
  pinMode(INPUT_PINS_D[num],INPUT_PULLUP);
}
int system_get_input_pin(int num){
  if(num<0||num>=6){
    return -1;
  }
  return INPUT_PINS_D[num];
}

struct MotorLimitPin{
  int input_pin_index;
  LimitType type;
};

class MotorStopper{
  private:
    int limits_length;
    MotorLimitPin* limits;
    bool allowed[4]={};
  public:
    MotorStopper();
    MotorStopper(int motorID);
    bool allow(DrivingMode mode); 
    void update();
};
MotorStopper::MotorStopper(){
  this->limits_length=0;
  this->limits=NULL;
}
MotorStopper::MotorStopper(int motorID){
  this->limits_length=0;
  for(int i=0;i<9;i++){
    if(MotorLimits[i].motorID==motorID && MotorLimits[i].type!=LimitType::None)this->limits_length++;
  }
  this->limits=(MotorLimitPin*)malloc(sizeof(MotorLimitPin)*limits_length);
  int current;
  for(int i=0;i<9;i++){
    if(MotorLimits[i].motorID==motorID && MotorLimits[i].type!=LimitType::None){
      MotorLimitPin pin={i,MotorLimits[i].type};
      this->limits[current++]=pin;
      system_setup_input(i);
    }
  }
}
bool MotorStopper::allow(DrivingMode mode){
  switch(mode){
    case DrivingMode::Stop:
    case DrivingMode::Brake:
      return true;
    case DrivingMode::Drive:
      return this->allowed[(int)LimitType::Drive] && this->allowed[(int)LimitType::All];
    case DrivingMode::ReverseDrive:
      return this->allowed[(int)LimitType::Reverse] && this->allowed[(int)LimitType::All];
  }
  return false;
}
void MotorStopper::update(){
  for(int i=0;i<4;i++){
    this->allowed[i]=true;
  }
  for(int i=0;i<this->limits_length;i++){
    if(system_input_is_on(this->limits[i].input_pin_index)){
      this->allowed[(int)(this->limits[i].type)]=false;
    }
  }
}

class MotorDriver{
  private: 
    int pin_in1,pin_in2;
    DrivingMode current_state;
    MotorStopper stopper;
  public:
    MotorDriver(int pin_in1,int pin_in2,MotorStopper stopper);
    bool checkStopper();
    void updateStopper();
    void changeState(DrivingMode mode);
};

MotorDriver::MotorDriver(int pin_in1,int pin_in2,MotorStopper stopper){
  pinMode(pin_in1,OUTPUT);
  pinMode(pin_in2,OUTPUT);
  this->pin_in1=pin_in1;
  this->pin_in2=pin_in2;
  digitalWrite(pin_in1,LOW);
  digitalWrite(pin_in2,LOW);
  this->current_state=DrivingMode::Stop;
  this->stopper=stopper;
}
void MotorDriver::updateStopper(){
  this->stopper.update();
}
bool MotorDriver::checkStopper(){
  if(!(this->stopper.allow(this->current_state))){
    this->changeState(DrivingMode::Stop);
    return true;
  }
  return false;
}
void MotorDriver::changeState(DrivingMode mode){
  if(!(this->stopper.allow(mode))){
    this->current_state=DrivingMode::Stop;
    digitalWrite(pin_in1,LOW);
    digitalWrite(pin_in2,LOW);
    return;
  }
  
  this->current_state=mode;
  switch(mode){
    case DrivingMode::Stop:
      digitalWrite(pin_in1,LOW);
      digitalWrite(pin_in2,LOW);
      break;
    case DrivingMode::Brake:
      digitalWrite(pin_in1,HIGH);
      digitalWrite(pin_in2,HIGH);
      break;
    case DrivingMode::Drive:
      digitalWrite(pin_in1,LOW);
      digitalWrite(pin_in2,HIGH);
      break;
    case DrivingMode::ReverseDrive:
      digitalWrite(pin_in1,HIGH);
      digitalWrite(pin_in2,LOW);
      break;
  }
}

MotorDriver drivers[]={
  MotorDriver(7,6,MotorStopper(0)),
  MotorDriver(4,5,MotorStopper(1)),
  MotorDriver(2,3,MotorStopper(2)),
  MotorDriver(A0,A1,MotorStopper(3)),
  MotorDriver(A3,A2,MotorStopper(4))
};

void onReceive(int length){
  Serial.println("message receive");
  while(Wire.available()){
    int data=Wire.read();
    Serial.println(data);
    int id=(data & 0b11110000) >> 2;
    if(id>=5){
      Serial.print("Target ID is ");
      Serial.print(id);
      Serial.println(", skipped");
    }
    DrivingMode mode=(DrivingMode)(data & 0b00000011);
    drivers[id].changeState(mode);
    Serial.print("change status of ");
    Serial.print(id);
    Serial.print(" motor to ");
    Serial.println((int)mode);
  }
}
void onRequest(void){
  Serial.println("I2C Request Received :");
  Wire.write(I2C_ADDRESS);
}

void setup() {
  Serial.begin(9600);
  Serial.println("- Scout Motor Driver -");
  Serial.print("I2C Address:0x");
  Serial.println(I2C_ADDRESS,HEX);
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Serial.println("Initialization Finished : Ready..>");
}

long int lastStopperTimeStamp=0;
void loop() {
  long int time = millis();
  long int timePassedFromStopperCheck=lastStopperTimeStamp-time;

  if(timePassedFromStopperCheck>100){
    bool stopperWorking=false;
    lastStopperTimeStamp=time;
    for(int i=0;i<5;i++){
      drivers[i].updateStopper();
      stopperWorking|=drivers[i].checkStopper();
    }
  }
}
