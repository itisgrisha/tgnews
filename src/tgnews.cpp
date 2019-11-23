#include <iostream>
#include <string>

#include "utils.hpp"
#include "html_parse.hpp"
#include "debug.hpp"
#include "langrec.hpp"

int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);

    auto html_files = GlobHTML(input_folder);

    if (task == "languages") {
        auto j = SortByLanguage(html_files, input_folder);


    } else if (task == "dump") {
        MakeTsv(html_files, argv[3]);
    }
    
    return 0;
}
