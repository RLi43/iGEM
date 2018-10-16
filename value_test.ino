//show light intensity

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//hardware
#define PIN_LED_OD A1
#define PIN_LED_FL A0
#define PIN_SEN_LIGHT_OD 0
#define PIN_SEN_LIGHT_FL 1
#define PIN_SEN_TEMP A2//Analog
#define PIN_BTN_GOON 11

#define REACTION_TIMER 1800 //half hour
#define MEASURE_TIMES 8

unsigned int timer_cnt = 0;
unsigned long timer_oldMilis = 0;
void time_display(unsigned int seconds){
    unsigned short mins = seconds/60;
    unsigned short sec = seconds-mins*60;  
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(10,10); 
    display.print(mins);                
    display.print(":");
    display.print(sec);   
    display.display();   
}

bool btn_old_stat = false;
bool goon(){
  bool now_stat = digitalRead(PIN_BTN_GOON);
  if(now_stat!=btn_old_stat){
    btn_old_stat = now_stat;
    return true;
  }
  return false;
}

unsigned long oldMilis = 0;
volatile unsigned long cnt = 0;
void interrupt()
{
  cnt++;
}

unsigned long _getLightIntenFreq(int Pin)
//num 0 for a sensor, 1 for another
//1000ms - 1Hz
{
  int num = PIN_SEN_LIGHT_OD ;
  if(Pin == PIN_LED_FL)num = PIN_SEN_LIGHT_FL;
  attachInterrupt(num+2,interrupt,RISING);
  delay(100);
  oldMilis = micros();
  cnt = 0;
  while(micros()-oldMilis<100000){}
  unsigned long detect_cnt = cnt;
  return detect_cnt;// it needs to be calculate
}
unsigned long getLightIntenFreq(int int_Pin,int Inten){
  analogWrite(int_Pin,Inten);
  delay(10);//actually, there may be no need of this delay
  unsigned long sum = 0;
  for(int i = 0; i < MEASURE_TIMES; i++){
    sum += _getLightIntenFreq(int_Pin);
  }
  //unsigned long lightInten=getLightIntenFreq(int_Pin);
  digitalWrite(int_Pin,LOW);
  return sum / MEASURE_TIMES;
}
float getTemp(int anv=8) // defaut sample times is 8
{
  float sum = 0;
  for(int i = 0; i < anv; i++){
    sum+=analogRead(PIN_SEN_TEMP);
    delay(100);// this can change
  }
  return sum*500.0/anv/1024;
}

void setup(){
  pinMode(PIN_LED_FL,OUTPUT);
  pinMode(PIN_LED_OD,OUTPUT);
  pinMode(PIN_SEN_TEMP,INPUT);
  pinMode(PIN_SEN_LIGHT_FL,INPUT);
  pinMode(PIN_SEN_LIGHT_OD,INPUT);
  pinMode(PIN_BTN_GOON,INPUT_PULLUP);

  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)  
  //show logo

    display.clearDisplay();
    display.setCursor(0,10); 
    display.setTextSize(2);             //设置字体大小
    display.setTextColor(WHITE);        //设置字体白色
    display.println("Tsinghua-A");                //输出字符
    display.println("Welcome!");                //输出字符
    display.display();
    delay(2000);

  btn_old_stat = digitalRead(PIN_BTN_GOON);
  timer_cnt = REACTION_TIMER;
  timer_oldMilis = millis();
}
int state = 0;
void loop(){
    if(goon())state++;
    state = state % 4;
    display.setTextSize(2);    
    display.clearDisplay();
    switch (state)
    {
        case 0:{
            display.setCursor(0,10); 
            display.println("OD:");
            unsigned long od = getLightIntenFreq(PIN_LED_OD,300);
            Serial.println(od);
            display.println(od);                //输出字符            
            break;}
        case 1:{
            display.setCursor(0,10); 
            display.println("FL:");
            display.setCursor(0,30); 
            unsigned long fl = getLightIntenFreq(PIN_LED_FL,800);
            Serial.println(fl);
            display.println(fl);                //输出字符            
            break;}
        case 2:{
            display.setCursor(0,10);
            display.println("Black");            
            unsigned long bl = _getLightIntenFreq(PIN_LED_OD);
            display.print(bl);
            display.print("-");
            delay(1000);
            bl = _getLightIntenFreq(PIN_LED_FL);
            display.println(bl);
            break;}
        case 3:{
            display.setCursor(0,10); 
            display.print("TP:  ");
            display.println(getTemp());                //输出字符            
            delay(2000);
            display.setCursor(0,10); 
            display.println("Time:");
            if(timer_cnt == 0)timer_cnt == REACTION_TIMER;
            unsigned long now_mili = millis();
            timer_cnt-=(now_mili - timer_oldMilis)/1000;
            time_display(timer_cnt);
            if(now_mili - timer_oldMilis > 1000){
                timer_oldMilis = now_mili;
            }
            break;}
        default:
            break;
    }
    display.display();
    delay(1000);
    display.invertDisplay(true);
    delay(200);
    display.invertDisplay(false);
}

