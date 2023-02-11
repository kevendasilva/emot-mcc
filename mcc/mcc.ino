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
#include "src/Component/Sensor/Sensor.h"
#include "src/HttpRequest/HttpRequest.h"

using namespace std;

#define RESPONSE_STATUS_LED D0
#define WIFI_STATUS_LED D1

const String SERVER_ADDRESS = "SERVER_ADDRESS";
HttpRequest request;

unsigned long lagTime = 1500;
unsigned long lastTime = 0;

vector<Actuator*> actuators;
vector<Sensor*> sensores;

void setup() {
  pinMode(RESPONSE_STATUS_LED, OUTPUT);
  pinMode(WIFI_STATUS_LED, OUTPUT);

  wifiSetup("SSID", "PASSWORD");

  // Solicitando informações sobre os componentes 
  String url = SERVER_ADDRESS + "/components.json";
  String body;
  int responseStatusCode;

  do {
    if (wifiIsConnected()) {
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
        actuators.push_back(new Actuator(component["id"], component["name"], component["port"], component["max_value"], component["min_value"]));
      }

      if (String(component["kind"]) == "sensor") {
        sensores.push_back(new Sensor(component["id"], component["name"], component["port"]));
      }
    }
  }
}

void loop() {
  if ((millis() - lastTime) > lagTime) {
    if (wifiIsConnected()) {
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

        // Aplicando as saídas desejadas
        for (int i = 0; i < outputs.length(); i++) {
          JSONVar output = outputs[i];
          int index = searchActuatorById(output["component_id"]);

          if (index != -1) {
            int outputStatus = actuators[index]->output(output["value"], output["kind"]);
          }
        }
      }

      // Sensores   
      for (int i = 0; i < sensores.size(); i++) {
        String value = String(sensores[i]->read());
        String componentId = String(sensores[i]->getComponentId());

        String url = SERVER_ADDRESS + "/readings.json";
        String payload = "reading[component_id]=" + componentId + "&" + "reading[value]=" + value;

        request.post(url, payload);
      }
    } else {
      digitalWrite(WIFI_STATUS_LED, HIGH);
      delay(500);
      digitalWrite(WIFI_STATUS_LED, LOW);
    }

    lastTime = millis();
  }
}

int searchActuatorById(int id) {  
  for (int i = 0; i < actuators.size(); i++) {
    if (actuators[i]->getComponentId() == id) {
      return i;
    }
  }

  return -1;
}

bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void wifiSetup(char* ssid, char* password) {
  WiFi.begin(ssid, password);

  while(!wifiIsConnected()) {
    delay(1000);
  }
}
