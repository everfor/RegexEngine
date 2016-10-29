#include "StateFactory.h"
#include "State.h"

#include <algorithm>

using namespace std;

vector<unique_ptr<State>> StateFactory::states;

StateFactory::StateFactory()
{
}

StateFactory::~StateFactory()
{
}

State* StateFactory::CreateState(int condition, State *next, State *alt_next)
{
	unique_ptr<State> state(new State(condition, next, alt_next));
	auto state_ptr = state.get();
	states.push_back(move(state));

	return state_ptr;
}

void StateFactory::DeleteAllStates()
{
	for_each(begin(states), end(states), [](unique_ptr<State>& state) { state.reset(); });
	states.clear();
}