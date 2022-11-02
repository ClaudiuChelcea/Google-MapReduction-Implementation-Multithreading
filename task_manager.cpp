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
    if(!_myTasks) {
        std::cerr << "Tasks received by mapper thread send an error!\n";
        exit(-1);
    }

    // Thread pull algorithm
    while(true) {
        bool found = false; // check if we have found a file to work with
        struct MapperTask myTask;

        // Create task
        pthread_mutex_lock(&(((struct MapperTaskList*) _myTasks)->mutexTaskList));
        if(((struct MapperTaskList*) _myTasks)->taskPQ->size() > 0) {
            found = true;
            myTask.file_name = ((struct MapperTaskList*)_myTasks)->taskPQ->front();
            myTask.mapper_partial_list_array = &(((struct MapperTaskList*)_myTasks)->mappers->at((((struct MapperTaskList*)_myTasks)->thread_id)));
            myTask.mutexTaskList = (((struct MapperTaskList*)_myTasks)->mutexTaskList);
            myTask.number_of_reducers = (((struct MapperTaskList*)_myTasks)->number_of_reducers);
            myTask.thread_id = (((struct MapperTaskList*)_myTasks)->thread_id);
            ((struct MapperTaskList*) _myTasks)->taskPQ->pop_front();
        } else {
            break; // No more files (tasks)
        }

        pthread_mutex_unlock(&(((struct MapperTaskList*) _myTasks)->mutexTaskList));

        if(found) {
            solveTask(myTask);
        }
    }

    // Wait all mappers to finish
    pthread_barrier_wait(&((struct MapperTaskList*)_myTasks)->barrier);

    return NULL;
}

/**
 * Check if number is perfect power and, if true, adds it to the lists
 * @param n - the number to check if it's a perfect power
 * @param myTask - receives the task so we can save the values in the reductors arrays
 * @returns {void} - nothing
 */
void perfect_power(int n, struct MapperTask& myTask)
{
    // If n == 1, add to all lists
    if (n == 1) {
        for(int i = 0; i < myTask.number_of_reducers; ++i) {
            myTask.mapper_partial_list_array->at(i).push_back(1);
        }
        return;
    }

    // If not, binary search to see if it's a perfect number
    int exponent = 2;
    while(true) {
        if (pow(2, exponent) > n) {
            return;
        }

        int low = 2;
        int hi = low;
        while (pow(hi, exponent) <= n) {
            hi *= 2;
        }

        while((hi-low) > 1) {
            int middle = floor((low + hi) / 2);
            if(pow(middle, exponent) <= n) {
                low = middle;
            } else {
                hi = middle;
            }
        }

        // If it is a perfect number finally
        if(pow(low, exponent) == n) { // Put it in our lists
            // Max low with min exponent
            if(exponent <= myTask.number_of_reducers + 1)
                myTask.mapper_partial_list_array->at(exponent - 2).push_back(low); // -2 since 2nd power is mapped as 0
            
            // Also get all other lows and exponent combos
            float sqrt_low;;
            while((sqrt_low = sqrt(low)) == (int) sqrt_low && (exponent = exponent *2) <= (myTask.number_of_reducers + 1)) {
                myTask.mapper_partial_list_array->at(exponent - 2).push_back(low);
            }
            return;
        }

        // If not, search for the next exponent
        ++ exponent;
    }
}

static int q = 0;


/**
 * Solve the actual mapper task
 * @param myTask - a structure containing data for the mapper threads
 * @returns {void} - just solves the task and puts the value in the mapper array of lists
 */
static inline void solveTask(struct MapperTask& myTask)
{
    #if DEBUG_ONLY_SHOW_THREADS_AND_SLOW_TIME
    _sleep(5);
    #endif

    // Lock it because other threads might be using it at runtime
    // pthread_mutex_lock(&myTask.mutexTaskList);

    // Open file
    std::ifstream inputFile (myTask.file_name, std::ios::in);
    if(inputFile.is_open() == false) {
        std::cerr << "Task execution failure! Task file couldn't be opened!" << std::endl;
        std::exit(-1);
    }

    // Map values
    int value_Holder {0};
    inputFile >> value_Holder;
    while(inputFile >> value_Holder) {
        //   q++;
        //  std::cout << "Value nr: " << q << " solving value " << value_Holder << " with thread " << myTask.thread_id << " in file: " << myTask.file_name << std::endl;
        perfect_power(value_Holder, myTask);
    }

    // Close file
    inputFile.close();

    // Unlock mutex
    // pthread_mutex_unlock(&myTask.mutexTaskList);
}

/**
 * Solve the actual reducer task
 * @param myTask - a structure containing data for the reducer threads
 * @returns {void} - just solves the task and puts the value in the reducer array of lists
 */
static inline void solveReduce(struct BarrierTask& myTask);

/**
 * Execute the reducer
 * @param _myTasks - a structure containing data for the reducer threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _myTasks)
{
    // Get arguments
    if(_myTasks == NULL) {
        std::cerr << "Arguments not sent correctly to reducer!";
        exit(-1);
    }

    // Create task
    struct BarrierTask myTask;
    myTask.reducer_list = &((BarrierTaskList*)_myTasks)->reducers->at(((BarrierTaskList*)_myTasks)->thread_id);
    myTask.mappers = ((BarrierTaskList*)_myTasks)->mappers;
    myTask.thread_id = ((BarrierTaskList*)_myTasks)->thread_id;

    // Solve task
    solveReduce(myTask);

    return NULL;
}

/**
 * Solve the actual reducer task
 * @param myTask - a structure containing data for the reducer threads
 * @returns {void} - just solves the task and puts the value in the reducer array of lists
 */
static inline void solveReduce(struct BarrierTask& myTask)
{
    try
    {
        // Get reductor
        for(int i = 0; i < myTask.mappers->size(); ++i) { // for every mapper vector
            int items_to_check = 0;
            try 
            {
                // Check if we have items in that list and on that mapper
                items_to_check = myTask.mappers->at(i).at(myTask.thread_id).size();
            } catch(std::exception e) {
                std::cerr << e.what();
                exit(-1);
            }
            for(int j = 0; j < items_to_check; ++j) { // all the items of the power equal to our reductor power order
                myTask.reducer_list->push_back(myTask.mappers->at(i).at(myTask.thread_id).at(j));
            } 
        }

        // Get number of unique items in list
        std::sort(myTask.reducer_list->begin(), myTask.reducer_list->end());
        for(int i = 0; i < myTask.reducer_list->size(); ++i) {
        }
        int uniqueCount = std::unique(myTask.reducer_list->begin(), myTask.reducer_list->end()) - myTask.reducer_list->begin();

        // Create the file and send the answer
        std::string filename = "out" + std::to_string(myTask.thread_id + 2) + ".txt";
        std::ofstream f(filename, std::fstream::in | std::fstream::out | std::fstream::app);
        if(!f.is_open()) {
            std::cerr << "Reducer file couldn't be opened!";
            exit(-1);
        }

        // Display answer
        f << uniqueCount;

        // Close file
        f.close();
    } catch (std::exception e) {
        std::cerr << "Error is solving reducer task!" << std::endl;
        std::cerr << e.what();
        exit(-1);
    }
}