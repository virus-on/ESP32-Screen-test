#include "Utils.h"

Tickable::Tickable(size_t inDelay)
  : accumulatedTime(0)
  , delayMs(inDelay)
{}

void Tickable::tickSceduler(unsigned long delataTime)
{
  accumulatedTime += delataTime;
  if (accumulatedTime >= delayMs)
  {
    tick(accumulatedTime);
    accumulatedTime = 0;
  }
}

AbstractTaskQueue::AbstractTaskQueue()
{
  _USR_LOG("Create Task Queue!");
  taskQueue = xQueueCreate(10, sizeof(Task_t));
}

// ToDo: ref or pofig?
void AbstractTaskQueue::push(Task_t asyncTask)
{
  // Send the task (lambda function) to the queue
  if (xQueueSend(taskQueue, &asyncTask, portMAX_DELAY) != pdPASS) 
  {
    _USR_WARNING("Task wasn`t sent to the queue!");
  }
}

AsyncTaskQueue::AsyncTaskQueue()
  : AbstractTaskQueue()
{
  xTaskCreatePinnedToCore(AsyncTaskQueue::runQueue, "AsyncTasks", 10000, this, 3, &asyncTaskHandle, 0);
}

void AsyncTaskQueue::runQueue(void *pvParameters)
{
  Task_t receivedTask;
  AsyncTaskQueue* asyncTaskQueue = (AsyncTaskQueue*)(pvParameters);
  while (1) {
    if (xQueueReceive(asyncTaskQueue->taskQueue, &receivedTask, portMAX_DELAY) == pdPASS) {
      receivedTask();
    }
  }
}

AsyncTaskQueue& AsyncTaskQueue::get()
{
  static AsyncTaskQueue asyncTaskQueue;
  return asyncTaskQueue;
}

LoopQueue::LoopQueue()
  : Tickable(100)
  , AbstractTaskQueue()
{
}

void LoopQueue::tick(unsigned long delataTime)
{
  Task_t receivedTask;
  if (xQueueReceive(taskQueue, &receivedTask, 1) == pdPASS) {
    receivedTask();
  }
}

LoopQueue& LoopQueue::get()
{
  static LoopQueue loopQueue;
  return loopQueue;
}

TickManager::TickManager()
  : lastTick(0)
{}

void TickManager::add(Tickable* TickableObj)
{
  if (TickableObj)
  {
    data.push_back(TickableObj);
  }
}

void TickManager::tick()
{
  unsigned long deltaTimeMs = millis() - lastTick;
  lastTick = millis();
  for (auto* TickableObj : data)
  {
    TickableObj->tickSceduler(deltaTimeMs);
  }
}

