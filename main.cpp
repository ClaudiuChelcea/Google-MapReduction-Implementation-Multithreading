#include "io_manager.h"
#include "task_manager.h"

#define DEBUG_MAIN_MAPPER false
#define DEBUG_MAIN_THREADS true

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
    /* VIEW
            mappers: 
                * mapper_partial_list_array:
                    * list of power 2
                    * list of power 3
                    * ...
                    * list of power n
                * mapper_partial_list_array
                    * list of power 2
                    * list of power 3
                    * ...
                    * list of power n
                * mapper_partial_list_array
                   * list of power 2
                    * list of power 3
                    * ...
                    * list of power n
    */
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
    pthread_t mappers_threads[number_of_mappers];

    // All reducers
    pthread_t reducers_threads[number_of_reducers];

    #if DEBUG_MAIN_MAPPER
        // Test with only the first file
        struct MapperFunctArgs testArgs = {
            .file_name = taskPQ.front(),
            .mapper_partial_list_array = mappers.front()
        };
        executeTaskMapper((void*) &testArgs);

         // Show the results
        int i = 2;
        for(auto list : testArgs.mapper_partial_list_array) {
            std::cout << "List of power " << i++ << ": ";
            for(auto el : list) {
                std::cout << el << " ";
            }
            std::cout << std::endl;
        }
    #endif

    // Create threads
    int total_threads = number_of_mappers + number_of_reducers;
    #if DEBUG_MAIN_THREADS
        total_threads = 1;
    #endif

    for(int i = 0; i < total_threads; ++i) {
        if (i < number_of_mappers) {
            // Create mapper arg
            struct MapperFunctArgs mapperArgs = {
            .file_name = taskPQ.at(i),
            .mapper_partial_list_array = &(mappers.at(i))
         };

            // Create mapper thread
            if(pthread_create(&mappers_threads[i], NULL, executeTaskMapper, (void*) &mapperArgs) != 0) {
                std::cerr << "Error creating mapper thread!\n";
                END_FUNCTION_ERROR
            }
        } else { // If all mappers created, create reducer thread
            // Create reducer arg
            struct ReducerFunctArgs reducerArgs = {
                    .file_name = taskPQ.at(i),
            };

            if(pthread_create(&reducers_threads[i-number_of_mappers], NULL, executeTaskReduce, (void*) &reducerArgs) != 0) {
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

    #if DEBUG_MAIN_THREADS
        // Show the results - for first mapper thread
        int i = 2;
        std::cout << "Starting showing first thread\n";
        for(auto list : mappers.front()) {
            std::cout << "List of power " << i++ << ": ";
            for(auto el : list) {
                std::cout << el << " ";
            }
            std::cout << std::endl;
        }
    #endif


    END_FUNCTION_SUCCESS
}