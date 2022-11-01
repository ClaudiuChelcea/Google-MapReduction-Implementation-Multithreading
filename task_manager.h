#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <deque>
#include <math.h>
#include <stdlib.h>

#define DEBUG_TASK_MANAGER false
#define DEBUG_ONLY_SHOW_THREADS_AND_SLOW_TIME true
#define MAX_NUMBER_OF_THREADS 100

// Task list - the list of tasks that have to be completed by the threads
struct MapperTaskList
{
    std::deque <std::string>* taskPQ; // the list of 'tasks' - files, in a priority queue
    std::vector<std::vector<std::list<int>>>* mappers; // the mappers list
    pthread_mutex_t mutexTaskList; // the mutex used
    int thread_id; // the list of ids
};

// Used for sending multiple arguments to mapper threads
struct MapperTask
{
    std::string file_name; // the name of the file
    std::vector<std::list<int>>* mapper_partial_list_array; // the partial list array
    pthread_mutex_t mutexTaskList; // the mutex used
    int thread_id; // the id of the thread
};

// Used for sending multiple arguments to mapper threads
struct ReducerFunctArgs
{
    std::string file_name; // the name of the file
};


/**
 * Execute the Mapper
 * @param mapperFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskMapper(void* _myTasks);

/**
 * Execute the Mapper
 * @param reducerFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _reducerFunctArgs);


#endif // __TASK_MANAGER_H