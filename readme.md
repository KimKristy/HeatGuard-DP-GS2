# HeatGuard 🌡️

### Sistema inteligente de monitoramento de temperatura e alerta de risco térmico

## 1. Sobre o projeto

O **HeatGuard** é um sistema de monitoramento de temperatura e umidade desenvolvido com ESP32 e sensor DHT22. Seu objetivo é identificar situações de elevação da temperatura e emitir alertas visuais e sonoros automaticamente.

O projeto foi pensado para auxiliar no monitoramento de ambientes frequentados por pessoas mais vulneráveis ao calor, como idosos, trabalhadores expostos ao sol e moradores de regiões com temperaturas elevadas.

A solução utiliza **Edge Computing**, permitindo que o próprio ESP32 processe as informações do sensor e tome decisões localmente, sem depender de um servidor externo para acionar os alertas.

## 2. Objetivos

- Monitorar temperatura e umidade do ambiente.
- Identificar diferentes níveis de risco térmico.
- Acionar alertas visuais por meio de LEDs.
- Emitir alertas sonoros em situações de perigo.
- Realizar decisões automaticamente no ESP32.
- Enviar os dados por MQTT para monitoramento remoto.
- Apresentar as informações em um dashboard.

## 3. Componentes utilizados

| Componente          | Função                              |
| ------------------- | ----------------------------------- |
| ESP32               | Processamento e controle do sistema |
| DHT22               | Medição de temperatura e umidade    |
| LED verde           | Indicação de estado normal          |
| LED amarelo         | Indicação de atenção                |
| LED vermelho        | Indicação de perigo                 |
| Buzzer              | Emissão de alerta sonoro            |
| Resistores de 220 Ω | Proteção dos LEDs                   |

## 4. Funcionamento do sistema

O sensor DHT22 realiza as medições de temperatura e umidade. O ESP32 recebe esses valores e analisa a temperatura para identificar o estado atual do ambiente.

O sistema possui três estados:

| Estado  | Temperatura       | Ação                  |
| ------- | ----------------- | --------------------- |
| NORMAL  | Abaixo de 28 °C   | LED verde             |
| ATENÇÃO | A partir de 28 °C | LED amarelo           |
| PERIGO  | A partir de 32 °C | LED vermelho e buzzer |

Os limites utilizados são parâmetros definidos para a simulação e demonstração do projeto, não representando limites médicos ou recomendações oficiais de segurança.

### Recuperação automática

O HeatGuard também possui uma lógica de recuperação com histerese, evitando mudanças constantes de estado quando a temperatura oscila próxima aos limites.

- De PERIGO para ATENÇÃO: temperatura abaixo de 30 °C durante três leituras consecutivas.
- De ATENÇÃO para NORMAL: temperatura abaixo de 26 °C durante três leituras consecutivas.

## 5. Edge Computing e decisões autônomas

Um dos principais recursos do HeatGuard é a capacidade de tomar decisões localmente.

O ESP32 processa as leituras do sensor, identifica o nível de risco e controla os LEDs e o buzzer sem precisar que o Node-RED ou outro serviço externo envie comandos.

Essa característica foi testada desligando o Node-RED durante a simulação. Mesmo sem o serviço de monitoramento, o ESP32 continuou identificando os estados e acionando os alertas normalmente.

Dessa forma, o Node-RED é utilizado para monitoramento e visualização dos dados, enquanto as decisões de segurança são realizadas diretamente no dispositivo.

## 6. Comunicação MQTT e integração

O projeto utiliza o protocolo MQTT para transmitir os dados do ESP32.

**Broker utilizado:** mqtt-dashboard.com  
**Porta:** 1883

**Tópicos utilizados:**

- `fiap/esp32/sensor`: envio das informações do sensor.
- `fiap/esp32/led`: recebimento de comandos para controle manual do LED vermelho.

As informações enviadas incluem:

- Temperatura atual.
- Umidade.
- Média das últimas três leituras de temperatura.
- Estado atual do sistema.

O Node-RED recebe os dados por MQTT, organiza as informações e realiza a integração com o FIWARE Orion Context Broker, além de atualizar o dashboard.

## 7. Dashboard

O dashboard foi desenvolvido utilizando Node-RED Dashboard 2.0 e apresenta:

- Temperatura do ambiente.
- Umidade do ambiente.
- Estado atual do HeatGuard.
- Histórico das medições de temperatura.

O painel permite acompanhar as condições do ambiente de maneira visual e facilita a interpretação das informações recebidas pelo sistema.

## 8. Testes realizados

Durante o desenvolvimento, foram realizados os seguintes testes:

| Teste                                       | Resultado |
| ------------------------------------------- | --------- |
| Leitura de temperatura e umidade            | Aprovado  |
| Identificação do estado NORMAL              | Aprovado  |
| Identificação do estado ATENÇÃO             | Aprovado  |
| Identificação do estado PERIGO              | Aprovado  |
| Acionamento dos LEDs                        | Aprovado  |
| Acionamento do buzzer                       | Aprovado  |
| Recuperação automática dos estados          | Aprovado  |
| Recuperação após três leituras consecutivas | Aprovado  |
| Comunicação MQTT                            | Aprovado  |
| Recebimento dos dados no Node-RED           | Aprovado  |
| Integração com FIWARE Orion                 | Aprovado  |
| Funcionamento autônomo sem Node-RED         | Aprovado  |

## 9. Tecnologias utilizadas

- C++ / Arduino
- ESP32
- Wokwi
- Sensor DHT22
- MQTT
- Node-RED
- Node-RED Dashboard 2.0
- FIWARE Orion Context Broker
- Docker

## 10. Como executar a simulação

1. Acesse o link público do projeto disponibilizado na seção de links.
2. Aguarde o carregamento do projeto no Wokwi.
3. Clique no botão de iniciar a simulação.
4. Abra o sensor DHT22 e altere o valor da temperatura para testar os diferentes estados do sistema.
5. Observe os LEDs e o buzzer:
   - Abaixo de 28 °C: estado NORMAL, com LED verde.
   - A partir de 28 °C: estado ATENÇÃO, com LED amarelo.
   - A partir de 32 °C: estado PERIGO, com LED vermelho e buzzer.
6. Reduza a temperatura para verificar a recuperação automática dos estados, que exige três leituras consecutivas dentro dos limites de recuperação.

### Monitoramento remoto

Para visualizar os dados remotamente, é necessário manter o broker MQTT, o Node-RED e a integração com FIWARE configurados e em execução.

O monitoramento remoto é complementar: as decisões e os acionamentos dos alertas são realizados localmente pelo ESP32, sem depender do Node-RED.

## 11. Links do projeto

- **Simulação Wokwi:** [[DP2026 - WOKWI](https://wokwi.com/projects/477641251470860289)]
- **Vídeo de demonstração:** [[HeatGuard](https://youtu.be/iOv3D-53_Vo)]

## 12. Considerações finais

O desenvolvimento do HeatGuard permitiu aplicar conceitos de Internet das Coisas (IoT), programação embarcada, comunicação MQTT e Edge Computing.

Os testes demonstraram que o sistema consegue monitorar as condições do ambiente, identificar níveis de risco, emitir alertas e realizar a recuperação automática dos estados.

Um dos principais resultados foi comprovar que as decisões são tomadas localmente pelo ESP32, permitindo que os alertas continuem funcionando mesmo quando o Node-RED está indisponível.

O projeto demonstra como dispositivos IoT podem realizar o processamento de dados próximo à fonte de coleta, oferecendo respostas rápidas e maior autonomia ao sistema.
