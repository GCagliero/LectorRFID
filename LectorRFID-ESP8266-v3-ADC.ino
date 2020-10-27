/*Creado por Gonzalo Cagliero 2/2/2020*/
/*Modificado 17/05/2020 para Esp8266-12E*/
/*Modo configuracion WiFi, encendido y apagadode antena*/
/*Conexion a red WiFi*/

/*PinOut del proyecto*/
/* |HTRC110 | TMS3705 |DISP 16X2| Buzzer | Botón | ESP8266-12E |
   |--------|---------|---------|--------|-------|-------------|
   |8(SCLK) |         |         |        |       |  D7(gpio13) |
   |--------|---------|---------|--------|-------|-------------|
   |9(DIN)  |         |         |        |       |  D6(gpio12) |
   |--------|---------|---------|--------|-------|-------------|
   |10(DOUT)|         |         |        |       |  D5(gpio14) |
   |--------|---------|---------|--------|-------|-------------|
   |        |14(SCIO) |         |        |       |  D8(gpio15) |
   |--------|---------|---------|--------|-------|-------------|
   |        |16(TXCT) |         |        |       |  D4(gpio2) |
   |--------|---------|---------|--------|-------|-------------|
   |        |         |   SDA   |        |       |  D2(gpio4)  |  
   |--------|---------|---------|--------|-------|-------------|
   |        |         |   SCL   |        |       |  D1(gpio5)  | 
   |--------|---------|---------|--------|-------|-------------|
   |        |         |         |   X    |       |             |  
   |--------|---------|---------|--------|-------|-------------|
   |        |         |         |        |   X   |  D3(gpio0)  | 
   |--------|---------|---------|--------|-------|-------------|*/

#include <Operaciones.h>
#include <TMS3705.h>
#include <HTRC110.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>




const int TXCT = 2;
const int SCIO = 15;
const int btn = D3;
//const int bzzr = D4;
char tecla;
String str;
int a = 0;
int bat;
byte dataHDX[13];
byte dataFDX[13];
unsigned long p1;
unsigned long p2;
String datoC;
String datoN;
boolean WF = HIGH;

char ssid[50];      
char pass[50];
const char *ssidConf = "Baston";
const char *passConf = "12345678";//Pasword corto no va
String modo;
int contconexion = 0;
String mensaje = "";

//WiFiClient espClient;
ESP8266WebServer server(80);

Calculos miCalc;
TMS tms(15, 2);
HTRC htrc(14,12,13);
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  htrc.Send(B01010001);
  Serial.begin(115200);
  WiFi.forceSleepBegin();
  pinMode(btn, INPUT_PULLUP);  
  pinMode(TXCT, OUTPUT); //Inicializa pin 13 como salida
  pinMode(SCIO, INPUT);  //Inicializa pin 12 como entrada
  lcd.init();
  lcd.backlight();
  EEPROM.begin(512);
  boolean b = digitalRead(btn);
  if(b == LOW) {
    modoconf();
  }
  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);
  delay(1000);
  presentacion();
  p1 = millis();
}
void loop() {
    // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  //Serial.println(sensorValue);
  if(sensorValue < 702) {
    //Serial.println("Bat0");
    bat = 1;
  }
  if(sensorValue >= 702 && sensorValue <= 774 ) {
    //Serial.println("Bat1");
    bat = 2;
  }
  if(sensorValue >= 775 && sensorValue <= 846 ) {
    //Serial.println("Bat2");
    bat = 3;
  }
  if(sensorValue > 846) {
    //Serial.println("Bat3");
    bat = 4;
  }
  p2 = millis();
  //bat = 4;
  digitalWrite(TXCT, HIGH);//Comience con TXCT alta
  htrc.Send(B01010001);
  boolean b = digitalRead(btn);
  a = Serial.read();
  bateria(bat);
  mostrarWiFi();

//----------------------------------------------//  

  //funcion hibernar
  if((p2 - p1) > 30000) {
    while(b == HIGH) {
      b = digitalRead(btn);
      lcd.noBacklight();
      ESP.wdtFeed();
    }
    lcd.backlight();
    delay(500);
    b = HIGH;
    p1 = millis();     
  }
  //--------------//

//Evalua el tiempo que esta apretado el boton
//Si es menor que 2s modo lectura
//Si es mayor que 2s enciende o apaga modulo WiFi  
  unsigned long t1 = millis();
  unsigned long t2 = millis();
  if(b == LOW) {
    p1 = millis();     
    while(b == LOW && (t2 - t1) < 2000) {
      b = digitalRead(btn);
      t2 = millis();
      ESP.wdtFeed();
    }
    b = LOW;
  }
  if((t2 - t1) >= 2000) {
    WF = !WF; 
    //Serial.println(WF);  
    mostrarWiFi();
    if(WF == LOW) {
      WiFi.forceSleepWake();
      setup_wifi(&WF);
      delay(1);
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Apagando WiFi...");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reiniciando...");
        delay(2000);
        ESP.restart(); //ESP.reset();
    }
    while(b == LOW) {
      b = digitalRead(btn);
    }
  } 
//-----------------------------------------------//
//-----------------------------------------------//      
   

  
//Lectrura----------------------//  
  if(a == 'a' || b == LOW) {
    lcd.clear();
    for(int i = 0; i < 16; i++) {
      for(int j = 1; j < 4; j++) {
        barra(i, j);
        digitalWrite(7,HIGH);
        digitalWrite(8,LOW);
        htrc.Send(B01010000);
        delay(2);
        //CONFIG PAGES
        //Page 3
        htrc.Send(B01110000);
        delay(2);
        //Page 0
        htrc.Send(B01001111);
        delay(2);
        //Page 2
        htrc.Send(B01101100);
        delay(2);
        //Page 1
        htrc.Send(B01010000);
        delay(2);
        //CALCULATING SAMPLING TIME
        byte t_ant;
        t_ant = htrc.Transfer(B00001000);
        //Calculation
        t_ant <<= 1;
        t_ant += B00110111;
        t_ant &= B00111111;
        delay(2);
        //Set sampling time
        htrc.Send((B10000000 | t_ant));
        delay(2);
        //Get sampling time
        byte resp_samp_time;
        resp_samp_time = htrc.Transfer(B00000010);
        //GENERAL SETTLING SEQUENCE
        htrc.Send(B01101011);
        delay(4);
        htrc.Send(B01101000);                                                                                                                                                                                                                                                                 
        delay(1);
        htrc.Send(B01100000);
        delay(2);
        htrc.Send(B01010000);
        delay(2);
        htrc.ReadMode();
        htrc.saltarEncFDX();
        htrc.DataReadFDX(dataFDX);
        htrc.Send(B01010001);
              
        digitalWrite(TXCT, LOW);//Establezca TXCT bajo para Twake (50 µsec según el pdf de instrucciones, pero no se menciona en la hoja de datos)
        delayMicroseconds(50);
        digitalWrite(TXCT, HIGH);//Establecer TXCT alto para Tinit (20 ms) para permitir que el TMS3705 despierte
        delay(20);
        digitalWrite(TXCT, LOW);//Establezca TXCT bajo para 128 µsec (Tmcr) para el primer bit, cero,
        delayMicroseconds(128);
        digitalWrite(TXCT, HIGH);//Ajuste TXCT alto para 5 * Tmcr = 640 µsec (factor de división automática = 1111 y modo síncrono = 1)
        delayMicroseconds(640);
        digitalWrite(TXCT, LOW);//Establezca TXCT bajo para 2 * Tmcr. 
        delayMicroseconds(256);
        digitalWrite(TXCT, LOW); //Establezca TXCT bajo durante 50 mseg (período de carga)
        htrc.Send(B01010000);
        delay(50);
        digitalWrite(TXCT, HIGH);//Establezca TXCT alto y espere a que SCIO suba.
        htrc.Send(B01010001);
        a = digitalRead(SCIO);
        while(a == HIGH) {
          a = digitalRead(SCIO); //Espera que SCIO se ponga en bajo
        }
        tms.saltarEncHDX();
        tms.DataReadHDX(dataHDX);
        if(miCalc.CalCRC(dataHDX) || miCalc.CalCRC(dataFDX)) {
          i = 16;
          j = 5;
        }
      }
    }
    lcd.clear(); 
    if(!miCalc.CalCRC(dataHDX) && !miCalc.CalCRC(dataFDX)) {
      lcd.setCursor(0, 0);
      lcd.print("Intente");   
      lcd.setCursor(0, 1);
      lcd.print("Nuevamente"); 
    }    
    if(miCalc.CalCRC(dataHDX)) {
      datoC = "\0";
      datoN = "\0";
      datoC = miCalc.HexToDec("001", miCalc.PaisCode(miCalc.OrdDatBin(dataHDX)));
      datoN = miCalc.HexToDec("000000000001", miCalc.NumCode(miCalc.OrdDatBin(dataHDX)));
      Serial.println(datoC + datoN);
      lcd.print("HDX:");
      lcd.setCursor(0, 1);
      lcd.print(datoC + " " + datoN);
      
      //buzzer();

    }
    if(miCalc.CalCRC(dataFDX)) {
      datoC = "\0";
      datoN = "\0";
      datoC = miCalc.HexToDec("001", miCalc.PaisCode(miCalc.OrdDatBin(dataFDX)));
      datoN = miCalc.HexToDec("000000000001", miCalc.NumCode(miCalc.OrdDatBin(dataFDX)));
      lcd.print("FDX:");
      lcd.setCursor(0, 1);
      lcd.print(datoC + " " + datoN); 
      Serial.println(datoC + datoN);
      
      //buzzer();      
    }
//------------Fin Lectura-------------------------//  
    
    /*if(WF == LOW && (miCalc.CalCRC(dataHDX) || miCalc.CalCRC(dataFDX))) {
      mySerial.print(datoC + datoN);
      while(!mySerial.available()){}
      if(mySerial.available()) {
        str = mySerial.readString();       
      }       
      if(str == (datoC + datoN)) {
        datoOk();
      }
    }*/
    p1 = millis();//Utilizado para hibernar
  } 
}

void buzzer() {
  //Buzzer-----------//
  int t1 = millis();
  int t2 = millis();
  for(int i = 0; i < 3; i++) {
    int t1 = millis();
    int t2 = millis();
    while(t2 - t1 < 150) {
//      digitalWrite(bzzr, HIGH);
      delayMicroseconds(250);
//      digitalWrite(bzzr, LOW);       
      delayMicroseconds(250);  
      t2 = millis();   
    }
    delay(10);
  }
  //------------------//
}
void presentacion() {  
  byte c1[8] = {B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000};
  byte c3[8] = {B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100};
  byte c5[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
  byte Caliz0[8] = {B11000, B11000, B01100, B01100, B00111, B00111, B00011, B00001};
  byte Caliz4[8] = {B00011, B00011, B00110, B00110, B11100, B11100, B11000, B10000};
  byte Caliz5[8] = {B11111, B01111, B00001, B00000, B00000, B00000, B00011, B00011};
  byte Caliz7[8] = {B11111, B11110, B10000, B00000, B00000, B00000, B11000, B11000};

  lcd.createChar(1, c1);
  lcd.createChar(2, c3);
  lcd.createChar(3, c5);
  lcd.createChar(4, Caliz0);
  lcd.createChar(5, Caliz4);
  lcd.createChar(6, Caliz5);
  lcd.createChar(7, Caliz7);  

  lcd.setCursor(0, 0);
  lcd.write(4);
  lcd.write(3);
  lcd.write(3);
  lcd.write(3);
  lcd.write(5);
  lcd.setCursor(1, 1);
  lcd.write(6);
  lcd.write(3);
  lcd.write(7);
  //Kylix Tec.
  lcd.setCursor(6, 0);
  lcd.print("Kylix");
  lcd.setCursor(6, 1);
  lcd.print("Tec.");  
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Listo para leer");
}

void barra(int i, int j) {
  byte c1[8] = {B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000};
  byte c3[8] = {B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100};
  byte c5[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};

  lcd.createChar(1, c1);
  lcd.createChar(2, c3);
  lcd.createChar(3, c5);
 
  lcd.setCursor(i, 1);
  lcd.write(j);
}

void bateria(int i) {
  byte Bateria1[8] = {B01110, B11011, B10001, B10001, B10001, B10001, B10001, B11111};
  byte Bateria2[8] = {B01110, B11011, B10001, B10001, B10001, B11111, B11111, B11111};
  byte Bateria3[8] = {B01110, B11011, B10001, B11111, B11111, B11111, B11111, B11111};
  byte Bateria4[8] = {B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
  
  lcd.createChar(1, Bateria1);
  lcd.createChar(2, Bateria2);
  lcd.createChar(3, Bateria3);
  lcd.createChar(4, Bateria4);

  lcd.setCursor(15, 0);
  lcd.write(i);
}

void datoOk() {
  byte Ok[8] = {B00000, B00000, B00001, B00010, B10110, B01100, B00100, B00000};

  lcd.createChar(5, Ok);

  lcd.setCursor(9, 0);
  lcd.write(5);
}

void mostrarWiFi() {
  if(WF == LOW) {
    lcd.setCursor(10, 0);
    lcd.print("WiFi");
  }
  else {
    lcd.setCursor(10, 0);
    lcd.print("    ");
  }       
}

