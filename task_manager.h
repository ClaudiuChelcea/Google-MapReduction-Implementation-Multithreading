#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <math.h>

#define DEBUG_TASK_MANAGER false

// Used for sending multiple arguments to mapper threads
struct MapperFunctArgs
{
    std::string file_name; // the name of the file
    std::vector<std::list<int>>* mapper_partial_list_array; // the partial list array
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
void* executeTaskMapper(void* _mapperFunctArgs);

/**
 * Execute the Mapper
 * @param reducerFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _reducerFunctArgs);


#endif // __TASK_MANAGER_H