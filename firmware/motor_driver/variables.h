const int I2C_ADDRESS=55;

enum class LimitType{
  None,Drive,Reverse,All
};

struct MotorLimit{
  int motorID;
  LimitType type;
};

MotorLimit MotorLimits[]={
  {0,LimitType::Drive},//1
  {0,LimitType::Reverse},//2
  {1,LimitType::Drive},//3
  {1,LimitType::Reverse},//4
  {2,LimitType::Drive},//5
  {2,LimitType::Reverse},//6
};