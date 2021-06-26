#include <iostream>

#include "lexical_analyzer.h"
#include "parser.h"
#include "translator.h"
#include "virtual_machine.h"

int main()
{

    try
    {
        auto la = LexicalAnalyzer::getInstance();
        auto parser = Parser::getInstance("words.txt");
        auto translator = Translator::getInstance("words.txt", "parser-symboltable.txt");
        auto vm = VirtualMachine::getInstance();
        vm->run();
    }
    catch (std::runtime_error &error)
    {
        std::cerr << std::endl
                  << error.what() << std::endl;
    }

    return 0;
}