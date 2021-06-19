#include <iostream>

#include "lexical_analyzer.h"
#include "parser.h"
#include "translator.h"

int main()
{

    try
    {
        auto la = LexicalAnalyzer::getInstance();
        auto parser = Parser::getInstance("data/words.txt");
        auto translator = Translator::getInstance("data/words.txt", "data/parser-symboltable.txt");
    }
    catch (std::runtime_error &error)
    {
        std::cerr << std::endl
                  << error.what() << std::endl;
    }

    return 0;
}