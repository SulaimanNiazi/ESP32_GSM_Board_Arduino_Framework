#define SIM_PIN ""
#define NETWORK_APN "jazzconnect.mobilinkworld.com"

#define TINY_GSM_RX_BUFFER                    1024    // Set RX buffer to 1Kb

#define LILYGO_T_A7670
#if defined(LILYGO_T_A7670)
  #define MODEM_BAUDRATE                      115200
  #define MODEM_DTR_PIN                       25
  #define MODEM_TX_PIN                        26
  #define MODEM_RX_PIN                        27
  #define BOARD_PWRKEY_PIN                    4       // The modem boot pin needs to follow the startup sequence.
  #define BOARD_ADC_PIN                       35
  #define BOARD_POWERON_PIN                   12      // The modem power switch must be set to HIGH for the modem to supply power.
  #define MODEM_RING_PIN                      33
  #define MODEM_RESET_PIN                     5
  #define BOARD_MISO_PIN                      2
  #define BOARD_MOSI_PIN                      15
  #define BOARD_SCK_PIN                       14
  #define BOARD_SD_CS_PIN                     13
  #define BOARD_BAT_ADC_PIN                   35
  #define MODEM_RESET_LEVEL                   HIGH
  #define SerialAT                            Serial1
  #define MODEM_GPS_ENABLE_GPIO               (-1)
  #define MODEM_GPS_ENABLE_LEVEL              (-1)
  #ifndef TINY_GSM_MODEM_A7670
    #define TINY_GSM_MODEM_A7670
  #endif
#endif

#include <TinyGsmClient.h>
#ifndef TINY_GSM_FORK_LIBRARY
  #error "The correct library was NOT found. You must install TinyGSM-fork by lewisxhe - https://github.com/lewisxhe/TinyGSM-fork"
#endif

// #define DUMP_AT_COMMANDS // See all AT commands, if wanted
#ifdef DUMP_AT_COMMANDS  // if enabled it requires the StreamDebugger lib
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

void setup() {
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

  pinMode(BOARD_POWERON_PIN, OUTPUT);
  digitalWrite(BOARD_POWERON_PIN, HIGH);

  // Set modem reset pin, reset modem
  pinMode(MODEM_RESET_PIN, OUTPUT);
  digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
  digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
  digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

  pinMode(BOARD_PWRKEY_PIN, OUTPUT);
  digitalWrite(BOARD_PWRKEY_PIN, LOW);
  delay(100);
  digitalWrite(BOARD_PWRKEY_PIN, HIGH);
  delay(100);
  digitalWrite(BOARD_PWRKEY_PIN, LOW);

  Serial.print("Starting modem...");

  int retry = 0;
  while (!modem.testAT(1000)) {
    Serial.print(".");
    if (retry++ > 10) {
      digitalWrite(BOARD_PWRKEY_PIN, LOW);
      delay(100);
      digitalWrite(BOARD_PWRKEY_PIN, HIGH);
      retry = 0;
    }
  }
  Serial.println();

  // Check if SIM card is online
  SimStatus sim = SIM_ERROR;
  while (sim != SIM_READY) {
    sim = modem.getSimStatus();
    switch (sim) {
      case SIM_READY:
        Serial.println("SIM card online");
        break;
      case SIM_LOCKED:
        Serial.println("The SIM card is locked. Unlocking the SIM card using the pin...");
        modem.simUnlock(SIM_PIN);
        break;
      default:
        break;
    }
    delay(1000);
  }

  if (!modem.setNetworkMode(MODEM_NETWORK_AUTO)) {
    Serial.println("Set network mode failed!");
  }
  String mode = modem.getNetworkModeString();
  Serial.print("Current network mode: ");
  Serial.println(mode);

#ifdef NETWORK_APN
  Serial.printf("Set network apn: %s\n", NETWORK_APN);
  modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
  if (modem.waitResponse() != 1) {
    Serial.println("Set network apn error !");
  }
#endif

  // Check network registration status and network signal status
  int16_t sq ;
  Serial.print("Wait for the modem to register with the network.");
  RegStatus status = REG_NO_RESULT;
  while (status == REG_NO_RESULT || status == REG_SEARCHING || status == REG_UNREGISTERED) {
    status = modem.getRegistrationStatus();
    switch (status) {
      case REG_UNREGISTERED:
      case REG_SEARCHING:
        sq = modem.getSignalQuality();
        Serial.printf("[%lu] Signal Quality: %d\n", millis() / 1000, sq);
        delay(1000);
        break;
      case REG_DENIED:
        Serial.println("Network registration was rejected, please check if the APN is correct");
        return ;
      case REG_OK_HOME:
        Serial.println("Online registration successful");
        break;
      case REG_OK_ROAMING:
        Serial.println("Network registration successful, currently in roaming mode");
        break;
      default:
        Serial.printf("Registration Status: %d\n", status);
        delay(1000);
        break;
    }
  }
  Serial.println();

  Serial.printf("Registration Status: %d\n", status);
  delay(1000);

  String ueInfo;
  if (modem.getSystemInformation(ueInfo)) {
    Serial.println("Inquiring UE system information: " + ueInfo);
  }

  if (!modem.setNetworkActive()) {
    Serial.println("Enable network failed!");
  }
  delay(5000);

  String ipAddress = modem.getLocalIP();
  Serial.println("Network IP: " + ipAddress);
}

void loop() {
  // Debug AT
  if (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
  if (Serial.available()) {
    SerialAT.write(Serial.read());
  }
  delay(1);
}

