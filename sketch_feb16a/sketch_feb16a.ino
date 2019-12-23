#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Password.h>
LiquidCrystal_I2C lcd(0x27,16,2);
Servo myservo;
int led = 5;
int solenoid = A0;
int pb = 2;
int position=0;
int count=0;
int hallsensor=3; 
int buzzer=A1;
int PB,Hall;
int pushb = A2;
char data=0;
Password passwd = Password("54321");
char passwd2[6]="";
const byte rows=4;
const byte cols=4;
char keys[rows][cols]={
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte rowPins[rows]={6,7,8,9};
byte colPins[cols]={10,11,12,13};
int i,flag;
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,rows,cols);

void hall()
{
  Hall=digitalRead(hallsensor);
  if(Hall==0)
{
  while(Hall==1);
  digitalWrite(led,HIGH);
}
  else if(Hall==1)
{
  while(Hall==0);
  digitalWrite(led,LOW);
}
}

void pushbutton()
{
PB=digitalRead(pb);
if(PB==1)
  {
  }
}
void setup()
{
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(hallsensor),hall,CHANGE);
  attachInterrupt(digitalPinToInterrupt(pb),pushbutton,CHANGE);
  pinMode(hallsensor,INPUT);
  pinMode(led,OUTPUT);
  pinMode(solenoid,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(pushb,INPUT);
  pinMode(pb,INPUT); 
  myservo.attach(4);
  digitalWrite(solenoid,HIGH);
  digitalWrite(buzzer,LOW);
  myservo.write(87);
  lcd.setCursor(0,0);
  lcd.print("Face Unlock");
  lcd.setCursor(0,1);
  lcd.print("Zarni Tun");
  delay(2000);
  lcd.clear();
}

void loop()
{
char key;
flag=1;
position=0;
key=0;
  lcd.setCursor(0,0);
  lcd.print("Enter password   ");
  key = keypad.getKey();
   if (key != NO_KEY){
      delay(60); 
      switch (key){
      case 'C': changepasswd(); break; 
      case '*': checkpasswd(); break;
      case '#': passwd.reset(); break;
      case 'A': activate(); break;
     default: processNumberKey(key);
      }
   }
}

void processNumberKey(char key) {
for(i=6;i<11;i++)
{
flag=1;
    while((key=keypad.getKey())==0);
   Serial.print('*');
   lcd.setCursor(i,1);
   lcd.print('*');
   passwd.append(key);
   } 
 }

void activate()
{
  if (passwd.evaluate()){ 
   }  
    else{
      Serial.println("New Password Activated");
      lcd.setCursor(0,1);
      lcd.print("Password Activate"); 
      delay(500); 
      lcd.setCursor(0,1);
      lcd.print("                 "); 
   } 
   passwd.reset();
}

void checkpasswd() {
   if (passwd.evaluate()){
      Serial.println(" OK.");
      lcd.setCursor(13,1);
      lcd.print(" OK.");
      delay(500);
      flag=1;
   } else {
      Serial.println(" Wrong password!");
      lcd.setCursor(0,1);
      lcd.print("         ");
      lcd.setCursor(0,1);
      lcd.print("Wrong Password");
      for(i=0;i<5;i++)
{
  flag=1;
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(200);
  flag=0;
}}
//      delay(500);
//      flag=0;
//   } 
   passwd.reset();

if(flag==1)
{
  lcd.setCursor(0,1);
  lcd.print("                 ");
  lcd.setCursor(0,0);
  lcd.print("                 "); 
  while(1)
  {
  int push = digitalRead(pushb);   
if(push == 1)
{
  lcd.setCursor(0,1);
  lcd.print("                ");
  myservo.write(87);
  break;
}
for(i=0;i<1;i++)
{
if(Hall==0 && flag==1 || PB==1)
  {
  lcd.setCursor(0,1);
  lcd.print("Door UnLock        ");
  digitalWrite(solenoid,LOW);
  for(i=0;i<1;i++)
{
  digitalWrite(buzzer,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  delay(50);
}
  delay(1000);
  myservo.write(10);
  delay(5000);
  myservo.write(87);
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("Door Locked        ");
  digitalWrite(solenoid,HIGH);
  for(i=0;i<1;i++)
{
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(200);
}
  delay(2000);
  flag=0;
  }}
if (Hall==1)
{
  position=0;
  char key=0;
}

    if(Serial.available() > 0) 
  {
    char received = Serial.read();
    Serial.println(received);
      
   if(received == 'c') // HUMAN FACE ON front
 {
    lcd.setCursor(0,0);
    lcd.print("Face Detected    ");
    lcd.setCursor(0,1);
    lcd.print("                 ");
    lcd.setCursor(0,1);
    lcd.print("Door Unlock      ");
  digitalWrite(solenoid,LOW);
   for(i=0;i<1;i++)
{
  digitalWrite(buzzer,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  delay(50);
}
  delay(1000);
    myservo.write(10);
    delay(5000);
    myservo.write(87);
    delay(1000);
    lcd.setCursor(0,1);
  lcd.print("Door Locked        ");
    digitalWrite(solenoid,HIGH);
      for(i=0;i<1;i++)
{
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(200);
}
}
  if(received == 'd') // human not in front
 {
    lcd.setCursor(0,0);
    lcd.print("                 ");
    lcd.setCursor(0,1);
    lcd.print("                 ");
    }
 }  }}
 else
{
char key=0;
  lcd.setCursor(0,1);
  lcd.print("                ");
}
}
 
 void changepasswd(){
      Serial.println("Password Changed to");
        lcd.setCursor(6,1);
  lcd.print("         ");
  lcd.setCursor(0,0);
  lcd.print("                 ");
      lcd.setCursor(0,0);
      lcd.print("Change Password  ");
    for(byte i=0;i<5;i++){
        char key = NO_KEY;
        while(key == NO_KEY || key=='*' || key=='#' || key=='D') 
        key = keypad.getKey();
        passwd2[i] = key;
        Serial.print(key);
        lcd.setCursor(i,1);
        lcd.print(key);
        delay(60);
    }
    passwd2[5] = '\0'; //ensure correct string ending
    passwd.set(passwd2);
}
