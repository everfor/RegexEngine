#include "State.h"


State::State(int cond, State *nxt, State *alt_nxt) :
	condition(cond), next(nxt), alt_next(alt_nxt)
{
}

State::~State()
{
	next = nullptr;
	alt_next = nullptr;
}

State::State(const State& other):
	condition(other.getCondition()), next(other.getNext()), alt_next(other.getAltNext())
{
}

State& State::operator=(const State& other)
{
	condition = other.getCondition();
	next = other.getNext();
	alt_next = other.getAltNext();

	return *this;
}

State::State(const State&& other) :
	condition(other.getCondition()), next(other.getNext()), alt_next(other.getAltNext())
{
}

State& State::operator=(const State&& other)
{
	condition = other.getCondition();
	next = other.getNext();
	alt_next = other.getAltNext();

	return *this;
}

void State::setNext(State *new_nxt)
{
	next = new_nxt;
}

void State::setAltNext(State *new_alt_nxt)
{
	alt_next = new_alt_nxt;
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