#include "parser.h"

#include <iostream>

#include "error_handler.h"

Parser *Parser::m_Instance;

Parser::Parser(const std::string &wListPath)
{
    run(wListPath);
}

std::vector<Parser::Token> Parser::loadTokens(const std::string &wListPath)
{

    std::ifstream in(wListPath);
    ASSERT(!in.is_open(), ERRORS::TOK_FILE_OPEN_ERROR);

    std::string line;
    std::vector<Parser::Token> toks;

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
        toks.push_back(Parser::Token(id, tok));
    }

    return toks;
}

void Parser::writeST()
{
    std::ofstream symTable("data/parser-symboltable.txt");

    symTable << "ID | DT" << std::endl;
    for (SYMBOL_MAP::iterator itr = symbols.begin(); itr != symbols.end(); itr++)
    {
        symTable << itr->first << " | " << itr->second << '\n';
    }

    symTable.close();
}

void Parser::printParseTree(const std::string &str)
{
    if (tabsCount >= 1)
    {
        parseTreeFile << "|";
        for (int i = 0; i < tabsCount - 1; i++)
        {
            parseTreeFile << "-";
        }
        parseTreeFile << "|";
    }

    parseTreeFile << str << std::endl;
}

Parser::Token Parser::peekToken()
{
    Parser::Token next_tok = *(++look);
    look--;
    return next_tok;
}

void Parser::match(const std::string &token)
{

    if (look->lexeme == token)
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, token, look->lexeme);
}

void Parser::datatype()
{

    if (look->lexeme == "char" || look->lexeme == "Integer")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "char or Integer", look->lexeme);
}

void Parser::identifier()
{

    if (look->type == "ID")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type Identifier", look->lexeme);
}

void Parser::variable()
{

    SYMBOL cSymbol;
    tabsCount++;
    cSymbol.second = look->lexeme;
    datatype();
    printParseTree(":");
    match(":");
    cSymbol.first = look->lexeme;
    identifier();
    tabsCount--;

    symbols.insert(cSymbol);
}

void Parser::variableDeclerationB(SYMBOL cSymbol)
{

    tabsCount++;
    if (look->lexeme == ",")
    {
        printParseTree(",");
        match(",");

        cSymbol.first = look->lexeme;
        symbols.insert(cSymbol);

        printParseTree("VariableDeclerationA");
        variableDeclerationA(cSymbol.second);
    }
    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::variableDeclerationA(std::string currDT)
{

    SYMBOL cSymbol;

    tabsCount++;
    cSymbol.second = currDT;
    cSymbol.first = look->lexeme;
    symbols.insert(cSymbol);
    identifier();
    printParseTree("VariableDeclerationB");
    variableDeclerationB(cSymbol);
    tabsCount--;
}

void Parser::variableDecleration()
{

    std::string currDT = look->lexeme;
    tabsCount++;
    datatype();
    printParseTree(":");
    match(":");
    printParseTree("VariableDeclerationA");
    variableDeclerationA(currDT);
    printParseTree(";");
    match(";");
    tabsCount--;
}

void Parser::literalConstant()
{

    if (look->type == "LC")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type Literal Constant", look->lexeme);
}

void Parser::numConstant()
{

    if (look->type == "NUMC")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type Number Constant/Number", look->lexeme);
}

void Parser::F()
{

    tabsCount++;
    if (look->type == "ID")
        identifier();

    else if (look->type == "NUMC")
        numConstant();

    else if (look->lexeme == "(")
    {
        printParseTree("(");
        match("(");
        printParseTree("Expression");
        expression();
        printParseTree(")");
        match(")");
    }
    else
        throw std::runtime_error("Invalid Expression found!");
    tabsCount--;
}

void Parser::termPrime()
{

    tabsCount++;
    if (look->lexeme == "*")
    {
        printParseTree("*");
        match("*");
        printParseTree("F");
        F();
        printParseTree("TermPrime");
        termPrime();
    }
    else if (look->lexeme == "/")
    {
        printParseTree("/");
        match("/");
        printParseTree("F");
        F();
        printParseTree("TermPrime");
        termPrime();
    }
    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::term()
{

    tabsCount++;
    printParseTree("F");
    F();
    printParseTree("TermPrime");
    termPrime();
    tabsCount--;
}

void Parser::expressionPrime()
{

    tabsCount++;
    if (look->lexeme == "+")
    {
        printParseTree("+");
        match("+");
        printParseTree("Term");
        term();
        printParseTree("ExpressionPrime");
        expressionPrime();
    }
    else if (look->lexeme == "-")
    {
        printParseTree("-");
        match("-");
        printParseTree("Term");
        term();
        printParseTree("ExpressionPrime");
        expressionPrime();
    }
    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::expression()
{
    tabsCount++;
    printParseTree("Term");
    term();
    printParseTree("ExpressionPrime");
    expressionPrime();
    printParseTree("--");
    tabsCount--;
}

void Parser::value()
{

    tabsCount++;
    if (look->type == "LC")
    {
        printParseTree("LiteralConstant");
        literalConstant();
    }

    else if (look->type == "ID" && peekToken().lexeme == "(")
    {
        printParseTree("FunctionalCall");
        functionCall();
    }

    else
    {
        printParseTree("Expression");
        expression();
    }
    tabsCount--;
}

void Parser::functionCallArgPrime()
{

    tabsCount++;
    if (look->lexeme == ",")
    {
        printParseTree(",");
        match(",");
        printParseTree("Value");
        value();
        printParseTree("MultipleFunctionalCallArgs");
        functionCallArgPrime();
    }
    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::functionCallArg()
{

    tabsCount++;
    if (look->lexeme == ")")
    {
        printParseTree("^");
    }
    else
    {
        printParseTree("Value");
        value();
        printParseTree("MultipleFunctionalArgs");
        functionCallArgPrime();
    }
    tabsCount--;
}

void Parser::functionCall()
{

    tabsCount++;
    identifier();
    printParseTree("(");
    match("(");
    printParseTree("FunctionalArgs");
    functionCallArg();
    printParseTree(")");
    match(")");
    printParseTree(";");
    match(";");
    tabsCount--;
}

void Parser::variableAssignment()
{

    tabsCount++;
    identifier();
    printParseTree(":=");
    match(":=");
    printParseTree("Value");
    value();
    printParseTree(";");
    match(";");
    tabsCount--;
}

void Parser::relationalOperator()
{

    std::string curr_lexeme = look->lexeme;
    if (look->type == "RO")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type RO", look->lexeme);
}

void Parser::with()
{

    if (look->type == "ID")
        identifier();
    else if (look->type == "NUMC")
        numConstant();
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type Identifier or Number", look->lexeme);
}

void Parser::comparison()
{

    tabsCount++;
    // assuming first is supposed to be identifier, not any other thing
    // to maintain consistency and order
    identifier();
    printParseTree("RelationalOp");
    relationalOperator();
    printParseTree("CompWith");
    with();
    tabsCount--;
}

void Parser::whileLoop()
{
    tabsCount++;
    printParseTree("while");
    match("while");
    printParseTree("CompCondition");
    comparison();
    printParseTree(":");
    match(":");
    printParseTree("{");
    match("{");
    printParseTree("CodeBlock");
    codeBlock();
    printParseTree("}");
    match("}");
    tabsCount--;
}

void Parser::stringLiteral()
{

    if (look->type == "STR")
        look++;
    else
        PAR_LOG(ERRORS::BAD_TOK, "String", look->lexeme);
}

void Parser::output()
{

    if (look->type == "STR")
        stringLiteral();

    else if (look->type == "ID")
        identifier();
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type String or Identifier", look->lexeme);
}

void Parser::printStatement()
{

    tabsCount++;
    if (look->lexeme == "print")
    {
        printParseTree("print");
        match("print");
        printParseTree("(");
        match("(");
        printParseTree("Output");
        output();
        printParseTree(")");
        match(")");
        printParseTree(";");
        match(";");
    }

    else if (look->lexeme == "println")
    {
        printParseTree("println");
        match("println");
        printParseTree("(");
        match("(");
        printParseTree("Output");
        output();
        printParseTree(")");
        match(")");
        printParseTree(";");
        match(";");
    }
    else
        PAR_LOG(ERRORS::BAD_TOK, "of type print or pritln", look->lexeme);
    tabsCount--;
}

void Parser::inputStatement()
{

    tabsCount++;
    printParseTree("In");
    match("In");
    printParseTree(">>");
    match(">>");
    identifier();
    printParseTree(";");
    match(";");
    tabsCount--;
}

void Parser::returnStatement()
{

    tabsCount++;
    printParseTree("ret");
    match("ret");
    identifier();
    printParseTree(";");
    match(";");
    tabsCount--;
}

void Parser::ifStatement()
{

    tabsCount++;
    printParseTree("if");
    match("if");
    printParseTree("CompCondition");
    comparison();
    printParseTree(":");
    match(":");
    printParseTree("{");
    match("{");
    printParseTree("CodeBlock");
    codeBlock();
    printParseTree("}");
    match("}");
    tabsCount--;
}

void Parser::elifStatement()
{

    tabsCount++;
    printParseTree("elif");
    match("elif");
    printParseTree("CompCondition");
    comparison();
    printParseTree(":");
    match(":");
    printParseTree("{");
    match("{");
    printParseTree("CodeBlock");
    codeBlock();
    printParseTree("}");
    match("}");
    tabsCount--;
}

void Parser::elseStatement()
{

    tabsCount++;
    printParseTree("else");
    match("else");
    printParseTree("{");
    match("{");
    printParseTree("CodeBlock");
    codeBlock();
    printParseTree("}");
    match("}");
    tabsCount--;
}

void Parser::conditionalStatementPrime()
{

    tabsCount++;
    if (look->lexeme == "if")
    {
        printParseTree("ifStatement");
        ifStatement();
        printParseTree("ConditionalStatement");
        conditionalStatementPrime();
    }

    else if (look->lexeme == "elif")
    {
        printParseTree("elifStatement");
        elifStatement();
        printParseTree("ConditionalStatement");
        conditionalStatementPrime();
    }

    else if (look->lexeme == "else")
    {
        printParseTree("elseStatement");
        elseStatement();
        printParseTree("ConditionalStatement");
        conditionalStatementPrime();
    }

    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::conditionalStatement()
{

    tabsCount++;
    printParseTree("ifStatement");
    ifStatement();
    printParseTree("ConditionalStatement");
    conditionalStatementPrime();
    tabsCount--;
}

void Parser::statement()
{

    tabsCount++;
    Token curr_tok = *look;

    if (curr_tok.type == "DT")
    {
        printParseTree("VariableDecleration");
        variableDecleration();
    }

    else if (curr_tok.type == "ID" && peekToken().lexeme == "(")
    {
        printParseTree("FunctionCall");
        functionCall();
    }

    else if (curr_tok.type == "ID")
    {
        printParseTree("VariableAssignment");
        variableAssignment();
    }

    else if (curr_tok.lexeme == "while")
    {
        printParseTree("WhileLoop");
        whileLoop();
    }

    else if (curr_tok.lexeme == "print" || curr_tok.lexeme == "println")
    {
        printParseTree("PrintStatement");
        printStatement();
    }

    else if (curr_tok.lexeme == "In")
    {
        printParseTree("InputStatement");
        inputStatement();
    }

    else if (curr_tok.lexeme == "if")
    {
        printParseTree("ConditionalStatement");
        conditionalStatement();
    }

    else if (curr_tok.lexeme == "ret")
    {
        printParseTree("ReturnStatement");
        returnStatement();
    }

    else
        throw std::runtime_error("Unkown Statement found!");
    tabsCount--;
}

void Parser::codeBlock()
{

    tabsCount++;
    Token curr_tok = *look;

    if (curr_tok.type == "DT" || curr_tok.type == "ID" || curr_tok.lexeme == "print" || curr_tok.lexeme == "println" || curr_tok.lexeme == "while" || curr_tok.lexeme == "In" || curr_tok.lexeme == "if" || curr_tok.lexeme == "ret")
    {

        printParseTree("Statement");
        statement();
        printParseTree("CodeBlock");
        codeBlock();
    }
    else
        printParseTree("^");
    tabsCount--;
}

void Parser::functionArgument()
{

    tabsCount++;
    if (look->type == "PUNC")
    {
        printParseTree(",");
        match(",");
        printParseTree("Variable");
        variable();
        printParseTree("FunctionArguments");
        functionArgument();
    }
    else if (look->type == "DT")
    {
        printParseTree("Variable");
        variable();
        printParseTree("FunctionArguments");
        functionArgument();
    }
    else
        printParseTree("^");
    ;
    tabsCount--;
}

void Parser::functionDecleration()
{

    tabsCount++;

    printParseTree("func");
    match("func");

    datatype();
    printParseTree(":");
    match(":");
    identifier();

    printParseTree("(");
    match("(");
    printParseTree("FunctionArguments");
    functionArgument();
    printParseTree(")");
    match(")");

    printParseTree("{");
    match("{");
    printParseTree("CodeBlock");
    codeBlock();
    printParseTree("}");
    match("}");

    tabsCount--;
}

void Parser::source()
{

    tabsCount++;
    if (look != end)
    {

        // This assumes that everything in this language is
        // contained within a function, and there is no "stray" code
        printParseTree("FunctionDecleration");
        functionDecleration();
        printParseTree("SourceProgram");
        source();
    }
    else
        printParseTree("^");
    tabsCount--;
}

std::vector<Parser::Token> Parser::run(const std::string &wListPath)
{

    std::vector<Parser::Token> toks = loadTokens(wListPath);

    look = toks.begin();
    end = toks.end();

    parseTreeFile = std::ofstream("data/parsetree.txt");
    source();
    writeST();

    return toks;
}

Parser::~Parser()
{
    delete m_Instance;
}