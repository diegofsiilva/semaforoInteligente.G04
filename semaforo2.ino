
// ============================================================================
// CÓDIGO 2: ESP32_Semaforo2.ino
// Carregar este código no ESP32 #2 (que controla apenas o Semáforo 2)
// ============================================================================
#include "UbidotsEsp32Mqtt.h"
/****************************************
 * Credenciais WiFi e Ubidots
 ****************************************/
const char *UBIDOTS_TOKEN = "BBUS-KZ9ycvCEpIU6mEFYTcRWSFUdOFTUpf";
const char *WIFI_SSID = "Inteli.Iot";
const char *WIFI_PASS = "%(Yk(sxGMtvFEs.3";
const char *DEVICE_LABEL = "semaforo_inteligente";
const char *VARIABLE_MODO = "modo_dia"; // Recebe do ESP32 #1
const char *CLIENT_ID = "esp32_semaforos";
const int SUBSCRIBER_FREQUENCY = 500; // Lê a cada 500ms para sincronização
unsigned long timer_subscribe;
/****************************************
 * Pinos dos Semáforos
 ****************************************/
// Semáforo 1
#define VERMELHO1 18
#define AMARELO1 17
#define VERDE1 16
// Semáforo 2
#define VERMELHO2 25
#define AMARELO2 26
#define VERDE2 27
/****************************************
 * Variáveis do Sistema
 ****************************************/
unsigned long timer;
const int TEMPO_ESPERA_SEMAFORO = 2000;
const int TEMPO_PISCA_SEMAFORO = 500;
int ESTADO_PISCA = 0;
int ESTADO_SEMAFORO = 0;
bool modo_dia_recebido = true;
bool modo_anterior = true;
Ubidots ubidots(UBIDOTS_TOKEN, CLIENT_ID);
/****************************************
 * Callback para receber dados
 ****************************************/
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  // Processar o valor recebido
  float value = atof(message.c_str());
  // Identificar qual variável foi recebida pelo tópico
  if (strstr(topic, VARIABLE_MODO) != NULL) {
    modo_dia_recebido = (value > 0.5);
    Serial.print("Modo atualizado: ");
    Serial.println(modo_dia_recebido ? "DIA" : "NOITE");
  }
}
/****************************************
 * Funções dos Semáforos
 ****************************************/
void acender_semaforos_dia() {
  switch (ESTADO_SEMAFORO) {
    case 0: // Semáforo 1: Verde | Semáforo 2: Vermelho
      if(timer == 0) timer = millis();
      // Semáforo 1
      digitalWrite(VERDE1, 1);
      digitalWrite(AMARELO1, 0);
      digitalWrite(VERMELHO1, 0);
      // Semáforo 2
      digitalWrite(VERMELHO2, 1);
      digitalWrite(AMARELO2, 0);
      digitalWrite(VERDE2, 0);
      if (millis() - timer >= TEMPO_ESPERA_SEMAFORO) {
        digitalWrite(VERDE1, 0);
        timer = millis();
        ESTADO_SEMAFORO = 1;
      }
      break;
    case 1: // Semáforo 1: Amarelo | Semáforo 2: Vermelho
      // Semáforo 1
      digitalWrite(AMARELO1, 1);
      digitalWrite(VERDE1, 0);
      digitalWrite(VERMELHO1, 0);
      // Semáforo 2
      digitalWrite(VERMELHO2, 1);
      digitalWrite(AMARELO2, 0);
      digitalWrite(VERDE2, 0);
      if (millis() - timer >= TEMPO_ESPERA_SEMAFORO) {
        digitalWrite(AMARELO1, 0);
        timer = millis();
        ESTADO_SEMAFORO = 2;
      }
      break;
    case 2: // Semáforo 1: Vermelho | Semáforo 2: Verde
      // Semáforo 1
      digitalWrite(VERMELHO1, 1);
      digitalWrite(AMARELO1, 0);
      digitalWrite(VERDE1, 0);
      // Semáforo 2
      digitalWrite(VERDE2, 1);
      digitalWrite(AMARELO2, 0);
      digitalWrite(VERMELHO2, 0);
      if (millis() - timer >= TEMPO_ESPERA_SEMAFORO) {
        digitalWrite(VERDE2, 0);
        timer = millis();
        ESTADO_SEMAFORO = 3;
      }
      break;
    case 3: // Semáforo 1: Vermelho | Semáforo 2: Amarelo
      // Semáforo 1
      digitalWrite(VERMELHO1, 1);
      digitalWrite(AMARELO1, 0);
      digitalWrite(VERDE1, 0);
      // Semáforo 2
      digitalWrite(AMARELO2, 1);
      digitalWrite(VERDE2, 0);
      digitalWrite(VERMELHO2, 0);
      if (millis() - timer >= TEMPO_ESPERA_SEMAFORO) {
        digitalWrite(VERMELHO1, 0);
        digitalWrite(AMARELO2, 0);
        timer = millis();
        ESTADO_SEMAFORO = 0;
      }
      break;
  }
}
void acender_semaforos_noite(){
  switch(ESTADO_PISCA){
    case 0: // Ambos amarelos acesos
      digitalWrite(AMARELO1, 1);
      digitalWrite(VERDE1, 0);
      digitalWrite(VERMELHO1, 0);
      digitalWrite(AMARELO2, 1);
      digitalWrite(VERDE2, 0);
      digitalWrite(VERMELHO2, 0);
      if(millis() - timer >= TEMPO_PISCA_SEMAFORO){
        timer = millis();
        ESTADO_PISCA = 1;
      }
      break;
    case 1: // Ambos amarelos apagados
      digitalWrite(AMARELO1, 0);
      digitalWrite(AMARELO2, 0);
      if(millis() - timer >= TEMPO_PISCA_SEMAFORO){
        timer = millis();
        ESTADO_PISCA = 0;
      }
      break;
  }
}
void apagar_semaforos(){
  // Semáforo 1
  digitalWrite(VERDE1, 0);
  digitalWrite(AMARELO1, 0);
  digitalWrite(VERMELHO1, 0);
  // Semáforo 2
  digitalWrite(VERDE2, 0);
  digitalWrite(AMARELO2, 0);
  digitalWrite(VERMELHO2, 0);
}
/****************************************
 * Setup
 ****************************************/
void setup() {
  Serial.begin(115200);
  // Configurar pinos Semáforo 1
  pinMode(VERMELHO1, OUTPUT);
  pinMode(AMARELO1, OUTPUT);
  pinMode(VERDE1, OUTPUT);
  // Configurar pinos Semáforo 2
  pinMode(VERMELHO2, OUTPUT);
  pinMode(AMARELO2, OUTPUT);
  pinMode(VERDE2, OUTPUT);
  // Conectar WiFi e Ubidots
  ubidots.setDebug(true);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ESTADO_PISCA = 0;
  ESTADO_SEMAFORO = 0;
  timer = millis();
  timer_subscribe = millis();
  Serial.println("ESP32 Controlador de Semáforos iniciado!");
}
/****************************************
 * Loop Principal
 ****************************************/
void loop() {
  // Reconectar se necessário
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }
  // Subscrever para receber dados do ESP32 #1
  if (millis() - timer_subscribe > SUBSCRIBER_FREQUENCY) {
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_MODO);
    timer_subscribe = millis();
  }
  // Os valores são recebidos automaticamente através do callback
  // Detectar mudança de modo
  if (modo_dia_recebido != modo_anterior) {
    apagar_semaforos();
    timer = millis();
    ESTADO_SEMAFORO = 0;
    ESTADO_PISCA = 0;
    Serial.println(modo_dia_recebido ? "Mudou para MODO DIA" : "Mudou para MODO NOITE");
  }
  // Controlar semáforos baseado no modo
  if (modo_dia_recebido) {
    acender_semaforos_dia();
  } else {
    acender_semaforos_noite();
  }
  modo_anterior = modo_dia_recebido;
  // Debug
  Serial.print("Estado: ");
  Serial.print(ESTADO_SEMAFORO);
  Serial.print(" | Modo: ");
  Serial.print(modo_dia_recebido ? "DIA" : "NOITE");
  Serial.print(" | Pisca: ");
  Serial.println(ESTADO_PISCA);
  ubidots.loop();
  delay(100); // Pequeno delay para não sobrecarregar
}