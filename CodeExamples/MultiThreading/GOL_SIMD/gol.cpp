#include "gol.h"
#include <pthread.h>
#include <iostream>
#include <semaphore.h>

typedef struct Parameters
{
  int x;
  int y;
}Parameters;

std::vector<std::vector<bool>>* map;
int max;
int numIterations;

sem_t barrier;
sem_t barrier2;
sem_t mutex;

void* UpdateCell(void* arguments)
{
  //static variable is shared
  static int Count = 0;

  Parameters* parameters = reinterpret_cast<Parameters*>(arguments);
  int x = parameters->x;
  int y = parameters->y;

  for (int i = 0; i < numIterations; ++i)
  {
    int alive = 0;
    bool state = true;

    //check neighbours but not self
    for (int X = x - 1; X <= x + 1; ++X)
    {
      for (int Y = y - 1; Y <= y + 1; ++Y)
      {
        if (X == x && Y == y)
        {
          continue;
        }

        if ((*map)[Y][X])
        {
          alive++;
        }
      }
    }

    //decide next state
    if ((*map)[y][x])
    {
      if (alive < 2 || alive > 3)
      {
        state = false;
      }
    }
    else
    {
      if (alive != 3)
      {
        state = false;
      }
    }

    //reading finished
    sem_wait(&mutex);
    Count += 1;

    //when all threads reach this point
    //release them and block at barrier2
    if (Count == max)
    {
     for (int i = 0; i < max; i++)
     {
       sem_post(&barrier);
     }
    }

    sem_post(&mutex);
    sem_wait(&barrier);

    //critical section
    //write to array
    (*map)[y][x] = state;

    sem_wait(&mutex);
    Count -= 1;

    //when all threads finished calculating
    //block barrier and unlock barrier2
    if (Count == 0)
    {
     for (int i = 0; i < max; i++)
     {
       sem_post(&barrier2);
     }
    }

    sem_post(&mutex);
    sem_wait(&barrier2);
  }

  pthread_exit(NULL);
}

std::vector< std::tuple<int, int> > run(std::vector< std::tuple<int, int> > initial_population, int num_iter, int max_x, int max_y)
{
  std::vector<std::vector<bool>> mapData(max_y + 2, std::vector<bool>(max_x + 2, false));
  map = &mapData;
  numIterations = num_iter;

  sem_init(&barrier, 0, 0);
  sem_init(&barrier2, 0, 0);
  sem_init(&mutex,0, 1);

  max = max_x * max_y;
  pthread_t* threads = new pthread_t[max];
  Parameters* parameters = new Parameters[max];

  size_t size = initial_population.size();
  for (size_t i = 0; i < size; ++i)
  {
    std::tuple<int, int>& coord = initial_population[i];
    mapData[std::get<0>(coord) + 1][std::get<1>(coord) + 1] = true;
  }

  //create threads
  int count = 0;
  for (int y = 1; y <= max_y; ++y)
  {
    for (int x = 1; x <= max_x; ++x)
    {
      parameters[count].x = x;
      parameters[count].y = y;

      pthread_create(&threads[count], NULL, reinterpret_cast<void* (*)(void*)>(&UpdateCell), reinterpret_cast<void*>(&parameters[count]));
      ++count;
    }
  }

  void* status;
  for (int i = 0; i < count; i++)
  {
    pthread_join(threads[i], &status);
  }

  std::vector<std::tuple<int, int>> live;
  for (int y = 1; y <= max_y; ++y)
  {
    for (int x = 1; x <= max_x; ++x)
    {
      if (mapData[y][x])
      {
        live.push_back(std::make_pair(y - 1, x - 1));
      }
    }
  }


  sem_destroy(&barrier);
  sem_destroy(&barrier2);
  sem_destroy(&mutex);
  delete[] threads;
  delete[] parameters;
  return live;
}