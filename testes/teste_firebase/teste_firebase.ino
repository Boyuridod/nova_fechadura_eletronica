#include <ESP8266WiFi.h>          // Inclusão da biblioteca do WI-Fi
#include <SPI.h>                  // Inclui a biblioteca SPI.h
#include <Wire.h>                 // Inclui a biblioteca Wire.h
#include <MFRC522.h>              // Inclui a biblioteca do sensor RFID
#include <Firebase_ESP_Client.h>  // Inclui a biblioteca que faz conexão com Firebase
#include <addons/TokenHelper.h>   // Fornece informações sobre geração de token.
#include <addons/RTDBHelper.h>    // Fornece informações sobre payload do RTDB e outras funções auxiliares.

// Definição das Portas

// RFID
#define RFID_RST D0   // Reset do RFID
#define RFID_CLK D5   // CLK do RFID
#define RFID_MISO D6  // MISO do RFID
#define RFID_MOSI D7  // MOSI do RFID
#define RFID_SDA D8   // SDA do RFID

MFRC522 mfrc522(RFID_SDA, RFID_RST);  // Cria instância do MFRC522
MFRC522::MIFARE_Key key;

// Fechadura
#define FechaduraPinA D1  // Ponte-H A-1A | Positivo
#define FechaduraPinB D2  // Ponte-H A-1B | Negativo

// Sensor magnético da porta
#define Sensor_Porta D4  // Fio do Sensor Magnético | O outro vai no GND
int portaIsFechada = 1;

// Campainha
#define Campainha D3

// Para conexão com Wi-Fi
const char* ssid = "BugWare";       // Nome da rede
const char* password = "ej#bw@23";  // Senha da rede

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

void conecta_wifi();
void conecta_firebase();
void verifica_abrePorta();
void abre_fechadura();
void fecha_fechadura();
void verifica_SigningUp();

// Código

void setup() {
  Serial.begin(9600);  // Inicia a comunicação com o Serial
  delay(100);          // Pequeno atraso para garantir que a serial está inicializada

  Serial.println("Inicializando...");

  conecta_wifi();  // Chama a função que irá conectar no Wi-Fi

  Serial.println("Conectado ao Wi-Fi");

  conecta_firebase();  // Chama a função que irá conectar no Firebase

  delay(1000);
}

void loop() {

  // Verifica se o Firebase está pronto
  if (Firebase.ready()) {

    

  }
}

// Funções

void conecta_wifi() {
  WiFi.begin(ssid, password);  // Inicia a conexão Wi-Fi

  Serial.print("Conectando a ");  // Escreve o texto na serial
  Serial.print(ssid);             // Escreve o nome da rede na serial

  while (WiFi.status() != WL_CONNECTED) {  // Enquanto status for diferente de conectado
    delay(500);
    Serial.print(".");
  }

  Serial.println("");                         // Pula uma linha na janela serial
  Serial.print("Conectado a rede sem fio ");  // Escreve o texto na serial
  Serial.println(ssid);                       // Escreve o nome da rede na serial
}

void conecta_firebase() {
  // Configuração do Firebase
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the API key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign-in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long-running token generation task */
  config.token_status_callback = tokenStatusCallback;  // veja addons/TokenHelper.h

  // Comente ou passe o valor falso quando a reconexão do Wi-Fi for controlada pelo seu código ou por uma biblioteca de terceiros, por exemplo, WiFiManager
  Firebase.reconnectNetwork(true);

  // A transmissão de grandes dados pode exigir um buffer de RX maior, caso contrário, problemas de conexão ou tempo limite de leitura de dados podem ocorrer.
  // Forneça tamanhos de buffer RX e TX
  fbdo.setBSSLBufferSize(4096, 4096); /* Tamanho do buffer Rx em bytes de 512 - 16384 /, 1024 / Tamanho do buffer Tx em bytes de 512 - 16384 */

  // Limita o tamanho do payload da resposta a ser coletado no FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}