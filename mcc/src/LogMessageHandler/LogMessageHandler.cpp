#include <Arduino.h>
#include "LogMessageHandler.h"

String LogMessageHandler::handleMessage(String type, String data[]) {
  if (type == "request") {
    return (requestMessage(data));
  } else if (type == "component") {
    return (componentMessage(data));
  } else if (type == "output") {
    return (outputMessage(data));
  }
}

String LogMessageHandler::requestMessage(String data[]) {
  String message;

  message = "Request send for: " + data[0] + "\n";
  
  if (data[1] == "-1") {
    message += "Error:\n  Message: The server is offline.\n";
  } else {
    message += "Response:\n  Code: " + data[1] + "\n" + "  Body: " + data[2] + "\n\n";
  }

  return message;
}

String LogMessageHandler::componentMessage(String data[]) {
  String message;

  message = "Component:\n";
  message += "  Name: " + data[0] + "\n";
  message += "  Port: " + data[1] + "\n";
  message += "  Kind: " + data[2] + "\n\n";

  return message;
}

String LogMessageHandler::outputMessage(String data[]) {      
  String message;
  
  message = "Output:\n";
  message += "  Component: " + data[0] + "\n";
  message += "  Port: " + data[1] + "\n";
  message += "  Kind: " + data[2] + "\n";

  if (data[2] == "digital") {
    message += data[3] == "255" ? "  Value: HIGH\n\n" : "  Value: LOW\n\n";
  } else if (data[2] == "analog") {
    message += "  Value: " + data[3] + "\n\n";
  }

  return message;
}