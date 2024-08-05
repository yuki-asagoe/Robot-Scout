#include <mcp_can.h>
#include <motor.h>
#include <can_communication.h>

#include "variables.h"

enum class Motor_Limitation : int{
  //制限なし
  Free = 0,
  //正転停止
  Forward,
  //逆転停止
  Backward,
  //全停止
  All
};

void on_receive_can(uint16_t, const int8_t*, uint8_t);
void warn(char*);
Motor_Limitation get_motor_limitation_of(int number);

unsigned long long last_can_timestamp=0;
unsigned long long limit_switch_output_timer_1=0;
unsigned long long limit_switch_output_timer_2=0;
signed int motor1_direction=0;
signed int motor2_direction=0;
uint16_t m2_stop_level=0;

void setup(){
  Serial.begin(115200);
  pinMode(STAT_LED1, OUTPUT); // CAN メッセージ受信したら光る
  pinMode(STAT_LED2, OUTPUT); // CAN制御信号が1秒途切れ(てセーフティが発動し)たら光る, セーフティ非動作中はリミットスイッチの入力検知で点滅

  if((int)Limit_Switch_For_M1){
    //INPUT1の1番(基盤内側)のピン M1用リミットスイッチ LOW(GND)入力で平常時、HIGH(VCC)入力で強制モーター停止
    pinMode(INPUT1_1,INPUT_PULLUP);
    if(Limit_Switch_For_M1 == Limit_Switch_Type::OR) pinMode(INPUT1_2,INPUT_PULLUP);
  }
  pinMode(INPUT2_1,INPUT);
  motorInit();

  CanCom.begin(CAN_Self_Address, CAN_Speed);
  CanCom.onReceive(on_receive_can);

  digitalWrite(STAT_LED1,HIGH);
  digitalWrite(STAT_LED2,HIGH);
  Serial.println("Ready to Drive - motor driver on Caucasus");
  Serial.print("CAN destination ID is\"");
  Serial.print(CAN_Self_Address,DEC);
  Serial.println("\".");
  if((int)Limit_Switch_For_M1){
    Serial.print("This board is accepting a limit switch input for M1 with INPUT1_1");
    if(Limit_Switch_For_M1 == Limit_Switch_Type::OR) Serial.println(" pin and INPUT1_2 pin.");
    else Serial.println(".");
  }
  Serial.println("Git Repository - https://github.com/yuki-asagoe/Robot-Caucasus");
  delay(100);
}

void loop(){
  //データ受信を確認し必要ならonReceiveで登録したリスナを呼び出す
  CanCom.tasks();
  unsigned long long now=millis();
  Motor_Limitation motor1_limit=get_motor_limitation_of(1);
  if(now - last_can_timestamp > 1000){ //セーフティストッパー
    motorStop(1);
    motorStop(2);
    digitalWrite(STAT_LED2,HIGH);
  }else{
    if((int)motor1_limit){//リミットスイッチ検知
      if((now/200) & 1 == 1){//200ms間隔
        digitalWrite(STAT_LED2,HIGH);
      }else{
        digitalWrite(STAT_LED2,LOW);
      }
    }else{
      digitalWrite(STAT_LED2,LOW);
    }
  }
  if(now -last_can_timestamp < 100){
    digitalWrite(STAT_LED1,HIGH);
  }else{
    digitalWrite(STAT_LED1,LOW);
  }
  if((int)motor1_limit){
    ///300秒間隔でリミットスイッチ押下のフィードバック
    if(now - limit_switch_output_timer_1>100){
      Serial.println("M1: Reaching Limit");
      limit_switch_output_timer_1=now;
    }
    if(motor1_limit==Motor_Limitation::All){
      motorStop(1);
    }else if(motor1_direction > 0 && motor1_limit==Motor_Limitation::Forward){
      motorStop(1);
    }else if(motor1_direction < 0 && motor1_limit==Motor_Limitation::Backward){
      motorStop(1);
    }
  }
  if(now - limit_switch_output_timer_2>100){
    limit_switch_output_timer_1=now;
    uint16_t a_in=analogRead(INPUT2_1);
    if(motor2_direction>0 && m2_stop_level<=a_in){
      motorStop(2);
    }else if(motor2_direction < 0 && m2_stop_level>=a_in){
      motorStop(2);
    }
  }
}

void on_receive_can(uint16_t std_id, const int8_t *data, uint8_t len) {
  last_can_timestamp=millis();
  uint8_t msg_type = CanCommunication::getDataTypeFromStdId(std_id);
  uint8_t dest = CanCommunication::getDestFromStdId(std_id);  
  
  if(Detailed_Logging_Enabled){
    Serial.println("Received Data");
    Serial.print("ID-dest:");
    Serial.print(dest,HEX);
    Serial.print(" / ID-msg-type:");
    Serial.print(msg_type,HEX);
    Serial.print(" / Length:");
    Serial.print(len,HEX);
    Serial.print(" / Data:");
    for(int i=0;i<len;i++){
      Serial.print(data[i],HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
  
  Motor_Limitation motor1_limit=get_motor_limitation_of(1);

  switch(msg_type){
    case CAN_DATA_TYPE_COMMAND:{
      if(len!=5){
        warn("illegal data length");
        return;
      }
      //M1のパース
      else if(data[0]==0){
        Serial.println("M1:Stopping");
        motorStop(1);
        motor1_direction=0;
      }
      else if(data[0]==-128){
        Serial.println("M1:Free");
        motorFree(1);
        motor1_direction=0;
      }
      else if(data[0]>0){
        if(motor1_limit==Motor_Limitation::Forward || motor1_limit==Motor_Limitation::All){
          Serial.println("M1:Stopping - Reaching Limit(+)");
          motorStop(1);
        }else{
          Serial.print("M1:Driving / ");
          Serial.print((uint8_t)data[1],DEC);
          Serial.println("");        
          motorWrite(1,(uint8_t)data[1]);
          motor1_direction=1;
        }
      }
      else {
        if(motor1_limit==Motor_Limitation::Backward || motor1_limit==Motor_Limitation::All){
          Serial.println("M1:Stopping - Reaching Limit(-)");
          motorStop(1);
        }else{
          Serial.print("M1:Driving / -");
          Serial.print((uint8_t)data[1],DEC);
          Serial.println("");        
          motorWrite(1,-(uint8_t)data[1]);
          motor1_direction=-1;
        }
      }

      //M2のパース
      m2_stop_level=(((uint16_t)data[3])<<8)|(data[4]);
      if(data[2]==0){
        Serial.println("M2:Stopping");
        motorStop(2);
        motor2_direction=0;
      }
      else if(data[2]>0){
        motor2_direction=1;
        Serial.print("M2:Driving / ");
        Serial.print((uint8_t)data[1],DEC);
        Serial.println("");       
        motorWrite(2,255);
      }
      else {
        motor2_direction=-1;
        Serial.print("M2:Driving / -");
        Serial.print((uint8_t)data[1],DEC);
        Serial.println("");       
        motorWrite(2,-255);
      }
      break;      
    }
    case CAN_DATA_TYPE_EMERGENCY:{
      Serial.println("Emergency Code Detected : All Motors are stopping");
      motorStop(1);
      motorStop(2);
      motor1_direction=0;
      motor2_direction=0;
      break;
    }
    Serial.println("");
  }
}

void warn(char* msg){
  Serial.print("Warning: ");
  Serial.print(msg);
  Serial.print(" -by motor drive(id:");
  Serial.print(CAN_Self_Address,DEC);
  Serial.println(")");
}

Motor_Limitation get_motor_limitation_of(int number){
  switch(number){
    case 1:
      if(Limit_Switch_For_M1 == Limit_Switch_Type::OR){ 

        bool input1=digitalRead(INPUT1_1) == HIGH;
        bool input2=digitalRead(INPUT1_2) == HIGH;

        if(input1){
          if(input2){
            return Motor_Limitation::All;
          }else{
            return Motor_Limitation::Forward;
          }
        }else if(input2){
          return Motor_Limitation::Backward;
        }else{
          return Motor_Limitation::Free;
        }

      }else if(Limit_Switch_For_M1 == Limit_Switch_Type::Single){ 
        return digitalRead(INPUT1_1) == HIGH ? Motor_Limitation::Forward : Motor_Limitation::Free;
      }else{ 
        return Motor_Limitation::Free;
      }
  }
  warn("Illegal Motor Number in func get_motor_limitation_of");
  return Motor_Limitation::Free;
}