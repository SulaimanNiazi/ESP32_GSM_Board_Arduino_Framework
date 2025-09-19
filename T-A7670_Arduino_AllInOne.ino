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

  SimStatus sim = SIM_ERROR;
  while (sim != SIM_READY) {
    sim = modem.getSimStatus();
    switch (sim) {
      case SIM_READY:
        Serial.println("SIM card is online.");
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
    Serial.println("Failed to set network mode!");
  }
  String mode = modem.getNetworkModeString();
  Serial.print("Current network mode: ");
  Serial.println(mode);

#ifdef NETWORK_APN
  Serial.printf("Set network apn: %s\n", NETWORK_APN);
  modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
  if (modem.waitResponse() != 1) {
    Serial.println("Errorin setting network apn!");
  }
#endif

  // Check network registration status and network signal status
  Serial.print("Waiting for the modem to register with the network...");
  RegStatus status = REG_NO_RESULT;
  while (status == REG_NO_RESULT || status == REG_SEARCHING || status == REG_UNREGISTERED) {
    status = modem.getRegistrationStatus();
    switch (status) {
      case REG_UNREGISTERED:
      case REG_SEARCHING:
        int16_t sq = modem.getSignalQuality();
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

void get_GPRS_details(){
  Serial.print("Connecting to ");Serial.println(NETWORK_APN);
  if (!modem.gprsConnect(NETWORK_APN, "", "")) {
    delay(10000);
    return;
  }

  bool res = modem.isGprsConnected();
  Serial.print("GPRS status: "); Serial.println( res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  Serial.println("CCID: " + ccid);

  String imei = modem.getIMEI();
  Serial.println("IMEI: " + imei);

  String imsi = modem.getIMSI();
  Serial.println("IMSI: " + imsi);

  String cop = modem.getOperator();
  Serial.println("Operator: " + cop);

  IPAddress local = modem.localIP();
  Serial.print("Local IP: "); Serial.println(local);

  int csq = modem.getSignalQuality();
  Serial.printf("Signal quality: %d\n", csq);
}

void call(const String number){
  Serial.print("Calling: "); Serial.println(number);

  // This is NOT supported on M590
  bool res = modem.callNumber(number);
  Serial.print("Call: "); Serial.println(res ? "OK" : "fail");

  if (res) {
    delay(1000L);

    // Play DTMF A, duration 1000ms
    modem.dtmfSend('A', 1000);

    // Play DTMF 0..4, default duration (100ms)
    for (char tone = '0'; tone <= '4'; tone++) { modem.dtmfSend(tone); }

    delay(5000);

    res = modem.callHangup();
    Serial.print("Hang up: "); Serial.println(res? "OK" : "fail");
  }
}

void sendSMS(const String number){
  bool res = modem.sendSMS(number, String("Hello from ") + modem.getIMEI());
  Serial.print("SMS: "); Serial.println(res ? "OK" : "fail");
}

void get_GSM_location(){
  float lat      = 0;
  float lon      = 0;
  float accuracy = 0;
  int   year     = 0;
  int   month    = 0;
  int   day      = 0;
  int   hour     = 0;
  int   min      = 0;
  int   sec      = 0;
  for (int8_t i = 15; i; i--) {
    Serial.println("Requesting current GSM location");
    if (modem.getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour, &min, &sec)) {
      Serial.printf("Latitude: %.8f\nLongitude: %.8f\nAccuracy: %f\nYear: %d\nMonth: %d\nDay: %d\nHour: %d\nMinute: %d\nSecond: %d\n", 
        lat, lon, accuracy, year, month, day, hour, min, sec);
      break;
    } else {
      Serial.println("Couldn't get GSM location, retrying in 15s.");
      delay(15000L);
    }
  }
  Serial.println("Retrieving GSM location again as a string");
  String location = modem.getGsmLocation();
  Serial.println("GSM Based Location String: " + location);
}

float get_GSM_Temp(){
  float temp = modem.getTemperature();
  Serial.printf("Chip temperature: %d°C\n", int(temp));
  return temp;
}

void HTTPSGetRequest(const String url) {
  modem.https_begin(); // Initialize HTTPS

  int retry = 3;
  while (retry--) {
    Serial.println("Request URL : " + url);

    // Set GET URT
    if (!modem.https_set_url(url)) {
      Serial.print("Failed to request : "); Serial.println(url);
      // Debug
      // modem.sendAT("+CSSLCFG=\"enableSNI\",0,1");
      // modem.waitResponse();
      delay(3000);
      continue;
    }

    // Send GET request
    int httpCode = 0;
    httpCode = modem.https_get();
    if (httpCode != 200) {
      Serial.print("HTTP get failed ! error code = ");
      Serial.println(httpCode);
      delay(3000);
      continue;
    }

    // Get HTTPS header information
    String header = modem.https_header();
    Serial.print("HTTP Header : ");
    Serial.println(header);
    delay(1000);

    // Get HTTPS response
    String body = modem.https_body();
    Serial.print("HTTP body : ");
    Serial.println(body);
    delay(3000);
    break;
  }
  Serial.println("-------------------------------------");
}

// Experimental Functions Begin

void printBalanceAndNumber(){
  String ussd_balance = modem.sendUSSD("*111#");
  Serial.print("Balance (USSD): "); Serial.println(ussd_balance);

  String ussd_phone_num = modem.sendUSSD("*161#");
  Serial.println("Phone number (USSD): " ); Serial.println( ussd_phone_num);
}

void get_GPS_location(){
  Serial.println("Enabling GPS/GNSS/GLONASS and waiting for GPS to be enabled...");
  modem.enableGPS();
  while(!modem.isEnableGPS());
  Serial.println("GPS enabled.");
  uint8_t status = 0;
  float lat2 = 0, lon2 = 0, speed2 = 0, alt2 = 0, accuracy2 = 0;
  int vsat2 = 0, usat2 = 0, year2 = 0, month2 = 0, day2 = 0, hour2 = 0, min2 = 0, sec2 = 0;
  for (int8_t i = 15; i; i--) {
    Serial.println("Requesting current GPS/GNSS/GLONASS location");
    if (modem.getGPS(&status, &lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                     &year2, &month2, &day2, &hour2, &min2, &sec2)) {
      Serial.printf("Status: %s\nLatitude: %.8f\nLongitude: %.8f\nSpeed: %f\nAltitude: %f\nVisible Satellites: %d\nUsed Satellites: %d\nAccuracy: %f\nYear: %d\nMonth: %d\nDay: %d\nHour: %d\nMinute: %d\nSecond: %d\n", 
                    (status == 0) ? "No Fix" : ((status == 1) ? "2D Fix" : "3D Fix"), lat2, lon2, speed2, alt2, vsat2, usat2, accuracy2, year2, month2, day2, hour2, min2, sec2);
      break;
    } else {
      Serial.println("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
      delay(15000L);
    }
  }
  Serial.println("Retrieving GPS/GNSS/GLONASS location again as a string");
  String gps_raw = modem.getGPSraw();
  Serial.println("GPS/GNSS Based Location String: " + gps_raw);
  Serial.println("Disabling GPS");
  modem.disableGPS();
}

// Experimental Functions End
