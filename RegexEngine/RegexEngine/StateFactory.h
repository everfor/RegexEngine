#pragma once

#include <memory>
#include <vector>

class State;
/* Manage memories for states */
class StateFactory
{
public:
	static State* CreateState(int condition, State *next = nullptr, State *alt_next = nullptr);
	static void DeleteAllStates();
private:
	StateFactory();
	virtual ~StateFactory();
	static std::vector<std::unique_ptr<State>> states;
};

