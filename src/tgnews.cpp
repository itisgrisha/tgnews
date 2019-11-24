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

    auto docs = LoadDocs(html_files);

    if (task == "languages") {
    } else if (task == "dump") {
        RecognizeLanguage(&docs);
        MakeTsv(docs, argv[3]);
    }
    
    return 0;
}
