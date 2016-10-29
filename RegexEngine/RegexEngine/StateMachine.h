#pragma once

#include <vector>

class State;

class StateMachine
{
public:
	StateMachine(int n_states, ...);
	~StateMachine();
private:
	State *start;
	std::vector<State*> outputs;
};

