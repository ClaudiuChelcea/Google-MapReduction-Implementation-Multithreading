#ifndef __IO_MANAGER_H
#define __IO_MANAGER_H

#include <iostream>
#include <deque>
#include <algorithm>
#include <fstream>

#define END_FUNCTION_ERROR return 1;
#define END_FUNCTION_ERROR_RETURN 1
#define END_FUNCTION_SUCCESS return 0;
#define END_FUNCTION_SUCCESS_RETURN 0

/**
 * Get the command line parameters
 * @param number_of_mappers - gets the number of mapper threads that will be used
 * @param number_of_reducers - gets the number of reducer threads that will be used
 * @param input_file - gets the file from which we will read the data
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
bool read_CMDLINE_Params(int& number_of_mappers, int& number_of_reducers, std::string& input_file,
    const char** argv);

/**
 * Open input file and scan the number of files; Also add the all the files in a deque and sort it by file sizes
 * @param nr_Of_Files - gets the number of files that will be used
 * @param taskPQ - in this deque we will save all the files, sorted by size
 * @param input_file - from this file we will read the data
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
bool read_Input_File(int& nr_Of_Files, std::deque<std::string>& taskPQ, std::string& input_file);

#endif // __IO_MANAGER_H