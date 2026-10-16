const int btn1=2;
const int btn2=3;
const int btn3=4;
const int btn4=5;

void checkButton(int pin,char cmd){
  if(digitalRead(pin)==LOW){
    delay(20);
    if(digitalRead(pin)==LOW){
      Serial.write(cmd);
      Serial.write('\n');
      while(digitalRead(pin)==LOW);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(btn1,INPUT_PULLUP);
  pinMode(btn2,INPUT_PULLUP);
  pinMode(btn3,INPUT_PULLUP);
  pinMode(btn4,INPUT_PULLUP);

}
void loop() {
  checkButton(btn1,'1');
  checkButton(btn2,'2');
  checkButton(btn3,'3');
  checkButton(btn4,'4');

}

