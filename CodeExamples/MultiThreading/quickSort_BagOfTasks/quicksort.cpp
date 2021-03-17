#include <iostream>
#include "quicksort.h"
#include "sort_small_arrays.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class Semaphore {
public:
    Semaphore (int count_ = 0)
    :mtx(),cv(), count(count_)
    {
    }
    
    inline void notify( ) {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        //notify the waiting thread
        cv.notify_one();
    }
    inline void wait( ) {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            //wait on the mutex until notify is called
            cv.wait(lock);
        }
        count--;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

static std::mutex containerLock;
int arrSize = 0;
std::atomic<int> counter(0);
Semaphore sm(1);

template< typename T>
unsigned partition(T* a, unsigned begin, unsigned end) {
  unsigned i = begin, last = end - 1;
  T pivot = a[last];

  for (unsigned j = begin; j < last; ++j) {
    if (a[j] < pivot) {
      std::swap(a[j], a[i]);
      ++i;
    }
  }
  std::swap(a[i], a[last]);
  return i;
}

template< typename T>
unsigned partition_new(T* a, unsigned begin, unsigned end) {
  if (end - begin > 8) return partition_old(a, begin, end);

  unsigned i = begin, last = end - 1, step = (end - begin) / 4;

  T* pivots[5] = { a + begin, a + begin + step, a + begin + 2 * step, a + begin + 3 * step, a + last };
  quicksort_base_5_pointers(pivots);

  std::swap(a[last], a[begin + 2 * step]);
  T pivot = a[last];

  for (unsigned j = begin; j < last; ++j) {
    if (a[j] < pivot /*|| a[j]==pivot*/) {
      std::swap(a[j], a[i]);
      ++i;
    }
  }
  std::swap(a[i], a[last]);
  return i;
}

/* recursive */
template< typename T>
void quicksort_rec(T* a, unsigned begin, unsigned end)
{
  if (end - begin < 6) {
    switch (end - begin) {
    case 5: quicksort_base_5(a + begin); break;
    case 4: quicksort_base_4(a + begin); break;
    case 3: quicksort_base_3(a + begin); break;
    case 2: quicksort_base_2(a + begin); break;
    }
    return;
  }

  unsigned q = partition(a, begin, end);

  quicksort_rec(a, begin, q);
  quicksort_rec(a, q, end);
}

/* iterative */
#define STACK
#define xVECTOR
#define xPRIORITY_QUEUE 

#include <utility> // std::pair

template <typename T>
using triple = typename std::pair< T*, std::pair<unsigned, unsigned>>;

template< typename T>
struct compare_triples {
  bool operator() (triple<T> const& op1, triple<T> const& op2) const {
    return op1.second.first > op2.second.first;
  }
};

#ifdef STACK
#include <stack>
template< typename T>
using Container = std::stack< triple<T>>;
#define PUSH push
#define TOP  top
#define POP  pop
#endif

#ifdef VECTOR
#include <vector>
template< typename T>
using Container = std::vector< triple<T>>;
#define PUSH push_back
#define TOP  back
#define POP  pop_back
#endif

#ifdef PRIORITY_QUEUE
#include <queue>
template< typename T>
using Container = std::priority_queue< triple<T>, std::vector<triple<T>>, compare_triples<T> >;
#define PUSH push
#define TOP  top
#define POP  pop
#endif

template< typename T>
void quicksort_iterative_aux(Container<T>& ranges);

template< typename T>
void quicksort_iterative(T* a, unsigned begin, unsigned end)
{
  Container<T> ranges;
  ranges.PUSH(std::make_pair(a, std::make_pair(begin, end)));
  quicksort_iterative_aux(ranges);
}

template< typename T>
void quicksort_iterative_aux(Container<T>& ranges)
{
  while (counter < arrSize)
  {
    T* a = nullptr;
    unsigned b = 0;
    unsigned e = 0;
    
    //check if container is empty before getting the triple
    sm.wait();
    containerLock.lock();
    //only if not empty then try to pop
    if (!ranges.empty())
    {
      triple<T> r = ranges.TOP();
      ranges.POP();
      containerLock.unlock();

      a = r.first;
      b = r.second.first;
      e = r.second.second;
    }
    else
    {
      containerLock.unlock();
    }

    //base case
    if (e - b < 6)
    {
      counter += e - b;
      switch (e - b)
      {
      case 5: quicksort_base_5(a + b); break;
      case 4: quicksort_base_4(a + b); break;
      case 3: quicksort_base_3(a + b); break;
      case 2: quicksort_base_2(a + b); break;
      }
      sm.notify();
    }
    else
    {
      unsigned q = partition(a, b, e);
      ++counter;

      containerLock.lock();
      ranges.PUSH(std::make_pair(a, std::make_pair(b, q)));
      containerLock.unlock();
      sm.notify();

      containerLock.lock();
      ranges.PUSH(std::make_pair(a, std::make_pair(q + 1, e)));
      containerLock.unlock();
      sm.notify();
    }
  }
}

template< typename T>
void quicksort(T* a, unsigned begin, unsigned end, int numThreads)
{
  //according to the tests provided 8 threads is the fastest to
  //sort 200 ratios with delay

  //size of array
  arrSize = end - begin;

  //create array of threads
  std::thread* threads = new std::thread[numThreads]();

  Container<T> ranges;
  ranges.PUSH(std::make_pair(a, std::make_pair(begin, end)));

  //create threads
  for (int i = 0; i < numThreads; ++i)
  {
    threads[i] = std::thread(quicksort_iterative_aux<T>, std::ref(ranges));
  }

  //join threads
  for (int i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }

  //delete array
  delete[] threads;
}