#include <iostream>

#include "lexical_analyzer.h"
#include "parser.h"

int main() {

    try {
        lexicalAnalyzer(fetchSourceCode());
        parser("data/words.txt");
    }
    catch (std::runtime_error& error) {
        std::cerr << std::endl << error.what() << std::endl;
    }

    return 0;
}