/*
*
*   FROM BONUS handles nested ifs etc, but not functions
*/

#pragma once

#include <map>
#include <cstdarg>
#include <string>
#include <vector>
class Translator
{

public:
    static Translator *getInstance(const std::string &wListPath, const std::string &stPath)
    {
        if (!m_Instance)
            m_Instance = new Translator(wListPath, stPath);

        return m_Instance;
    }

private:
    static Translator *m_Instance;

    struct STData
    {
        std::string lex, dt;
        int addr;
    };

    struct less_than_key
    {
        inline bool operator()(const STData &struct1, const STData &struct2)
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

private:
    Translator(const std::string &wListPath, const std::string &stPath);

    std::vector<Token> loadTokens(const std::string &wListPath);

    std::vector<std::string> splitStr(const std::string &line, const char &delim);

    void loadST(const std::string &stPath);

    void writeST();

    void writeTAC();

    static inline void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
    }

    static inline void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                s.end());
    }

    static inline void trim(std::string &s)
    {
        ltrim(s);
        rtrim(s);
    }

    void emit(int num, ...);

    void backpatch(int lineNo, int value);

    void addToST(int size, std::string &lex);

    Token peekToken();

    void match(const std::string &token);

    std::pair<std::string, int> datatype();

    std::string identifier();

    void variable();

    std::string variableDeclerationB();

    std::string variableDeclerationA();

    void variableDecleration();

    std::string literalConstant();

    std::string numConstant();

    std::string F();

    std::string termPrime(std::string &prevLex);

    std::string term();

    std::string expressionPrime(std::string &prevLex);

    std::string expression();

    std::string value();

    void functionCallArgPrime();

    void functionCallArg();

    void functionCall();

    void variableAssignment();

    std::string relationalOperator();

    std::string with();

    std::pair<int, int> comparison();

    void whileLoop();

    std::string stringLiteral();

    std::string output();

    void printStatement();

    void inputStatement();

    void returnStatement();

    int ifStatement();

    int elifStatement();

    void elseStatement();

    void conditionalStatementPrime();

    // Handles nested ifs
    void conditionalStatement();

    void statement();

    void codeBlock();

    void functionArgument();

    void functionDecleration();

    void source();

    void run(const std::string &wListPath, const std::string &stPath);
};