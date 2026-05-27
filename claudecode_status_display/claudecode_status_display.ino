#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "HWCDC.h"
#include "pin_config.h"

HWCDC USBSerial;

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus,
  LCD_RST,
  0,
  true,
  LCD_WIDTH,
  LCD_HEIGHT,
  0,
  20,
  0,
  0
);

enum StatusMode {
  MODE_IDLE,
  MODE_THINKING,
  MODE_APPROVAL,
  MODE_WORKING,
  MODE_DONE,
  MODE_ERROR
};

StatusMode currentMode = MODE_IDLE;
String serialLine;
unsigned long lastFrameMs = 0;
uint8_t frameIndex = 0;

const char *thinkingFrames[] = {
  "  .  \n .o. \n..o..\n .o. \n  .  ",
  " .o. \n.oOo.\n oOo \n.oOo.\n .o. ",
  "..o..\n oOo \nOoOoO\n oOo \n..o.."
};

const char *workingFrames[] = {
  "[=   ]",
  "[==  ]",
  "[=== ]",
  "[====]",
  "[ ===]",
  "[  ==]",
  "[   =]"
};

void drawCenteredText(const char *title, const char *body, uint16_t bg, uint16_t fg, uint16_t accent) {
  gfx->fillScreen(bg);
  gfx->setTextWrap(false);

  gfx->fillRect(0, 0, LCD_WIDTH, 34, accent);
  gfx->setTextSize(2);
  gfx->setTextColor(BLACK);
  gfx->setCursor(10, 9);
  gfx->print("Claude Code");

  gfx->setTextColor(fg);
  gfx->setTextSize(3);
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;
  gfx->getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor((LCD_WIDTH - w) / 2, 58);
  gfx->print(title);

  gfx->setTextSize(2);
  gfx->getTextBounds(body, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor((LCD_WIDTH - w) / 2, 108);
  gfx->print(body);
}

void drawAsciiBlock(const char *title, const char *art, uint16_t bg, uint16_t fg, uint16_t accent) {
  drawCenteredText(title, "", bg, fg, accent);
  gfx->setTextSize(2);
  gfx->setTextColor(fg);
  gfx->setCursor(42, 128);
  gfx->print(art);
}

void showStatus(StatusMode mode) {
  switch (mode) {
    case MODE_IDLE:
      drawCenteredText("READY", "send: thinking / approval", BLACK, WHITE, CYAN);
      break;
    case MODE_THINKING:
      drawAsciiBlock("THINKING", thinkingFrames[frameIndex % 3], 0x0008, WHITE, YELLOW);
      break;
    case MODE_APPROVAL:
      drawCenteredText("APPROVE?", "waiting for you", 0x4208, WHITE, ORANGE);
      gfx->setTextSize(5);
      gfx->setTextColor(YELLOW);
      gfx->setCursor(92, 154);
      gfx->print("?");
      break;
    case MODE_WORKING:
      drawCenteredText("WORKING", workingFrames[frameIndex % 7], 0x0011, WHITE, GREEN);
      break;
    case MODE_DONE:
      drawCenteredText("DONE", "task finished", 0x0200, WHITE, GREEN);
      break;
    case MODE_ERROR:
      drawCenteredText("ERROR", "check Claude / serial", 0x4000, WHITE, RED);
      break;
  }
}

StatusMode parseStatus(String text) {
  text.trim();
  text.toLowerCase();

  if (text == "thinking" || text == "think") return MODE_THINKING;
  if (text == "approval" || text == "approve" || text == "permission") return MODE_APPROVAL;
  if (text == "working" || text == "tool" || text == "busy") return MODE_WORKING;
  if (text == "done" || text == "stop" || text == "idle") return MODE_DONE;
  if (text == "error" || text == "fail" || text == "failed") return MODE_ERROR;
  return MODE_IDLE;
}

void setup() {
  USBSerial.begin(115200);

  if (!gfx->begin()) {
    USBSerial.println("gfx->begin() failed");
  }

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  showStatus(currentMode);
  USBSerial.println("Claude Code status display ready.");
  USBSerial.println("Try: thinking, approval, working, done, error");
}

void loop() {
  while (USBSerial.available() > 0) {
    char c = USBSerial.read();
    if (c == '\n' || c == '\r') {
      if (serialLine.length() > 0) {
        currentMode = parseStatus(serialLine);
        frameIndex = 0;
        showStatus(currentMode);
        USBSerial.print("status=");
        USBSerial.println(serialLine);
        serialLine = "";
      }
    } else {
      serialLine += c;
    }
  }

  if ((currentMode == MODE_THINKING || currentMode == MODE_WORKING) && millis() - lastFrameMs > 500) {
    lastFrameMs = millis();
    frameIndex++;
    showStatus(currentMode);
  }
}
