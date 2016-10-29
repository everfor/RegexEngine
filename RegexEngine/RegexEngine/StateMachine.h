#pragma once

#include <vector>

class State;

class StateMachine
{
public:
	friend class StateMachine;
	StateMachine(State *start_state, int n_outlists, ...);
	~StateMachine();
	StateMachine(const StateMachine& other);
	StateMachine& operator=(const StateMachine& other);
	State* getStart();
	std::vector<State*>& getOutputs();
	void patchOutput(State *out);
	// Move semantics
	StateMachine(const StateMachine&& other);
	StateMachine& operator=(const StateMachine&& other);
private:
	// Start state of the machine
	State *start;
	// Output states of the machine - outmost layer of states
	std::vector<State*> outputs;
};

