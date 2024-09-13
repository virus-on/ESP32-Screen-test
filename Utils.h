#ifndef USR_UTILS_H
#define USR_UTILS_H

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <functional>
#include <vector>

#define _USR_LOG(...)                    \
  Serial.printf("LOG(:%d): ", __LINE__); \
  Serial.printf( __VA_ARGS__ );          \
  Serial.println();
#define _USR_WARNING( message )                         \
  Serial.printf("Warning: %s\n", message);                  \
  Serial.printf("File: %s, Line: %d\n", __FILE__, __LINE__);


class TickManager;

class Tickable 
{
public:
  Tickable(size_t inDelay = 0);

  virtual void tick(unsigned long delataTime) = 0;

  virtual ~Tickable(){}

private:
  void tickSceduler(unsigned long delataTime);

  unsigned long accumulatedTime;
  unsigned long delayMs;

  friend class TickManager;
};

using Task_t = std::function<void()>;

class AbstractTaskQueue 
{
protected:
  AbstractTaskQueue();

public:
  void push(Task_t asyncTask);

protected:
  QueueHandle_t taskQueue;
};

class AsyncTaskQueue: public AbstractTaskQueue
{
public:
  AsyncTaskQueue();

  static void runQueue(void *pvParameters);

  static AsyncTaskQueue& get();
private:
  TaskHandle_t asyncTaskHandle;
};

class LoopQueue: public Tickable, public AbstractTaskQueue
{
public:
  LoopQueue();

  void tick(unsigned long delataTime);

  static LoopQueue& get();
};

class TickManager 
{
public:
  TickManager();

  void add(Tickable* TickableObj);

  void tick();

public:
  std::vector<Tickable*> data;
  unsigned long lastTick;
};

#endif
