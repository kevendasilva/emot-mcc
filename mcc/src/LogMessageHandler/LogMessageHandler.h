#ifndef LOG_MESSAGE_HANDLER_H
#define LOG_MESSAGE_HANDLER_H

class LogMessageHandler {
  public:
    String handleMessage(String type, String data[]);

  private:
    String requestMessage(String data[]);
    String componentMessage(String data[]);
    String outputMessage(String data[]);
};

#endif