 /*
 * Projeto emot
 * Desenvolvido por: Keven da Silva
 */

// Bibliotecas
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <vector>

// Classes personalizadas
#include "src/LogMessageHandler/LogMessageHandler.h"
#include "src/Component/Actuator/Actuator.h"

// Variáveis usadas nas requisições HTTP
String path;
String request[2];
// Endereço do servidor
String serverAddress = "SERVER_ADDRESS";

// Delay entre requisições - Todos os tempos estão em milissegundos
unsigned long lagTime = 1500;
unsigned long lastTime = 0; // Variável de controle

// Variável de controle das mensagens
bool messagesAreEnabled = false;

// Manipulador das mensagens de log
LogMessageHandler lmh;

// Teste com a classe Componente
std::vector<Component*> components;

void setup() {
  // Inicialização a comunicação serial   
  Serial.begin(115200);

  // Iniciando as configurações da conexão WiFi
  // Informações para conexão WiFi
  // O primeiro parâmetro é o SSID e o segundo é a SENHA, da sua rede WiFi
  wifiSetup("SSID", "PASSWORD");

  //  Caminho para obter as informações dos componentes
  path = serverAddress + "components.json";

  do {
    if (WiFi.status() == WL_CONNECTED) {
      requestGET(path.c_str(), request);

      String data[3] = {path, request[0], request[1]};
      logMessage(lmh.handleMessage("request", data));

      delay(500);
    }
  } while (request[0] != "200");

  JSONVar response = JSON.parse(request[1]);

  if (JSON.typeof(response) == "undefined") {
    logMessage("Error:\n  Message: Error creating JSON object.\n\n");
  } else {
    logMessage("Setting the following components:\n");
    
    // Inicializando os componentes
    for (int i = 0; i < response.length(); i++) {
      JSONVar component = response[i];

      if (String(component["kind"]) == "actuator") {
        components.push_back(new Actuator(component["id"], component["name"], component["port"], component["max_value"], component["min_value"]));
      }

      String data[3] = {String(component["name"]), String(component["port"]), String(component["kind"])};
      logMessage(lmh.handleMessage("component", data));
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
        // Caminho para verificar as saídas dos componentes
        path = serverAddress + "outputs.json";

        requestGET(path.c_str(), request);

        String data[3] = {path, request[0], request[1]};
        logMessage(lmh.handleMessage("request", data));
    
        JSONVar outputs = JSON.parse(request[1]);
    
        if (JSON.typeof(outputs) == "undefined") {
          logMessage("Error:\n  Message: Error creating JSON object.\n\n");
        } else {
          // Se existirem saídas
          if (outputs.length()) {
            logMessage("Sending the following outputs:\n");
            
            // Aplicando as saídas
            for (int i = 0; i < outputs.length(); i++) {
              JSONVar output = outputs[i];
              int index = searchComponentByID(output["component_id"]);
              Actuator *actuator = static_cast<Actuator*>(components[index]);
            
              int outputStatus = actuator->output(output["value"], output["kind"]);

              if (outputStatus) {
                String data[4] = {actuator->getName(), String(actuator->getPin()), String(output["kind"]), String(output["value"])};
                logMessage(lmh.handleMessage("output", data));
              }
            }
          } else {
            logMessage("No output records.\n\n");
          }
        }
      } else {
        logMessage("Error:\n  Message: WiFi has been disconnected");
      }
    }

    lastTime = millis();
  }
}

void logMessage(String message) {
  if (messagesAreEnabled) {
    Serial.print(message);
  }
}

int searchComponentByID(int id) {  
  for (int i = 0; i < components.size(); i++) {
    if (components[i]->getComponentId() == id) {
      return i;
    }
  }
}

void requestGET(const char* path, String request[]) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, path);

  int responseCode = http.GET();
  String response = "[]"; 
  request[0] = String(responseCode);
  request[1] = response;
  
  if (responseCode > 0) {
    response = http.getString();
    request[1] = response;
  }

  http.end();
}

void wifiSetup(char* ssid, char* password) {
  WiFi.begin(ssid, password);
  logMessage("\nConnecting to WiFi...\n");

  while(WiFi.status() != WL_CONNECTED) {
    logMessage("Waiting...\n");
    delay(500);
  }

  logMessage("\nConnected to WiFi network with IP Address: ");
  logMessage(WiFi.localIP().toString() + ".\n\n");
}
