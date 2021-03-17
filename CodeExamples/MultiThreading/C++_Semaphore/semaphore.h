#include <mutex>
#include <condition_variable>
#include <atomic>

class Semaphore
{
public:
  Semaphore(int Count = 0);
	void wait();
	void signal();
private:
  std::atomic<int> count;
  std::atomic<int> waiting;
  std::mutex mutex;
  std::condition_variable condition;
};
