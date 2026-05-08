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

#define OTAPORT 4992
#define EMERPORT 80


const char* otalogo = "<img src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGQAAABACAYAAADs39J0AAAABGdBTUEAALGOfPtRkwAAACBjSFJNAAB6JQAAgIMAAPn/AACA6AAAdTAAAOpgAAA6lwAAF2+XqZnUAAAVwUlEQVR4nGL8//8/wxAHfkB8HogfD7RDqAEAAohpoB1AIWAF4k4gDhhoh1ALAAQQvSIEmz2MRIox4zHXDIg1gNgHhx1DDgAEEAud7DEH4mog/gvlgwJvBRAvBeJuIFZngETGJCDeDcSzgFgCqg6UAw7jMFcdSisDMTsQf6eB2+kKAAKIXhHyAIgdgJgbSWwqlBYCYl8ouwpKS0DFngPxFTzmvoPSH4D4DxXcOeAAIIDolc1BATsJiQ8K5P1Q9i4ofRCIL0PZK6D0ZCB+j8fcT1D6IxD/ptyZAw8AAoie5e5MIP4KZf+HYhDgRFIDq0M4oPRBAmZ+hNLfKHbdIAEAAUTPCHkJxSAgxYAovuShtCwDIiJ0oPQTAmZ+RaOHPAAIIHrVISDwkwHSV1ACYh4gFmeAFEewCBEBYj4GSMUMqqRB9cMbBkhlDco52DpMP6A0SA8op/2D8kHq/zCg1iugJjILkhoGJLWg4u4vAybgRLIX2TwuqDn/ofoZGRANCnT3guz9isP9GAAggOgZISAHPYWy2YBYEohvMEByBgiAPAmKJFAuUmSANARAnnRlgNQ/yIEL8vBnIC6C8kFyTUDsxYAIpC9AfASI+xkgOQ2UK9cxQHLhfzSzyoB4KxY3RwBxOQMksjYDcQVUPBGIc6H2gprlCxggrUEQsALiaVA9oBJoHxAXMBDZ6AAIIAZQT52OuO0/AsRCxe4giXkBMTMQfwXilVB5fiC+9x8T/AJicyDeBMTxQByIRQ0InAZiPiBmBOL1ONTA3IKO5YD4PVTNHiRxKSB+ChX/C8QmSHI8QHwdyWxXHGZjxQABRO/O1DMkNiiHgFKrKJIYKIcIM0Byy0OoGKjiXgdlg7L+JSB+wQApMkDNXdDQyUIGSP8FNnxyFIjXQtkmDJCePChXrISKgfReA+K7UD624goEHgHxTiibnwFS5MH8sQHKBuXCM0h6viDZg9yaJAoABBC9IwR5vAkU+KD6hAtJTAYqBgL3kcRh5fM5IDYF4nQGRNkNA6B6ANYMBgVCCBBvh/KjoTRyI8AFiO0YIAHIisfNsMYHKOGwIYnDWnafGDDBFygNKlZJGiwECCB6RwioDoGVpaDOnyoDItWBACgyNKHsR0jiMHeCUvIvID7NAMkdyMMqjEjqYJEMS8WgSARVtsiBA4pAUEo/xoBo3aEDUERJQ9kCDJDGCLqbsIUhshy24SCcACCABiJCYCkLFCHqaPKgyIA1eR8yYAKY514B8VUG1BSLDdyA0qBUDgpQ5BYWLCHswaOfA+pOGFuYgH0UA4AAoneEgFL1GyhbDogNGCARBCuSQK0rQwZIkfIMQzcCgHLKBQbCEQJrboIiElexBCoGP+OQE2SA1B0MULtEcaijGgAIIHo2e0EA1G+A9UVAEQJKcWcZIEWMMQNkXAs0EAkKpA84zAAFLijVgooaQj10Qaj6fwyIPgsMgIosUHGELSfCAEj+K9QeESimKQAIIHrnEFBqhVXsoBQHCjBQqwnWSgElEFDkgCac0DtwIADKGaDix5MB0qp6hEUNA5JecygNqnhBEQzz73+oWc4MkIg6h8McUCMD1C+CFX1iONRRDQAE0EDMIaAPh9xhgDRBkcFdBuwAFJCgOsaIAdIc/ohDHagCB82VJEP5oEYArBMHAqAIA3VInRggLSJcA5igCHkNxPegfJrnEIAAGogIuYXGB0UQekrHlfJBvWBQj1qAgB1pDJC+iCKUvxRKw/wLGqIB5TBQpKEXZcgANKwD6vPAEogMAXs58cixMxAxiQYQQAMRIdcZUDtiIA+DWk3IY0a45sdBHhYiwg6Q51mg9oD6IhvQ5EH+FoeqwddPUIa6D+YeeTxqQUABSmMzEyTHhUUcBQAE0EBEyG0GRBEBGnAE5ZCnUDYIgIoQXBUtKOJA9Q3Mw/imd0HgFBAHM2BW/qAK/TBUHGQGtnAARRZoevgxA2IMDjYziQ0wQeUZGFAjBMZWZEDtc2EFAAE0EBECamK+QGKDmsFvGRC9W1Aliq33CwIXgTiOAVKug5qg4jjUgfSDAgLUksPW3AW1nOIZIC01UI6TxqKGFyoH6uXDBhVFoeLYACfUPhBA7gyC3AGKdFCdRXCAESAAI3ZwAyAIQ2GYxBkcwDiGszsLDuDZs/lie5Em2iOUBFqg/+tf7EVE9MEaG6KWBRV99MIflgpa1SOy1q1TnL2OKw5AdyiaR/hUNsW81sjWHiCo9AqUVvgFmkrfX/MuoleqT4X05jZ+k4gqcTwtxeHZRuO7FOMSggyh/mdCbgFEKIeAsmcCEB9igAyigSrHGUA8D4i3MUCmXDcyQJqPMACaC29lwF82P0GiQS0eUMTBioXHePSCIgTUiQTNJII6lbjKZOSIMsIiD+sogswBDYcIYlEDSlSghPiTAVGc4uscCuOQ+w+1g6gcAhBA+CIElAJB893zGSDteWy5CRTzoNFW2EqRLCBewwApSrD1I2DgMRoNArBIwtXCQgagIkkejx2gyS1Y38YUhxpQ5IL6JqCWEzcWeVgx5gHElgyQ4hUUXrgiBJQ7cCUQkFmglh3BgUaAAMJVZIEiYwsD4eYlDIBSXCoS/ycuhVDwHEo/QBKDpWhC07YgAHI3viIAVAyChlZiGSA5BFTU4GregophbKMCklDzT0D1gtSAimtcfRFQAsHV7FVgQJ3RxAkAAghbDgG1LEDj+dgiA1S2g2bOJjJAiixc8wiEAKxYuookBiuy8A1lwABofAmUInFFCCj1w3IIKKDUsKiBpVZ9BuwjsqCcA6pnYDN/sHoDV4SoMOAeNQYlHm4GInIIQAChRwioVQFaggPLrsgpHRRQoOlUUBEFmpL0ZoBkZ2ICEB2AcgGoWEHuocOavtiKLOQ+CgiAikRQuf8PTQ1MHaisB1XsoGIGlJtMkdTAaFjT25IBe46WhrrpN9Set1BxWJEFsxu2/AhUp8ES6H80NQoMhJvoYAAQQOgRAgpofSgbNBf8AMoGORhUJB1CUw8aug5HciyxAKT+Lpo+UKsNFLmvsaiHpWDQ/EkLEDcwQCpl9CIA1sQFeR7UtIWNQTlCaVhAgVJyBwNkvh1U2WJr1YGasE+R+LAcAosQmJtACQNUxIPCDRaxsHAFVeagxKCHww4MABBAyPO50kD8DjoPfAWIa6Hz1iCwkMBcsA+SWhBYS0C9ABBPBGJOtPnrBUDMhKYWNMd+FMs8+A8gVkdS5wCd3waBM1CxKVD+TyBWA+J2LOaAQDSanSBz/wDxISSxzVC1j4BYAohPIOn/B6VXQdXuQpKDuWk3gTABY4AAQs4hCQyI5h9oURsoK7NCU9UcAvEKagBMJyoFQACo6AH1opFXG4JS1wEGzFQPSn1mDIgiCYaR6y9Qao1hgKRMkByozwQa99oONQ9UhIFWpQRhMQe5qIMBUKcRlMssoBhUwdtA1YHYoPAwQNIHyy3foXbbIckhz3YSBAABBFqJAaJBS2RArRJQdgRlc1BvGFRk8UBpVwbCLQSQXtAwNqgyBC1KCMZnLwOknf8cSQwU+SJoYiAAKlKQhyT+MiDmOECjuLCiALSYAbaOChQID6HYGspHL8P/MiAmr0B1GXLxCQpUHqg+0EgvqPgDRQAsAYGKvJ8MmMuJQMUuqEWmyoBan8Cmix9gCQsUABBAsGYvqGMHKxtBLSwpBsT88XoGIpprDJCyfwEQ12CRA7XY/jCgVqqgZiRy+x+bGAiAxq9g/RVGqJt/Q9kcSOpBcyjIqRCUK0ARdIwBEUnI/gDJ/YDKsaPZe4kBEfEgcdCwzj4sZiAD2EAlSB96wwQUtm5AnM8ACQtQgwaUeEF1MGw1JygHngEIIFiEuEBpUPt9AQNiHgFk+GkcDsAGZjNAGgbIU6uw4gS0sp1QxQZLscgAxAetlO9igOSCuQyY8+PEAJA5oID2Z4D0J0CL71wJuAnkj0YGSBEOaoGCOr3SWOwGRShoCAnUMb6JJA6yB1RUgvyPrbkMyjUgv4FKAVCjwB4ggEAVCdt/xMKuE9DKBbag7C0QCxNTGSHhqVgqMFDlvQ1HhUoM6EQyK5wCcy4AMet/RCPiAQH164CYHcnuFBzqQIv9LP6j+pkFiJeS4DZQw4UJIIBAWVCBATGOfwpKw8rsBwy4Z+VwgcUMmINvoMoOtE5qFYlmwQByUQQqUjMZiG1GogLkRgSoWAHllps41C5igCwlRe6jgCrzZWjqQAOXoGb1CTRxUM6IQuKDctVJBsh4IGj87yWaenCuAwggUJGlxYDoYV6C0lJQGlR2k7oRBrRaD1TMgbI7cqCBikNQSywMix7QIN8mBkRxBaqAQet2cQ2vgwIGtOZWAU0c1CCYiOZmUEfWDoc5oOF8UHHUhyb+B2oOeqSDiiA9JD5osg20KvILmjrQ6EAeEh+UEEDDOGuR3AZq/ID6QtHIGgECCBQhCkh8UBkOWhvFB+WjxyIxANQiOQA1G91DuHqroOlW9EABNcErsKgFAVhzHB2A3NuJJgZKuQfx2I1rOAPbOB8oEegg8UEbitAjAwRAOQa5kQCqvFeiqQGNVoBas6AcmAQTBAggUJGlhKQogQEyugvr8eLbvYQLgDwIakJj2++HaxUftlk4UAMB174PXOYwYzEL1Mo6gkM9PrPQxRUYICveYQC0NmAhDr0GaPxLWFVBiilQSQBqLoMHJgECCBQhyBP3oKaXORIf18wdIQDbO0EJALX/N1JoBgPUHV0MiKYvuQBUb/Eh8acw4F4Xhl7U4lutAqqjQaUDeK0yQACBsqUUHsXYUjmoDAU1k4nZhAJKsaDIWcJA3i4nUJMwioHIgTk8ADQyDSq2JMnUD0q0aUh8UAkwF4969FFf0DoyUKRgm2kEAdBcEmi0mB0ggEARgryAGNT5Aa23hY32YttICerUgDo57ngchAzqGAjPj2ADoEg4zgApfwmu1sADYJ05UBFYyUDi4mcoABUryNMRID/hWzWJnvhAiR60/Ru0ah9b6xCUS0DTyewAAQRyLKzyAtUXoQwQR8MAts4XqDICNRfX4HEQDBQCcTMD6S01UCWnywDJgSD97/ArxwlAZgRB2aDiDzRajW/tFDYAaqEh1x2g2dEtBPRgm5JIYICMrWGbmwEB0DqBrwABhFymgipDCQbEam98AJTiQb35ozjkQREJ2tY1gQiz0AGoeAD1jmHFFGiiaRsZ5oBADwMkUkAAlGrnk6AXVhyDRh6QW1wTGAgX1biWpoJWSoKmxbE1/UEZ4hdAAIEiBJaFQMUCaNyqFkkRvrIbVOGD2tbYmn2glJiLRRwXQM7GoOIAFCmwKVdQ0XCSSHNACQFWPIIWW4CKVuSV7aDIJbajC4pA0CCjD5IYqC7cQYReUNGPayoa1EcBNYFXM6C2cMEAIIBY0ByInp3w7SwCAVCkYKtnQAONpIw1gTwOilxQ7gRtqCT3VAbQyg9QzgX17GEbQsmZZga5HZRAQYmTFUlsJgNx/gKl9j4GzL4VMgCNXIAmr0DF8z6YIEAAgSIEeeUHyPHIuQK5wscGQGpJPTAGG/CAYhggpxEAAqCcRWjuhhgAypWgiEXOHaDSBDSlgK9PgwxARRuok52KRw0ot4CWuSYwQPdRAgQQyJIHSApAWXIGEp+fYWQCUC8b1ItGT2yg0VwdTOVYAaieyWFALPTGBUDDMcsZoGvbAAIINpEDA6AKB7TIDZZCiVnYTA3wmQH3fDopAFR8PoZiUofnkQEohxzAIg4an4slwRxQ3ZjAADkJCd8qe5C5oNYkG0AAgSLkNpIEqEPzmgFRyeLrNFITgFo/2gyQIQfQkATy8AcnA+ZQBC4A0guaVwAVFTOwyIM6tcTkelC9ASr6sFXMoLkNYtergQCoPmxjgPTbcLW+QAA0Ze4KEECgCAENP8OGSEBTj6DcAevYgCZOCO3jowaA2QmasAENW8Nm60AA1Em1J9IckL5PULM6oGxYrxlUr4EChZj1zCA/g0ZyJ2GRAyVSfNPToMjSxiIOanmCJsRm4dHrARBAoAh5AbUcBECpCyQGm9cGRQjNNzoyoI4xgcpTUCcOtsrcgYHwvgwYgE3xggCo2AJNJ8P6DKCABPUDiD3kDJRLQb17bJ1SUOcZV04DNYRCcMiBzAL11kGLKLC1/owAAogJKnEcKgCaIgU1HWG7nECVmwYhl1MZgMr+UgZE3QZa90Wo+Y0LgFIj7BQIUI8blNuIbVKDcglojn89FjnQBB6ucx5BdQWodYYvIYMmv5qxiMsABBAsZcJ6wiBDQL1IWEcMlOKc8BhMKwAa6QUVX6DJM9BAJrlH94GKQljiAg33oB88gw/AisypDNib4RkM2Is/UP0LqsMIVf6geZvraGLcAAEEixDQbiLYmD0oO4GW88NaKaAIwrY6nB7AFUqTu4YYBkDFCyhh4Wvp4AKg1Szok0sgAJqmsMQiDopwUC4sZsB/0ADILehnSTICBBATkmQHlA0qtkBdethZh6BhYR8G+gNQCvWGsimdWwHNjYMCh5x5eBDoZcDMJSD3peBQDwpPUJ1FaPgI/cCCDwABhFyZgnqMt6AWgbrzoKlPWC7JYsDeI6flKcyg8h5bCiQHwEZ8yXUvqPTAdqoPqB5RwaMP1DGUxSMvh8a/AxBAyBECKqcboGxQex00vgSbUAFViCVYDCR1iANXgGAzB1Rc4Rq6wWXOXyxmgVKqFRluQhfH1q8BzSBWYRGHAVCuBK3CwXbgAKhh4IwmtgcgAB93cANACAIB0KquB7uym2vzMg+TlRz69EGICSwsixWUlJx0tChnxFd2u0DI8Ao1TkxgAqas7JYg/073c6jHV0llFM6LnS71iCqEXso4AeyNk+s6+uqrwodM6in3ZkhrnGrFtKmHkn2A/N5356d5fDIhSN73E0DYFrpx/4ecnoYLvALi80D8Ao8abKvfQQvH5pGwcAwZtKGZBVqt/p0Mc0Cny8EWyoGwKhCfw6F28n/MlfjTcKhd8R+xkh90otwTLGqeAfFyIF4GxA+xyNeA9AMEEK7Vh6DVivvx+QwLAC27/whlo0cIaEvBYhLNQwYdSGa5/Ufd+kAKACUkWISA/HiZgPrZ/xGRogXEH/Co3fofsgqUA4hfQsX+EOmuvv+QMGIACEDJHeMAAIIwAPQpPsL/v83cgGGwMTIaEg1DgVJMNAIluLqdLora4sXdEMYhJUEZxrhDBBjSnUqA5gS/ibWWb5gVZXSGfJe+VErmHTplcAb7VU5z5PEwf02pnszWcRU2N3/noEmepdVC2YB9cRSXFe7AlVGvHC3ZFoCTO0YBGIShMJyLCb13dw/TA3RyKB8mtFMFBRdBySC+/yVqPUf4a/Jbqm9HTKLgSerLV7cv0McZr5gSK5jMUI0MluHcvRpk/p2dn1A42sVX8TiruXBCuzKcBT3ghmauMsg2OOSVy+o5JgXTYoIR4qKV9UpY9fH6LvAIMAAwCcIvq9upxwAAAABJRU5ErkJggg==' style='width:100px; margin-bottom:20px;'>";
const char* otastyle =   "<style>"
  "body{background-color:#121212;color:#ffffff;font-family:sans-serif;display:flex;flex-direction:column;justify-content:center;align-items:center;height:100vh;margin:0;}"
  ".loader{border:4px solid #333;border-top:4px solid #4caf50;border-radius:50%;width:30px;height:30px;animation:spin 1s linear infinite;margin:20px auto;}"
  "@keyframes spin{0%{transform:rotate(0deg);}100%{transform:rotate(360deg);}}"
  "</style>";

//OTA==================================================================
AsyncWebServer updateserver(OTAPORT);
AsyncWebServer emergencyserver(EMERPORT);

String updateIndex = String(otastyle) + "<form method='POST' action='/doupdate' enctype='multipart/form-data' style='text-align:center;' onsubmit='showWait()'>"
                   + String(otalogo)  + "<h5>F4992-ESP32S3 turntable controller Firmware update</h5>"
  "<p style='font-size:0.8em;color:#aaaaaa;'>current fw version = " + String(firmwareVersion()) + "</p>"
  "<div id='fcontent'>"
    "<input type='file' name='update' accept='.bin' style='margin-bottom:20px;' required>"
    "<br>"
    "<input type='submit' value='UPDATE' style='padding:10px 20px;cursor:pointer;'>"
  "</div>"
  "<div id='wait' style='display:none;'>"
    "<div class='loader'></div>"
    "<h4>Uploading firmware...</h4>"
    "<h5>Please wait, do not close this page</h5>"
  "</div>"
  "</form>"

  "<script>"
  "function showWait(){"
    "document.getElementById('fcontent').style.display='none';"
    "document.getElementById('wait').style.display='block';"
  "}"
  "</script>";

//OTA==================================================================
void setupOTA(AsyncWebServer &otaServer, const char* path) {
    otaServer.on(path, HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", updateIndex);
    });

    otaServer.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest *request){
      bool failure = Update.hasError();
      String statusColor = failure ? "#f44336" : "#4caf50";
      String statusText = failure ? "Update Failed" : "Update Success: Rebooting...";
      
      String html = "<html><body style='font-family:sans-serif; background-color:#121212; color:white; display:flex; justify-content:center; align-items:center; height:100vh; margin:0;'>";
      html += "<div style='text-align:center;'>";
      html += "<h4 style='color:" + statusColor + ";'>" + statusText + "</h4>";
      if(!failure) html += "<h5 style='color:#888;'>Device will be back online in a few seconds</h5>";
      html += "</div></body></html>";

      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
      response->addHeader("Connection", "close");
      request->send(response);

      if(!failure) {
        xTimerStart(xTimerCreate("reboot", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, [](TimerHandle_t xTimer){
          ESP.restart();
        }), 0);
      }
    }, 
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if (!index) {
        otaupdateInProgress  = true;
        setCpuFrequencyMhz(240);
        WiFi.setSleep(false);
        client.disconnect();

        Serial.printf("Update start: %s\n", filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

        if (!Update.begin(maxSketchSpace, U_FLASH)) { 
          Update.printError(Serial);
        }
      }
      if (len) {
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
        delay(1);
      }
      if (final) {
        if (Update.end(true)) {
          Serial.printf("Update end: %u bytes\n", index + len);
        } else {
          Update.printError(Serial);
          otaupdateInProgress = false; 
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
  
void emergencyServerSetup() {
  emergencyserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head>" + String(otastyle) + "</head><body>";
    html += String(otalogo);
    html += "<h2>F4992-ESP32S3 turntable controller Recovery Panel</h2>";
    html += "<a href='http://" + WiFi.localIP().toString() + ":" + String(OTAPORT) + "' style='color:#4caf50;text-decoration:none;border:1px solid #4caf50;padding:10px 20px;border-radius:5px;'>Go to Firmware Update</a>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  emergencyserver.begin();
}
//OTA==================================================================
