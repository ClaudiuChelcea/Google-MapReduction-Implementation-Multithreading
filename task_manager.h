#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <math.h>

#define DEBUG_TASK_MANAGER true

/**
 * Execute the Mapper
 * @param file_name - gets the file from which we will read the data
 * @param mapper_partial_list - an array of lists, each list representing a power, from 0 to (number_of_reducers + 1) power
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void executeTask(const std::string& file_name, std::vector<std::list<int>>& mapper_partial_list_array);

#endif // __TASK_MANAGER_H