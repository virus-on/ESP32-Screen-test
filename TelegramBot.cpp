#include "TelegramBot.h"
#include "Render.h"
#include "Config.h"
#include "Utils.h"

#include <HTTPClient.h>
#include <SPIFFS.h>
#include <FS.h>


TelegramBot::TelegramBot() : Tickable(2000)
{
  bot.setToken(BOT_TOKEN);
  bot.attach(handleMessageExternal);
}

void TelegramBot::tick(unsigned long delataTime)
{
  AsyncTaskQueue::get().push([this](){
    bot.tick();
  });
}

void TelegramBot::handleMessage(FB_msg& msg)
{
  Serial.println(msg.toString());

  if (msg.isFile)
  {
    downloadFile(msg);
  }
  else
  {
    String messageText = "@" + msg.username + ": " + msg.text;
    messageText.replace("\n", " ");
    Render::get().showText(messageText);
    bot.sendMessage(messageText, msg.chatID);
  }
}

void TelegramBot::downloadFile(FB_msg& msg) 
{
  Serial.print("Downloading ");
  Serial.println(msg.fileName);

  bool OK = false;
  String path = BACKGROUND_IMAGE_PATH;
  File f = SPIFFS.open(path, "w");

  if (f) 
  {
    Serial.println("Opened File");
    HTTPClient http;
    http.begin(msg.fileUrl);

    if (http.GET() == HTTP_CODE_OK) 
    {
      Serial.println("Made request");
      if (http.writeToStream(&f) > 0) 
      {
        Serial.println("Wrote to file!");
        OK = true;
      }
    }
    http.end();
    f.close();
    Render::get().updateBackground();
  }
  Serial.println(OK ? "OK" : "Error");
}

TelegramBot& TelegramBot::get()
{
  static TelegramBot telegramBot;
  return telegramBot;
}

