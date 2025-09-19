# ESP32 LilyGO board GSM/GPRS/GPS Example

This project demonstrates how to use the **LilyGO T-A7670** (SIMCOM A7670-based) board with an ESP32 to access GSM/GPRS, make calls, send SMS, fetch USSD info, and obtain GPS/GNSS locations. It uses the **TinyGSM-fork library by lewisxhe**. It maybe used on other TinyGsm boards but results are not guaranteed.

---

## Table of Contents

* [Features](#features)
* [Hardware Requirements](#hardware-requirements)
* [Software Requirements](#software-requirements)
* [Installation](#installation)
* [Configuration](#configuration)
* [Usage](#usage)
* [Functions](#functions)
* [Troubleshooting](#troubleshooting)
* [References](#references)

---

## Features

* GSM/GPRS network registration and IP acquisition
* Send and receive SMS messages
* Make and hang up calls
* Play DTMF tones during calls
* USSD queries (balance, phone number, etc.)
* GPS/GNSS/GLONASS location acquisition
* HTTPS GET requests
* Chip temperature reading
* Automatic SIM unlocking if required

---

## Hardware Requirements

* **LilyGO T-A7670 board** (ESP32 + SIMCOM A7670)
* Micro SIM card with GPRS enabled
* C-type USB cable for programming and serial monitor

Optional:

* External GPS antenna for improved location accuracy
* 3.7v 18650 Battery

---

## Software Requirements

* Arduino IDE (>=1.8.19 recommended)
* **TinyGSM-fork library by lewisxhe** (not vshymanskyy/TinyGSM)
  * [GitHub Repo](https://github.com/lewisxhe/TinyGSM-fork)
* Arduino JSON (optional, for parsing JSON responses)
* StreamDebugger library (optional, for debugging AT commands)

> **Important:** Remove any existing `vshymanskyy/TinyGSM` library to prevent conflicts.

---

## Installation

1. Install **Arduino IDE** and configure ESP32 board support:

   * Board manager URL: `https://dl.espressif.com/dl/package_esp32_index.json`
2. Install **TinyGSM-fork** from GitHub (lewisxhe version).
3. Clone this project or copy the `main.ino` file into a new Arduino sketch.
4. Connect the board to your PC and select the correct **COM port**.
5. Set the correct **board** in Arduino IDE: `LilyGO T-A7670`.

---

## Configuration

Edit the top of the code to set your network and SIM info:

```cpp
#define SIM_PIN ""                   // Your SIM PIN (if any)
#define NETWORK_APN "jazzconnect.mobilinkworld.com" // APN of your operator
#define CALL_TARGET "+923145372506"  // Phone number to call
#define SMS_TARGET "+923145372506"   // Phone number to send SMS
```

> Replace `NETWORK_APN` with your operator's APN.

---

## Usage

1. Upload the code to your ESP32 board.
2. Open the Serial Monitor at **115200 baud**.
3. The board will initialize the modem, register on the network, and unlock the SIM if required.
4. Once registered, it will enable GPS and periodically attempt to fetch location, signal quality, and network info.
5. You can interact with the modem directly by typing commands in the Serial Monitor (if needed).

---

## Functions

### GPRS / Network

* `get_GPRS_details()` → Prints CCID, IMEI, IMSI, operator, local IP, and signal quality.
* `HTTPSGetRequest(String url)` → Performs an HTTPS GET request to the given URL.

### GSM / SMS / Calls

* `call(String number)` → Makes a call to the specified number and plays DTMF tones.
* `sendSMS(String number)` → Sends an SMS to the specified number.
* `printBalanceAndNumber()` → Retrieves SIM balance and phone number via USSD.

### Location

* `get_GSM_location()` → Obtains approximate GSM-based location.
* `get_GPS_location()` → Obtains GPS/GNSS/GLONASS location with status, coordinates, speed, altitude, and satellite info.

### Utility

* `get_GSM_Temp()` → Returns the modem chip temperature.

---

## Troubleshooting

* **Status code 715** → Firmware update may be required:
  [Update Firmware Guide](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/blob/main/docs/update_fw.md)
* **Network registration failed** → Check APN settings and SIM status.
* **GPRS/HTTPS failures** → Some websites may block ESP32 connections; try another URL.
* **SIM locked** → Ensure the correct `SIM_PIN` is provided.

> Use `#define DUMP_AT_COMMANDS` to debug AT commands with the StreamDebugger library.

---

## References

* [TinyGSM-fork by lewisxhe](https://github.com/lewisxhe/TinyGSM-fork)
* [LilyGO T-A7670 GitHub](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX)
* [Firmware Update Instructions](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/blob/main/docs/update_fw.md)
* [SIMCOM AT Command Reference](https://simcom.ee/documents/SIMCOM%20Module%20AT%20Command%20Manual/)

---

> **Note:** This code is experimental. Functions like GPS acquisition and HTTPS requests may require retries and proper antenna placement for reliable operation.
