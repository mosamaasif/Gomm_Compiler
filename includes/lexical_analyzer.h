#pragma once

#include <vector>
#include <map>
#include <string>

typedef std::pair<std::string, std::string> PAIR;
class LexicalAnalyzer
{

public:
    static LexicalAnalyzer *getInstance()
    {

        if (!m_Instance)
            m_Instance = new LexicalAnalyzer();

        return m_Instance;
    }

    ~LexicalAnalyzer();

private:
    static LexicalAnalyzer *m_Instance;

private:
    LexicalAnalyzer();

    std::string loadFile(const std::string &scPath);

    void saveToFile(const std::vector<PAIR> &tokens);

    void handleArithmeticOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens);

    void handleRelationalOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens);

    void handleMiscOps(const std::string &sc, int &idx, std::vector<PAIR> &tokens);

    void handleLiterals(const std::string &sc, int &idx, std::vector<PAIR> &tokens);

    std::string fetchSourceCode();

    PAIR checkType(const std::string &buff);

    void run(const std::string &sc);
};