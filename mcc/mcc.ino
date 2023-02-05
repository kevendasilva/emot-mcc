/*
 * Projeto emot
 * Desenvolvido por: Keven da Silva
 */

// Bibliotecas
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Classes personalizadas
#include "src/LogMessageHandler/LogMessageHandler.h"

// Variáveis usadas nas requisições HTTP
String path;
String request[2];
// Endereço do servidor
String serverAddress = "SERVER_ADDRESS";

// Delay entre requisições - Todos os tempos estão em milissegundos
unsigned long lagTime = 1500;
unsigned long lastTime = 0; // Variável de controle

// Components
JSONVar components;

// Variável de controle das mensagens
bool messagesAreEnabled = false;

// Manipulador das mensagens de log
LogMessageHandler lmh;


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

      String data[3];
      data[0] = path; data[1] = request[0]; data[2] = request[1];
      logMessage(lmh.handleMessage("request", data));

      delay(lagTime);
    }
  } while (request[0] != "200");

  components = JSON.parse(request[1]);

  if (JSON.typeof(components) == "undefined") {
    logMessage("Error:\n  Message: Error creating JSON object.\n\n");
  } else {
    logMessage("Setting the following components:\n");
    
    // Inicializando os pinos
    for (int i = 0; i < components.length(); i++) {
      JSONVar component = components[i];

      String componentName = component["name"];
      int pin = component["port"];
      String kind = component["kind"];
    
      String data[3];
      data[0] = componentName; data[1] = String(pin); data[2] = kind;
      logMessage(lmh.handleMessage("component", data));

      if (kind == "actuator") {
        pinMode(pin, OUTPUT);
      } else if (kind == "sensor") {
        pinMode(pin, INPUT);
      }
    }
  }
}

void loop() {
  // Verificando o tempo entre as requisições
  if ((millis() - lastTime) > lagTime) {
    // Caso existam componentes
    if (components.length()) {
      // Se ainda estiver conectado a rede WiFi
      if (WiFi.status() == WL_CONNECTED) {
        // Caminho para verificar as saídas dos componentes
        path = serverAddress + "outputs.json";

        requestGET(path.c_str(), request);

        String data[3];
        data[0] = path; data[1] = request[0]; data[2] = request[1];
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
              JSONVar component = searchComponentByID(output["component_id"]);

              int pin = component["port"];
              int outputValue = output["value"];
              String outputKind = output["kind"];


              String data[4];
              data[0] = String(component["name"]); data[1] = String(pin); data[2] = outputKind; data[3] = String(outputValue);
              logMessage(lmh.handleMessage("output", data));
       
              if (outputKind == "digital") {
                if (outputValue == 255) {
                  digitalWrite(pin, HIGH);
                } else {
                  digitalWrite(pin, LOW);
                }
              } else if (outputKind == "analog") {
                analogWrite(pin, outputValue);
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

JSONVar searchComponentByID(int id) {
  JSONVar component;
  
  for (int i = 0; i < components.length(); i++) {
    component = components[i];
    int componentId = component["id"];

    if (componentId == id) {
      return component;
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
