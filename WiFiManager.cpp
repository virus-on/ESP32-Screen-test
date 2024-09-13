#include "WiFiManager.h"
#include "Utils.h"
#include "Config.h"

#include <Arduino.h>
#include <WiFi.h>

WiFiManager::WiFiManager()
  : Tickable(10 * 1000) // Once per 10 seconds
{
  WiFi.setAutoReconnect(true);
}

void WiFiManager::connect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int maxWaitingTime = 1000 * 30; // 30 seconds
  while (WiFi.status() != WL_CONNECTED && maxWaitingTime > 0)
  {
    delay(1000);
    maxWaitingTime -= 1000;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    _USR_LOG("\nWiFi connected. IP address: ");
    Serial.print(WiFi.localIP());

    updateTime();
  }
  else
  {
    _USR_WARNING("Timeout error!");
  }
}

void WiFiManager::tick(unsigned long delataTime)
{
  if (!WiFi.isConnected())
  {
    WiFi.reconnect();
  }
}

void WiFiManager::updateTime()
{
  // Time zone offset for Kyiv is UTC+2 hours and +1 hour during DST
  const long gmtOffset_sec = 2 * 3600; // UTC+2 hours in seconds
  const int daylightOffset_sec = 3600; // 1 hour for daylight saving time

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org"); // Kyiv Time (UTC+2 with DST)
  time_t now = time(nullptr);

  int maxWaitingTime = 1000 * 30; // 30 seconds
  while (now < 24 * 3600 && maxWaitingTime > 0)
  {
    delay(500);
    now = time(nullptr);
    maxWaitingTime -= 500;
  }
}
