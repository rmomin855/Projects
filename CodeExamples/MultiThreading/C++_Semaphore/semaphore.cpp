#include "semaphore.h"

Semaphore::Semaphore(int Count) : count(Count)
{}

void Semaphore::wait()
{
  --count;

  if (count < 0)
  {
    std::unique_lock<std::mutex> lock(mutex);

    do
    {
      condition.wait(lock);
    } while (waiting > 0);

    ++waiting;
  }
}

void Semaphore::signal()
{
  ++count;
  --waiting;
  condition.notify_one();
}