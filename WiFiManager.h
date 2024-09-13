#ifndef USR_WIFI_MANAGER_H
#define USR_WIFI_MANAGER_H

#include "Utils.h"

class WiFiManager : public Tickable
{
public:
  WiFiManager();

  void connect();

  void tick(unsigned long delataTime) override;

private:
  void updateTime();
};

#endif
