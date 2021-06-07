#pragma once

#include <stdexcept>
#include <cstdio>
#include <string>

enum ERRORS {
    WRONG_FILE_EXT,
    SC_FILE_OPEN_ERROR,
    TOK_FILE_OPEN_ERROR,
    NOT_KEY_ID_INT,
    CMNT_DANGLING_MARKER_END,
    CMNT_DANGLING_MARKER_START,
    LITERAL_CONST_ERR,
    LITERAL_STR_ERR,
    BAD_TOK,
    SYMBOL_TABLE_FILE_OPEN_ERROR,
    TAC_FILE_OPEN_ERROR
};

static std::string FILENAME;
static int LINENUM;

static const std::string ERR_MSGS[] = {
        "File Extension is wrong, should be \".go\"",
        "Failed to Open the Source Code File",
        "Failed to Open Tokens File",
        "Does not match any keyword, identifier or integer",
        "Comment has no starting marker, only dangling end marker",
        "Comment has no ending marker, only dangling start marker",
        "Literal Constant has more than one char",
        "Literal String has no termination quote",
        "Bad Token",
        "Failed to Open Symbol Table File",
        "Failed to Open Three Address Code File"
};


static std::string buildErrorStr(const ERRORS& err) {
    std::string error = std::string()
                        + "[ERROR]"
                        + "\n\tMESSAGE: "
                        + ERR_MSGS[err]
                        + "\n\tFILE: "
                        + __FILE__
                        + "\n\tLINE: "
                        + std::to_string(__LINE__)
                        + "\n\n";

    return error;
}

static std::string buildLAErrorStr(const ERRORS& err) {
    std::string error = std::string()
                        + "[LEXICAL ANALYZER ERROR]"
                        + "\n\tMESSAGE: "
                        + ERR_MSGS[err]
                        + "\n\tFILE: "
                        + FILENAME
                        + "\n\tLINE: "
                        + std::to_string(LINENUM)
                        + "\n\n";

    return error;
}

static std::string buildParErrorStr(const ERRORS& err, const std::string& expTok,  const std::string& foundTok) {
    std::string error = std::string()
                        + "[PARSER ERROR]"
                        + "\n\tTYPE: "
                        + ERR_MSGS[err]
                        + "\n\tMESSAGE: "
                        + "Expected token \'"
                        + expTok
                        + "\', but found token \'"
                        + foundTok
                        + "\'"
                        + "\n\tFILE: "
                        + FILENAME
                        + "\n\n";

    return error;
}

#define ASSERT(cmp, err_type)                                                           \
            if(cmp)                                                                     \
                throw std::runtime_error(buildErrorStr(err_type))

#define LA_LOG(err_type)                                                                \
            throw std::runtime_error(buildLAErrorStr(err_type))

#define PAR_LOG(err_type, expTok, foundTok)                                                               \
            throw std::runtime_error(buildParErrorStr(err_type, expTok, foundTok))
