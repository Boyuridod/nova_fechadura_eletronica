#include <SPI.h>                 // Inclui a biblioteca SPI.h
#include <RFID.h>                // Inclui a biblioteca RFID.h
#include <Wire.h>                // Inclui a biblioteca Wire.h

#define FechaduraPinA D0                //Ponte-H A-1A | Positivo
#define RFID_MISO D2                    //MISO do RFID
#define RFID_MOSI D3                    //MOSI do RFID
#define RFID_CLK D4                     //CLK do RFID
#define RFID_SDA D5                     //SDA do RFID
#define RFID_RST D8                     //Reset do RFID

RFID rfid(RFID_SDA, RFID_RST);             //Iniciliza as configurações da biblioteca RFID

bool access = false;

int alarm = 0;
uint8_t alarmStat = 0;
uint8_t maxError = 5;

void setup() {
  Serial.begin(9600);
  
  SPI.begin();
  rfid.init();

  pinMode(FechaduraPinA, OUTPUT);
  digitalWrite(FechaduraPinA, LOW);

}

void loop() {
  if (alarm >= maxError) {
    alarmStat = 1;
  }

  if (alarmStat == 0) {
    
    if (rfid.isCard()) {

  Serial.println("aaaaaaaaaaa");
      if (rfid.readCardSerial()) {
        Serial.print(rfid.serNum[0]);
        Serial.print(" ");
        Serial.print(rfid.serNum[1]);
        Serial.print(" ");
        Serial.print(rfid.serNum[2]);
        Serial.print(" ");
        Serial.print(rfid.serNum[3]);
        Serial.print(" ");
        Serial.print(rfid.serNum[4]);
        Serial.println("");

      if (access) {
        Serial.println("Bem-Vindo(a)!");
        digitalWrite(FechaduraPinA, HIGH);
        for (int i = 5; i > 0; i--) {
          delay (1000);
        }
        digitalWrite(FechaduraPinA, LOW);
      } else {
        alarm = alarm + 1;
        Serial.println("Acesso Negado!");
      }
    }
    rfid.halt();
  }
  else {
    for (int i = 10; i > 0; i--) {
    }
    alarmStat = 0;
    alarm = 0;
  }
}
}
