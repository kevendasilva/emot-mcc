 /*
 * Projeto emot
 * Desenvolvido por: Keven da Silva
 */

// Bibliotecas
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <vector>

// Classes personalizadas
#include "src/Component/Actuator/Actuator.h"
#include "src/HttpRequest/HttpRequest.h"

// LEDs de notificação
#define RESPONSE_STATUS_LED D0
#define WIFI_STATUS_LED D1

// Endereço do servidor
const String SERVER_ADDRESS = "SERVER_ADDRESS";

HttpRequest request;

// Delay entre requisições - Todos os tempos estão em milissegundos
unsigned long lagTime = 1500;
unsigned long lastTime = 0; // Variável de controle

// Teste com a classe Componente
std::vector<Component*> components;

void setup() {
  // Inicializando LEDs de notificação
  pinMode(RESPONSE_STATUS_LED, OUTPUT);
  pinMode(WIFI_STATUS_LED, OUTPUT);

  // Iniciando as configurações da conexão WiFi
  // Informações para conexão WiFi
  // O primeiro parâmetro é o SSID e o segundo é a SENHA, da sua rede WiFi
  wifiSetup("SSID", "PASSWORD");

  //  Caminho para obter as informações dos componentes
  String url = SERVER_ADDRESS + "/components.json";
  String body;
  int responseStatusCode;

  do {
    if (WiFi.status() == WL_CONNECTED) {
      request.get(url);
      body = request.getResponse();
      responseStatusCode = request.getResponseStatusCode();

      delay(500);
    }
  } while (responseStatusCode != 200);

  digitalWrite(WIFI_STATUS_LED, HIGH);

  JSONVar response = JSON.parse(body);

  if (JSON.typeof(response) == "undefined") {
    digitalWrite(RESPONSE_STATUS_LED, LOW);
  } else {
    digitalWrite(RESPONSE_STATUS_LED, HIGH);
    
    // Inicializando os componentes
    for (int i = 0; i < response.length(); i++) {
      JSONVar component = response[i];

      if (String(component["kind"]) == "actuator") {
        components.push_back(new Actuator(component["id"], component["name"], component["port"], component["max_value"], component["min_value"]));
      }
    }
  }
}

void loop() {
  // Verificando o tempo entre as requisições
  if ((millis() - lastTime) > lagTime) {
    // Caso existam componentes
    if (components.size()) {
      // Se ainda estiver conectado a rede WiFi
      if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(WIFI_STATUS_LED, HIGH);

        // Caminho para verificar as saídas dos componentes
        String url = SERVER_ADDRESS + "/outputs.json";
        String body;
        
        request.get(url);
        body = request.getResponse();

        JSONVar outputs = JSON.parse(body);
    
        if (JSON.typeof(outputs) == "undefined") {
          digitalWrite(RESPONSE_STATUS_LED, LOW);
        } else {
          digitalWrite(RESPONSE_STATUS_LED, HIGH);
          // Se existirem saídas
          if (outputs.length()) {
            // Aplicando as saídas
            for (int i = 0; i < outputs.length(); i++) {
              JSONVar output = outputs[i];
              int index = searchComponentByID(output["component_id"]);
              Actuator *actuator = static_cast<Actuator*>(components[index]);
            
              int outputStatus = actuator->output(output["value"], output["kind"]);
            }
          }
        }
      } else {
        digitalWrite(WIFI_STATUS_LED, HIGH);
        delay(500);
        digitalWrite(WIFI_STATUS_LED, LOW);
      }
    }

    lastTime = millis();
  }
}

int searchComponentByID(int id) {  
  for (int i = 0; i < components.size(); i++) {
    if (components[i]->getComponentId() == id) {
      return i;
    }
  }
}

void wifiSetup(char* ssid, char* password) {
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}
