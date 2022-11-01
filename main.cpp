#include "io_manager.h"

int main(int argc, const char** argv) {
    
    /* Variables */

    // Command line parameters
    int number_of_mappers {0};
    int number_of_reducers {0};
    std::string input_file {""};

    // Input file
    int nr_Of_Files {0};
    std::list<std::string> list_Of_Files;

    /* Code */

    // Read command line parameters
    if (read_CMDLINE_Params(number_of_mappers, number_of_reducers, input_file, argv) == END_FUNCTION_ERROR_RETURN) {
        END_FUNCTION_ERROR
    }

    // Open input file and scan the number of files
    // Also add the all the files in a list and sort it by file sizes
    if (read_Input_File(nr_Of_Files, list_Of_Files, input_file) == END_FUNCTION_ERROR_RETURN) {
        END_FUNCTION_ERROR
    }

    END_FUNCTION_SUCCESS
}