#include <PS4Controller.h>
#include <esp_now.h>
#include <WiFi.h>

// A fila para passar dados do loop principal para a tarefa de log
QueueHandle_t dataQueue;

struct SensorData {
  int lx, ly, rx, ry, l2, r2;
  int gx, gy, gz;
  int ax, ay, az;
  uint16_t buttons;
};

// A função readButtons continua a mesma...
uint16_t readButtons() {
  uint16_t b = 0;
  if (PS4.Right()) b |= 1 << 0;
  if (PS4.Down()) b |= 1 << 1;
  if (PS4.Up()) b |= 1 << 2;
  if (PS4.Left()) b |= 1 << 3;
  if (PS4.Square()) b |= 1 << 4;
  if (PS4.Cross()) b |= 1 << 5;
  if (PS4.Circle()) b |= 1 << 6;
  if (PS4.Triangle()) b |= 1 << 7;
  if (PS4.L1()) b |= 1 << 8;
  if (PS4.R1()) b |= 1 << 9;
  if (PS4.Share()) b |= 1 << 10;
  if (PS4.Options()) b |= 1 << 11;
  if (PS4.L3()) b |= 1 << 12;
  if (PS4.R3()) b |= 1 << 13;
  if (PS4.PSButton()) b |= 1 << 14;
  if (PS4.Touchpad()) b |= 1 << 15;
  return b;
}

// MAC do receptor (ESP2)
uint8_t receiverMac[] = {0xA0, 0xDD, 0x6C, 0x0E, 0x8E, 0x9C};

void onSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  // Não vamos imprimir nada aqui para manter o log limpo
}

void setup() {
  Serial.begin(921600);

  // --- CORREÇÃO 1: Criar a Fila ---
  // Cria uma fila que pode conter até 5 itens do tipo SensorData
  dataQueue = xQueueCreate(5, sizeof(SensorData));
  if (dataQueue == NULL) {
    Serial.println("Erro ao criar a fila!");
  }

  // --- CORREÇÃO 2: Ordem de Inicialização ---
  // 1. PS4 (Bluetooth) primeiro
  PS4.begin();
  
  // 2. Wi-Fi e ESP-NOW depois
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  // 3. Configuração do ESP-NOW
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  if (PS4.isConnected()) {
    SensorData data;
    data.lx = PS4.LStickX();
    data.ly = PS4.LStickY();
    data.rx = PS4.RStickX();
    data.ry = PS4.RStickY();
    data.l2 = PS4.L2Value();
    data.r2 = PS4.R2Value();
    data.gx = PS4.GyrX();
    data.gy = PS4.GyrY();
    data.gz = PS4.GyrZ();
    data.ax = PS4.AccX();
    data.ay = PS4.AccY();
    data.az = PS4.AccZ();
    data.buttons = readButtons();

    // Envia os dados pela rede
    esp_now_send(receiverMac, (uint8_t*)&data, sizeof(data));

    // --- CORREÇÃO 3: Enviar dados para a Fila ---
    // Envia uma cópia dos dados para a fila de log.
    // O 0 no final significa não esperar se a fila estiver cheia.
    xQueueSend(dataQueue, &data, 0);
  }
  delay(10);
}