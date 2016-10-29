#include "State.h"


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
