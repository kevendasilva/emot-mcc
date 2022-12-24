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
      Serial.print("For ");
      Serial.println(path);
      Serial.print("HTTP Response code: ");
      Serial.println(request[0]);
    }
  } while (request[0] != "200");

  components = JSON.parse(request[1]);

  if (JSON.typeof(components) == "undefined") {
    Serial.println("Parsing input failed!");
  } else {
    Serial.println("");
    Serial.println("Setting the following components: ");
    
    // Inicializando os pinos
    for (int i = 0; i < components.length(); i++) {
      JSONVar component = components[i];

      Serial.print("Component: ");
      Serial.println(component);

      int pin = component["port"];
      String kind = component["kind"];

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
        Serial.print("For ");
        Serial.println(path);
        Serial.print("HTTP Response code: ");
        Serial.println(request[0]);

        requestGET(path.c_str(), request);
    
        JSONVar outputs = JSON.parse(request[1]);
    
        if (JSON.typeof(outputs) == "undefined") {
          Serial.println("Parsing input failed!");
        } else {
          // Se existirem saídas
          if (outputs.length()) {
            // Aplicando as saídas
            for (int i = 0; i < outputs.length(); i++) {
              JSONVar output = outputs[i];
              JSONVar component = searchComponentByID(output["component_id"]);

              int pin = component["port"];
              int outputValue = output["value"];
              String outputKind = output["kind"];
       
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
          }
        }
      }
    }

    lastTime = millis();
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
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
