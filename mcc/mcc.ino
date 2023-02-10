 /*
  The Amazing Flower Monitor (emot)

  Sistema de monitoramento de flores e plantas. Por meio de uma interface web,
  é possível controlar valores de saída, para atuadores conectados a placa. E,
  ler valores, para o caso de sensores. O propósito deste código é ser usado 
  no sistema de monitoramento, mas nada impede que o mesmo seja usado para
  outros projetos de internet das coisas.

  Projeto emot - Desenvolvido por Keven da Silva.
 */

#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <vector>
#include "src/Component/Actuator/Actuator.h"
#include "src/HttpRequest/HttpRequest.h"

#define RESPONSE_STATUS_LED D0
#define WIFI_STATUS_LED D1

const String SERVER_ADDRESS = "SERVER_ADDRESS";
HttpRequest request;

unsigned long lagTime = 1500;
unsigned long lastTime = 0;

std::vector<Component*> components;

void setup() {
  pinMode(RESPONSE_STATUS_LED, OUTPUT);
  pinMode(WIFI_STATUS_LED, OUTPUT);

  wifiSetup("SSID", "PASSWORD");

  // Solicitando informações sobre os componentes 
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
    
    // Configurando os componentes
    for (int i = 0; i < response.length(); i++) {
      JSONVar component = response[i];

      if (String(component["kind"]) == "actuator") {
        components.push_back(new Actuator(component["id"], component["name"], component["port"], component["max_value"], component["min_value"]));
      }
    }
  }
}

void loop() {
  if ((millis() - lastTime) > lagTime) {
    if (components.size()) {
      if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(WIFI_STATUS_LED, HIGH);

        // Solicitando informações de saídas desejadas, para os atuadores
        String url = SERVER_ADDRESS + "/outputs.json";
        String body;
        
        request.get(url);
        body = request.getResponse();

        JSONVar outputs = JSON.parse(body);
    
        if (JSON.typeof(outputs) == "undefined") {
          digitalWrite(RESPONSE_STATUS_LED, LOW);
        } else {
          digitalWrite(RESPONSE_STATUS_LED, HIGH);

          if (outputs.length()) {
            // Aplicando as saídas desejadas
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
