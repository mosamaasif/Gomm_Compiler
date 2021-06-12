/*
*
*   FROM BONUS handles nested ifs etc, but not functions
*/

#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <sstream>

#include "error_handler.h"

class Translator
{

private:
    static Translator* m_Instance;

    struct STData {
        std::string lex, dt;
        int addr;
    };

    struct less_than_key
    {
        inline bool operator() (const STData& struct1, const STData& struct2)
        {
            return (struct1.addr < struct2.addr);
        }
    };

    struct Token
    {
        std::string type;
        std::string lexeme;

        Token(std::string type, std::string lex)
        {
            this->type = type;
            lexeme = lex;
        }
    };

    std::vector<Token>::iterator look;
    std::vector<Token>::iterator end;

    std::map<std::string, STData> symbols;

    std::vector<std::string> tacLines;

    int addr = 0, n = 1;
    int tempValIdx = 1;

public:
    static Translator* getInstance(const std::string& wListPath, const std::string &stPath) {
        if(!m_Instance)
            m_Instance = new Translator(wListPath, stPath);

        return m_Instance;
    }

private:
    Translator(const std::string& wListPath, const std::string &stPath) {
        run(wListPath, stPath);
    }

    std::vector<Token> loadTokens(const std::string &wListPath)
    {

        std::ifstream in(wListPath);
        ASSERT(!in.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

        std::string line;
        std::vector<Token> toks;

        while (std::getline(in, line))
        {
            std::string tok, id;
            if (line[line.length() - 2] == '^')
            {
                tok = line.substr(1, line.find(',') - 1);
                id = "^";
                std::for_each(tok.begin(), tok.end(), [](char &c)
                              { c = std::tolower(c); });
            }
            else
            {
                int s_idx = line.find(',') + 2;
                tok = line.substr(s_idx, line.length() - s_idx - 1);
                id = line.substr(1, line.find(',') - 1);
            }
            toks.push_back(Token(id, tok));
        }

        return toks;
    }

    std::vector<std::string> splitStr(const std::string &line, const char &delim)
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

    void loadST(const std::string &stPath)
    {

        std::ifstream in(stPath);
        ASSERT(!in.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

        std::string line;

        char delim = '|';

        std::getline(in, line);
        while (std::getline(in, line))
        {
            auto vals = splitStr(line, delim);
            symbols[vals[0]] = {vals[0], vals[1], 0};
        }

        in.close();
    }

    void writeST()
    {
        std::ofstream symTable("data/translator-symboltable.txt");

        symTable << "ID | DT | S" << std::endl;

        std::vector<STData> vec;
        for(std::map<std::string, STData>::iterator itr = symbols.begin(); itr != symbols.end(); itr++)
            vec.push_back(itr->second);

        std::sort(vec.begin(), vec.end(), less_than_key());

        for (int i = 0; i < vec.size(); i++)
        {
            symTable << vec[i].lex
                     << " | "
                     << vec[i].dt
                     << " | " 
                     << vec[i].addr
                     << '\n';
        }

        symTable.close();
    }

    void writeTAC() {

        std::ofstream out("data/tac.txt");
        ASSERT(!out.is_open(), ERRORS::TAC_FILE_OPEN_ERROR);

        for(int line = 0; line < tacLines.size(); line++)
            out << tacLines[line];
    }

    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    void emit(int num, ...) {
        va_list valist;
        va_start(valist, num);
        std::string temp = "";
        for(int i = 0; i < num; i++) {
            temp += (va_arg(valist, char*));
            if (i < num - 1)
                temp += " ";
        }
        trim(temp);
        temp += "\n";
        tacLines.push_back(temp);
        va_end(valist);

        n++;
    }

    void backpatch(int lineNo, int value) {
        tacLines[lineNo - 1].pop_back();
        tacLines[lineNo - 1].append(" " + std::to_string(value));
        tacLines[lineNo - 1].push_back('\n');
    }

    void addToST(int size, std::string& lex) {

        trim(lex);
        auto vec = splitStr(lex, ' ');

        if(!vec.empty()) {

            for(const std::string& id: vec) {
                symbols[id].addr = addr;
                addr += size;
            }
        }
        else {
            symbols[lex].addr = addr;
            addr += size;
        }
    }

    Token peekToken()
    {
        Token next_tok = *(++look);
        look--;
        return next_tok;
    }

    void match(const std::string &token)
    {

        if (look->lexeme == token)
            look++;
        else
            PAR_LOG(ERRORS::BAD_TOK, token, look->lexeme);
    }

    std::pair<std::string, int> datatype()
    {

        if (look->lexeme == "char" || look->lexeme == "Integer") {
            std::pair<std::string, int> ret = {look->lexeme, (look->lexeme == "char") ? 1 : 4};
            look++;
            return ret;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "char or Integer", look->lexeme);
    }

    std::string identifier()
    {

        std::string lex = look->lexeme;
        if (look->type == "ID") {
            look++;
            return lex;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type Identifier", look->lexeme);
    }

    void variable()
    {

        std::pair<std::string, int> dt = datatype();
        match(":");
        std::string lex = identifier();

        addToST(dt.second, lex);
    }

    std::string variableDeclerationB()
    {

        if (look->lexeme == ",")
        {
            match(",");
            return variableDeclerationA();
        }
        else
            return "";
    }

    std::string variableDeclerationA()
    {

        std::string lex = identifier() + " ";
        lex += variableDeclerationB();
        return lex;
    }

    void variableDecleration()
    {

        std::pair<std::string, int> dt = datatype();
        match(":");
        std::string lex = variableDeclerationA();
        match(";");

        addToST(dt.second, lex);
    }

    std::string literalConstant()
    {

        if (look->type == "LC") {
            std::string lex = look->lexeme;
            look++;
            return lex;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type Literal Constant", look->lexeme);
    }

    std::string numConstant()
    {

        if (look->type == "NUMC") {
            std::string lex = look->lexeme;
            look++;
            return lex;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type Number Constant/Number", look->lexeme);
    }

    std::string F()
    {

        if (look->type == "ID")
            return identifier();

        else if (look->type == "NUMC")
            return numConstant();

        else if (look->lexeme == "(")
        {
            match("(");
            std::string lex = expression();
            match(")");
            return lex;
        }
        else
            throw std::runtime_error("Invalid Expression found!");
    }

    std::string termPrime(std::string& prevLex)
    {

        if (look->lexeme == "*")
        {
            match("*");
            std::string termLex = "* " + F() + " ";

            std::string temp = "t" + std::to_string(tempValIdx++);
            symbols[temp] = { temp, "Integer", 0};
            addToST(4, temp);
            emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());

            return termPrime(temp);
        }
        else if (look->lexeme == "/")
        {
            match("/");
            std::string termLex = "/ " + F() + " ";

            std::string temp = "t" + std::to_string(tempValIdx++);
            symbols[temp] = { temp, "Integer", 0};
            addToST(4, temp);
            emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
            
            return termPrime(temp);
        }
        else
            return prevLex;
    }

    std::string term()
    {

        std::string lex = F();
        return termPrime(lex);
    }

    std::string expressionPrime(std::string& prevLex)
    {
        if (look->lexeme == "+")
        {
            match("+");
            std::string termLex = "+ " + term() + " ";

            std::string temp = "t" + std::to_string(tempValIdx++);
            symbols[temp] = { temp, "Integer", 0};
            addToST(4, temp);
            emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());

            return expressionPrime(temp);
        }
        else if (look->lexeme == "-")
        {
            match("-");
            std::string termLex = "- " + term() + " ";

            std::string temp = "t" + std::to_string(tempValIdx++);
            symbols[temp] = { temp, "Integer", 0};
            addToST(4, temp);
            emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
            
            return expressionPrime(temp);
        }
        else
            return prevLex;
    }

    std::string expression()
    {
        std::string termLex = term();
        std::string ePLex = expressionPrime(termLex);
        return ePLex;
    }

    std::string value()
    {

        if (look->type == "LC")
        {
            return literalConstant();
        }

        else if (look->type == "ID" && peekToken().lexeme == "(")
        {
            functionCall();
            return "";
        }

        else
        {
            return expression();
        }
    }

    void functionCallArgPrime()
    {

        if (look->lexeme == ",")
        {
            match(",");
            value();
            functionCallArgPrime();
        }
        else
            ;
    }

    void functionCallArg()
    {

        if (look->lexeme == ")")
        {
        }
        else
        {
            value();
            functionCallArgPrime();
        }
    }

    void functionCall()
    {

        identifier();
        match("(");
        functionCallArg();
        match(")");
        match(";");
    }

    void variableAssignment()
    {

        std::string idLex = identifier();
        match(":=");
        std::string valLex = value();
        match(";");

        emit(3, idLex.c_str(), "=", valLex.c_str());
    }

    std::string relationalOperator()
    {

        if (look->type == "RO") {
            std::string lex = look->lexeme;
            look++;
            return lex;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type RO", look->lexeme);
    }

    std::string with()
    {

        if (look->type == "ID")
            return identifier();
        else if (look->type == "NUMC")
            return numConstant();
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type Identifier or Number", look->lexeme);
    }

    std::pair<int, int> comparison()
    {

        // assuming first is supposed to be identifier, not any other thing
        // to maintain consistency and order
        std::string idLex = identifier();
        std::string reLex = relationalOperator();
        std::string wLex = with();

        std::pair<int, int> ret;
        ret.first = n;
        emit(5, "if", idLex.c_str(), reLex.c_str(), wLex.c_str(), "goto");
        ret.second = n;
        emit(1, "goto");

        return ret;
    }

    void whileLoop()
    {

        match("while");
        int start = n;
        std::pair<int, int> cmpRet = comparison();
        backpatch(cmpRet.first, n);
        match(":");
        match("{");
        codeBlock();
        match("}");

        emit(2, "goto", std::to_string(start).c_str());
        backpatch(cmpRet.second, n);
    }

    std::string stringLiteral()
    {
        
        std::string lex;
        if (look->type == "STR") {
            lex = look->lexeme;
            look++;
            return lex;
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "String", look->lexeme);
    }

    std::string output()
    {

        if (look->type == "STR")
            return stringLiteral();

        else if (look->type == "ID")
            return identifier();
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type String or Identifier", look->lexeme);
    }

    void printStatement()
    {

        if (look->lexeme == "print")
        {
            match("print");
            match("(");
            std::string lex = output();
            match(")");
            match(";");

            emit(2, "out", lex.c_str());
        }

        else if (look->lexeme == "println")
        {
            match("println");
            match("(");
            std::string lex = output();
            match(")");
            match(";");

            emit(3, "out", lex.c_str(), "\\n");
        }
        else
            PAR_LOG(ERRORS::BAD_TOK, "of type print or pritln", look->lexeme);
    }

    void inputStatement()
    {

        match("In");
        match(">>");
        std::string idLex = identifier();
        match(";");

        emit(2, "in", idLex.c_str());
    }

    void returnStatement()
    {

        match("ret");
        std::string lex = identifier();
        match(";");

        emit(2, "ret", lex.c_str());
    }

    int ifStatement()
    {

        match("if");
        std::pair<int, int> cmpRet = comparison();
        backpatch(cmpRet.first, n);
        match(":");
        match("{");
        codeBlock();
        match("}");

        return cmpRet.second;
    }

    int elifStatement()
    {

        match("elif");
        std::pair<int, int> cmpRet = comparison();
        backpatch(cmpRet.first, n);
        match(":");
        match("{");
        codeBlock();
        match("}");

        return cmpRet.second;
    }

    void elseStatement()
    {

        match("else");
        match("{");
        codeBlock();
        match("}");
    }

    void conditionalStatementPrime()
    {

        if (look->lexeme == "if")
        {
            int cmpF = ifStatement();
            int next = n;
            emit(1, "goto");
            backpatch(cmpF, n);
            conditionalStatementPrime();

            backpatch(next, n);
        }

        else if (look->lexeme == "elif")
        {
            int cmpF = elifStatement();
            int next = n;
            emit(1, "goto");
            backpatch(cmpF, n);
            conditionalStatementPrime();

            backpatch(next, n);
        }

        else if (look->lexeme == "else")
        {
            elseStatement();
            conditionalStatementPrime();
        }

        else
            ;
    }

    // Handles nested ifs
    void conditionalStatement()
    {

        int cmpF = ifStatement();
        int next = n;
        emit(1, "goto");
        backpatch(cmpF, n);
        conditionalStatementPrime();

        backpatch(next, n);
    }

    void statement()
    {

        Token curr_tok = *look;

        if (curr_tok.type == "DT")
        {
            variableDecleration();
        }

        else if (curr_tok.type == "ID" && peekToken().lexeme == "(")
        {
            functionCall();
        }

        else if (curr_tok.type == "ID")
        {
            variableAssignment();
        }

        else if (curr_tok.lexeme == "while")
        {
            whileLoop();
        }

        else if (curr_tok.lexeme == "print" || curr_tok.lexeme == "println")
        {
            printStatement();
        }

        else if (curr_tok.lexeme == "In")
        {
            inputStatement();
        }

        else if (curr_tok.lexeme == "if")
        {
            conditionalStatement();
        }

        else if (curr_tok.lexeme == "ret")
        {
            returnStatement();
        }

        else
            throw std::runtime_error("Unkown Statement found!");
    }

    void codeBlock()
    {

        Token curr_tok = *look;

        if (curr_tok.type == "DT" || curr_tok.type == "ID" || curr_tok.lexeme == "print" || curr_tok.lexeme == "println" || curr_tok.lexeme == "while" || curr_tok.lexeme == "In" || curr_tok.lexeme == "if" || curr_tok.lexeme == "ret")
        {

            statement();
            codeBlock();
        }
        else
            ;
    }

    void functionArgument()
    {

        if (look->type == "PUNC")
        {
            match(",");
            variable();
            functionArgument();
        }
        else if (look->type == "DT")
        {
            variable();
            functionArgument();
        }
        else
            ;
    }

    void functionDecleration()
    {

        match("func");

        datatype();
        match(":");
        identifier();

        match("(");
        functionArgument();
        match(")");

        match("{");
        codeBlock();
        match("}");
    }

    void source()
    {

        if (look != end)
        {

            // This assumes that everything in this language is
            // contained within a function, and there is no "stray" code
            functionDecleration();
            source();
        }
    }

    void run(const std::string& wListPath, const std::string &stPath)
    {

        auto wList = loadTokens(wListPath);
        loadST(stPath);

        look = wList.begin();
        end = wList.end();

        source();

        writeST();

        writeTAC();
    }
};