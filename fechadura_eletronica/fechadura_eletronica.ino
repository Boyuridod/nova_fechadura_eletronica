#include <ESP8266WiFi.h>                        // Inclusão da biblioteca do WI-Fi
#include <SPI.h>                                // Inclui a biblioteca SPI.h
#include <Wire.h>                               // Inclui a biblioteca Wire.h
#include <MFRC522.h>                            // Inclui a biblioteca do sensor RFID
#include <ESP8266Firebase.h>                    // Inclui a biblioteca que faz conexão com Firebase

// Definição das Portas

// RFID
#define RFID_RST D0                             // Reset do RFID
#define RFID_CLK D5                             // CLK do RFID
#define RFID_MISO D6                            // MISO do RFID
#define RFID_MOSI D7                            // MOSI do RFID
#define RFID_SDA D8                             // SDA do RFID

MFRC522 mfrc522(RFID_SDA, RFID_RST);  // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

// Fechadura
#define FechaduraPinA D1                        // Ponte-H A-1A | Positivo
#define FechaduraPinB D2                        // Ponte-H A-1B | Negativo

// Sensor magnético da porta
#define Sensor_Porta D4                         // Fio do Sensor Magnético | O outro vai no GND

// Firebase
#define URL_Firebase "https://bancopi7-default-rtdb.firebaseio.com/"  // URL para o banco de dados Firebase
Firebase firebase(URL_Firebase);

//Definição de Constantes

const char* ssid = "Inside The Forest";         //Nome da rede
const char* password = "youarealone";           //Senha da rede

//Variáveis

int portaIsFechada = 1;

//Início do código

void setup() {
  Serial.begin(115200);                           //Inicia a comunicação com o Serial | Padrão: 9600 bps | Usando: 115200 bps

  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522
  // Serial.println("Coloque sua tag RFID perto do leitor...");
  // Serial.println();

  conecta_wifi();                               //Chama a função que irá conectar no WIFi

  //Setando os pinos
  pinMode(FechaduraPinA, OUTPUT);               //Configura D1 como saida
  pinMode(FechaduraPinB, OUTPUT);               //Configura D2 como saida
  // pinMode(Sensor_Porta, INPUT_PULLUP);

  //Inicializando os pinos
  digitalWrite(FechaduraPinA, LOW);             //Configuração para fechar a fechadura
  digitalWrite(FechaduraPinB, LOW);             //Configuração para fechar a fechadura

  delay(1000);
}

void loop() {

  portaIsFechada = digitalRead(Sensor_Porta);

  Serial.println(portaIsFechada);

  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent()){

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    String leitura = "";

    // Show UID on serial monitor
    Serial.print("UID da tag: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if(i == 0)
        leitura.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : ""));
      else
        leitura.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));

      leitura.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    leitura.toUpperCase();

    Serial.println(leitura);
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();

    String query = "Tag/";

    query.concat(leitura);
    query.concat("/Nome");

    Serial.println(query);
    Serial.println(leitura);

    String data = firebase.getString(query);
    Serial.print("Received String: ");
    Serial.println(data);

    if(data != "ul"){

      abre_fechadura();

      firebase.setInt("isTrancada", false);

      while(!portaIsFechada){

        delay(1000);

        portaIsFechada = digitalRead(Sensor_Porta);

      }

      fecha_fechadura();

    }

  }
  
}

//Funções

void conecta_wifi() {

  WiFi.begin(ssid, password);                   //Inicia a conexão Wi-Fi

  Serial.print("Conectando a ");                //ESCREVE O TEXTO NA SERIAL
  Serial.print(ssid);                           //ESCREVE O NOME DA REDE NA SERIAL

  while (WiFi.status() != WL_CONNECTED) {       //ENQUANTO STATUS FOR DIFERENTE DE CONECTADO

    delay(500);
    Serial.print(".");
    
  }

  Serial.println(""); //PULA UMA LINHA NA JANELA SERIAL
  Serial.print("Conectado a rede sem fio "); //ESCREVE O TEXTO NA SERIAL
  Serial.println(ssid); //ESCREVE O NOME DA REDE NA SERIAL

}

void abre_fechadura() {

  digitalWrite(FechaduraPinA, HIGH);    //Configuração para abrir a fechadura
  firebase.setString("isTrancada", "false");

}

void fecha_fechadura() {

  digitalWrite(FechaduraPinA, LOW);  //Configuração para fechar a fechadura
  firebase.setString("isTrancada", "true");

}