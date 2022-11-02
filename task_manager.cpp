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
    pthread_mutex_lock(&myTask.mutexTaskList);

    // Open file
    std::ifstream inputFile (myTask.file_name, std::ios::in);
    if(inputFile.is_open() == false) {
        std::cerr << "Task execution failure! Task file couldn't be opened!" << std::endl;
        std::exit(-1);
    }
    #if DEBUG_TASK_MANAGER
        std::cout << "File opened is: " << myTask.file_name << " opened by THREAD: " << myTask.thread_id << std::endl;
    #endif

    #if DEBUG_ONLY_SHOW_THREADS_AND_SLOW_TIME
        std::cout << "File opened is: " << myTask.file_name << " opened by THREAD: " << myTask.thread_id << std::endl;
    #endif

    // Map values
    int value_Holder {0};
    while(inputFile >> value_Holder) {
        #if DEBUG_TASK_MANAGER
        std::cout << value_Holder << std::endl;
        #endif

        if(value_Holder > 0) {
            // If we have 1, add it to every vector
            if(value_Holder == 1) {
                // Add to every vector
                for(int i = 0; i < myTask.mapper_partial_list_array->size(); ++i) {
                    myTask.mapper_partial_list_array->at(i).push_back(value_Holder);
                }
            } else {
                // Check if it's a perfect power and add to lists if so
                for(int i = 2; i <= sqrt(value_Holder); ++i) { // the maximum we wi;; ever find a value is the root of value_Holder
                    // If value_Holder is power of I, we'll get an integer number, so a true boolean
                    bool isPerfectPower = (log10(value_Holder) / log10(i)) == ((int) (log10(value_Holder) / log10(i)));
                    if(isPerfectPower) {
                        // Get power
                        int powerOrder = ((int) (log10(value_Holder) / log10(i)));
                        if(powerOrder == 1) {  // If the value is 1, it means the same a^1, so we skip
                            continue;
                        } else {
                            myTask.mapper_partial_list_array->at(powerOrder - 2).push_back(value_Holder); // -2 since 2nd power is mapped as 0
                        }
                    }
                }       
            }
        }
    }

    #if DEBUG_TASK_MANAGER
         // Show the results
         std::cout << std::endl;
        int i = 2;
        for(auto vector : *(myTask.mapper_partial_list_array)) {
            std::cout << "List of power " << i++ << ": ";
            for(auto el : vector) {
                std::cout << el << " ";
            }
            std::cout << std::endl;
        }
    #endif

    // Close file
    inputFile.close();

    // Unlock mutex
    pthread_mutex_unlock(&myTask.mutexTaskList);
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
    std::cout << "solved by " << myTask.thread_id << std::endl;
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
            std::cout << "Thread: " << myTask.thread_id << " with item " << myTask.reducer_list->at(i) << std::endl;
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