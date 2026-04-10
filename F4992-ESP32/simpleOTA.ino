//============================================================
// Simple OTA for ESP32
// 
// Opens a webserver onto which a .bin file can be uploaded
//  to the ESP for update. 
//
// Uses update.h
//
// let's build geeky things.
//                   We're all geeks
//
//============================================================
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>


//OTA==================================================================
AsyncWebServer updateserver(4992);

const char* updateIndex = 
"<form method='POST' action='/doupdate' enctype='multipart/form-data'>"
  "<h3>Let's build geeky things</h2>"
  "<p></p>"
  "<h2>Firmware update</h3>"
  "<p></p>"
  "<input type='file' name='update' accept='.bin'>"
  "<input type='submit' value='UPDATE'>"
"</form>";
//OTA==================================================================
void setupOTA(AsyncWebServer &otaServer, const char* path) {
    //OTA update  
    // 1. form
    otaServer.on(path, HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", updateIndex);
    });
    // 2. ending upload and reboot
    otaServer.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest *request){
       bool failure = Update.hasError();
      request->send(200, "text/plain", failure ? "Error" : "SUCCES : Reboot...");
      delay(1000);
      if(!failure) ESP.restart();
    }, 
    // 3. manage .bin
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if (!index) { // 1st pack
        Serial.printf("Update start: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
          Update.printError(Serial);
        }
      }
      if (len) { // write each part in flash
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
      }
      if (final) { // done. validate
        if (Update.end(true)) {
          Serial.printf("Update end: %u bytes\n", index + len);
        } else {
          Update.printError(Serial);
        }
      }
    });
}

void simpleOTAsetup() {
  setupOTA(updateserver, "/");
}
void simpleOTAbegin() {
  updateserver.begin();
}
  

//OTA==================================================================
