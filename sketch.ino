// Kimberly Kristina - RM: 564080

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <math.h>

// -------------------- PINOS --------------------

#define DHTPIN 15
#define DHTTYPE DHT22

#define LEDVERDE 18
#define LEDAMARELO 19
#define LEDVERMELHO 4
#define BUZZER 5

// -------------------- WIFI E MQTT --------------------

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* MQTT_SERVER = "mqtt-dashboard.com";
const int MQTT_PORT = 1883;

const char* TOPICO_SENSOR = "fiap/esp32/sensor";
const char* TOPICO_LED = "fiap/esp32/led";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// -------------------- CONFIGURACOES --------------------

const float LIMITE_ATENCAO = 28.0;
const float LIMITE_PERIGO = 32.0;

// Limites para sair dos estados
const float SAIDA_ATENCAO = 26.0;
const float SAIDA_PERIGO = 30.0;

const unsigned long INTERVALO_LEITURA = 2000;

const int QUANTIDADE_LEITURAS = 3;
const int LEITURAS_PARA_REDUZIR = 3;

// -------------------- VARIAVEIS --------------------

float temperaturas[QUANTIDADE_LEITURAS];

int indiceTemperatura = 0;
int totalLeituras = 0;

float temperaturaAtual = 0;
float umidadeAtual = 0;
float temperaturaMedia = 0;

int contadorReducao = 0;

bool comandoManual = false;
bool estadoManual = false;

enum Estado {
  NORMAL,
  ATENCAO,
  PERIGO,
  FALHA
};

Estado estadoAtual = FALHA;

// Controle das tentativas de reconexao
unsigned long ultimaTentativaWiFi = 0;
unsigned long ultimaTentativaMQTT = 0;

const unsigned long INTERVALO_RECONEXAO = 5000;

bool wifiConectadoAnterior = false;

// -------------------- WIFI --------------------

void manterWiFi() {

  if (WiFi.status() == WL_CONNECTED) {

    if (!wifiConectadoAnterior) {
      Serial.println("\nWi-Fi conectado!");
      Serial.println(WiFi.localIP());

      wifiConectadoAnterior = true;
    }

    return;
  }

  if (wifiConectadoAnterior) {
    Serial.println("Conexao Wi-Fi perdida!");

    client.disconnect();

    wifiConectadoAnterior = false;
  }

  if (millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO) {

    ultimaTentativaWiFi = millis();

    Serial.println("Tentando reconectar ao Wi-Fi...");

    WiFi.reconnect();
  }
}

// -------------------- MQTT --------------------

void receberComando(char* topic, byte* mensagem,
                    unsigned int tamanho) {

  String comando = "";

  for (unsigned int i = 0; i < tamanho; i++) {
    comando += (char)mensagem[i];
  }

  Serial.print("Comando recebido: ");
  Serial.println(comando);

  if (String(topic) == TOPICO_LED) {

    if (comando == "ON") {

      comandoManual = true;
      estadoManual = true;

      Serial.println("LED vermelho ligado manualmente!");

    }

    else if (comando == "OFF") {

      comandoManual = true;
      estadoManual = false;

      Serial.println("LED vermelho desligado manualmente!");

    }

    else if (comando == "AUTO") {

      comandoManual = false;

      Serial.println("Controle automatico ativado!");

    }
  }
}

void manterMQTT() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (client.connected()) {
    return;
  }

  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }

  ultimaTentativaMQTT = millis();

  Serial.println("Tentando conectar ao MQTT...");

  String id = "ESP32-HeatGuard-" + String(random(10000));

  if (client.connect(id.c_str())) {

    Serial.println("MQTT conectado!");

    client.subscribe(TOPICO_LED);

    Serial.println("Inscrito no topico do LED!");

  } else {

    Serial.print("Falha MQTT. Codigo: ");
    Serial.println(client.state());

  }
}

// -------------------- LEITURA DO SENSOR --------------------

bool lerSensor() {

  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {

    Serial.println("Erro ao ler o DHT22!");

    return false;
  }

  if (temperatura < -40 || temperatura > 80 ||
      umidade < 0 || umidade > 100) {

    Serial.println("Leitura fora da faixa valida!");

    return false;
  }

  temperaturaAtual = temperatura;
  umidadeAtual = umidade;

  // Armazena as ultimas tres temperaturas

  temperaturas[indiceTemperatura] = temperatura;

  indiceTemperatura++;

  if (indiceTemperatura >= QUANTIDADE_LEITURAS) {
    indiceTemperatura = 0;
  }

  if (totalLeituras < QUANTIDADE_LEITURAS) {
    totalLeituras++;
  }

  return true;
}

// -------------------- MEDIA DE TEMPERATURA --------------------

void calcularMediaTemperatura() {

  if (totalLeituras == 0) {
    return;
  }

  float soma = 0;

  for (int i = 0; i < totalLeituras; i++) {
    soma += temperaturas[i];
  }

  temperaturaMedia = soma / totalLeituras;
}

// -------------------- PROCESSAMENTO LOCAL --------------------

void processarTemperatura() {

  // PERIGO: entrada imediata

  if (temperaturaAtual >= LIMITE_PERIGO) {

    estadoAtual = PERIGO;
    contadorReducao = 0;

    return;
  }

  // ATENCAO: entrada imediata

  if (estadoAtual == NORMAL &&
      temperaturaAtual >= LIMITE_ATENCAO) {

    estadoAtual = ATENCAO;
    contadorReducao = 0;

    return;
  }

  // Recuperacao do estado de perigo

  if (estadoAtual == PERIGO) {

    if (temperaturaAtual < SAIDA_PERIGO) {

      contadorReducao++;

      if (contadorReducao >= LEITURAS_PARA_REDUZIR) {

        estadoAtual = ATENCAO;
        contadorReducao = 0;
      }

    } else {

      contadorReducao = 0;
    }

    return;
  }

  // Recuperacao do estado de atencao

  if (estadoAtual == ATENCAO) {

    if (temperaturaAtual < SAIDA_ATENCAO) {

      contadorReducao++;

      if (contadorReducao >= LEITURAS_PARA_REDUZIR) {

        estadoAtual = NORMAL;
        contadorReducao = 0;
      }

    } else {

      contadorReducao = 0;
    }

    return;
  }

  // Recuperacao depois de uma falha no sensor

  if (estadoAtual == FALHA) {

    if (temperaturaAtual >= LIMITE_PERIGO) {

      estadoAtual = PERIGO;

    }

    else if (temperaturaAtual >= LIMITE_ATENCAO) {

      estadoAtual = ATENCAO;

    }

    else {

      estadoAtual = NORMAL;
    }

    contadorReducao = 0;
  }
}

// -------------------- ATUADORES --------------------

void desligarTudo() {

  digitalWrite(LEDVERDE, LOW);
  digitalWrite(LEDAMARELO, LOW);
  digitalWrite(LEDVERMELHO, LOW);

  noTone(BUZZER);
}

void controlarAtuadores() {

  desligarTudo();

  switch (estadoAtual) {

    case NORMAL:

      digitalWrite(LEDVERDE, HIGH);

      break;

    case ATENCAO:

      digitalWrite(LEDAMARELO, HIGH);

      break;

    case PERIGO:

      digitalWrite(LEDVERMELHO, HIGH);

      tone(BUZZER, 2000);

      break;

    case FALHA:

      digitalWrite(LEDVERMELHO, HIGH);

      tone(BUZZER, 2000);

      break;
  }

  // Comando manual controla o LED vermelho.
  // Perigo e falha sempre possuem prioridade.

  if (comandoManual &&
      estadoAtual != PERIGO &&
      estadoAtual != FALHA) {

    digitalWrite(
      LEDVERMELHO,
      estadoManual ? HIGH : LOW
    );
  }
}

// -------------------- SERIAL --------------------

void mostrarEstado() {

  Serial.println("\n------------------------");

  Serial.print("Temperatura atual: ");
  Serial.print(temperaturaAtual);
  Serial.println(" C");

  Serial.print("Umidade: ");
  Serial.print(umidadeAtual);
  Serial.println(" %");

  Serial.print("Media das ultimas leituras: ");
  Serial.print(temperaturaMedia);
  Serial.println(" C");

  Serial.print("Estado: ");

  switch (estadoAtual) {

    case NORMAL:

      Serial.println("NORMAL");

      break;

    case ATENCAO:

      Serial.println("ATENCAO");

      break;

    case PERIGO:

      Serial.println("PERIGO");

      break;

    case FALHA:

      Serial.println("FALHA NO SENSOR");

      break;
  }

  if (contadorReducao > 0) {

    Serial.print("Leituras para reduzir o alerta: ");
    Serial.print(contadorReducao);
    Serial.print("/");
    Serial.println(LEITURAS_PARA_REDUZIR);
  }

  Serial.println("------------------------");
}

// -------------------- PUBLICACAO MQTT --------------------

void publicarDados() {

  if (!client.connected()) {
    return;
  }

  const char* nomeEstado;

  switch (estadoAtual) {

    case NORMAL:

      nomeEstado = "NORMAL";

      break;

    case ATENCAO:

      nomeEstado = "ATENCAO";

      break;

    case PERIGO:

      nomeEstado = "PERIGO";

      break;

    case FALHA:

      nomeEstado = "FALHA";

      break;
  }

  char mensagem[150];

  snprintf(
    mensagem,
    sizeof(mensagem),
    "{\"temperatura\":%.1f,\"umidade\":%.1f,\"media\":%.1f,\"estado\":\"%s\"}",
    temperaturaAtual,
    umidadeAtual,
    temperaturaMedia,
    nomeEstado
  );

  if (client.publish(TOPICO_SENSOR, mensagem)) {

    Serial.print("Dados enviados: ");
    Serial.println(mensagem);

  } else {

    Serial.println("Erro ao publicar MQTT!");
  }
}

// -------------------- SETUP --------------------

void setup() {

  Serial.begin(115200);

  pinMode(LEDVERDE, OUTPUT);
  pinMode(LEDAMARELO, OUTPUT);
  pinMode(LEDVERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  desligarTudo();

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  ultimaTentativaWiFi = millis();

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(receberComando);
  client.setSocketTimeout(2);

  Serial.println("\nHeatGuard iniciado!");
}

// -------------------- LOOP --------------------

void loop() {

  manterWiFi();

  manterMQTT();

  if (client.connected()) {
    client.loop();
  }

  static unsigned long ultimaLeitura = 0;

  if (millis() - ultimaLeitura >= INTERVALO_LEITURA) {

    ultimaLeitura = millis();

    if (lerSensor()) {

      calcularMediaTemperatura();

      processarTemperatura();

    } else {

      estadoAtual = FALHA;
      contadorReducao = 0;
    }

    controlarAtuadores();

    mostrarEstado();

    publicarDados();
  }
}