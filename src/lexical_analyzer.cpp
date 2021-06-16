#include "lexical_analyzer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include "error_handler.h"

LexicalAnalyzer *LexicalAnalyzer::m_Instance;

static const std::map<std::string, std::string> KEYWORDS = {
    {"while", "^"}, {"func", "^"}, {"ret", "^"}, {"if", "CS"}, {"elif", "CS"}, {"else", "CS"}, {"In", "IOS"}, {"print", "IOS"}, {"println", "IOS"}, {"Integer", "DT"}, {"char", "DT"}};

static const std::map<std::string, std::string> OPERATORS = {
    {"+", "AO"}, {"-", "AO"}, {"*", "AO"}, {"/", "AO"}, {">", "RO"}, {">=", "RO"}, {"<", "RO"}, {"<=", "RO"}, {"=", "RO"}, {"/=", "RO"}, {":", "^"}, {":=", "^"}, {">>", "^"}};

static const std::map<std::string, std::string> SYMBOLS = {
    {"/*", "CMNT"}, {"*/", "CMNT"}, {"(", "BRKT"}, {")", "BRKT"}, {"[", "BRKT"}, {"]", "BRKT"}, {"{", "BRKT"}, {"}", "BRKT"}, {",", "PUNC"}, {";", "PUNC"}, {":", "PUNC"}};

static const std::string ID_REGEX = "[a-zA-Z][a-zA-Z0-9]*";
static const std::string NUMC_REGEX = "[0-9]+";
static const std::string QUOTE_REGEX = "[\'\"]";

LexicalAnalyzer::LexicalAnalyzer()
{
    run(fetchSourceCode());
}

std::string LexicalAnalyzer::loadFile(const std::string &scPath)
{

    ASSERT(scPath.substr(scPath.length() - 3, scPath.length() - 1) != ".go", ERRORS::WRONG_FILE_EXT);

    std::ifstream in(scPath);
    ASSERT(!in.is_open(), ERRORS::SC_FILE_OPEN_ERROR);

    if (scPath.find('/') != std::string::npos)
        FILENAME = scPath.substr(scPath.find_last_of('/') + 1);
    else
        FILENAME = scPath;

    std::ostringstream stream;
    stream << in.rdbuf();

    std::string data = stream.str();

    in.close();

    return data;
}

void LexicalAnalyzer::saveToFile(const std::vector<PAIR> &tokens)
{

    std::ofstream out("data/words.txt");
    ASSERT(!out.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

    for (const PAIR token : tokens)
    {
        std::string str = "(";
        if (token.second == "^")
            str += token.first + ", " + token.second + ")";
        else
            str += token.second + ", " + token.first + ")";
        out << str << std::endl;
    }

    out.close();
}

void LexicalAnalyzer::handleArithmeticOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens)
{
    char prevCh = sc[idx - 1];
    if (prevCh == '+' || prevCh == '-')
        tokens.emplace_back(std::string(1, prevCh), "AO");

    else if (prevCh == '/')
    {
        //not equal to
        if (sc[idx] == '=')
        {
            tokens.emplace_back("/=", "RO");
            idx++;
        }

        //comment start
        else if (sc[idx] == '*')
        {
            auto cmntEndIdx = sc.find("*/", idx);
            if (cmntEndIdx != std::string::npos)
                idx = cmntEndIdx + 2;
            else
            {
                LA_LOG(ERRORS::CMNT_DANGLING_MARKER_START);
            }
        }

        //division case
        else
            tokens.emplace_back("/", "AO");
    }

    else if (prevCh == '*')
    {
        if (sc[idx] == '/')
        {
            LA_LOG(ERRORS::CMNT_DANGLING_MARKER_END);
        }
        else
            tokens.emplace_back("*", "AO");
    }
}

void LexicalAnalyzer::handleRelationalOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens)
{
    char prevCh = sc[idx - 1];
    if (prevCh == '=')
        tokens.emplace_back("=", "RO");

    else if (prevCh == '<')
    {
        //incase of <=
        if (sc[idx] == '=')
        {
            tokens.emplace_back("<=", "RO");
            idx++;
        }

        //incase of <
        else
            tokens.emplace_back("<", "RO");
    }

    else if (prevCh == '>')
    {
        //incase of >=
        if (sc[idx] == '=')
        {
            tokens.emplace_back(">=", "RO");
            idx++;
        }

        //incase of >>
        else if (sc[idx] == '>')
        {
            tokens.emplace_back(">>", "^");
            idx++;
        }

        //incase of >
        else
            tokens.emplace_back(">", "RO");
    }
}

void LexicalAnalyzer::handleMiscOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens)
{
    char prevCh = sc[idx - 1];
    if (prevCh == ':')
    {
        if (sc[idx] == '=')
        {
            tokens.emplace_back(":=", "^");
            idx++;
        }

        else
            tokens.emplace_back(":", "^");
    }
}

void LexicalAnalyzer::handleLiterals(const std::string &sc, int &idx, std::vector<PAIR> &tokens)
{
    char prevCh = sc[idx - 1];
    if (prevCh == '\'')
    {
        if (sc[idx + 1] == '\'')
        {
            std::string str = "'";
            str.push_back(sc[idx]);
            str.push_back('\'');
            tokens.emplace_back(str, "LC");
            idx += 2;
        }
        else
        {
            LA_LOG(ERRORS::LITERAL_CONST_ERR);
        }
    }

    else if (prevCh == '\"')
    {
        auto endQuoteIdx = sc.find('\"', idx);
        if (endQuoteIdx != std::string::npos)
        {
            tokens.emplace_back("\"" + sc.substr(idx, (endQuoteIdx - idx)) + "\"", "STR");
            idx = endQuoteIdx + 1;
        }
        else
        {
            LA_LOG(ERRORS::LITERAL_STR_ERR);
        }
    }
}

std::string LexicalAnalyzer::fetchSourceCode()
{

    std::cout << "Enter Source Code File Path: ";

    std::string scPath;
    std::getline(std::cin, scPath);

    return loadFile(scPath);
}

PAIR LexicalAnalyzer::checkType(const std::string &buff)
{

    PAIR ret = {"", ""};

    //checking for keywords
    auto value = KEYWORDS.find(buff);
    if (value != KEYWORDS.end())
    {
        if (buff == "while" || buff == "func" || buff == "ret")
        {
            std::string temp;
            for (int i = 0; i < buff.size(); i++)
                temp.push_back(toupper(buff[i]));
            ret = {temp, value->second};
        }
        else
            ret = {buff, value->second};
    }

    //checking for identifiers
    else if (std::regex_match(buff, std::regex(ID_REGEX)))
        ret = {buff, "ID"};

    //checking for integer values
    else if (std::regex_match(buff, std::regex(NUMC_REGEX)))
        ret = {buff, "NUMC"};

    return ret;
}

void LexicalAnalyzer::run(const std::string &sc)
{

    std::vector<PAIR> tokens;

    int idx = 0;
    LINENUM = 1;
    std::string buff;
    while (idx < sc.length())
    {
        //incase word ends, or any other whitespace char is found
        if (sc[idx] == ' ' || sc[idx] == '\n' || sc[idx] == '\t' || sc[idx] == '\r')
        {
            if (!buff.empty())
            {
                PAIR ret = checkType(buff);
                if (ret.first.empty())
                {
                    LA_LOG(ERRORS::NOT_KEY_ID_INT);
                }
                tokens.push_back(ret);
                buff.clear();
            }
            if (sc[idx] == '\n' || sc[idx] == '\t' || sc[idx] == '\r')
                LINENUM++;
            idx++;
        }

        //incase char is either letter or digit, or it's - sign followed by number, showing it's part of integer
        else if ((isalnum(sc[idx]) != 0) || (idx < sc.length() - 1 && sc[idx] == '-' && isdigit(sc[idx + 1])))
        {
            buff.push_back(sc[idx++]);
        }

        //incase char is an operator
        else if (OPERATORS.find(std::string(1, sc[idx])) != OPERATORS.end())
        {
            std::string type = OPERATORS.find(std::string(1, sc[idx]))->second;
            if (!buff.empty())
            {
                PAIR ret = checkType(buff);
                if (ret.first.empty())
                {
                    LA_LOG(ERRORS::NOT_KEY_ID_INT);
                }
                tokens.push_back(ret);
                buff.clear();
            }
            idx++;
            if (idx < sc.length())
            {
                if (type == "AO") //for arithmetic
                    handleArithmeticOps(sc, idx, tokens);
                else if (type == "RO") //for relational
                    handleRelationalOps(sc, idx, tokens);
                else //for others
                    handleMiscOps(sc, idx, tokens);
            }
        }

        //incase of symbols
        else if (SYMBOLS.find(std::string(1, sc[idx])) != SYMBOLS.end())
        {
            std::string type = SYMBOLS.find(std::string(1, sc[idx]))->second;
            if (!buff.empty())
            {
                PAIR ret = checkType(buff);
                if (ret.first.empty())
                {
                    LA_LOG(ERRORS::NOT_KEY_ID_INT);
                }
                tokens.push_back(ret);
                buff.clear();
            }
            if (type == "BRKT") //for arithmetic
                tokens.emplace_back(std::string(1, sc[idx]), "BRKT");
            else if (type == "PUNC") //for punctuations
                tokens.emplace_back(std::string(1, sc[idx]), "PUNC");
            idx++;
        }

        //incase of single or double quotations
        else if (std::regex_match(std::string(1, sc[idx]), std::regex(QUOTE_REGEX)))
        {
            if (!buff.empty())
            {
                PAIR ret = checkType(buff);
                if (ret.first.empty())
                {
                    LA_LOG(ERRORS::NOT_KEY_ID_INT);
                }
                tokens.push_back(ret);
                buff.clear();
            }
            idx++;
            handleLiterals(sc, idx, tokens);
        }

        else
            LA_LOG(ERRORS::NOT_KEY_ID_INT);
    }

    saveToFile(tokens);
}

LexicalAnalyzer::~LexicalAnalyzer()
{
    delete m_Instance;
}