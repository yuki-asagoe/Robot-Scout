#include <Wire.h>
#include "variables.h"

enum class DrivingMode{
  Stop=0,Brake=1,Drive=2,ReverseDrive=3,
};

class MotorDriver{
  private: 
    int pin_in1,pin_in2;
  public:
    MotorDriver(int pin_in1,int pin_in2);
    void changeState(DrivingMode mode);
};

MotorDriver::MotorDriver(int pin_in1,int pin_in2){
  pinMode(pin_in1,OUTPUT);
  pinMode(pin_in2,OUTPUT);
  this->pin_in1=pin_in1;
  this->pin_in2=pin_in2;
}
void MotorDriver::changeState(DrivingMode mode){
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
  MotorDriver(7,6),
  MotorDriver(4,5),
  MotorDriver(2,3),
  MotorDriver(A0,A1),
  MotorDriver(A3,A2)
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
    DrivingMode mode=(DrivingMode)(data & 0b00000011)
    drivers[id].changeState(mode);
    Serial.print("change status of ");
    Serial.print(id);
    Serial.print(" motor to ");
    Serial.println((int)mode);
  }
}
void onRequest(void){
  Serial.println("message receive");
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

void loop() {
}
