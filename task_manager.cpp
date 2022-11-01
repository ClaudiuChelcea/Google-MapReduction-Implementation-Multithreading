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
    std::cout << "Here" << std::endl;
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
            myTask = {
                .file_name = ((struct MapperTaskList*) _myTasks)->taskPQ->front(),
                .mapper_partial_list_array = &(((struct MapperTaskList*) _myTasks)->mappers->at((((struct MapperTaskList*) _myTasks)->thread_id))),
                .mutexTaskList = (((struct MapperTaskList*) _myTasks)->mutexTaskList),
                .thread_id = (((struct MapperTaskList*) _myTasks)->thread_id),
            };
            std::cout << "size: " << ((struct MapperTaskList*) _myTasks)->taskPQ->size() << std::endl;
            ((struct MapperTaskList*) _myTasks)->taskPQ->pop_front();
        } else {
            break; // No more files (tasks)
        }

        pthread_mutex_unlock(&(((struct MapperTaskList*) _myTasks)->mutexTaskList));

        if(found) {
            solveTask(myTask);
        }
    }

    return NULL;
}

/**
 * Solve the actual mapper task
 * @param myTask - a structure containing data for the mapper threads
 * @returns {void} - just solves the task and puts the value in the mapper array of lists
 */
static inline void solveTask(struct MapperTask& myTask)
{
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

    std::cout << "File opened is: " << myTask.file_name << " opened by THREAD: " << myTask.thread_id << std::endl;

    // Map values
    int value_Holder {0};
    while(inputFile >> value_Holder) {
        #if DEBUG_TASK_MANAGER
        std::cout << value_Holder << std::endl;
        #endif

        if(value_Holder > 0) {
            // If we have 1, add it to every list
            if(value_Holder == 1) {
                // Add to every list
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
        for(auto list : *(myTask.mapper_partial_list_array)) {
            std::cout << "List of power " << i++ << ": ";
            for(auto el : list) {
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
 * Execute the Mapper
 * @param reducerFunctArgs - a structure containing data for the mapper threads
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void* executeTaskReduce(void* _reducerFunctArgs)
{
    // Get arguments
    if(_reducerFunctArgs == NULL) {
        std::cerr << "Arguments not sent correctly to reducer!";
        exit(-1);
    }
    struct ReducerFunctArgs reducerFunctArgs;
    try {
        reducerFunctArgs = *((struct ReducerFunctArgs*) _reducerFunctArgs);
    } catch (std::invalid_argument e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Couldn't get reducer arguments";
        exit(-1);
    }

    return NULL;
}