#include <OLED_I2C.h>
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include <SimpleDHT.h>


//codigo biblioteca oled - oled i2c
OLED  myOLED(SDA, SCL, 8);
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

//codigo biblioteca relogio -RTClib
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//codigo biblioteca simple dht
int pinDHT11 = 9;
SimpleDHT11 dht11(pinDHT11);
byte temperature = 0;
byte humidity = 0;

//pinos e variaveis
//botao mudar a tela
int bTelapin = 7;
int bTelaEstado = 1;
int indexTela = 0;
int ntelas = 5;

//variaveis relay
int pinRel1 = 8;//pino solenoide
int pinRel2 = 4;

//variaveis estado relay desligado
int rel1state = 1; //estado solenoide
int rel2state = 1;

//variaveis para hora de ligar e desligar a iirrigação
int horaLiga = 14;
int minutoLiga = 37;

int minutoDesligado = minutoLiga + 1;
int tempoLigado = 50;//segundos

//variavel para corrigir o comportamento do botão de irrigação manual
int desperta = 0;


//botao 2
int botSolpin = 12;
int botSolState = 0;

//botao 3 para acionar os relays manualmente
int botRel2Pin = 5;
int botRel2State = 0;

//leitura e pino analógico LDR
int ldrRead = 0;
int ldrPin = 0;

//leitura e pino analógico solo
int soloRead = 0;
int soloPin = 1;

String unidade = "Sesc Registro";



void setup () {
  Serial.begin(57600);
  myOLED.begin();
  myOLED.setFont(SmallFont);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


  //define pino botão tela
  pinMode(bTelapin, INPUT);

  //define pinos relays
  pinMode(pinRel1, OUTPUT);
  pinMode(pinRel2, OUTPUT);

  //define pinos dos botões para acionar relays 1 e 2
  pinMode (botSolpin, INPUT);   //botão relay solenoide
  pinMode (botRel2Pin, INPUT);  //botão relay fita led



}

void loop () {
  //inicia RTC
  DateTime now = rtc.now();

  //DHT a cada 2 segundos
  if (now.second() % 2 == 0) {
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err=");
      Serial.print(SimpleDHTErrCode(err));
      Serial.print(","); Serial.println(SimpleDHTErrDuration(err));
      //delay(1000);
      return;
    }

    //le LDR e solo a cada 2 segundos tbm
    ldrRead = analogRead(ldrPin);
    //Serial.println(ldrRead);

    //Lê sensor de solo
    soloRead = analogRead(soloPin);
    //Serial.println(soloRead);
  }

  //logica botão tela
  bTelaEstado = digitalRead(bTelapin);

  //le estado do pino do botão e testa se foi apertado ou não
  if (bTelaEstado == 1) {
    delay(300);
    if (indexTela < ntelas) {
      //testa ses a variavel indextela é menor que a quantidade de telas e se for maior que 3 ela zera
      indexTela = indexTela + 1;

    } else {
      indexTela = 0;
    }

  }
  //fim logica botão tela

  //botão relay 1 solenoide
  botSolState = digitalRead(botSolpin);

  //liga nahora
  if (now.hour() == horaLiga && now.minute() == minutoLiga && now.minute() < minutoDesligado) {
    desperta = 1;
    rel1state = 0;
    //digitalWrite(pinRel1, rel1state);
    //Serial.println("Ligado");
    delay(20);
  }
  //desliga na hora
  if (now.hour() == horaLiga && now.minute() >= minutoDesligado && desperta == 1) {
    rel1state = 1;
    //digitalWrite(pinRel1, rel1state);
    // Serial.println("desligado");
    desperta = 0;
    delay(20);
  }

  //timer desligado e botão apertado
  if (desperta == 0 && botSolState == 1) {
    rel1state = 0;
    //digitalWrite(pinRel1, rel1state);
    //ligo
  }

  if (desperta == 0 && botSolState == 0) {
    rel1state = 1;
    //digitalWrite(pinRel1, rel1state);
    //desligo
  }

  //fim relay solenoide

  //segundo relay
  botRel2State = digitalRead(botRel2Pin);
  if (botRel2State == HIGH ) {
    rel2state = 0;
  } else {
    rel2state = 1;
  }

  //escreve nas portas digitais dos relays
  digitalWrite(pinRel1, rel1state);
  digitalWrite(pinRel2, rel2state);


  //desenha as telas no final
  if (indexTela == 0) {

    //tela oled 0 relógio e data
    myOLED.clrScr();
    myOLED.setFont(SmallFont);
    myOLED.printNumI(now.day(), 10, 56);//print para String
    myOLED.print("/", 40, 56);
    myOLED.printNumI(now.month(), CENTER, 56);
    myOLED.print("/", 85, 56);
    myOLED.printNumI(now.year(), RIGHT, 56);
    myOLED.print(":", 40, 34);
    myOLED.print(":", 85, 34);


    myOLED.setFont(BigNumbers);
    myOLED.printNumI(now.hour(), LEFT, 26);//printNumI para numero inteiro
    
    myOLED.printNumI(now.minute(), CENTER, 26);
    
    myOLED.printNumI(now.second(), RIGHT, 26);
    //myOLED.printNumI(temperature, CENTER, 46);
    // myOLED.printNumI(humidity, CENTER, 56);
    
    //animação
    myOLED.setFont(SmallFont);
    myOLED.print("|", LEFT, 0);
    myOLED.print("|", RIGHT, 0);
    //myOLED.update();
    //delay(100);
    int t = map(now.hour(), 0, 24, 0, 19);
    for (int i = 0; i < t; i++)
    {
      myOLED.print("\\", 7 + (i * 6), 0);
      //myOLED.update();
      //delay(250);
    }


    myOLED.update();


  }
  //DHT tela 02
  if (indexTela == 1) {
     myOLED.clrScr();
    myOLED.setFont(SmallFont);
    myOLED.print("t celcius", LEFT, 16);//print para String
    myOLED.print("%", RIGHT, 16);
    


    myOLED.setFont(BigNumbers);
    //myOLED.printNumI(now.hour(), LEFT, 26);//printNumI para numero inteiro
    //myOLED.printNumI(now.minute(), CENTER, 26);
   // myOLED.printNumI(now.second(), RIGHT, 26);
    myOLED.printNumI(temperature, LEFT, 26);
     myOLED.printNumI(humidity, RIGHT, 26);
    
    //animação
    myOLED.setFont(SmallFont);
    myOLED.print("|", LEFT, 0);
    myOLED.print("|", RIGHT, 0);

    myOLED.print("|", LEFT, 56);
    myOLED.print("|", RIGHT, 56);
    //myOLED.update();
    //delay(500);
    //
    int t = map(temperature, 0, 50, 0, 19);
    int h = map(humidity,0,100,0,19);
    for (int i = 0; i < t; i++)
    {
      myOLED.print("\\", 7 + (i * 6), 0);
      //myOLED.update();
      //delay(250);
    }

    for (int i = 0; i < h; i++)
    {
      myOLED.print("\\", 7 + (i * 6), 56);
      //myOLED.update();
      //delay(250);
    }


    myOLED.update();
    //while (1);

  }

  if (indexTela == 2) {
    
    myOLED.clrScr();
    myOLED.setFont(SmallFont);
    myOLED.print("Hora para Irrigar", CENTER, 0);//print para String
    myOLED.print(":", 60, 36);


    myOLED.setFont(BigNumbers);
    myOLED.printNumI(horaLiga, 20, 26);//printNumI para numero inteiro
    
    myOLED.printNumI(minutoLiga, 80, 26);


    myOLED.update();
    //while (1);

  }
  if (indexTela == 4) {
    myOLED.clrScr();
    myOLED.setFont(SmallFont);
     myOLED.print("calebav.com.br", CENTER, 0);
    myOLED.print("Horta Inteligente ", CENTER, 16);
     myOLED.print("com Energia Solar", CENTER,25);//print para String
    myOLED.print("Escola Alay Jose ", CENTER, 35);
    myOLED.print("Correa Vereador", CENTER, 45);//print para String
    myOLED.print(unidade, CENTER, 55);//print para String
   
    myOLED.update();
    //while (1);

  }
  
  //tela solo e luz
  if (indexTela == 3) {
    myOLED.clrScr();
    myOLED.setFont(SmallFont);
    myOLED.print("luminosidade", LEFT, 16);//print para String
    myOLED.print("solo", RIGHT, 16);



    myOLED.setFont(BigNumbers);
    //myOLED.printNumI(now.hour(), LEFT, 26);//printNumI para numero inteiro
    //myOLED.printNumI(now.minute(), CENTER, 26);
    // myOLED.printNumI(now.second(), RIGHT, 26);
    myOLED.printNumI(ldrRead, LEFT, 26);
    myOLED.printNumI(soloRead, RIGHT, 26);

    //animação
    myOLED.setFont(SmallFont);
    myOLED.print("|", LEFT, 0);
    myOLED.print("|", RIGHT, 0);

    myOLED.print("|", LEFT, 56);
    myOLED.print("|", RIGHT, 56);
    //myOLED.update();
    //delay(500);
    //
    int t = map(ldrRead, 0, 1024, 0, 19);
    int h = map(soloRead, 0, 1024, 0, 19);
    for (int i = 0; i < t; i++)
    {
      myOLED.print("\\", 7 + (i * 6), 0);
      // myOLED.update();
      //delay(250);
    }

    for (int i = 0; i < h; i++)
    {
      myOLED.print("\\", 7 + (i * 6), 56);
      // myOLED.update();
      //delay(250);
    }


    myOLED.update();
    //while (1);

  }
  if (indexTela == ntelas) {
    myOLED.clrScr();
    
    myOLED.update();
    //while (1);

  }





}

//RTC functions
/**
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.println();
    delay(300);
  **/
