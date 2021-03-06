// Function that printf and related will use to print
int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}

const int gNumberOfRelays = sizeof(gRelayConfig) / sizeof(RelayConfigDef);
const int gNumberOfButtons = sizeof(gButtonConfig) / sizeof(ButtonConfigDef);


int getRelayNum(int sensorId) {
  
  if (sensorId > -1) {
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      if (gRelayConfig[relayNum].sensorId == sensorId) return(relayNum);
    }
  }
  return(-1);
}
