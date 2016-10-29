#pragma once

#include "StateFactory.h"

/* Represents a single state in state machine */
class State
{
public:
	enum state_special_conditions
	{
		state_split = 256,
		state_match = 257
	};
	void setNext(State *new_nxt);
	void setAltNext(State *new_alt_nxt);
	int getCondition() const;
	State* getNext() const;
	State* getAltNext() const;
	virtual ~State();
	// Friend
	friend State* StateFactory::CreateState(int condition, State *next, State *alt_next);
private:
	State(int cond, State *nxt = nullptr, State *alt_nxt = nullptr);
	State(const State& other);
	State& operator=(const State& other);
	State(const State&& other);
	State& operator=(const State&& other);
	int condition;
	State *next;
	State *alt_next;
};

