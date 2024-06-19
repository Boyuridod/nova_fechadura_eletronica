#include <ESP8266WiFi.h>          // Inclusão da biblioteca do WI-Fi
#include <SPI.h>                  // Inclui a biblioteca SPI.h
#include <Wire.h>                 // Inclui a biblioteca Wire.h
#include <MFRC522.h>              // Inclui a biblioteca do sensor RFID
#include <Firebase_ESP_Client.h>  // Inclui a biblioteca que faz conexão com Firebase
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Definição das Portas

// RFID
#define RFID_RST D0   // Reset do RFID
#define RFID_CLK D5   // CLK do RFID
#define RFID_MISO D6  // MISO do RFID
#define RFID_MOSI D7  // MOSI do RFID
#define RFID_SDA D8   // SDA do RFID

MFRC522 mfrc522(RFID_SDA, RFID_RST);  // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

// Fechadura
#define FechaduraPinA D1  // Ponte-H A-1A | Positivo
#define FechaduraPinB D2  // Ponte-H A-1B | Negativo

// Sensor magnético da porta
#define Sensor_Porta D4  // Fio do Sensor Magnético | O outro vai no GND
int portaIsFechada = 1;



// Para conexão com Wi-Fi

// const char* ssid = "Inside The Forest";  // Nome da rede
// const char* password = "youarealone";    // Senha da rede

const char* ssid = "S23 Ultra de Vinicius Gabriel";         // Nome da rede
const char* password = "pcgkkdr9y54r5qn";                  // Senha da rede



// Conexão com Firebase

/* 2. Define the API Key */
#define API_KEY "AIzaSyBUiYGI7-oV9mmKp_6XHm5D2v9_zRVvJBU"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://bancodochaves-default-rtdb.firebaseio.com/"  // <databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that already registered or added in your project */
#define USER_EMAIL "projetoint7@gmail.com"
#define USER_PASSWORD "Senha@123456"

// Variáveis Firebase

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;



// Código

void setup() {
  Serial.begin(9600);  // Inicia a comunicação com o Serial | Padrão: 9600 bps | Usando: 115200 bps

  conecta_wifi();  // Chama a função que irá conectar no WIFi

  conecta_firebase();  // Chama a função que irá conectar no FIREBASE

  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522

  //Setando os pinos
  pinMode(FechaduraPinA, OUTPUT);  // Configura D1 como saida
  pinMode(FechaduraPinB, OUTPUT);  // Configura D2 como saida
  pinMode(Sensor_Porta, INPUT_PULLUP);

  //Inicializando os pinos
  digitalWrite(FechaduraPinA, LOW);  //Configuração para fechar a fechadura
  digitalWrite(FechaduraPinB, LOW);  //Configuração para fechar a fechadura

  delay(1000);
}

void loop() {

  if (Firebase.ready()) {

    // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent()) {

      // Select one of the cards
      if (!mfrc522.PICC_ReadCardSerial()) {
        return;
      }

      String leitura = "";

      // Show UID on serial monitor
      Serial.print("UID da tag: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (i == 0)
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

      String nodePath = "/ChavesCadastradas/" + leitura;

      // Serial.println("Path: " + nodePath);

      if (Firebase.RTDB.getJSON(&fbdo, nodePath)) {
        Serial.println("Código encontrado no Firebase!");
        abre_fechadura();
      } else {
        Serial.println("Código não encontrado no Firebase.");
        // Serial.println(fbdo.errorReason());
      }
    }

    delay(5000);

    while(portaIsFechada){

      // Confere se a porta está fechada | 0 é fechada | 1 é aberta
      portaIsFechada = digitalRead(Sensor_Porta);

    }

    fecha_fechadura();

  }
}

// Funções

void conecta_wifi() {

  WiFi.begin(ssid, password);  //Inicia a conexão Wi-Fi

  Serial.print("Conectando a ");  //ESCREVE O TEXTO NA SERIAL
  Serial.print(ssid);             //ESCREVE O NOME DA REDE NA SERIAL

  while (WiFi.status() != WL_CONNECTED) {  //ENQUANTO STATUS FOR DIFERENTE DE CONECTADO

    delay(500);
    Serial.print(".");

  }

  Serial.println("");                         //PULA UMA LINHA NA JANELA SERIAL
  Serial.print("Conectado a rede sem fio ");  //ESCREVE O TEXTO NA SERIAL
  Serial.println(ssid);                       //ESCREVE O NOME DA REDE NA SERIAL
}

void conecta_firebase() {

  // Configuração do fb no void setup() - >
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  // Provide both RX and TX buffer sizes
  fbdo.setBSSLBufferSize(4096, 4096); /* Rx buffer size in bytes from 512 - 16384 /, 1024 / Tx buffer size in bytes from 512 - 16384 */

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}

void abre_fechadura() {

  digitalWrite(FechaduraPinA, HIGH);  //Configuração para abrir a fechadura
  // firebase.setString("isTrancada", "false");
}

void fecha_fechadura() {

  digitalWrite(FechaduraPinA, LOW);  //Configuração para fechar a fechadura
  // firebase.setString("isTrancada", "true");
}

// Código pra reutilizar

// // Confere se a porta está fechada | 0 é fechada | 1 é aberta
//   portaIsFechada = digitalRead(Sensor_Porta);
//   // Serial.println(portaIsFechada);

//   // Look for new cards
//   if (mfrc522.PICC_IsNewCardPresent()) {

//     // Select one of the cards
//     if (!mfrc522.PICC_ReadCardSerial()) {
//       return;
//     }

//     String leitura = "";

//     // Show UID on serial monitor
//     Serial.print("UID da tag: ");
//     for (byte i = 0; i < mfrc522.uid.size; i++) {
//       if (i == 0)
//         leitura.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : ""));
//       else
//         leitura.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));

//       leitura.concat(String(mfrc522.uid.uidByte[i], HEX));
//     }

//     leitura.toUpperCase();

//     Serial.println(leitura);

//     // Halt PICC
//     mfrc522.PICC_HaltA();

//     // Stop encryption on PCD
//     mfrc522.PCD_StopCrypto1();
//   }