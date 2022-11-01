#include "io_manager.h"
#include "task_manager.h"

#define DEBUG_MAIN false

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
    std::vector<std::vector<std::list<int>>> mappers;
    for(int i = 0; i < number_of_mappers; ++i) {
        // One mapper
        std::vector<std::list<int>> mapper_partial_list_array;
        for (int i = 0; i < number_of_reducers; ++i) {
            std::list<int> mapper_partial_list;
            mapper_partial_list_array.push_back(mapper_partial_list);
        }

        // Put the mapper in the mappers vector
        mappers.push_back(mapper_partial_list_array);
    }
    pthread_t mappers_threads[number_of_mappers];

    #if DEBUG_MAIN
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

        // Test with only the first file
        executeTask(taskPQ.front(), mappers.front());

        // Show the results
        int i = 2;
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