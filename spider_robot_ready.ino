//    ***PINS***
#define DEFINE_COMMAND_0        2
#define DEFINE_COMMAND_1        3
// - read Command(0 or 1)

#define SERVOMIN  100 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  450 // This is the 'maximum' pulse length count (out of 4096)

//    ***DEFAULT ANGLES***
int DEFAULT_OFFSET[6][3]    ={{275,230,275},{275,230,275},{275,220,275},{275,210,275},{275,200,275},{275,210,275}}; //определение начального положения сервоприводов - для настройки
int   LEG_PINS[6][3]      ={{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 1, 2}, {3, 4, 5}, {6, 7, 8}};   //определение пинов на шилде PCA9685, address 0x40 and 0x41
float IS_TURN_UP[6][3]    ={{1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {-1, 1, 1}, {-1, 1, 1}, {-1, 1, 1}};    //does positive angle turn the leg up? (1 or -1) - для настройки

/* -------- Includes -------------------------------------------------------*/
#include <Wire.h>                       //to use address of I2C port
#include <Adafruit_PWMServoDriver.h>    //to use PCA9685

bool is_Stay;
Adafruit_PWMServoDriver pwm_right = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm_left  = Adafruit_PWMServoDriver(0x41);

//класс для определения одной ноги
class ONE_LEG{
public:
  Adafruit_PWMServoDriver pwm_side; //right or left
  int default_offset[3];
  int current_offset[3];            //keep position
  int is_turn_Up[3];                //positive derection is up
  int LEG_PINS_1[3];
  
    //  *functions*
  void set_offset(int num_of_servo, int offset);  //change on offset
};

void ONE_LEG::set_offset(int num_of_servo, int offset){         //change the offset
  pwm_side.setPWM(LEG_PINS_1[num_of_servo], 0 , default_offset[num_of_servo]+is_turn_Up[num_of_servo]*offset*(SERVOMAX-SERVOMIN)/180);
  current_offset[num_of_servo] = offset;
}


//      ***initialization***
ONE_LEG leg[6];

//      ***Commands***
#define STAY        0
#define SIT         1
#define GO_FORWARD  2
#define GO_BACK     3

//      for debug
//-----SIT----------
#define SIT_OFFSET_POSITION_OF_0  0
#define SIT_OFFSET_POSITION_OF_1  50
#define SIT_OFFSET_POSITION_OF_2  50

// переменная для работы bluetooth
char incomingByte;  // входящие данные

/*
            - setup function
   ------(выполняется только в начале)--------*/
void setup()
{
  //инициализация порта - для проверки, настройки
  Serial.begin(9600);
  Serial.println("Setup begin!");
  
  //инициализация и регулировка I2C-порта
  pwm_right.begin();
  pwm_right.setOscillatorFrequency(27000000);
  pwm_right.setPWMFreq(50);
  pwm_left.begin();
  pwm_left.setOscillatorFrequency(27000000);
  pwm_left.setPWMFreq(50);
  
  //инициализация каждой ноги
  for(int i = 0; i<6;i++){
    if (i<3)  leg[i].pwm_side = pwm_right;
    else      leg[i].pwm_side = pwm_left;
    
    for(int j = 0; j<3; j++){
      leg[i].LEG_PINS_1[j] = LEG_PINS[i][j];
      leg[i].default_offset[j]     = DEFAULT_OFFSET[i][j];  //установка начального положения ноги
      leg[i].is_turn_Up[j]         = IS_TURN_UP[i][j];    //YES(1) or NO(-1)
      leg[i].set_offset(j,0);
    }
  }
  
  is_Stay = true;
}


/*
  - loop function
   ---(выполняется в цикле/после окончания заново)-----*/
void loop()
{
  
  //определение входящей команды
  int command = defineCommand();
  
  //печать команды на экран ноутбука - для проверки, настройки
  Serial.println(command);
  
  //движение ноги - в зависимости от входящей команды
  switch(command){
    case (STAY):  // 0
    {
      stay();
      break;
    }
    case (SIT): // 1
    {
      sit();
      break;
    }
    case (GO_FORWARD):  // 2
    {
      go_forward();
      break;
    }
    case (GO_BACK): // 3
    {
      go_back();
      break;
    }
    default:
      stay();
  }
  delay(500);
}

//функция для определения текущей команды. Выходные значения функции приведены в конце кода
int defineCommand(){
  //считываем актуальную команду
  if (Serial.available() > 0)   //если пришли данные
  {  
    incomingByte = Serial.read(); // считываем байт
    if(incomingByte == '0') 
    {
      Serial.println("Spider is standing.");  // и выводим обратно сообщение
      return 0;
    }
    if(incomingByte == '1') 
    {
      Serial.println("Spider is sitting.");
      return 1;
    }
    if(incomingByte == '2') 
    {
      incomingByte = 0;
      Serial.println("Spider is going forward.");
      return 2;
    }
    if(incomingByte == '3') 
    {
      return 3;
      Serial.println("Spider is going backward.");
    }
  }  
}
/*
  Выходные значения функции
  Выход   Что_значит
    0    STAY
    1    SIT
    2    GO_FORWARD
    3    GO_BACK 
*/

//функция чтобы сидеть
void sit(){
  for(int i = 0; i<6; i++){
    leg[i].set_offset(0,0);
    leg[i].set_offset(1,0);
    leg[i].set_offset(2,30);
    is_Stay = true;
  }
  leg[2].set_offset(1,80);
 // leg[3].set_offset(1,50);
  is_Stay = false;
  
}

//функция чтобы стоять
void stay(){
  for(int i = 0; i<6; i++){
    leg[i].set_offset(0,0);
    leg[i].set_offset(1,0);
    leg[i].set_offset(2,0);
    is_Stay = true;
  }
}

//функция чтобы идти вперед
int go_forward(){
  if(!is_Stay){     //Stand up if robot does not stay
    stay();
    delay(2000);
  }
  
    leg[1].set_offset(1,50);
    delay(500);
    leg[1].set_offset(0,20);
    delay(500);
    leg[1].set_offset(1,-30);
    delay(500);
    leg[4].set_offset(1,70);
    delay(500);
    leg[4].set_offset(0,20);
    delay(500);
    leg[4].set_offset(1,-30);
    delay(500);
    Serial.println("1 - 5 ready");
  delay(1000);
    leg[0].set_offset(1,50);
    delay(500);
    leg[0].set_offset(0,20);
    delay(500);
    leg[0].set_offset(1,0);
    delay(500);
    leg[5].set_offset(1,50);
    delay(500);
    leg[5].set_offset(0,20);
    delay(500);
    leg[5].set_offset(1,0);
    delay(500);
    Serial.println("0 - 5 ready");
  delay(1000);

  leg[0].set_offset(0,0);
  leg[1].set_offset(0,-30);
  leg[2].set_offset(0,-30);
  leg[3].set_offset(0,-30);
  leg[4].set_offset(0,-30);
  leg[5].set_offset(0,0);
  delay(500);
  leg[1].set_offset(1,-30);
  leg[4].set_offset(1,-30);
  delay(500);

    leg[2].set_offset(1,50);
    delay(500);
    leg[2].set_offset(0,0);
    delay(500);
    leg[2].set_offset(1,0);
    delay(500);
    leg[3].set_offset(1,70);
    delay(500);
    leg[3].set_offset(0,0);
    delay(500);
    leg[3].set_offset(1,0);
    delay(500);

    leg[1].set_offset(1,50);
    delay(500);
    leg[1].set_offset(0,0);
    delay(500);
    leg[1].set_offset(1,0);
    delay(500);
    leg[4].set_offset(1,50);
    delay(500);
    leg[4].set_offset(0,0);
    delay(500);
    leg[4].set_offset(1,0);
    delay(500);
  delay(1000);
  Serial.println("Finished");
  
    
}

//функция чтобы идти назад
void go_back(){
  if(!is_Stay){     //Stand up if robot does not stay
    stay();
    delay(500);
  }

    leg[1].set_offset(1,50);
    leg[3].set_offset(1,50);
    leg[5].set_offset(1,50);
  
}
