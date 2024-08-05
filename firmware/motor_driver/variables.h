const int I2C_ADDRESS=0x55;

enum class LimitType{
  None,Drive,Reverse,All
};

struct MotorLimit{
  int motorID;
  LimitType type;
};

MotorLimit MotorLimits[]={
  {0,LimitType::None},//1
  {0,LimitType::None},//2
  {1,LimitType::None},//3
  {1,LimitType::None},//4
  {2,LimitType::None},//5
  {2,LimitType::None},//6
};