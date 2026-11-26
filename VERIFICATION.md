# Verification

- Arduino ESP32 3.3.12, LovyanGFX 1.2.33, WiFiManager 2.0.17, ArduinoJson 7.4.2.
- Build passed: 1,336,343 bytes program storage (42% of 3MB), 62,104 bytes static RAM (18%).
- Automatic flasher detected COM10 and verified ESP32-S3 with 8MB PSRAM.
- Bootloader, partition table, boot app data and application uploaded; each hash verified.
- Serial boot confirmed CO5300 initialized at 466x466, RGB565, QSPI 40MHz; Wi-Fi connection started.
- ADS-B alternative API returned 13 aircraft around VOBL from the PC during testing.
- Physical display appearance and live fetch on the device require Wi-Fi setup and user confirmation.

Firmware checksums: firmware-viewe/SHA256.json.
