// ============================================================================
// CÓDIGO 1: ESP32_Semaforo1_LDR.ino
// Carregar este código no ESP32 #1 (que tem o LDR conectado)
// ============================================================================
#include "UbidotsEsp32Mqtt.h"
/****************************************
 * Credenciais WiFi e Ubidots
 ****************************************/
const char *UBIDOTS_TOKEN = "BBUS-KZ9ycvCEpIU6mEFYTcRWSFUdOFTUpf";
const char *WIFI_SSID = "Inteli.Iot";
const char *WIFI_PASS = "%(Yk(sxGMtvFEs.3";
const char *DEVICE_LABEL = "semaforo_inteligente";
const char *VARIABLE_MODO = "modo_dia"; // true=dia, false=noite
const char *VARIABLE_LDR = "leitura_ldr"; // Valor do LDR
const char *CLIENT_ID = "esp32_ldr";
const int PUBLISH_FREQUENCY = 500; // Atualiza a cada 500ms para sincronização
unsigned long timer_publish;
/****************************************
 * Pinos do LDR
 ****************************************/
#define LDR 34
/****************************************
 * Variáveis do Sistema
 ****************************************/
const int LIMITE_LDR_NOITE = 1000;
bool modo_dia = false;
bool modo_anterior = false;
Ubidots ubidots(UBIDOTS_TOKEN, CLIENT_ID);
/****************************************
 * Setup
 ****************************************/
void setup() {
  Serial.begin(115200);
  // Configurar pino do LDR
  pinMode(LDR, INPUT);
  // Conectar WiFi e Ubidots
  ubidots.setDebug(true);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setup();
  ubidots.reconnect();
  timer_publish = millis();
  Serial.println("ESP32 LDR Sensor iniciado!");
}
/****************************************
 * Loop Principal
 ****************************************/
void loop() {
  // Reconectar se necessário
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }
  // Ler LDR
  int LDR_LEITURA = analogRead(LDR);
  Serial.print("LDR: ");
  Serial.println(LDR_LEITURA);
  modo_dia = (LDR_LEITURA > LIMITE_LDR_NOITE);
  // Detectar mudança de modo
  if (modo_dia != modo_anterior) {
    Serial.println(modo_dia ? "Mudou para MODO DIA" : "Mudou para MODO NOITE");
  }
  modo_anterior = modo_dia;
  // Publicar dados no Ubidots
  if (millis() - timer_publish > PUBLISH_FREQUENCY) {
    ubidots.add(VARIABLE_MODO, modo_dia ? 1 : 0);
    ubidots.add(VARIABLE_LDR, LDR_LEITURA);
    ubidots.publish(DEVICE_LABEL);
    Serial.print("Publicado - Modo: ");
    Serial.print(modo_dia ? "DIA" : "NOITE");
    Serial.print(" | LDR: ");
    Serial.println(LDR_LEITURA);
    timer_publish = millis();
  }
  ubidots.loop();
}


