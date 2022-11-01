#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <deque>
#include <math.h>
#include <stdlib.h>

#define DEBUG_TASK_MANAGER false
#define DEBUG_ONLY_SHOW_THREADS_AND_SLOW_TIME false
#define MAX_NUMBER_OF_THREADS 100

// Task vector - the vector of tasks that have to be completed by the threads
struct MapperTaskList
{
    std::deque <std::string>* taskPQ; // the vector of 'tasks' - files, in a priority queue
    std::vector<std::vector<std::vector<int>>>* mappers; // the mappers lists
    pthread_mutex_t mutexTaskList; // the mutex used
    int thread_id; // the vector of ids
};

// Used for sending multiple arguments to mapper threads
struct MapperTask
{
    std::string file_name; // the name of the file
    std::vector<std::vector<int>>* mapper_partial_list_array; // the partial vector array
    pthread_mutex_t mutexTaskList; // the mutex used
    int thread_id; // the id of the thread
};

// Used for sending multiple arguments to barrier threads
struct BarrierTaskList
{
    pthread_barrier_t barrier; // the barrier
    std::vector<std::vector<int>>* reducers; // the reducers lists
    std::vector<std::vector<std::vector<int>>>* mappers; // the mappers lists
    int thread_id; // the id of the thread
};

// Used for sending multiple arguments to barrier threads
struct BarrierTask
{
    std::vector<int>* reducer_list; // the reducers lists
    std::vector<std::vector<std::vector<int>>>* mappers; // the mappers lists
    int thread_id; // the id of the thread
};

/**
 * Execute the Mapper
 * @param mapperFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskMapper(void* _myTasks);

/**
 * Execute the reducer
 * @param _myTasks - a structure containing data for the reducer threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _myTasks);


#endif // __TASK_MANAGER_H