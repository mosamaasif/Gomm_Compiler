#include "virtual_machine.h"

#include <iostream>
#include <fstream>

#include "error_handler.h"

VirtualMachine* VirtualMachine::m_Instance;

VirtualMachine::VirtualMachine() {
    populateDataSection();
    populateQuadruple();
}

std::vector<std::string> VirtualMachine::splitStr(const std::string& line, const char& delim)
{
    size_t start;
    size_t end = 0;

    std::vector<std::string> out;
    while ((start = line.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = line.find(delim, start);
        out.push_back(line.substr(start, end - start));
        trim(out[out.size() - 1]);
    }

    return out;
}

void VirtualMachine::populateDataSection() {
    std::ifstream in("translator-symboltable.txt");
    ASSERT(!in.is_open(), ERRORS::SYMBOL_TABLE_FILE_OPEN_ERROR);

    std::string line;

    char delim = '|';

    std::getline(in, line);
    dataSection.resize(300);
    while (std::getline(in, line))
    {
        auto vals = splitStr(line, delim);
        if(vals[1] == "Integer")
            dataSection[atoi(vals[2].c_str())] = atoi(vals[3].c_str());
    }

    in.close();
}

void VirtualMachine::populateQuadruple() {
    std::ifstream in("machine-code.txt");
    ASSERT(!in.is_open(), ERRORS::MC_FILE_OPEN_ERROR);

    std::string line;

    char delim = ' ';

    while (std::getline(in, line))
    {
        auto vals = splitStr(line, delim);
        std::vector<int> quad = {
            atoi(vals[0].c_str()), atoi(vals[1].c_str()), atoi(vals[2].c_str()), atoi(vals[3].c_str()),
        };
        quadruples.push_back(quad);
    }

    in.close();
}

void VirtualMachine::run() {

    for (int pc = 0; pc < quadruples.size(); pc++) {
        switch (quadruples[pc][0])
        {

        case 0: {  // goto
            pc = quadruples[pc][1] - 2;
            break;
        }

        case 2: { // in
            std::cout << "(This prompt MSG is for evaluation only, not part of GO-- code. Input itself is from GO--) Enter Input: ";
            int pos = quadruples[pc][1];

            std::string input;
            std::cin >> input;
            ASSERT(input.find_first_not_of("0123456789") != std::string::npos, ERRORS::IN_NOT_NUM);
            dataSection[pos] = atoi(input.c_str());
            break;
        }

        case 3: { // print
            int pos = quadruples[pc][1];
            std::cout << dataSection[pos];
            break;
        }

        case 4: { // println
            int pos = quadruples[pc][1];
            std::cout << dataSection[pos] << std::endl;
            break;
        }

        case 5: {  // Less Than (<)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] < dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 6: {  // Less Than Equal (<=)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] <= dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 7: {  // Greater Than (>)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] > dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 8: {  // Greater Than Equal (>=)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] >= dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 9: {  // Equals (==)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] == dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 10: {  // Equals (!=)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int gotoVal = quadruples[pc][3];

            if (dataSection[op1] != dataSection[op2])
                pc = gotoVal - 2;

            break;
        }

        case 11: {  // Assignment (=)
            int from = quadruples[pc][1];
            int to = quadruples[pc][2];

            dataSection[to] = dataSection[from];
            break;
        }

        case 12: { // addition (+)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int res = quadruples[pc][3];

            dataSection[res] = dataSection[op1] + dataSection[op2];
            break;
        }

        case 13: { // subtraction (-)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int res = quadruples[pc][3];

            dataSection[res] = dataSection[op1] - dataSection[op2];
            break;
        }

        case 14: { // multiplication (*)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int res = quadruples[pc][3];

            dataSection[res] = dataSection[op1] * dataSection[op2];
            break;
        }

        case 15: { // division (/)
            int op1 = quadruples[pc][1];
            int op2 = quadruples[pc][2];
            int res = quadruples[pc][3];

            ASSERT(dataSection[op2] == 0, ERRORS::DIV_BY_ZERO);
            dataSection[res] = dataSection[op1] / dataSection[op2];
            break;
        }

        default:
            break;
        }
    }
}

VirtualMachine::~VirtualMachine() {
	delete m_Instance;
}