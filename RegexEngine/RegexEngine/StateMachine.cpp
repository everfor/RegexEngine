#include "StateMachine.h"
#include "State.h"

#include <cstdarg>
#include <algorithm>
#include <iterator>

using namespace std;

StateMachine::StateMachine(State * start_state, int n_outlists, ...)
{
	start = start_state;

	va_list lists;
	va_start(lists, n_outlists);

	for (int i = 0; i < n_outlists; i++)
	{
		auto curr_list = va_arg(lists, vector<State*>);
		copy(begin(curr_list), end(curr_list), back_inserter(outputs));
	}

	va_end(lists);
}

StateMachine::~StateMachine()
{
}

StateMachine::StateMachine(const StateMachine& other)
{
	start = other.start;
	outputs = other.outputs;
}

StateMachine& StateMachine::operator=(const StateMachine& other)
{
	start = other.start;
	outputs = other.outputs;

	return *this;
}

StateMachine::StateMachine(const StateMachine&& other)
{
	start = other.start;
	outputs = move(other.outputs);
}

StateMachine& StateMachine::operator=(const StateMachine&& other)
{
	start = other.start;
	outputs = move(other.outputs);

	return *this;
}

State* StateMachine::getStart()
{
	return start;
}

vector<State*>& StateMachine::getOutputs()
{
	return outputs;
}

void StateMachine::patchOutput(State *out)
{
	for_each(begin(outputs), 
		end(outputs), 
		[&](State* state)
		{
			state->setNext(out);
			if (state->altNextEnabled())
			{
				state->setAltNext(out);
			}
		});
}
