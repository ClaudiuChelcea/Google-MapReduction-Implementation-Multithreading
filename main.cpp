#include "io_manager.h"
#include "task_manager.h"

#define DEBUG_ONLY_1_MAPPER false
#define REDUCERS_ACTIVE true
#define DEBUG_ALL_THREADS true

int main(int argc, const char** argv)
{
    // Command line parameters
    int number_of_mappers {0};
    int number_of_reducers {0};
    std::string input_file {""};

    // Read command line parameters
    if (read_CMDLINE_Params(number_of_mappers, number_of_reducers, input_file, argv) == END_FUNCTION_ERROR_RETURN) {
        END_FUNCTION_ERROR
    }

    // Input file variables
    int nr_Of_Files {0};
    std::deque <std::string> taskPQ;

    // Open input file and scan the number of files
    // Also add the all the files in a deque and sort it by file sizes
    if (read_Input_File(nr_Of_Files, taskPQ, input_file) == END_FUNCTION_ERROR_RETURN) {
        END_FUNCTION_ERROR
    }

    // All mappers
    pthread_t mappers_threads[number_of_mappers];
    std::vector<std::vector<std::list<int>>> mappers;
    for(int i = 0; i < number_of_mappers; ++i) {
        // One mapper
        std::vector<std::list<int>> mapper_partial_list_array;
        for (int i = 0; i < number_of_reducers; ++i) { // a mapper partial list has number_of_reducers lists
            std::list<int> mapper_partial_list;
            mapper_partial_list_array.push_back(mapper_partial_list);
        }

        // Put the mapper in the mappers vector
        mappers.push_back(mapper_partial_list_array);
    }
    /* VIEW - the structure will be like this
    mappers
        . mapper_partial_list_array[0]:
            . list of power 2
            . list of power 3
            . ...
            . list of power n
        . mapper_partial_list_array[1]
            . list of power 2
            . list of power 3
            . ...
            . list of power n
        .
        .
        .
        . mapper_partial_list_array[number_of_mappers - 1]
            . list of power 2
            . list of power 3
            . ...
            . list of power n
    */

    // All reducers
    pthread_t reducers_threads[number_of_reducers];

    // Create threads
    int total_threads = number_of_mappers;// + number_of_reducers;

    #if DEBUG_ONLY_1_MAPPER
        total_threads = 3;
    #endif

    // Create mutex
    pthread_mutex_t mutexTaskList;
    pthread_mutex_init(&mutexTaskList, NULL);

    // Create task array
    struct MapperTaskList myMapperTasks[number_of_mappers];
    for(int i = 0; i < number_of_mappers; ++i) {
        myMapperTasks[i].taskPQ = &(taskPQ);
        myMapperTasks[i].mappers = &(mappers);
        myMapperTasks[i].mutexTaskList = mutexTaskList;
        myMapperTasks[i].thread_id = i;
    };

    // Create the threads to work on the tasks above
    for(int i = 0; i < total_threads; i++) {
        if (i < number_of_mappers) {
            // Create mapper thread
            if(pthread_create(&mappers_threads[i], NULL, executeTaskMapper, (void*) &myMapperTasks[i]) != 0) {
                std::cerr << "Error creating mapper thread!\n";
                END_FUNCTION_ERROR
            }
        }
        #if REDUCERS_ACTIVE
            else { // If all mappers created, create reducer thread
                // Create reducer arg
                struct ReducerFunctArgs reducerArgs = {
                    .file_name = taskPQ.at(i),
                };

                if(pthread_create(&reducers_threads[i-number_of_mappers], NULL, executeTaskReduce, (void*) &reducerArgs) != 0) {
                    std::cerr << "Error creating reducer thread!\n";
                    END_FUNCTION_ERROR
                }
            }
        #endif
    }

    // Join threads
    for(int i = 0; i < total_threads; ++i) {
        if (i < number_of_mappers) {
            if(pthread_join(mappers_threads[i], NULL) != 0) {
                std::cerr << "Error closing mapper thread!\n";
                END_FUNCTION_ERROR
            }
        #if REDUCERS_ACTIVE
            } else {
                if(pthread_join(reducers_threads[i-number_of_mappers], NULL) != 0) {
                    std::cerr << "Error closing mapper thread!\n";
                    END_FUNCTION_ERROR
                }
            }
        #endif
    }

    // Destroy mutex
    pthread_mutex_destroy(&mutexTaskList);

    _sleep(1000);
    #if DEBUG_ALL_THREADS
        std::cout << std::endl;
        int mapper_id = 0;
        for(auto mapper : mappers) {
            std::cout << "mapper id: "<< mapper_id++ << std::endl;
            int power = 2;
            for(auto list : mapper) {
                std::cout << "List of power " << power++ << ": ";
                for(auto elem : list) {
                    std::cout << elem << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    #endif

    END_FUNCTION_SUCCESS
}