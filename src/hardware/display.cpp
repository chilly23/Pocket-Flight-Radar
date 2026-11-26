#include "hardware/display.h"
#include "hardware/display_font.h"
#include <Arduino.h>
#include <Preferences.h>
#include <esp_arduino_version.h>
#include <esp_heap_caps.h>
#include <esp_lcd_panel_io.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <math.h>

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error Select ESP32S3 Dev Module in Tools > Board.
#endif
#if ESP_ARDUINO_VERSION_MAJOR < 3
#error Install esp32 by Espressif Systems version 3.3.0 or newer.
#endif

// ----- confirmed-working hardware config -----
static constexpr int LCD_WIDTH = 466, LCD_HEIGHT = 466;
static constexpr int LCD_X_OFFSET = 6, LCD_Y_OFFSET = 0;
static constexpr int LCD_CS = 12, LCD_CLK = 48;
static constexpr int LCD_D0 = 13, LCD_D1 = 47, LCD_D2 = 21, LCD_D3 = 14;
static constexpr int LCD_RST = 11;
static constexpr int STRIPE_ROWS = 16;
static constexpr uint8_t BRIGHTNESS = 180;
#define BOOT_BTN_PIN 0





uint16_t *frame = nullptr, *stripe = nullptr;
esp_lcd_panel_io_handle_t io = nullptr;
SemaphoreHandle_t sent = nullptr;

LGFX tft;
void fatal(const char *msg) { Serial.println(msg); while (true) { delay(1000); Serial.println(msg); } }
void check(esp_err_t r, const char *where) {
  if (r != ESP_OK) {
    Serial.printf("%s: %s\n", where, esp_err_to_name(r));
    fatal("Stopped. Check board settings and press RST.");
  }
}
bool IRAM_ATTR transferDone(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *) {
  BaseType_t wake = pdFALSE;
  xSemaphoreGiveFromISR(sent, &wake);
  return wake == pdTRUE;
}
void command(uint8_t cmd, const void *data, size_t len) {
  check(esp_lcd_panel_io_tx_param(io, 0x02000000 | (uint32_t(cmd) << 8), data, len), "Display command");
}
void byteCommand(uint8_t cmd, uint8_t value) { command(cmd, &value, 1); }

void flush() {
  for (int y = 0; y < LCD_HEIGHT; y += STRIPE_ROWS) {
    int rows = (STRIPE_ROWS < (LCD_HEIGHT - y)) ? STRIPE_ROWS : (LCD_HEIGHT - y);
    int pixels = rows * LCD_WIDTH;
    const uint16_t *src = frame + y * LCD_WIDTH;
    for (int i = 0; i < pixels; i++) stripe[i] = (src[i] << 8) | (src[i] >> 8);
    int x0 = LCD_X_OFFSET, x1 = x0 + LCD_WIDTH - 1;
    int y0 = y + LCD_Y_OFFSET, y1 = y0 + rows - 1;
    uint8_t col[] = { uint8_t(x0 >> 8), uint8_t(x0), uint8_t(x1 >> 8), uint8_t(x1) };
    uint8_t row[] = { uint8_t(y0 >> 8), uint8_t(y0), uint8_t(y1 >> 8), uint8_t(y1) };
    command(0x2A, col, sizeof(col)); command(0x2B, row, sizeof(row));
    check(esp_lcd_panel_io_tx_color(io, 0x32002C00, stripe, pixels * 2), "Pixel transfer");
    if (xSemaphoreTake(sent, pdMS_TO_TICKS(1000)) != pdTRUE)
      fatal("Display DMA timeout. Check board variant and reset.");
  }
}

void initDisplay() {
  if (!psramFound()) fatal("Enable Tools > PSRAM > OPI PSRAM, then upload again.");
  frame = (uint16_t *)heap_caps_calloc(LCD_WIDTH * LCD_HEIGHT, 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  stripe = (uint16_t *)heap_caps_malloc(LCD_WIDTH * STRIPE_ROWS * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  sent = xSemaphoreCreateBinary();
  if (!frame || !stripe || !sent) fatal("Display buffer allocation failed.");

  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, LOW); delay(20);
  digitalWrite(LCD_RST, HIGH); delay(150);

  spi_bus_config_t bus = {};
  bus.sclk_io_num = LCD_CLK;
  bus.data0_io_num = LCD_D0; bus.data1_io_num = LCD_D1;
  bus.data2_io_num = LCD_D2; bus.data3_io_num = LCD_D3;
  bus.data4_io_num = -1; bus.data5_io_num = -1; bus.data6_io_num = -1; bus.data7_io_num = -1;
  bus.max_transfer_sz = LCD_WIDTH * STRIPE_ROWS * 2;
  check(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO), "QSPI bus");

  esp_lcd_panel_io_spi_config_t cfg = {};
  cfg.cs_gpio_num = LCD_CS; cfg.dc_gpio_num = -1;
  cfg.spi_mode = 0; cfg.pclk_hz = 40000000;
  cfg.trans_queue_depth = 1;
  cfg.on_color_trans_done = transferDone; cfg.user_ctx = nullptr;
  cfg.lcd_cmd_bits = 32; cfg.lcd_param_bits = 8;
  cfg.flags.quad_mode = true;
  check(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &cfg, &io), "QSPI display IO");

  byteCommand(0xFE, 0x00);
  byteCommand(0x36, 0x00);
  byteCommand(0xC4, 0x80);
  byteCommand(0x3A, 0x55);
  byteCommand(0x35, 0x00); delay(10);
  byteCommand(0x53, 0x20); delay(10);
  byteCommand(0x51, BRIGHTNESS); delay(10);
  byteCommand(0x63, 0xFF); delay(10);
  command(0x11, nullptr, 0); delay(120);
  flush();
  command(0x29, nullptr, 0); delay(20);
  Serial.println("CO5300 initialized: 466x466, RGB565, QSPI 40 MHz.");
}
void displayPresent() {
  // Preserve upstream 240x240 geometry and scale uniformly to the physical panel.
  for (int y = 0; y < LCD_HEIGHT; ++y)
    for (int x = 0; x < LCD_WIDTH; ++x)
      frame[y * LCD_WIDTH + x] = tft.readPixel(x * 240 / LCD_WIDTH, y * 240 / LCD_HEIGHT);
  flush();
}
void displayInit() {
  initDisplay();
  tft.setColorDepth(16);
  tft.setPsram(true);
  if (!tft.createSprite(240, 240)) fatal("UI canvas allocation failed");
  tft.setTextWrap(false);
  tft.fillScreen(0);
  displayFontInit();
  displayPresent();
}
