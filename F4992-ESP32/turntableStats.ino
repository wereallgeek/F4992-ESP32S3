//================================================
// Turntable stats Handler
//
// stores useless turntable statistics to flash
//
//================================================
Preferences ttStats;

enum statsType {
  START_MANUAL, START_WEB, START_CMD, START_MQTT,
  STOP_MANUAL, STOP_WEB, STOP_CMD, STOP_MQTT,
  AUTO_STOP, REPEAT, REJECT, TIMEOUT,
  PLAYCOUNT, SIZE17, SIZE30,
  BOOTCOUNT,
  MAXCOUNTER
};

const char* statsKeys[] = {
  "st_man", "st_web", "st_cmd", "st_mqtt",
  "sp_man", "sp_web", "sp_cmd", "sp_mqtt",
  "sp_auto", "rep_ct", "rej_ct", "tmout_ct",
  "ply_ct", "sz17", "sz30",
  "boot_ct"
};

const char* statsLabels[] = {
  "Start (manual): ", "Start (web): ", "Start (serial): ", "Start (mqtt): ",
  "Stop (manual): ", "Stop (web): ", "Stop (serial): ", "Stop (mqtt): ",
  "Stop (auto): ", "Repeat: ", "Rejected: ", "Timed out: ",
  "Playcount: ", "7\" Record: ", "12\" Record: ",
  "Bootcount: "
};

const char* statsIcons[] = {
  "mdi:play", "mdi:play-circle", "mdi:play-circle", "mdi:play-circle",
  "mdi:stop", "mdi:stop-circle", "mdi:stop-circle", "mdi:stop-circle",
  "mdi:stop-circle-outline", "mdi:repeat", "mdi:eject", "mdi:camera-timer",
  "mdi:counter", "mdi:disc", "mdi:record-circle",
  "mdi:power-cycle"
};

const int startStatType[] = {START_MANUAL, START_CMD, START_WEB, START_MQTT, REPEAT};
const int stopStatType[] = {STOP_MANUAL,   STOP_CMD,  STOP_WEB,  STOP_MQTT,  AUTO_STOP};


volatile uint32_t numberStats[MAXCOUNTER];

void stats_setup() {
  ttStats.begin("stats4992", false);
  for (int i = 0; i < MAXCOUNTER; i++) {
    numberStats[i] = ttStats.getUInt(statsKeys[i], 0);
  }
  incrementStat(BOOTCOUNT);
}

void incrementStat(int type) {
  numberStats[type]++; 
  ttStats.putUInt(statsKeys[type], numberStats[type]);
}

void incrementRepeat() {
  incrementStart(AUTO);
}

void incrementStart(int actionType) { 
  if (actionType < MANUAL || actionType > AUTO) return;
  incrementStat(startStatType[actionType]);
}

void incrementStop(int actionType) { 
  if (actionType < MANUAL || actionType > AUTO) return;
  incrementStat(stopStatType[actionType]);
}

void incrementRecordCount(int DiscSize) {
  if (DiscSize == DISC30) incrementStat(SIZE30);
  else if (DiscSize == DISC17) incrementStat(SIZE17);
}

void incrementReject() {
  incrementStat(REJECT);
}

void incrementTimeout() {
  incrementStat(TIMEOUT);
}

void incrementPlaycount() {
  incrementStat(PLAYCOUNT);
}

uint32_t getStat(int type) {
  if (type < 0 || type >= MAXCOUNTER) return 0;
  return numberStats[type];
}

String getStatKey(int type) {
  if (type < 0 || type >= MAXCOUNTER) return "";
  return String(statsKeys[type]);
}

String getStatLabel(int type) {
  if (type < 0 || type >= MAXCOUNTER) return "";
  return String(statsLabels[type]);
}

String getStatIcon(int type) {
  if (type < 0 || type >= MAXCOUNTER) return "";
  return String(statsIcons[type]);
}

uint32_t getBootcount() {
  return getStat(BOOTCOUNT);
}

int maxStatIndex() {
  return MAXCOUNTER;
}

void printStatsReport() {
  webSerialPrintln("--- Turntable Stats ---");
  for (int i = 0; i < MAXCOUNTER; i++) {
    if (strlen(statsLabels[i]) > 0) {
      webSerialPrintln(formattedStatsColumn(i));
    }
  }
}

String formattedStatsColumn(int index) {
  return String(statsLabels[index]) + numberStats[index];
}

void statsReset() {
  ttStats.clear(); 
  for (int i = 0; i < MAXCOUNTER; i++) numberStats[i] = 0;
  webSerialPrintln("Turntable statistics reset");
}
