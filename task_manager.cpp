#include "task_manager.h"

/**
 * Execute the Mapper
 * @param file_name - gets the file from which we will read the data
 * @returns {bool} - returns true if the values were read correctly and false if not
 */
void executeTask(const std::string& file_name, std::vector<std::list<int>>& mapper_partial_list_array)
{
    // Open file
    std::ifstream inputFile (file_name, std::ios::in);
    if(inputFile.is_open() == false) {
        std::cerr << "Task execution failure! Task file couldn't be opened!" << std::endl;
        std::exit(-1);
    }
    #if DEBUG_TASK_MANAGER
        std::cout << "File opened is: " << file_name << std::endl;
    #endif

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
                for(int i = 0; i < mapper_partial_list_array.size(); ++i) {
                    mapper_partial_list_array.at(i).push_back(value_Holder);
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
                            mapper_partial_list_array.at(powerOrder - 2).push_back(value_Holder); // -2 since 2nd power is mapped as 0
                        }
                    }
                }       
            }
        }
    }

    // Close file
    inputFile.close();
}