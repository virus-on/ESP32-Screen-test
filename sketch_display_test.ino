#include "Render.h"
#include "WiFiManager.h"
#include "TelegramBot.h"
#include "Utils.h"

#include <FS.h>
#include <SPIFFS.h>

TickManager GTickManager;

void handleMessageExternal(FB_msg& msg)
{
  TelegramBot::get().handleMessage(msg);
}

void setup()
{
  Serial.begin(115200);
  _USR_LOG("\nBOOT STARTED!\n");

  Render* render = &Render::get();
  std::atomic<bool>& bBootComplete = render->showSplashScreen();

  if (!SPIFFS.begin())
  {
    SPIFFS.format();
    SPIFFS.begin();
  }

  WiFiManager* WiFi = new WiFiManager();
  WiFi->connect();
  GTickManager.add(WiFi);

  GTickManager.add( &TelegramBot::get() );
  GTickManager.add( &LoopQueue::get() );
  GTickManager.add( render );

  _USR_LOG("BOOT COMPLETE!");
  bBootComplete = true;
}

void loop() {
  GTickManager.tick();
}