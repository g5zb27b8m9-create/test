#include <Servo.h>
#include <math.h>

Servo base, fArm, rArm, claw;

const int baseMin = 0,  baseMax = 180;
const int fArmMin = 30, fArmMax = 180;
const int rArmMin = 30, rArmMax = 180;
const int clawMin = 10, clawMax = 120;

const float L1=80.0;//大臂长
const float L2=80.0;//小臂长
const float H1=55.0;//底座高度
const float UP=30.0;//抬笔高度
const float DOWN=10.0;//落笔高度

int moveDelay = 30;        
String cmd = "";           
int taskIndex = 0;        

struct RecordPoint {
  unsigned int time;       
  byte b_ang;
  byte f_ang;
  byte r_ang;
  byte c_ang;
};
const int MAX_RECORDS = 200;              
RecordPoint recordData[MAX_RECORDS];
int recordCount = 0;
bool isRecording = false;
bool isPlaying = false;

void handleCmd(String cmd);
void servoDataCmd(char serialCmd);
void servoMove(int xAng, int yAng, int zAng);
void servoCmd(char servoName, int toAngle, int servoDelay);
void remoteCmd(char rCmd);
void ABC(char serialCmd);
void runCycle();
void restore();
void reportStatus();
bool joystickControl();
int  qz(float v);
void servoMoveTo(float x,float y,float z);
void drawLine(float x1,float y1,float x2,float y2);

void setup() {
  base.attach(9);  delay(200);
  fArm.attach(8);  delay(200);
  rArm.attach(7);  delay(200);
  claw.attach(6);  delay(200);

  base.write(90);  delay(10);
  fArm.write(90);  delay(10);
  rArm.write(180); delay(10);
  claw.write(28);  delay(10);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(9600);
  Serial.println(F("begin"));
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {          
      if (cmd.length() > 0) {             
        handleCmd(cmd);
        cmd = "";
      }
    } else {
      cmd += c;
      if (cmd.length() > 32) cmd = "";     
    }
  }

  if (!isRecording && !isPlaying) {        // 摇杆控制
    joystickControl();
  }
}

void handleCmd(String Cmd) {
  cmd.trim();// 去掉首尾的空格、回车等空白字符
  if (cmd.length() == 0) return;

  char head = cmd.charAt(0);

  if (head == 'x') {  //x90,y80,x60
    int cmd1=cmd.indexOf(',');
    int cmd2=cmd.indexOf(',',cmd1+1);
    int xAng=cmd.substring(1,cmd1).toInt();
    int yAng=cmd.substring(cmd1+2,cmd2).toInt();//注意+1还是+2
    int zAng=cmd.substring(cmd2+2).toInt();
    servoMove(xAng,yAng,zAng);
  }

  if (head == 'b' || head == 'f' || head == 'r' || head == 'c') {
    String ang = cmd.substring(1);
    ang.trim();
    servoCmd(head, ang.toInt(), moveDelay);
    return;
  }

  if (cmd.length() == 1 && (head == 'A' || head == 'B' || head == 'C')) {  //指令A/B/C
    String stopBuf = "";
    ABC(head);
    return;
  }

  if (head >= '1' && head <= '4' && cmd.length() == 1) {  //指令1~4
    remoteCmd(head);
    return;
  }

  if (cmd.length() == 1 &&
      (head == 'O' || head == 'S' || head == 'H' || head == 'L' || head == 'i')) {  //指令 O/S/H/L/i
    servoDataCmd(head);
    return;
  }

  if(head=='E'){  
    Serial.println(F("画直线"));
    drawLine(3.0,3.0,10.0,3.0);
  }

  if(head=='V'){
    Serial.println(F("画V"));
    drawLine(5.0,5.0,15.0,5.0);
    delay(300);
    drawLine(15.0,5.0,7.0,10.0);
  }

  Serial.print(F("未知指令: "));
  Serial.println(cmd);
}

void servoDataCmd(char serialCmd) {
  switch (serialCmd) {
    case 'O':
      Serial.println(F("Claw Open"));
      claw.write(clawMin);
      break;

    case 'S':
      Serial.println(F("Claw Close"));
      claw.write(clawMax);
      break;

    case 'H':
      moveDelay = max(2, moveDelay - 10);               
      Serial.print(F("Speed Up, moveDelay="));
      Serial.println(moveDelay);
      break;

    case 'L':
      moveDelay = min(50, moveDelay + 10);
      Serial.print(F("Slow Down, moveDelay="));
      Serial.println(moveDelay);
      break;

    case 'i':
      Serial.println(F("舵机状态:"));
      reportStatus();
      break;

    default:
      Serial.println(F("未知指令"));
      break;
  }
}

void servoCmd(char servoName,int toAngle,int moveDelay){//基础指令
  Serial.println(F(" "));
  Serial.print(F("Command:Servo "));
  Serial.print(servoName);
  Serial.print(F("  to  "));
  Serial.print(toAngle);
  Serial.print(F("at moveDelay value "));
  Serial.print(moveDelay);
  Serial.println(F(" "));

  switch(servoName){
    case 'b':
    if(toAngle>=baseMin&&toAngle<=baseMax){
      int fromAngle=base.read();
      if(fromAngle<=toAngle){
        for(int i=fromAngle;i<=toAngle;i++){base.write(i);delay(moveDelay);}
        }else{for(int i=fromAngle;i>=toAngle;i--){base.write(i);delay(moveDelay);}}
    }else{Serial.println(F("Warning:Base Servo Value Out Of Limit!"));
      return;}
      break;
    
    case 'f':
    if(toAngle>=fArmMin&&toAngle<=fArmMax){
      int fromAngle=fArm.read();
      if(fromAngle<=toAngle){
        for(int i=fromAngle;i<=toAngle;i++){fArm.write(i);delay(moveDelay);}
        }else{for(int i=fromAngle;i>=toAngle;i--){fArm.write(i);delay(moveDelay);}}
    }else{Serial.println(F("Warning:fArm Servo Value Out Of Limit!"));
      return;}
      break;
    
    case 'r':
    if(toAngle>=rArmMin&&toAngle<=rArmMax){
      int fromAngle=rArm.read();
      if(fromAngle<=toAngle){
        for(int i=fromAngle;i<=toAngle;i++){rArm.write(i);delay(moveDelay);}
        }else{for(int i=fromAngle;i>=toAngle;i--){rArm.write(i);delay(moveDelay);}}
    }else{Serial.println(F("Warning:rArm Servo Value Out Of Limit!"));
      return;}
      break;
    
    case 'c':
    if(toAngle>=clawMin&&toAngle<=clawMax){
      int fromAngle=claw.read();
      if(fromAngle<=toAngle){
        for(int i=fromAngle;i<=toAngle;i++){claw.write(i);delay(moveDelay);}
        }else{for(int i=fromAngle;i>=toAngle;i--){claw.write(i);delay(moveDelay);}}
    }else{Serial.println(F("Warning:Claw Servo Value Out Of Limit!"));
      return;}
      break;

      default:
      Serial.println(F("未知指令"));
      break;
  }

  }

void servoMove(int xAng,int yAng,int zAng){   //多舵机协同控制
  if(xAng<baseMin||xAng>baseMax||yAng<rArmMin||yAng>rArmMax||zAng<fArmMin||zAng>fArmMax){
    Serial.println(F("Warning:Servo Out Of Limit!"));
    return;
  }
  int xNow=base.read();
  int yNow=rArm.read();
  int zNow=fArm.read();

  int stepMax=max(abs(xAng-xNow),max(abs(yAng-yNow),abs(zAng-zNow)));
  float dx=(float)(xAng-xNow)/stepMax;
  float dy=(float)(yAng-yNow)/stepMax;
  float dz=(float)(zAng-zNow)/stepMax;
  
  for(int i=1;i<=stepMax;i++){
    base.write(xNow + qz(dx * i));         
    rArm.write(yNow + qz(dy * i));        
    fArm.write(zNow + qz(dz * i));
    delay(moveDelay);                      
  }
  base.write(xAng);                         
  rArm.write(yAng);                        
  fArm.write(zAng);
  Serial.println(F("servo move finished"));
}

void remoteCmd(char rCmd) { //按键指令: 循环 / 录制 / 回放 / 回中
  switch (rCmd) {

    case '1':       //循环执行ABC
      runCycle();
      break;

     case '2':  //录制
    if(!isRecording){
      isRecording=true;
      recordCount=0;//清空记录
      Serial.println(F("开始录制"));
      unsigned long startTime=millis();//按下秒表
      while(isRecording&&recordCount<MAX_RECORDS){
        joystickControl();

        if(millis()-startTime>100){
          recordData[recordCount].time=millis()-startTime;//记录时间点
          recordData[recordCount].b_ang=base.read();//记录各个舵机的角度
          recordData[recordCount].f_ang=fArm.read();
          recordData[recordCount].r_ang=rArm.read();
          recordData[recordCount].c_ang=claw.read();
          recordCount++;
          startTime=millis();//重新计时
        }
        if(Serial.available()>0&&Serial.read()=='2'){
          isRecording=false;
          break;                     
        }
      }
      isRecording=false;
      Serial.print(F("Saved num:")); 
      Serial.println(recordCount);
    }break;

    case '3':    //回放
    if(!isRecording&&recordCount>0){
      isPlaying=true;
      Serial.println(F("开始播放"));
      unsigned long playStart=millis();//播放秒表
      unsigned long elapsed=0;
      for(int i=0;i<recordCount;i++){
        elapsed+=recordData[i].time;
        while(millis()-playStart<elapsed){};//还原操控的停顿和速度
        base.write(recordData[i].b_ang);
        fArm.write(recordData[i].f_ang);
        rArm.write(recordData[i].r_ang);
        claw.write(recordData[i].c_ang);
      }
      isPlaying=false;
      Serial.println(F("Record Action Finished"));
    }else{Serial.println(F("No Record Data!"));}
    break;

    case '4':    //回中
      Serial.println(F("还原"));
      restore();
      break;

    default:
      Serial.println(F("未知按键指令"));
      break;
  }
}

bool joystickControl() {
  bool moved = false;

  int a0 = analogRead(A0);
  int a1 = analogRead(A1);
  int a2 = analogRead(A2);
  int a3 = analogRead(A3);

  if (a0 > 800) {
    if (base.read() > baseMin) { base.write(base.read() - 1); moved = true; delay(moveDelay); }
  } else if (a0 < 200) {
    if (base.read() < baseMax) { base.write(base.read() + 1); moved = true; delay(moveDelay); }
  }

  if (a1 < 200) {
    if (fArm.read() > fArmMin) { fArm.write(fArm.read() - 1); moved = true; delay(moveDelay); }
  } else if (a1 > 800) {
    if (fArm.read() < fArmMax) { fArm.write(fArm.read() + 1); moved = true; delay(moveDelay); }
  }

  if (a2 > 800) {
    if (rArm.read() > rArmMin) { rArm.write(rArm.read() - 1); moved = true; delay(moveDelay); }
  } else if (a2 < 200) {
    if (rArm.read() < rArmMax) { rArm.write(rArm.read() + 1); moved = true; delay(moveDelay); }
  }

  if (a3 > 800) {
    if (claw.read() > clawMin) { claw.write(claw.read() - 1); moved = true; delay(moveDelay); }
  } else if (a3 < 200) {
    if (claw.read() < clawMax) { claw.write(claw.read() + 1); moved = true; delay(moveDelay); }
  }

  return moved;
}

void ABC(char serialCmd){
  if(serialCmd=='A'){
    servoMove(90,180,90);//移动到A的上方
    delay(300);
    servoMove(90,85,90);//下降
    delay(300);
    servoMove(90,85,149);//下降
    delay(300);
    claw.write(clawMax);
    delay(300);
    servoMove(90,180,90);//抬起
    delay(300);
    servoMove(30,180,90);//移动到放置地
    delay(300);
    servoMove(30,85,149);//下降
    delay(300);
    claw.write(clawMin);
    delay(300);
    servoMove(30,180,90);//抬起
    delay(300);
    restore();
    delay(300);
  }
  else if(serialCmd=='B'){
    servoMove(140,180,90);//移动到B的上方
    delay(300);
    servoMove(140,108,90);//下降
    delay(300);
    servoMove(140,108,112);//下降
    delay(300);
    claw.write(clawMax);
    delay(300);
    servoMove(140,180,70);//抬起
    delay(300);
    servoMove(60,180,70);//移动到放置地
    delay(300);
    servoMove(60,108,112);//下降
    delay(300);
    claw.write(clawMin);
    delay(300);
    servoMove(60,180,70);//抬起
    delay(300);
    restore();
    delay(300);
  }
  else if(serialCmd=='C'){
    servoMove(60,180,90);//移动到C的上方
    delay(300);
    servoMove(60,122,90);//下降
    delay(300);
    servoMove(60,122,156);//下降
    delay(300);
    claw.write(100);
    delay(300);  
    servoMove(60,180,70);//抬起
    delay(300);
    servoMove(150,180,70);//移动到放置地
    delay(300);
    servoMove(150,122,156);//下降
    delay(300);
    claw.write(clawMin);
    delay(300);
    servoMove(150,180,70) ;//抬起
    delay(300);
    restore();
    delay(300);
  }
}

void runCycle() {  //循环ABC
  if(taskIndex==0){
      ABC('A');
      taskIndex=1;
    }else if(taskIndex==1){
      ABC('B');
      taskIndex=2;
    }else if(taskIndex==2){ABC('C');taskIndex=0;}
    
}

void servoMoveTo(float x,float y,float z){
  float d=sqrt(x*x+y*y);
  float h=z-H1;
  float L3=sqrt(d*d+h*h);
  float theta_base=atan2(x,y);
  float cos_theta1=(L1*L1+L2*L2-L3*L3)/(2*L1*L2);
  float theta1=acos(cos_theta1);
  float theta4=atan2(h,d);
  float cos_theta3=(L1*L1+L3*L3-L2*L2)/(2*L1*L3);
  float theta3=acos(cos_theta3);
  float theta2=(PI/2)+theta3+theta4;
  float base_deg=theta_base*180.0/PI;
  float fArm_deg=theta2*180.0/PI;
  float rArm_deg=theta1*180.0/PI;
  int finalBase=60+base_deg;
  int finalFArm=90+fArm_deg;
  int finalRArm=180-rArm_deg;
  servoMove(finalBase,finalRArm,finalFArm);
}

void drawLine(float x1,float y1,float x2,float y2){
  servoMoveTo(x1,y1,UP);
  servoMoveTo(x1,y1,DOWN);
  int steps=50;//分成五十步
  for(int i=0;i<=steps;i++){
    float curX=x1+(x2-x1)*i/steps;
    float curY=y1+(y2-y1)*i/steps;
    servoMoveTo(curX,curY,DOWN);
  }
  servoMoveTo(x2,y2,UP);
}

int qz(float v) {   //四舍五入取整                     
  return (int)(v + (v >= 0 ? 0.5f : -0.5f));
}

void restore() {  //回中
  Serial.println(F("Restore Initial Position"));
  servoCmd('b', 90,  moveDelay);
  servoCmd('f', 90,  moveDelay);
  servoCmd('r', 180, moveDelay);
  servoCmd('c', 28,  moveDelay);
}

void reportStatus() {  //舵机状态信息
  Serial.println(F(" "));
  Serial.println(F("+ Robot Status Report +"));
  Serial.print(F("base Position: ")); Serial.println(base.read());
  Serial.print(F("fArm Position: ")); Serial.println(fArm.read());
  Serial.print(F("rArm Position: ")); Serial.println(rArm.read());
  Serial.print(F("Claw Position: ")); Serial.println(claw.read());
  Serial.println(F("    +++++++      "));
}