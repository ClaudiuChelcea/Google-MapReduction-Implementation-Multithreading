#include "io_manager.h"
#include "task_manager.h"

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
    std::vector<pthread_t> mappers_threads;
    mappers_threads.resize(number_of_mappers * sizeof(pthread_t));
    std::vector<std::vector<std::vector<int>>> mappers;
    for(int i = 0; i < number_of_mappers; ++i) {
        // One mapper
        std::vector<std::vector<int>> mapper_partial_list_array;
        for (int i = 0; i < number_of_reducers; ++i) { // a mapper partial vector has number_of_reducers lists
            std::vector<int> mapper_partial_list;
            mapper_partial_list_array.push_back(mapper_partial_list);
        }

        // Put the mapper in the mappers vector
        mappers.push_back(mapper_partial_list_array);
    }
    /* VIEW - the structure will be like this
    mappers
        . mapper_partial_list_array[0]:
            . vector of power 2
            . vector of power 3
            . ...
            . vector of power n
        . mapper_partial_list_array[1]
            . vector of power 2
            . vector of power 3
            . ...
            . vector of power n
        .
        .
        .
        . mapper_partial_list_array[number_of_mappers - 1]
            . vector of power 2
            . vector of power 3
            . ...
            . vector of power n
    */

    // All reducers
    std::vector<pthread_t> reducers_threads;
    reducers_threads.resize(number_of_reducers * sizeof(pthread_t));
    std::vector<std::vector<int>> reducers;
    for(int i = 0; i < number_of_reducers; ++i) {
        // One reducer
        std::vector<int> reducer_partial_list_array;
        reducers.push_back(reducer_partial_list_array);
    }

    // Create threads
    int total_threads = number_of_mappers + number_of_reducers;

    // Create mutex
    pthread_mutex_t mutexTaskList;
    if(pthread_mutex_init(&mutexTaskList, NULL) != 0) {
        std::cerr << "Couldn't open mutex!\n";
        END_FUNCTION_ERROR
    }

    // Create barrier
    pthread_barrier_t barrier;
    if(pthread_barrier_init(&barrier, NULL, number_of_mappers) != 0) {
        std::cerr << "Couldn't open barrier!\n";
        END_FUNCTION_ERROR
    }

    // Create task array mutex
    std::vector<struct MapperTaskList> myMapperTasks;
    myMapperTasks.resize(number_of_mappers * sizeof(struct MapperTaskList));
    for(int i = 0; i < number_of_mappers; ++i) {
        myMapperTasks[i].taskPQ = &(taskPQ);
        myMapperTasks[i].mappers = &(mappers);
        myMapperTasks[i].mutexTaskList = mutexTaskList;
        myMapperTasks[i].barrier = barrier;
        myMapperTasks[i].number_of_reducers = number_of_reducers;
        myMapperTasks[i].thread_id = i;
    }

    // Create task array barrier
    std::vector<struct BarrierTaskList> myReducerTasks;
    myReducerTasks.resize(number_of_reducers * sizeof(struct BarrierTaskList));
    for(int i = 0; i < number_of_reducers; ++i) {
        myReducerTasks[i].reducers = &(reducers);
        myReducerTasks[i].mappers = &(mappers);
        myReducerTasks[i].thread_id = i;
    }

    // Create the threads to work on the tasks above
    for(int i = 0; i < total_threads; i++) {
        if (i < number_of_mappers) {
            // Create mapper thread
            if(pthread_create(&mappers_threads[i], NULL, executeTaskMapper, (void*) &myMapperTasks[i]) != 0) {
                std::cerr << "Error creating mapper thread!\n";
                END_FUNCTION_ERROR
            }
        } else { // If all mappers created, create reducer thread
            int reducer_creation_status = pthread_create(&reducers_threads[i - number_of_mappers], NULL, executeTaskReduce, (void*)&myReducerTasks[i - number_of_mappers]);
            if(reducer_creation_status != 0) {
                std::cerr << "Error creating reducer thread!\n";
                END_FUNCTION_ERROR
            }
        }
    }

    // Join threads
    for(int i = 0; i < total_threads; ++i) {
        if (i < number_of_mappers) {
            if(pthread_join(mappers_threads[i], NULL) != 0) {
                std::cerr << "Error closing mapper thread!\n";
                END_FUNCTION_ERROR
            }
        } else {
            if(pthread_join(reducers_threads[i-number_of_mappers], NULL) != 0) {
                std::cerr << "Error closing mapper thread!\n";
                END_FUNCTION_ERROR
            }
        }
    }

    // Destroy mutex
    if (pthread_mutex_destroy(&mutexTaskList) != 0) {
        std::cerr << "Couldn't close mutex correctly!";
        END_FUNCTION_ERROR
    }

    // Destroy barrier
    if (pthread_barrier_destroy(&barrier) != 0) {
        std::cerr << "Couldn;t close barrier correctly!";
        END_FUNCTION_ERROR
    }

    END_FUNCTION_SUCCESS
}