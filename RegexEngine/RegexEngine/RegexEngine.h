#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

class StateMachine;

class RegexEngine
{
public:
	virtual ~RegexEngine();
	static RegexEngine* Get();
	void compile(const std::string regex, StateMachine& machine);
	bool match(const std::string input, StateMachine& machine);
	bool matchWithSubmatchExtraction(const std::string input, StateMachine& machine, std::vector<std::string>& submatches);
	// Regex operators
	// Most significant half byte = precedence
	// Least significant half byte = id
	static enum operators
	{
		operator_concatenate = 0x10,		// Concatenate
		operator_alternation = 0x00,		// |
		operator_oneorzero = 0x20,			// ?
		operator_oneormore = 0x21,			// +
		operator_zeroormore = 0x22,			// *
		operator_bracket = 0x30				// (
	};
	static const int operator_precedence_mask = 0xF0;
private:
	RegexEngine();
	std::string preCompile(const std::string regex);
	static void InitializeOperatorMap();
	// Singleton
	static std::unique_ptr<RegexEngine> compiler;
	static std::map<char, int> operator_map;
};

