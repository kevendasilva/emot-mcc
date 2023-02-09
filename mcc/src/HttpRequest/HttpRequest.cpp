#include "HttpRequest.h"

String HttpRequest::getResponse() {
  return response;
}

int HttpRequest::getResponseStatusCode() {
  return responseStatusCode;
}

void HttpRequest::get(const String &url) {
  http.begin(client, url);

  responseStatusCode = http.GET();

  if (responseStatusCode > 0) {
    response = http.getString();
  }

  http.end();
}

void HttpRequest::post(const String &url, const String &payload) {
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  responseStatusCode = http.POST(payload);
  
  if (responseStatusCode > 0) {
    response = http.getString();
  }

  http.end();
}