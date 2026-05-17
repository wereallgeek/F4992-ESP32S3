#include <ESPUI.h>
#include <deque>

// Configuration 
const int WS_MAX_LINES = 10;
std::deque<String> webLog;

SemaphoreHandle_t logMutex = xSemaphoreCreateMutex();
volatile bool haschanged = true;

void wsprint(String s) {
  Serial.print(s);
  if (!debugTabVisible()) return;
  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(5)) == pdTRUE) { //no doubledipping
    if (webLog.empty()) {
      webLog.push_back(s);
    } else {
      webLog.back() += s;
    }
    wsMaintainWebLog();
    haschanged = true;
    xSemaphoreGive(logMutex); //unlock
  }
}

void wsprintln(String s) {
  Serial.println(s);
  if (!debugTabVisible()) return;
  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(5)) == pdTRUE) { //no doubledipping
   if (webLog.empty()) {
      webLog.push_back(s);
    } else {
      webLog.back() += s;
    }
    webLog.push_back("");
    wsMaintainWebLog();
    haschanged = true;
    xSemaphoreGive(logMutex); //unlock
  }
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

bool computeWebserialDirty() { return haschanged; }

void wsMaintainWebLog() {
  while (webLog.size() > (WS_MAX_LINES + 1)) { //+1 for last linefeed
    webLog.pop_front();
  }
}

void updateWebSerial() {
  if (!firstPassCompleted || !debugTabVisible() || !haschanged) return;


  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(5)) == pdTRUE) {

    String buffer = "";

    for (const String& s : webLog) {
      if (s.length() > 0) buffer += s + "\n";
    }
    haschanged = false;
    xSemaphoreGive(logMutex); //unlock

    ESPUI.updateLabel(logLabelId, buffer);
    if (buffer.length() > 0) {
        ESPUI.updateLabel(logLabelId, buffer);
    }
  }
}
