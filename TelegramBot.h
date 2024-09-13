#ifndef USR_TELEGRAM_BOT_H
#define USR_TELEGRAM_BOT_H

#include "Utils.h"
#include <FastBot.h>

void handleMessageExternal(FB_msg& msg);

class TelegramBot : public Tickable 
{
public:
  TelegramBot();

  void tick(unsigned long delataTime) override;

  void handleMessage(FB_msg& msg);

  static TelegramBot& get();
private:
  void downloadFile(FB_msg& msg);

private:
  FastBot bot;
};

#endif
