#ifndef USR_RENDER_H
#define USR_RENDER_H

#include "Utils.h"
#include "Config.h"

#include <U8g2lib.h>

#include <atomic>

class Render : public Tickable 
{
public:
  Render();

  void tick(unsigned long delataTime) override;

  void showText(const String& inText);
  void updateBackground();
  std::atomic<bool>& showSplashScreen();

  static Render& get();
private:
  void drawText(unsigned long delataTime);
  void drawBackground();
  void drawFPS(int FPS);

  void loadBackgroundImage();
  int countFPS(unsigned long delataTime);
  
private:
  uint8_t backgroundImage[DISPLAY_BUFFER_SIZE];
  bool bBackgroundImageAvailable;

  String text;
  int textX = DISPLAY_WIDTH + 2;
  float textSpeed = 24.f;
  std::atomic<bool> bBootComplete = false;
};

#endif