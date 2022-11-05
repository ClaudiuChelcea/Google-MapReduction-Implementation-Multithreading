#include "io_manager.h"

/**
 * Get the command line parameters
 * @param number_of_mappers - gets the number of mapper threads that will be used
 * @param number_of_reducers - gets the number of reducer threads that will be used
 * @param input_file - gets the file from which we will read the data
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
bool read_CMDLINE_Params(int& number_of_mappers, int& number_of_reducers, std::string& input_file,
    const char** argv)
{
    // Get command line parameters
    try {
        number_of_mappers = atoi(argv[1]);
        number_of_reducers = atoi(argv[2]);
        input_file = argv[3];
    }
    catch (std::invalid_argument& argument) { // If we don't get the parameters correctly
        std::cerr << "Received wrong argument: " << argument.what() << "\n";
        std::cerr << "Command line arguments not received correctly! Ending program...\n";
        END_FUNCTION_ERROR
    }

    // Check parameters received correctly
    if (number_of_mappers == 0 || number_of_reducers == 0 || input_file.compare("") == 0) {
        std::cerr << "Command line arguments not received correctly! Ending program...\n";
        std::cerr << "number_of_mappers: " << number_of_mappers << std::endl;
        std::cerr << "number_of_reducers: " << number_of_reducers << std::endl;
        std::cerr << "input_file: " << input_file << std::endl;
        END_FUNCTION_ERROR
    }

    END_FUNCTION_SUCCESS
}

/**
 * Return the file size
 * @param file_name - the name of the file
 * @returns {int} - returns the size of the file
 */
int getFileSize(const std::string& file_name)
{
    std::streampos fsize = 0;

    std::ifstream myfile(file_name, std::ios::in); // File is of type const char*
    if (myfile.is_open() == false) {
        std::cerr << "File to get size from was not opened!" << std::endl;
        std::exit(-1);
    }

    fsize = myfile.tellg(); // The file pointer is currently at the beginning
    myfile.seekg(0, std::ios::end); // Place the file pointer at the end of file

    fsize = myfile.tellg() - fsize;
    myfile.close();

    return fsize;
}

/**
 * Check if a file is bigger than other
 * @param first - string name of first file
 * @param second - string name of second file
 * @returns {bool} - returns the difference in size of the two files by a boolean value, basically returns which one is bigger
 */
bool sort_by_file_size(const std::string& first, const std::string& second)
{
    return (getFileSize(first) > getFileSize(second));
}

/**
 * Open input file and scan the number of files; Also add the all the files in a deque and sort it by file sizes
 * @param nr_Of_Files - gets the number of files that will be used
 * @param taskPQ - in this deque we will save all the files, sorted by size
 * @param input_file - from this file we will read the data
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
bool read_Input_File(int& nr_Of_Files, std::deque<std::string>& taskPQ, std::string& input_file)
{
    // File variables
    std::ifstream inputFile;
    inputFile.open(input_file.c_str());
    if (inputFile.is_open() == false) {
        std::cerr << "Input file to read other files from was not opened!" << std::endl;
        std::exit(-1);
    }

    // Get nr of files and create a deque of the files
    try {
        inputFile >> nr_Of_Files;
        std::string tmp_File;
        for (int i = 0; i < nr_Of_Files; ++i) {
            try
            {
                inputFile >> tmp_File;
                taskPQ.push_back(tmp_File);
            } catch(std::exception e) {
                std::cerr << "Error creating list of files!\n";
                std::cerr << e.what();
                exit(-1);
            }
        }
    }
    catch (std::invalid_argument& argument) { // If we don't read correctly
        std::cerr << "Received wrong argument: " << argument.what() << "\n";
        std::cerr << "File was not read successfully! Ending program...\n";
        END_FUNCTION_ERROR
    }

    // Check parameters received correctly
    if (nr_Of_Files == 0) {
        std::cerr << "Input file was not read correctly! Ending program...\n";
        END_FUNCTION_ERROR
    }

    // Sort deque descending by size
    std::sort(taskPQ.begin(), taskPQ.end(), sort_by_file_size);

    // Close file
    inputFile.close();

    END_FUNCTION_SUCCESS
}