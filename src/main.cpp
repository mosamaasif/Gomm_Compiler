#include <iostream>

#include "lexical_analyzer.h"

int main() {

    try {
        lexicalAnalyzer(fetchSourceCode());
    }
    catch (std::runtime_error& error) {
        std::cerr << std::endl << error.what() << std::endl;
    }

    return 0;
}