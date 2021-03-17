#include <iostream>       // std::cout
#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <vector>         // std::vector
#include <deque>          // std::deque
#include <mutex>          // std::mutex

//DONT DELETE VECTORS UNTIL THE END
//This class collects all vectors to delete them at once at the end of program
//this is so because there is no garauntee how long a thread could hold to old memory
class MemoryGarbage
{
public:
  //keep track of all vectors previously allocated
  std::vector<std::vector<int>*> pointers;
  std::atomic<int> counter;
  std::mutex lock;

  MemoryGarbage() : pointers(10000, nullptr), counter(0), lock()
  {}

  //delete all vectors
  ~MemoryGarbage()
  {
    size_t size = pointers.size();
    for (size_t i = 0; i < size; i++)
    {
      delete pointers[i];
    }
  }

  //add a new pointer to the to be deleted list
  void Add(std::vector<int>* pointer)
  {
    //using locks since std::vector is not thread safe
    lock.lock();

    //valid pointer
    if (pointer)
    {
      pointers[counter] = pointer;
      ++counter;
    }

    lock.unlock();
  }
};

struct Pair {
  Pair() : pointer(nullptr), ref_count(0) {}
  Pair(std::vector<int>* data, long count) :pointer(data), ref_count(count) {}

  std::vector<int>* pointer;
  long              ref_count;
} __attribute__((aligned(16),packed));
// for some compilers alignment needed to stop std::atomic<Pair>::load to segfault

class LFSV {
  MemoryGarbage collector; //collects all vector pointers
  std::atomic< Pair > pdata;
public:

  LFSV() : collector(), pdata(Pair{ new std::vector<int>(), 1 })
  {
    //        std::cout << "Is lockfree " << pdata.is_lock_free() << std::endl;
  }

  ~LFSV()
  {
    delete pdata.load().pointer;
  }

  void Insert(int const& v)
  {
    Pair pdata_new, pdata_old;
    pdata_new.pointer = nullptr;
    do
    {
      //get current data and make a copy
      delete pdata_new.pointer;
      pdata_old = pdata.load();
      pdata_new.pointer = new std::vector<int>(*pdata_old.pointer);

      //add element
      // working on a local copy
      std::vector<int>::iterator b = pdata_new.pointer->begin();
      std::vector<int>::iterator e = pdata_new.pointer->end();
      if (b == e || v >= pdata_new.pointer->back())
      {
        pdata_new.pointer->push_back(v);
      } //first in empty or last element
      else
      {
        for (; b != e; ++b)
        {
          if (*b >= v)
          {
            pdata_new.pointer->insert(b, v);
            break;
          }
        }
      }

      //if current pointer did not change then update, else repeat insert on new pointer
    } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

    collector.Add(pdata_old.pointer);
  }

  int operator[] (int pos) { // not a const method anymore
    Pair pdata_new, pdata_old;
    // before read - increment counter, check if a writer is using it
    do
    {
      pdata_old = pdata.load();
      pdata_new = pdata_old;
      ++pdata_new.ref_count;
    } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

    // counter is >1 now - safely read
    int ret_val = (*pdata_new.pointer)[pos];

    // before return - decrement counter, check if a writer is using it
    do
    {
      pdata_old = pdata.load();
      pdata_new = pdata_old;
      --pdata_new.ref_count;
    } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

    return ret_val;
  }
};
