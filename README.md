# Microcontroller code - emot <!-- omit in toc -->

- [**Sobre**](#sobre)
- [**Funcionamento**](#funcionamento)
  - [Variáveis](#variáveis)
  - [Bloco Setup](#bloco-setup)
  - [Bloco Loop](#bloco-loop)
- [**NodeMCU**](#nodemcu)
  - [Limitações](#limitações)

## **Sobre**

Código usado no microcontrolador do projeto **The Amazing Flower Monitor** (emot). A descrição complementa do projeto está disponível em [emot-test](https://github.com/kevendasilva/emot-test#sobre).

## **Funcionamento**

### Variáveis

No código, inicialmente tem-se a definição de duas variáveis: `RESPONSE_STATUS_LED` e `WIFI_STATUS_LED`. Ambas variáveis são usadas para manipulação do estados de dois LEDs, que são usados para representar o estado das respostas dadas pelo servidor, e o estado da conexão wifi, respectivamente.

Em seguida, deve-se fornecer o endereço do servidor (`SERVER_ADDRESS`). No caso, como esta é a versão de teste, o servidor estará disponível apenas na rede local. Logo, deverá ser usado o IP e a porta da máquina que está rodando o servidor. 

A variável `lagTime` é usado para definir um intervalo de tempo entre as requisições. Ela define o tempo de solicitação das informações das saídas que devem ser aplicados nos atuadores, e também define o tempo em que o sensores devem enviar suas informações de leitura, para o servidor.

### Bloco Setup

No bloco `setup` deste código, temos as principais configurações do projeto. Inicialmente, os LEDs de notificação são configurados. Em seguida, fornecemos as credenciais da conexão wifi, para que a conexão seja realizada.

Logo após, temos o seguinte laço de repetição:

```c++
// Solicitando informações sobre os componentes
do {
  if (wifiIsConnected()) {
    request.get(url);
    body = request.getResponse();
    responseStatusCode = request.getResponseStatusCode();

    delay(500);
  }
} while (responseStatusCode != 200);
```

A aplicação fica presa neste loop, até que o servidor responda com o status *200*. Finalizada a requisição e com as informações sobre os componentes disponíveis, segue-se para criação dos objetos de cada componente, de acordo com o tipo atribuído (atuador ou sensor).

Criando os objetos de acordo com o tipo:

```c++
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
```

### Bloco Loop

Nesse bloco realizamos as operações de entrada e saída de informações, dos componentes. Inicialmente, temos um bloco dedicado a solicitação de informações sobre a saídas desejadas, para os atuadores.

```c++
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

    // O valor `-1` foi utilizado para indicar quando um objeto não foi encontrado na lista de atuadores.
    if (index != -1) {
      int outputStatus = actuators[index]->output(output["value"], output["kind"]);
    }
  }
}
```
Logo em seguida, realizamos o envio dos valores apresentados pelo sensores.

```c++
// Sensores   
for (int i = 0; i < sensores.size(); i++) {
  String value = String(sensores[i]->read());
  String componentId = String(sensores[i]->getComponentId());

  String url = SERVER_ADDRESS + "/readings.json";
  String payload = "reading[component_id]=" + componentId + "&" + "reading[value]=" + value;

  request.post(url, payload);
}
```

## **NodeMCU**

As portas usadas na criação de componentes, a partir da interface (APP descrito no repositório *emot-test*), deve corresponder aos pinos disponíveis no microcontrolador. Abaixo segue uma descrição simplificada dos pinos do microcontrolador usado neste projeto.

<div align="center">
  <img height="240em" src="assets/img/nodemcu-pinout.png">
  <p>Descrição dos pinos esp8266 NodeMCU.</p>
</div>

### Limitações

O microcontrolador usado neste projeto (apresentado acima), possui apenas uma porta analógica (A0), o que não é um número razoável de portas analógicas. Portanto, o uso de sensores fica limitado a este número.