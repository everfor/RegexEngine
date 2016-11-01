#pragma once

#include "StateFactory.h"

#include <vector>
#include <utility>

/* Represents a single state in state machine */
class State
{
public:
	static enum state_special_conditions
	{
		state_split = 256,		// Split into 2 branches
		state_match = 257,		// End of matching
		state_any = 258,		// Matches anything
		state_range = 259,		// Matches a range
		state_save = 260		// Save mark - used for submatch extraction
	};
	void setNext(State *new_nxt);
	void setAltNext(State *new_alt_nxt);
	int getCondition() const;
	State* getNext() const;
	State* getAltNext() const;
	bool altNextEnabled() const;
	bool isMatch(int test);
	void addRange(char lower, char upper);
	virtual ~State();
	// Friend
	friend State* StateFactory::CreateState(int condition, State *next, State *alt_next, bool alt_next_enabled);
private:
	State(int cond, State *nxt = nullptr, State *alt_nxt = nullptr, bool alt_next_enabled = false);
	State(const State& other);
	State& operator=(const State& other);
	State(const State&& other);
	State& operator=(const State&& other);
	int condition;
	State *next;
	State *alt_next;
	bool is_alt_next_enabled;
	std::vector<std::pair<char, char>> ranges;
};

