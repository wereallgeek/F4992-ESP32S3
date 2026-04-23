#include <ESPUI.h>
#include <deque>

// Configuration 
const int WS_MAX_LINES = 14;
std::deque<String> webLog;

SemaphoreHandle_t logMutex = xSemaphoreCreateMutex();
bool haschanged = true;

void wsprint(String s) {
  Serial.print(s);
  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(10)) == pdTRUE) { //no doubledipping
    if (webLog.empty()) {
      webLog.push_back(s);
    } else {
      webLog.back() += s;
    }
    xSemaphoreGive(logMutex); //unlock
    wsMaintainWebLog();
  }
  haschanged = true;
}

void wsprintln(String s) {
  Serial.println(s);
  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(10)) == pdTRUE) { //no doubledipping
   if (webLog.empty()) {
      webLog.push_back(s);
    } else {
      webLog.back() += s;
    }
    webLog.push_back("");
    xSemaphoreGive(logMutex); //unlock
    wsMaintainWebLog();
  }
  haschanged = true;
}


void webSerialPrint(String s)   { wsprint(s); }
void webSerialPrintln(String s) { wsprintln(s); }

void webSerialPrint(const char* c)   { wsprint(String(c)); }
void webSerialPrintln(const char* c) { wsprintln(String(c)); }

void webSerialPrint(bool b)   { wsprint(b ? "true" : "false"); }
void webSerialPrintln(bool b) { wsprintln(b ? "true" : "false"); }

void webSerialPrint(int i)   { wsprint(String(i)); }
void webSerialPrintln(int i) { wsprintln(String(i)); }

void webSerialPrint(long unsigned int l)   { wsprint(String(l)); }
void webSerialPrintln(long unsigned int l) { wsprintln(String(l)); }

void webSerialPrint(IPAddress ip)   { wsprint(ip.toString()); }
void webSerialPrintln(IPAddress ip) { wsprintln(ip.toString()); }

bool webserialDirty() { return haschanged; }

void wsMaintainWebLog() {
  while (webLog.size() > WS_MAX_LINES) {
    webLog.pop_front();
  }
}

void updateWebSerial() {
  if (!firstPassCompleted) return;

  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(10)) == pdTRUE) {

    String buffer = "";
    buffer.reserve(2600); 

    for (const String& s : webLog) {
      buffer += s + "\n";
    }
    xSemaphoreGive(logMutex); //unlock

    ESPUI.updateLabel(logLabelId, buffer);
  }
  haschanged = false;
}
