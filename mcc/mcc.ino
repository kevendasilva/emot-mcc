/*
 * Projeto emot
 * Desenvolvido por: Keven da Silva
 */

// Bibliotecas
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

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
bool enabledMessages = false;

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

      String message;
      message = "Request send for: " + path + "\n";
      
      if (request[0] == "-1") {
        message += "Error:\n  Message: The server is offline.\n\n";
      }

      logMessage(message);

      delay(lagTime);
    }
  } while (request[0] != "200");

  components = JSON.parse(request[1]);

  String message = "Response:\n  Code: " + request[0] + "\n" + "  Body: " + request[1] + "\n\n";
  logMessage(message);

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

      String message;
      message = "Component:\n";
      message += "  Name: " + componentName + "\n";
      message += "  Port: " + String(pin) + "\n";
      message += "  Kind: " + kind + "\n\n";
      logMessage(message);

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
        
        String message;
        message = "Request send for: " + path + "\n";

        if (request[0] == "-1") {
          message += "Error:\n  Message: The server is offline.\n";
        }
        
        message += "Response:\n  Code: " + request[0] + "\n" + "  Body: " + request[1] + "\n\n";
        logMessage(message);
    
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

              String message = "Output:\n";
              message += "  Component: " + String(component["name"]) + "\n";
              message += "  Port: " + String(pin) + "\n";
              message += "  Kind: " + outputKind + "\n";
       
              if (outputKind == "digital") {
                if (outputValue == 255) {
                  digitalWrite(pin, HIGH);
                  message += "  Value: HIGH\n\n";
                } else {
                  digitalWrite(pin, LOW);
                  message += "  Value: LOW\n\n";
                }
              } else if (outputKind == "analog") {
                analogWrite(pin, outputValue);
                message += "  Value: " + String(outputValue) + "\n\n";
              }

              logMessage(message);
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
  if (enabledMessages) {
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
