# Plane Radar for VIEWE 1.75-inch AMOLED

ESP32-S3 N16R8, 466x466 CO5300 QSPI. Default center: Bengaluru VOBL, 13.1979, 77.706299.

This port preserves upstream radar features and scales its 240x240 canvas to the full display. BOOT GPIO0 cycles range; hold 3 seconds to clear Wi-Fi/location settings. Touch is not used by the original radar interface.

## Build
Arduino ESP32 core 3.3.12; LovyanGFX 1.2.33; WiFiManager 2.0.17; ArduinoJson 7.4.2.
Run `python scripts/build_viewe.py`. Binaries are in `firmware-viewe`.
Use this Arduino CLI build, not the original C3 PlatformIO configuration.

## Setup
Connect your phone to PlaneRadar-Setup, open http://192.168.4.1, select your 2.4 GHz Wi-Fi and enter its password. Keep the prefilled Bengaluru coordinates and save. After connecting, http://plane-radar.local allows changing settings.
Live aircraft depend on internet access and ADS-B coverage; no radio receiver is needed.

## Hardware
CS 12, CLK 48, D0 13, D1 47, D2 21, D3 14, reset 11; 40MHz QSPI; X offset 6. OPI PSRAM enabled, flash 16MB. Display initialization adapted from the existing local CO5300 board example and checked against the vendor 1.75-inch example.

## One-click flash
Double-click `Flash Plane Radar.cmd`. It finds Espressif native-USB COM ports, probes the chip, and flashes only after detecting ESP32-S3. If multiple Espressif boards are connected it asks for the port. USB identification cannot identify the attached screen: use this image only on the VIEWE 1.75-inch CO5300 board. Firmware binaries are kept in firmware-viewe. No Wi-Fi passwords are embedded.

This port uses https://api.adsb.lol/v2/ because the original adsb.fi endpoint returned HTTP 403 during setup. The compatible endpoint returned aircraft data around VOBL.

