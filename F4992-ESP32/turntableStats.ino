//================================================
// Turntable stats Handler
//
// stores useless turntable statistics to flash
//
//================================================
Preferences ttStats;

enum statsType {
  START_MANUAL, STOP_MANUAL,
  START_CMD, STOP_CMD,
  START_WEB, STOP_WEB,
  START_MQTT, STOP_MQTT,
  REPEAT, AUTO_STOP,
  REJECT, PLAYCOUNT,
  SIZE17, SIZE30,
  BOOTCOUNT,
  MAXCOUNTER
};

const char* statsKeys[] = {
  "st_man", "sp_man",
  "st_cmd", "sp_cmd",
  "st_web", "sp_web",
  "st_mqtt", "sp_mqtt",
  "rep_ct", "sp_auto",
  "rej_ct", "ply_ct",
  "sz17", "sz30",
  "boot_ct"
};

const char* statsLabels[] = {
  "Man Start: ", "Man Stop: ",
  "Cmd Start: ", "Cmd Stop: ",
  "Web Start: ", "Web Stop: ",
  "MQTT Start: ", "MQTT Stop: ",
  "Repeat: ", "Auto Stop: ",
  "Reject: ", "Playcount: ",
  "7\" Record: ", "12\" Record: ",
  "Bootcount: ", ""
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

void incrementPlaycount() {
  incrementStat(PLAYCOUNT);
}

uint32_t getStat(int type) {
  return numberStats[type];
}

void printStatsReport() {
  webSerialPrintln("--- Turntable Stats ---");
  for (int i = 0; i < MAXCOUNTER; i += 2) {
    String line = formattedStatsColumn(i);
    if (i + 1 < MAXCOUNTER && strlen(statsLabels[i + 1]) > 0) {
      line += "  " + formattedStatsColumn(i + 1);
    }
    webSerialPrintln(line);
  }
}

String formattedStatsColumn(int index) {
  String output = String(statsLabels[index]) + numberStats[index];
  int targetWidth = 19; 
  while (output.length() < targetWidth) output += " ";
  return output;
}

void statsReset() {
  ttStats.clear(); 
  for (int i = 0; i < MAXCOUNTER; i++) numberStats[i] = 0;
  webSerialPrintln("Turntable statistics reset");
}
