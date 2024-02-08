#include <GyverPower.h>
#include <VarSpeedServo.h>
#include <math.h>
#include <EEPROM.h>

int CD = 0; //Центральный фоторезистор
int LD = 0; //Левый фоторезистор
int RD = 0; //Правый фоторезистор
int CP = 0; //Рассчитанный нижний центральный фоторезистор
int RDA = 0; //Упрощённый правый фоторезистор
int LDA = 0; //Упрощённый левый фоторезистор

double Angle = 90;
double Langle = EEPROM.read(0); //Значение старого угла
double A = 0; //Верхний угол
double B = 0; //Нижний угол
byte Panel = 81; //Расстояние между верхней и нижней линией фоторезисторов

int Q = 0; //Разрешение итоговых данных
float Mult = 0.35; //Множитель качества итоговых данных
int Add = 5; //Добавление итоговых данных

boolean HAE = false; //(Horizontal Angle Error) нахождение желаемого угла за пределами допустимого
int VAE = 0; //(Vertical Angle Error) колличество попыток корректировки угла за пределами допустимого
int VAEe = 2000; //(end) значение предела попыток
boolean Dir = false; //Направление мотора

VarSpeedServo myServo10; //Сервопривод, 10 пин

//void yield (){
//  Serial.println((String(CD)) + " " + (String(LD)) + " " + (String(RD)) + " " + (String(A)) + " " + (String(B)) + " " + (String(Langle)) + " " + (String(Q)));
}

void RunL() { //Поворот налево относительно верха
  digitalWrite(2, 1);
  digitalWrite(3, 0);
}

void RunR() { //Поворот направо относительно верха
  digitalWrite(2, 0);
  digitalWrite(3, 1);
}

void Stop() { //Остановка
  digitalWrite(2, 0);
  digitalWrite(3, 0);
}

void CorrectH() { //Корректировка наклона
  A = degrees(acos(constrain(((square(CD) + square(Panel) - square(CP)) / (2 * CD * Panel)),-1,1))); //Верхний угол
  B = degrees(acos(constrain(((square(CP) + square(Panel) - square(CD)) / (2 * CP * Panel)),-1,1))); //Нижний угол 
  Angle = abs(A - B);
  if (A > B) { Angle = -Angle; }
  Langle = int(constrain((Langle + Angle),35,90));
  if (((Langle + Angle) > 90) or ((Langle + Angle) < 35)) { HAE = true; } else { HAE = false; }
  myServo10.write(Langle,10, false);
  EEPROM.write(0, byte(Langle));
}

void CorrectV() { //Корректировка поворота
  if (RDA > LDA) {
    if (!Dir) { Stop(); VAE = 0; Dir = true; delay(50); }
    RunL();
    VAE = VAE++;
    }
  else if (RDA < LDA) {
    if (Dir) { Stop(); VAE = 0; Dir = false; delay(50); }
    RunR();
    VAE = VAE++;
    }
  VAE = VAE++;
}

void setup() {
  myServo10.attach(10);
  myServo10.write(Langle,10, true);
  //Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(2, 0);
  digitalWrite(3, 0);
  myServo10.attach(10);
  power.hardwareDisable(PWR_I2C | PWR_SPI);
  power.setSleepMode(EXTSTANDBY_SLEEP);

}

void loop() {
  Q = map(min(min(analogRead(A2), analogRead(A1)), analogRead(A0)),0,1023,511,0)*Mult + Add;
  CD = map((analogRead(A2)), 0, 1023, 1, Q);
  LD = map(analogRead(A1), 0, 1023, 1, Q);
  RD = map(analogRead(A0), 0, 1023, 1, Q);
  CP = int(((RD + LD) / 2));

  RDA = map(RD, 1, Q, 1, Q/3.5);
  LDA = map(LD, 1, Q, 1, Q/3.5)*1.3;

  if (((RD + LD) / 2) != CD) {
    CorrectH();
}
  if (RDA != LDA) {
    CorrectV();
} else { Stop(); }

  if ((((RDA == LDA) or (VAE >= VAEe)) and ((((RD + LD) / 2) == CD) or HAE)) or ((max(RD,max(LD,CD))) <= 5)) {
   digitalWrite(13, 1);
   Stop();
   power.sleepDelay(SLEEP_64MS);
  } else { digitalWrite(13, 0); delay(25); }

}
