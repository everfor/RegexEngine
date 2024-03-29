#include "State.h"

#include <algorithm>

using namespace std;

State::State(int cond, State *nxt, State *alt_nxt, bool alt_next_enabled) :
	condition(cond), next(nxt), alt_next(alt_nxt), is_alt_next_enabled(alt_next_enabled)
{
}

State::~State()
{
	next = nullptr;
	alt_next = nullptr;
}

State::State(const State& other):
	condition(other.getCondition()), next(other.getNext()), alt_next(other.getAltNext()), is_alt_next_enabled(other.altNextEnabled())
{
}

State& State::operator=(const State& other)
{
	condition = other.getCondition();
	next = other.getNext();
	alt_next = other.getAltNext();
	is_alt_next_enabled = other.altNextEnabled();

	return *this;
}

State::State(const State&& other) :
	condition(other.getCondition()), next(other.getNext()), alt_next(other.getAltNext()), is_alt_next_enabled(other.altNextEnabled())
{
}

State& State::operator=(const State&& other)
{
	condition = other.getCondition();
	next = other.getNext();
	alt_next = other.getAltNext();
	is_alt_next_enabled = other.altNextEnabled();

	return *this;
}

void State::setNext(State *new_nxt)
{
	if (next == nullptr)
	{
		next = new_nxt;
	}
}

void State::setAltNext(State *new_alt_nxt)
{
	if (alt_next == nullptr)
	{
		alt_next = new_alt_nxt;
	}
}

int State::getCondition() const
{
	return condition;
}

State* State::getNext() const
{
	return next;
}

State* State::getAltNext() const
{
	return alt_next;
}

bool State::altNextEnabled() const
{
	return is_alt_next_enabled;
}

bool State::isMatch(int test)
{
	if (condition == state_any)
	{
		return true;
	}
	else if (condition == state_range)
	{
		bool temp = any_of(begin(ranges), end(ranges), [&](pair<char, char> curr_range) { return curr_range.first <= test && test <= curr_range.second; });
		return temp;
	}
	else
	{
		return condition == test;
	}
}

void State::addRange(char lower, char upper)
{
	ranges.push_back(make_pair(lower, upper));
}
