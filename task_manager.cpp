#include "task_manager.h"

/**
 * Solve the actual mapper task
 * @param myTask - a structure containing data for the mapper threads
 * @returns {void} - just solves the task and puts the value in the mapper array of lists
 */
static inline void solveTask(struct MapperTask& myTask);

/**
 * Execute the Mapper
 * @param mapperFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskMapper(void* _myTasks)
{
    // Receive arguments
    if (!_myTasks) {
        std::cerr << "Tasks received by mapper thread send an error!\n";
        exit(-1);
    }

    // Thread pull algorithm
    while (((struct MapperTaskList*)_myTasks)->taskPQ->size() > 0) {
        bool found = false; // check if we have found a file to work with
        struct MapperTask myTask;

        // Create task
        pthread_mutex_lock(((struct MapperTaskList*)_myTasks)->mutexTaskList);
        if (((struct MapperTaskList*)_myTasks)->taskPQ->size() > 0) {
            try {
                // Get task
                found = true;
                myTask.file_name = ((struct MapperTaskList*)_myTasks)->taskPQ->front();
                ((struct MapperTaskList*)_myTasks)->taskPQ->pop_front();
                myTask.mapper_partial_list_array = &(((struct MapperTaskList*)_myTasks)->mappers->at((((struct MapperTaskList*)_myTasks)->thread_id)));
                myTask.mutexTaskList = (((struct MapperTaskList*)_myTasks)->mutexTaskList);
                myTask.number_of_reducers = (((struct MapperTaskList*)_myTasks)->number_of_reducers);
                myTask.thread_id = (((struct MapperTaskList*)_myTasks)->thread_id);
            }
            catch (std::exception e) {
                std::cerr << "Couldn't receive task!\n";
                std::cerr << e.what();
                exit(-1);
            }
        }
        else {
            break; // No more files (tasks)
        }

        // Unlock mutex
        pthread_mutex_unlock(((struct MapperTaskList*)_myTasks)->mutexTaskList);

        // If we found a task (file), execute the task
        if (found) {
            std::cout << "Solving " << myTask.file_name << " with thread " << myTask.thread_id << std::endl;
            solveTask(myTask);
        }
    }

    // Mark mapper as finished
    ((struct MapperTaskList*)_myTasks)->mappers_status->second++;

    return NULL;
}

/**
 * Check if number is perfect power and, if true, adds it to the lists
 * @param n - the number to check if it's a perfect power
 * @param myTask - receives the task so we can save the values in the reductors arrays
 * @returns {void} - nothing
 */
void perfect_power(int n, struct MapperTask& myTask, int exponent = 2)
{
    // Binary search to see if it's a perfect number
    while (true) {
        if (pow(2, exponent) > n) {
            return;
        }

        int low = 2;
        int hi = low;
        while (pow(hi, exponent) <= n) {
            hi *= 2;
        }

        while ((hi - low) > 1) {
            int middle = floor((low + hi) / 2);
            if (pow(middle, exponent) <= n) {
                low = middle;
            }
            else {
                hi = middle;
            }
        }

        // If it is a perfect number finally
        if (pow(low, exponent) == n) { // Put it in our lists

            if (exponent > myTask.number_of_reducers + 1) // check exponent to be available
                return;

            // Max low with min exponent
            if (exponent <= myTask.number_of_reducers + 1) {
                myTask.mapper_partial_list_array->at(exponent - 2).push_back(n); // -2 since 2nd power is mapped as 0
            }

            // Get all factors as well from this number (eg: if we have 59049, low is 243, exp is 2, we want also to get low 9 and exp 5)
            for (int i = 2; i * i <= low; ++i) {
                if (low % i == 0) {
                    int power = 0;
                    int tmp = low;
                    while (tmp % i == 0) {
                        ++power;
                        tmp /= i;
                    }
                    if (power * exponent <= myTask.number_of_reducers + 1 && tmp <= 1)
                        myTask.mapper_partial_list_array->at(power * exponent - 2).push_back(n);
                }
            }
            perfect_power(n, myTask, ++exponent);
            return;
        }

        // If not, search for the next exponent
        ++exponent;
    }
}

/**
 * Solve the actual mapper task
 * @param myTask - a structure containing data for the mapper threads
 * @returns {void} - just solves the task and puts the value in the mapper array of lists
 */
static inline void solveTask(struct MapperTask& myTask)
{
    // Open file
    std::ifstream inputFile(myTask.file_name, std::ios::in);
    if (inputFile.is_open() == false) {
        std::cerr << "Task execution failure! Task file couldn't be opened!" << std::endl;
        std::exit(-1);
    }

    // Map values
    int value_Holder{ 0 };
    inputFile >> value_Holder;
    while (inputFile >> value_Holder) {
        if (value_Holder == 1) {
            for (int i = 0; i < myTask.number_of_reducers; ++i) {
                myTask.mapper_partial_list_array->at(i).push_back(1);
            }
        }
        else {
            perfect_power(value_Holder, myTask);
        }
    }

    // Close file
    inputFile.close();
}

/**
 * Solve the actual reducer task
 * @param myTask - a structure containing data for the reducer threads
 * @returns {void} - just solves the task and puts the value in the reducer array of lists
 */
static inline void solveReduce(struct ReducerTask& myTask);

/**
 * Execute the reducer
 * @param _myTasks - a structure containing data for the reducer threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _myTasks)
{
    // Get arguments
    if (_myTasks == NULL) {
        std::cerr << "Arguments not sent correctly to reducer!";
        exit(-1);
    }

    // Create task
    struct ReducerTask myTask;
    myTask.reducer_list = &((ReducerTaskList*)_myTasks)->reducers->at(((ReducerTaskList*)_myTasks)->thread_id);
    myTask.mappers = ((ReducerTaskList*)_myTasks)->mappers;
    myTask.thread_id = ((ReducerTaskList*)_myTasks)->thread_id;
    myTask.mappers_status = ((ReducerTaskList*)_myTasks)->mappers_status;
    myTask.barrier =  ((struct ReducerTaskList*)_myTasks)->barrier;

    // Wait for mappers to be done
    while (myTask.mappers_status->second != ((ReducerTaskList*)_myTasks)->mappers->size()) {}
    pthread_barrier_wait(((struct ReducerTaskList*)_myTasks)->barrier);
    solveReduce(myTask);

    return NULL;
}

/**
 * Solve the actual reducer task
 * @param myTask - a structure containing data for the reducer threads
 * @returns {void} - just solves the task and puts the value in the reducer array of lists
 */
static inline void solveReduce(struct ReducerTask& myTask)
{
    try {
        // Get reductor
        for (int i = 0; i < myTask.mappers->size(); ++i) { // for every mapper vector
            int items_to_check = 0;
            try {
                // Check if we have items in that list and on that mapper
                items_to_check = myTask.mappers->at(i).at(myTask.thread_id).size();
            }
            catch (std::exception e) {
                std::cerr << "Couldn't solve reducer\n";
                std::cerr << e.what();
                exit(-1);
            }
            for (int j = 0; j < items_to_check; ++j) { // all the items of the power equal to our reductor power order
                myTask.reducer_list->push_back(myTask.mappers->at(i).at(myTask.thread_id).at(j));
            }
        }

        // Get number of unique items in list
        std::sort(myTask.reducer_list->begin(), myTask.reducer_list->end());
        int uniqueCount = std::unique(myTask.reducer_list->begin(), myTask.reducer_list->end()) - myTask.reducer_list->begin();

        // Create the file and send the answer
        std::string filename = "out" + std::to_string(myTask.thread_id + 2) + ".txt";
        std::ofstream f(filename, std::fstream::in | std::fstream::out | std::fstream::app);
        if (!f.is_open()) {
            std::cerr << "Reducer file couldn't be opened!";
            exit(-1);
        }

        // Display answer
        f << uniqueCount;

        // Close file
        f.close();
    }
    catch (std::exception e) {
        std::cerr << "Error is solving reducer task!" << std::endl;
        std::cerr << e.what();
        exit(-1);
    }
}