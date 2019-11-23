#include <iostream>
#include <string>

#include "utils.hpp"
#include "html_parse.hpp"


int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);

    auto html_files = GlobHTML(input_folder);

    if (task == "languages") {

    } else if (task == "print") {
        for (const auto& file : html_files) {
            HTMLDocument kek(file);
            std::cout << "META\n";
            for (const auto& key : kek.GetMetaKeys()) {
                std::cout << "  " << key << ": " << kek.GetMeta(key) << std::endl;
            }
            std::cout << "\nRelated Links\n";
            for (const auto& link : kek.GetLinks()) {
                std::cout << "  " << link << std::endl;
            }
            std::cout << "\nRaw Text\n";
            std::cout << "  " << kek.GetText() << std::endl;
        }
    }
    
    return 0;
}
