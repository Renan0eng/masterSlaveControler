#include <esp_now.h>
#include <WiFi.h>

// A estrutura dos dados recebidos
struct SensorData {
  int lx, ly, rx, ry, l2, r2;
  int gx, gy, gz;
  int ax, ay, az;
  uint16_t buttons;
};

// Função chamada quando uma mensagem é recebida
void onReceive(const esp_now_recv_info * info, const uint8_t *incomingData, int len) {
  SensorData data;
  memcpy(&data, incomingData, sizeof(data));

  // Imprime o endereço MAC de quem enviou a mensagem
  Serial.print("Mensagem recebida de: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // Imprime alguns dos dados recebidos para teste
  Serial.printf("LX:%d LY:%d RX:%d RY:%d B:%u\n\n", data.lx, data.ly, data.rx, data.ry, data.buttons);
}

void setup() {
  Serial.begin(115200);
  
  // Garante um estado limpo antes de começar
  WiFi.disconnect(); // <-- Adicionado para robustez

  // Coloca o Wi-Fi em modo Estação
  WiFi.mode(WIFI_STA);

  // Adiciona uma pequena pausa para o hardware Wi-Fi estabilizar
  delay(100); // <-- Adicionado para corrigir o problema

  // Agora, esta linha deve mostrar o MAC correto
  Serial.print("MAC Address deste Receptor: ");
  Serial.println(WiFi.macAddress());

  // Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // O loop pode ficar vazio
}