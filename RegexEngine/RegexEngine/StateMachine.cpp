#include "StateMachine.h"
#include "State.h"

#include <cstdarg>

using namespace std;

StateMachine::StateMachine(int n_states, ...)
{
	va_list states;
	va_start(states, n_states);

	// Add start state
	start = va_arg(states, State*);

	// Add outputs
	for (int i = 0; i < n_states - 1; ++i)
	{
		outputs.push_back(va_arg(states, State*));
	}

	va_end(states);
}


StateMachine::~StateMachine()
{
}
