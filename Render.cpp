#include "Render.h"
#include "Config.h"
#include "Utils.h"

#include <FS.h>
#include <SPIFFS.h>

// BMP header offsets
#define BMP_WIDTH_OFFSET 18
#define BMP_HEIGHT_OFFSET 22
#define BMP_DATA_OFFSET 10
#define BMP_BIT_DEPTH_OFFSET 28

// Create a display object for I2C communication (address 0x3C for SSD1315)
DISPLAY_TYPE u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, SDA_PIN, SCL_PIN);

uint16_t read16(File &f) 
{
  uint16_t result;
  f.read((uint8_t *)&result, sizeof(result));
  return result;
}

uint32_t read32(File &f) 
{
  uint32_t result;
  f.read((uint8_t *)&result, sizeof(result));
  return result;
}

unsigned char reverseBits(unsigned char b) 
{
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

Render::Render()
  : Tickable(0)
{
  memset(backgroundImage, 0, DISPLAY_BUFFER_SIZE);
  bBackgroundImageAvailable = false;
}

void Render::tick(unsigned long delataTime)
{
  u8g2.clearBuffer();

  drawBackground();
  drawFPS(
    countFPS(delataTime)
  );
  drawText(delataTime);

  u8g2.sendBuffer();
}

void Render::showText(const String& inText)
{
  if (text.length() != 0)
  {
    auto textLengthPix = 0;
    do {
      text += "        ";
      textLengthPix = u8g2.getStrWidth(text.c_str());
    } while(textLengthPix + textX < 130);
  }

  text += inText;
}

void Render::updateBackground()
{
  bBackgroundImageAvailable = false;
}

void Render::drawText(unsigned long delataTime)
{
  static float accumulatedMovement = 0;

  // early exit nothing to draw 
  if (text.length() == 0)
  {
    return;
  }

  auto textLengthPix = u8g2.getStrWidth(text.c_str());
  // early exit finished showing text
  if (textX + textLengthPix < -5)
  {
    text = "";
    textX = 130;
    return;
  }

  u8g2.drawBox(0, DISPLAY_HEIGHT - 12, DISPLAY_WIDTH, 10);
  u8g2.setDrawColor(0);
  u8g2.drawStr(textX, DISPLAY_HEIGHT - 1, text.c_str());
  u8g2.setDrawColor(1);

  accumulatedMovement += (delataTime * textSpeed) / 1000;
  int movement = accumulatedMovement;
  accumulatedMovement -= movement;
  textX -= movement;
}

void Render::drawBackground()
{
  if (!bBackgroundImageAvailable)
  {
    loadBackgroundImage();
  }

  u8g2.drawXBM(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, backgroundImage);
}

void Render::loadBackgroundImage()
{
  File bmpFile = SPIFFS.open(BACKGROUND_IMAGE_PATH, "r");
  if (!bmpFile)
  {
    _USR_WARNING("Failed to open BMP file");
    return;
  }

  // Read BMP header information
  bmpFile.seek(BMP_WIDTH_OFFSET, SeekSet);  // Seek to width
  _USR_LOG("SEEK BMP_WIDTH_OFFSET(%d) Actual %d", BMP_WIDTH_OFFSET, bmpFile.position());
  int16_t bmpWidth = read32(bmpFile);

  bmpFile.seek(BMP_HEIGHT_OFFSET, SeekSet);  // Seek to height
  _USR_LOG("SEEK BMP_HEIGHT_OFFSET(%d) Actual %d", BMP_HEIGHT_OFFSET, bmpFile.position());
  int16_t bmpHeight = read32(bmpFile);

  bmpFile.seek(BMP_BIT_DEPTH_OFFSET, SeekSet);  // Seek to bit depth
  _USR_LOG("SEEK BMP_BIT_DEPTH_OFFSET(%d) Actual %d", BMP_BIT_DEPTH_OFFSET, bmpFile.position());
  uint16_t bitDepth = read16(bmpFile);
  
  if (bitDepth != 1 || bmpWidth != DISPLAY_WIDTH || bmpHeight != DISPLAY_HEIGHT) 
  {
    _USR_LOG("Not a calidm BMP %d %dx%d", bitDepth, bmpWidth, bmpHeight);
    bmpFile.close();
    return;
  }

  bmpFile.seek(BMP_DATA_OFFSET, SeekSet);
  _USR_LOG("SEEK BMP_DATA_OFFSET(%d) Actual %d", BMP_DATA_OFFSET, bmpFile.position());
  uint32_t dataOffset = read32(bmpFile);
  bmpFile.seek(dataOffset, SeekSet);
  _USR_LOG("SEEK dataOffset(%d) Actual %d", dataOffset, bmpFile.position());

  // Row size calculation for BMP (each row is padded to a multiple of 4 bytes)
  int bytesPerRow = DISPLAY_WIDTH / 8;

  for (int i = DISPLAY_HEIGHT - 1; i >= 0; --i)
  {
    bmpFile.read(&(backgroundImage[i * bytesPerRow]), bytesPerRow);
  }

  for (int i = 0; i < DISPLAY_BUFFER_SIZE; ++i)
  {
    backgroundImage[i] = reverseBits(backgroundImage[i]);
  }

  bBackgroundImageAvailable = true;
  bmpFile.close();
}

int Render::countFPS(unsigned long delataTime)
{
  return 1000 / delataTime;
}

void Render::drawFPS(int FPS)
{
  char buffer[16];

  sprintf(buffer, "FPS: %d", FPS);
  int width = u8g2.getStrWidth(buffer) + 2;
  int height = 12;
  u8g2.drawBox(7, 6, width, height);
  u8g2.setDrawColor(0);
  u8g2.drawStr(8, 16, buffer);
  u8g2.setDrawColor(1);
}

void showWelcomeMessage(TimerHandle_t xTimer) {
  Render::get().showText("WELCOME!");
  xTimerDelete(xTimer, 0);
}

std::atomic<bool>& Render::showSplashScreen()
{
  bBootComplete = false;
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(2, 10, "Boot");
  u8g2.sendBuffer();

  AsyncTaskQueue::get().push([&bBootComplete = bBootComplete](){
    String bootString = "Boot";

    auto startTime = millis();
    auto delta = 1000;
    do {
      auto current = millis();
      if (current - startTime > delta)
      {
        bootString += '.';
        u8g2.drawStr(2, 10, bootString.c_str());
        u8g2.sendBuffer();
        startTime = current;
        _USR_LOG("BOOT SCREEN UPDATED");
      }
    } while(!bBootComplete);
    _USR_LOG("BOOT SCREEN TURNED OFF");

    auto timerHandle = xTimerCreate("Welcome delay", pdMS_TO_TICKS(1000 * 3), pdFALSE, (void*)0, showWelcomeMessage);
    xTimerStart(timerHandle, 0);
  });

  return bBootComplete;
}

Render& Render::get()
{
  static Render render;
  return render;
}

