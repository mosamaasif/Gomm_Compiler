#pragma once

#include <vector>
#include <string>
#include <map>

class VirtualMachine {

public:
	static VirtualMachine* getInstance() {
		if (m_Instance == nullptr)
			m_Instance = new VirtualMachine();
		return m_Instance;
	}

    void run();

	~VirtualMachine();

private:
	static VirtualMachine* m_Instance;

	std::vector<std::vector<int>> quadruples;

	std::vector<int> dataSection;

private:
	VirtualMachine();

    static inline void ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
            { return !std::isspace(ch); }));
    }

    static inline void rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
            { return !std::isspace(ch); })
            .base(),
            s.end());
    }

    static inline void trim(std::string& s)
    {
        ltrim(s);
        rtrim(s);
    }

	std::vector<std::string> splitStr(const std::string& line, const char& delim);

	void populateDataSection();

	void populateQuadruple();
};