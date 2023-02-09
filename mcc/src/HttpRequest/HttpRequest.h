#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

class HttpRequest {
  private:
    WiFiClient client;
    HTTPClient http;
    int responseStatusCode;
    String response;

  public:
    String getResponse();
    int getResponseStatusCode();
    void get(const String &url);
    void post(const String &url, const String &payload);
};

#endif