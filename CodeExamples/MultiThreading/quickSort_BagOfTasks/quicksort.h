#ifndef QUICKSORT
#define QUICKSORT
template< typename T>
void quicksort_rec( T* a, unsigned begin, unsigned end );

template< typename T>
void quicksort_iterative( T* a, unsigned begin, unsigned end );

template< typename T>
void quicksort(T* a, unsigned, unsigned, int);

#include "quicksort.cpp"
#endif




/*
//queue tasks
queue< pair<int, int> > tasks;
//mutexs for set and queue task
mutex q_mutex, s_mutex;
//condition variable
condition_variable cv;
//set
set<int> ss;

//quick sort
template <typename T>
void quick_sort(vector<T> &arr)
{
    while (true)
    {
        unique_lock<mutex> u_lock(q_mutex); //lock mutex

        //sort is fineshed
        if ( ss.size() == arr.size() ) //u_lock.unlock()
            return;

        //if queue task is not empty 
        if ( tasks.size() > 0 )
        {
            //get task from queue
            pair<int, int> cur_task = tasks.front();            
            tasks.pop();

            int l = cur_task.first, r = cur_task.second;        

            if (l < r)
            {
                int q = partition(arr, l, r); //split array

                //Add indexes in set
                s_mutex.lock();
                ss.insert(q);
                ss.insert(l);
                ss.insert(r);
                s_mutex.unlock();

                //push new tasks for left and right part
                tasks.push( make_pair(l, q - 1) );
                tasks.push( make_pair(q + 1, r) );

                //wakeup some thread which waiting 
                cv.notify_one();
            }
        }
        else
            //if queue is empty
            cv.wait(u_lock);
    }
}
*/