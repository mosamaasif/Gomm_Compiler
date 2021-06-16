#pragma once

#include <map>
#include <fstream>
#include <string>
#include <vector>

class Parser
{

public:
    static Parser *getInstance(const std::string &wListPath)
    {
        if (!m_Instance)
            m_Instance = new Parser(wListPath);

        return m_Instance;
    }

private:
    static Parser *m_Instance;

    typedef std::map<std::string, std::string> SYMBOL_MAP;
    typedef std::pair<std::string, std::string> SYMBOL;

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

    std::ofstream parseTreeFile;

    int tabsCount;

    SYMBOL_MAP symbols;

private:
    Parser(const std::string &wListPath);

    std::vector<Token> loadTokens(const std::string &wListPath);

    void writeST();

    void printParseTree(const std::string &str);

    Token peekToken();

    void match(const std::string &token);

    void datatype();

    void identifier();

    void variable();

    void variableDeclerationB(SYMBOL cSymbol);

    void variableDeclerationA(std::string currDT);

    void variableDecleration();

    void literalConstant();

    void numConstant();

    void F();

    void termPrime();

    void term();

    void expressionPrime();

    void expression();

    void value();

    void functionCallArgPrime();

    void functionCallArg();

    void functionCall();

    void variableAssignment();

    void relationalOperator();

    void with();

    void comparison();

    void whileLoop();

    void stringLiteral();

    void output();

    void printStatement();

    void inputStatement();

    void returnStatement();

    void ifStatement();

    void elifStatement();

    void elseStatement();

    void conditionalStatementPrime();

    void conditionalStatement();

    void statement();

    void codeBlock();

    void functionArgument();

    void functionDecleration();

    void source();

    std::vector<Token> run(const std::string &wListPath);
};