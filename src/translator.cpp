/* GENERATES BOTH TAC AND MACHINE CODE*/

#include "translator.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "error_handler.h"

Translator *Translator::m_Instance;

Translator::Translator(const std::string &wListPath, const std::string &stPath)
{
    run(wListPath, stPath);
}

std::vector<Translator::Token> Translator::loadTokens(const std::string &wListPath)
{

    std::ifstream in(wListPath);
    ASSERT(!in.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

    std::string line;
    std::vector<Translator::Token> toks;

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

std::vector<std::string> Translator::splitStr(const std::string &line, const char &delim)
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

void Translator::loadST(const std::string &stPath)
{

    std::ifstream in(stPath);
    ASSERT(!in.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

    std::string line;

    char delim = '|';

    std::getline(in, line);
    while (std::getline(in, line))
    {
        auto vals = splitStr(line, delim);
        symbols[vals[0]] = {vals[0], vals[1], 0, 0};
    }

    in.close();
}

void Translator::writeST()
{
    std::ofstream symTable("translator-symboltable.txt");

    symTable << "ID | DT | S | IV" << std::endl;

    std::vector<STData> vec;
    for (std::map<std::string, STData>::iterator itr = symbols.begin(); itr != symbols.end(); itr++)
        vec.push_back(itr->second);

    std::sort(vec.begin(), vec.end(), less_than_key());

    for (int i = 0; i < vec.size(); i++)
    {
        if (!vec[i].lex.empty()) {
            symTable << vec[i].lex
                << " | "
                << vec[i].dt
                << " | "
                << vec[i].addr
                << " | "
                << vec[i].initVal
                << '\n';
        }
    }

    symTable.close();
}

void Translator::writeTAC()
{

    std::ofstream out("tac.txt");
    ASSERT(!out.is_open(), ERRORS::TAC_FILE_OPEN_ERROR);

    for (int line = 0; line < tacLines.size(); line++)
        out << tacLines[line];
}

void Translator::writeMC()
{

    std::ofstream out("machine-code.txt");
    ASSERT(!out.is_open(), ERRORS::MC_FILE_OPEN_ERROR);

    for (int line = 0; line < tacLines.size(); line++)
        out << std::get<0>(quadTuples[line])
            << " "
            << std::get<1>(quadTuples[line])
            << " "
            << std::get<2>(quadTuples[line])
            << " "
            << std::get<3>(quadTuples[line])
            << "\n";
}

void Translator::emit(int num, ...)
{
    va_list valist;
    va_start(valist, num);
    std::string temp = "";
    for (int i = 0; i < num; i++)
    {
        temp += (va_arg(valist, char *));
        if (i < num - 1)
            temp += " ";
    }
    trim(temp);
    temp += "\n";
    tacLines.push_back(temp);
    va_end(valist);

    n++;
}

void Translator::quad(int a, int b, int c, int d)
{
    quadTuples.push_back({a, b, c, d});
}

std::string Translator::handleNumericConstant(const std::string &str)
{
    if (isNumber(str))
    {
        int val = atoi(str.c_str());
        std::string temp = "t" + std::to_string(tempValIdx++);
        symbols[temp] = {temp, "Integer", 0, val};
        addToST(4, temp);
        return temp;
    }
    return str;
}

void Translator::backpatch(int lineNo, int value)
{
    tacLines[lineNo - 1].pop_back();
    tacLines[lineNo - 1].append(" " + std::to_string(value));
    tacLines[lineNo - 1].push_back('\n');
}

void Translator::addToST(int size, std::string &lex)
{

    trim(lex);
    auto vec = splitStr(lex, ' ');

    if (!vec.empty())
    {

        for (const std::string &id : vec)
        {
            symbols[id].addr = addr;
            addr += size;
        }
    }
    else
    {
        symbols[lex].addr = addr;
        addr += size;
    }
}

Translator::Token Translator::peekToken()
{
    Translator::Token next_tok = *(++look);
    look--;
    return next_tok;
}

void Translator::match(const std::string &token)
{

    if (look->lexeme == token)
        look++;
    else
        TR_LOG(ERRORS::BAD_TOK, token, look->lexeme);
}

std::pair<std::string, int> Translator::datatype()
{

    if (look->lexeme == "char" || look->lexeme == "Integer")
    {
        std::pair<std::string, int> ret = {look->lexeme, (look->lexeme == "char") ? 1 : 4};
        look++;
        return ret;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "char or Integer", look->lexeme);
}

std::string Translator::identifier()
{

    std::string lex = look->lexeme;
    if (look->type == "ID")
    {
        look++;
        return lex;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "of type Identifier", look->lexeme);
}

void Translator::variable()
{

    std::pair<std::string, int> dt = datatype();
    match(":");
    std::string lex = identifier();

    addToST(dt.second, lex);
}

std::string Translator::variableDeclerationB()
{

    if (look->lexeme == ",")
    {
        match(",");
        return variableDeclerationA();
    }
    else
        return "";
}

std::string Translator::variableDeclerationA()
{

    std::string lex = identifier() + " ";
    lex += variableDeclerationB();
    return lex;
}

void Translator::variableDecleration()
{

    std::pair<std::string, int> dt = datatype();
    match(":");
    std::string lex = variableDeclerationA();
    match(";");

    addToST(dt.second, lex);
}

std::string Translator::literalConstant()
{

    if (look->type == "LC")
    {
        std::string lex = look->lexeme;
        look++;
        return lex;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "of type Literal Constant", look->lexeme);
}

std::string Translator::numConstant()
{

    if (look->type == "NUMC")
    {
        std::string lex = look->lexeme;
        look++;
        return lex;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "of type Number Constant/Number", look->lexeme);
}

std::string Translator::F()
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

std::string Translator::termPrime(std::string &prevLex)
{

    if (look->lexeme == "*")
    {
        match("*");
        auto fVal = F();
        std::string termLex = "* " + fVal + " ";

        std::string temp = "t" + std::to_string(tempValIdx++);
        symbols[temp] = {temp, "Integer", 0, 0};
        addToST(4, temp);
        emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
        quad(opcodes["*"], symbols[handleNumericConstant(prevLex)].addr, symbols[handleNumericConstant(fVal)].addr, symbols[temp].addr);

        return termPrime(temp);
    }
    else if (look->lexeme == "/")
    {
        match("/");
        auto fVal = F();
        std::string termLex = "/ " + fVal + " ";

        std::string temp = "t" + std::to_string(tempValIdx++);
        symbols[temp] = {temp, "Integer", 0, 0};
        addToST(4, temp);
        emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
        quad(opcodes["/"], symbols[handleNumericConstant(prevLex)].addr, symbols[handleNumericConstant(fVal)].addr, symbols[temp].addr);

        return termPrime(temp);
    }
    else
        return prevLex;
}

std::string Translator::term()
{

    std::string lex = F();
    return termPrime(lex);
}

std::string Translator::expressionPrime(std::string &prevLex)
{
    if (look->lexeme == "+")
    {
        match("+");
        auto tVal = term();
        std::string termLex = "+ " + tVal + " ";

        std::string temp = "t" + std::to_string(tempValIdx++);
        symbols[temp] = {temp, "Integer", 0, 0};
        addToST(4, temp);
        emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
        quad(opcodes["+"], symbols[handleNumericConstant(prevLex)].addr, symbols[handleNumericConstant(tVal)].addr, symbols[temp].addr);

        return expressionPrime(temp);
    }
    else if (look->lexeme == "-")
    {
        match("-");
        auto tVal = term();
        std::string termLex = "- " + tVal + " ";

        std::string temp = "t" + std::to_string(tempValIdx++);
        symbols[temp] = {temp, "Integer", 0, 0};
        addToST(4, temp);
        emit(4, temp.c_str(), "=", prevLex.c_str(), termLex.c_str());
        quad(opcodes["-"], symbols[handleNumericConstant(prevLex)].addr, symbols[handleNumericConstant(tVal)].addr, symbols[temp].addr);

        return expressionPrime(temp);
    }
    else
        return prevLex;
}

std::string Translator::expression()
{
    std::string termLex = term();
    std::string ePLex = expressionPrime(termLex);
    return ePLex;
}

std::string Translator::value()
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

void Translator::functionCallArgPrime()
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

void Translator::functionCallArg()
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

void Translator::functionCall()
{

    identifier();
    match("(");
    functionCallArg();
    match(")");
    match(";");
}

void Translator::variableAssignment()
{

    std::string idLex = identifier();
    match(":=");
    std::string valLex = value();
    match(";");

    emit(3, idLex.c_str(), "=", valLex.c_str());
    quad(opcodes["="], symbols[handleNumericConstant(valLex)].addr, symbols[idLex].addr, 0);
}

std::string Translator::relationalOperator()
{

    if (look->type == "RO")
    {
        std::string lex = look->lexeme;
        look++;
        return lex;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "of type RO", look->lexeme);
}

std::string Translator::with()
{

    if (look->type == "ID")
        return identifier();
    else if (look->type == "NUMC")
        return numConstant();
    else
        TR_LOG(ERRORS::BAD_TOK, "of type Identifier or Number", look->lexeme);
}

std::pair<int, int> Translator::comparison()
{

    // assuming first is supposed to be identifier, not any other thing
    // to maintain consistency and order
    std::string idLex = identifier();
    std::string reLex = relationalOperator();
    std::string wLex = with();

    std::pair<int, int> ret;
    ret.first = n;
    emit(5, "if", idLex.c_str(), reLex.c_str(), wLex.c_str(), "goto");
    std::string val = handleNumericConstant(wLex);
    quad(opcodes[reLex == "=" ? "==" : reLex], symbols[idLex].addr, symbols[val].addr, 0);
    ret.second = n;
    emit(1, "goto");
    quad(opcodes["goto"], 0, 0, 0);

    return ret;
}

void Translator::whileLoop()
{

    match("while");
    int start = n;
    std::pair<int, int> cmpRet = comparison();
    backpatch(cmpRet.first, n);
    std::get<3>(quadTuples[cmpRet.first - 1]) = n;
    match(":");
    match("{");
    codeBlock();
    match("}");

    emit(2, "goto", std::to_string(start).c_str());
    quad(opcodes["goto"], start, 0, 0);
    backpatch(cmpRet.second, n);
    std::get<1>(quadTuples[cmpRet.second - 1]) = n;
}

std::string Translator::stringLiteral()
{

    std::string lex;
    if (look->type == "STR")
    {
        lex = look->lexeme;
        look++;
        return lex;
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "String", look->lexeme);
}

std::string Translator::output()
{

    if (look->type == "STR")
        return stringLiteral();

    else if (look->type == "ID")
        return identifier();
    else
        TR_LOG(ERRORS::BAD_TOK, "of type String or Identifier", look->lexeme);
}

void Translator::printStatement()
{

    if (look->lexeme == "print")
    {
        match("print");
        match("(");
        std::string lex = output();
        match(")");
        match(";");

        emit(2, "out", lex.c_str());
        quad(opcodes["out"], symbols[lex].addr, 0, 0);
    }

    else if (look->lexeme == "println")
    {
        match("println");
        match("(");
        std::string lex = output();
        match(")");
        match(";");

        emit(2, "outln", lex.c_str());
        quad(opcodes["outln"], symbols[lex].addr, 0, 0);
    }
    else
        TR_LOG(ERRORS::BAD_TOK, "of type print or pritln", look->lexeme);
}

void Translator::inputStatement()
{

    match("In");
    match(">>");
    std::string idLex = identifier();
    match(";");

    emit(2, "in", idLex.c_str());
    quad(opcodes["in"], symbols[idLex].addr, 0, 0);
}

void Translator::returnStatement()
{

    match("ret");
    std::string lex = identifier();
    match(";");

    emit(2, "ret", lex.c_str());
    quad(opcodes["ret"], symbols[lex].addr, 0, 0);
}

int Translator::ifStatement()
{

    match("if");
    std::pair<int, int> cmpRet = comparison();
    backpatch(cmpRet.first, n);
    std::get<3>(quadTuples[cmpRet.first - 1]) = n;
    match(":");
    match("{");
    codeBlock();
    match("}");

    return cmpRet.second;
}

int Translator::elifStatement()
{

    match("elif");
    std::pair<int, int> cmpRet = comparison();
    backpatch(cmpRet.first, n);
    std::get<3>(quadTuples[cmpRet.first - 1]) = n;
    match(":");
    match("{");
    codeBlock();
    match("}");

    return cmpRet.second;
}

void Translator::elseStatement()
{

    match("else");
    match("{");
    codeBlock();
    match("}");
}

void Translator::conditionalStatementPrime()
{

    if (look->lexeme == "if")
    {
        int cmpF = ifStatement();
        int next = n;
        emit(1, "goto");
        quad(opcodes["goto"], 0, 0, 0);
        backpatch(cmpF, n);
        std::get<1>(quadTuples[cmpF - 1]) = n;
        conditionalStatementPrime();

        backpatch(next, n);
        std::get<1>(quadTuples[next - 1]) = n;
    }

    else if (look->lexeme == "elif")
    {
        int cmpF = elifStatement();
        int next = n;
        emit(1, "goto");
        quad(opcodes["goto"], 0, 0, 0);
        backpatch(cmpF, n);
        std::get<1>(quadTuples[cmpF - 1]) = n;
        conditionalStatementPrime();

        backpatch(next, n);
        std::get<1>(quadTuples[next - 1]) = n;
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
void Translator::conditionalStatement()
{

    int cmpF = ifStatement();
    int next = n;
    emit(1, "goto");
    quad(opcodes["goto"], 0, 0, 0);
    backpatch(cmpF, n);
    std::get<1>(quadTuples[cmpF - 1]) = n;

    conditionalStatementPrime();

    backpatch(next, n);
    std::get<1>(quadTuples[next - 1]) = n;
}

void Translator::statement()
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

void Translator::codeBlock()
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

void Translator::functionArgument()
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

void Translator::functionDecleration()
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

void Translator::source()
{

    if (look != end)
    {

        // This assumes that everything in this language is
        // contained within a function, and there is no "stray" code
        functionDecleration();
        source();
    }
}

void Translator::run(const std::string &wListPath, const std::string &stPath)
{

    auto wList = loadTokens(wListPath);
    loadST(stPath);

    look = wList.begin();
    end = wList.end();

    source();

    writeST();

    writeTAC();

    writeMC();
}

Translator::~Translator()
{
    delete m_Instance;
}