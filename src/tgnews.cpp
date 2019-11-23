#include <iostream>
#include <string>

#include "utils.hpp"


int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);

    auto html_files = GlobHTML(input_folder);

    if (task == "languages") {

    }
    
    return 0;
}
