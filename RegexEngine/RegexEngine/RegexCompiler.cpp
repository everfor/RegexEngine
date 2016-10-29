#include "RegexCompiler.h"
#include "StateMachine.h"
#include "StateFactory.h"
#include "State.h"

#include <stack>

using namespace std;

unique_ptr<RegexCompiler> RegexCompiler::compiler;
map<char, int> RegexCompiler::operator_map;

RegexCompiler::RegexCompiler()
{
}

RegexCompiler::~RegexCompiler()
{
}

RegexCompiler* RegexCompiler::Get()
{
	if (compiler == nullptr)
	{
		compiler.reset(new RegexCompiler());
		InitializeOperatorMap();
	}

	return compiler.get();
}

void RegexCompiler::InitializeOperatorMap()
{
	operator_map['|'] = RegexCompiler::operator_alternation;
	operator_map['+'] = RegexCompiler::operator_oneormore;
	operator_map['?'] = RegexCompiler::operator_oneorzero;
	operator_map['*'] = RegexCompiler::operator_zeroormore;
	operator_map['('] = RegexCompiler::operator_bracket;
}

/* Helper functions */
void createLiteralMatch(char literal_char, stack<StateMachine>& fragments)
{
	// Literal match
	auto curr_state = StateFactory::CreateState(literal_char, nullptr);

	// Create new state machine on stack
	fragments.push(StateMachine(curr_state, 1, vector<State*>{ curr_state }));
}

void createZeroOrOneMatch(stack<StateMachine>& fragments)
{
	// Retrieve the state to match ?
	auto last_machine = fragments.top();
	fragments.pop();

	// Zero or One match
	auto curr_state = StateFactory::CreateState(State::state_split, last_machine.getStart(), nullptr, true);

	// Create new state machine on stack
	fragments.push(StateMachine(curr_state, 2, last_machine.getOutputs(), vector<State*>{ curr_state }));
}

void createOneOrMoreMatch(stack<StateMachine>& fragments)
{
	// Retrieve the state to match +
	auto last_machine = fragments.top();
	fragments.pop();

	// One or more match
	auto curr_state = StateFactory::CreateState(State::state_split, last_machine.getStart(), nullptr, true);
	last_machine.patchOutput(curr_state);

	// Create new state machine on stack
	fragments.push(StateMachine(last_machine.getStart(), 1, vector<State*>{ curr_state }));
}

void createZeroOrMoreMatch(stack<StateMachine>& fragments)
{
	// Retrieve the state to match *
	auto last_machine = fragments.top();
	fragments.pop();

	// Zero or more match
	auto curr_state = StateFactory::CreateState(State::state_split, last_machine.getStart(), nullptr, true);
	last_machine.patchOutput(curr_state);

	// Create new state machine on stack
	fragments.push(StateMachine(curr_state, 1, vector<State*>{ curr_state }));
}

void createAlernation(stack<StateMachine>& fragments)
{
	// Retrieve the states to match |
	auto second_machine = fragments.top();
	fragments.pop();
	auto first_machine = fragments.top();
	fragments.pop();

	// Alternation
	auto curr_state = StateFactory::CreateState(State::state_split, first_machine.getStart(), second_machine.getStart(), true);

	// Create new state machine on stack
	fragments.push(StateMachine(curr_state, 2, first_machine.getOutputs(), second_machine.getOutputs()));
}

void createConcatenation(stack<StateMachine>& fragments)
{
	// Retrieve the states to concatenate
	auto second_machine = fragments.top();
	fragments.pop();
	auto first_machine = fragments.top();
	fragments.pop();

	// Concatenation
	first_machine.patchOutput(second_machine.getStart());

	// Create new state mcahine on stack
	fragments.push(StateMachine(first_machine.getStart(), 1, second_machine.getOutputs()));
}

bool isOperator(char curr)
{
	return curr == '+' ||
		curr == '|' ||
		curr == '*' ||
		curr == '(' ||
		curr == ')' ||
		curr == '?' ||
		curr == '\\';
}

void applyOperator(int op, stack<StateMachine>& fragments)
{
	switch (op)
	{
	case RegexCompiler::operator_alternation:
		createAlernation(fragments);
		break;
	case RegexCompiler::operator_oneormore:
		createOneOrMoreMatch(fragments);
		break;
	case RegexCompiler::operator_oneorzero:
		createZeroOrOneMatch(fragments);
		break;
	case RegexCompiler::operator_zeroormore:
		createZeroOrMoreMatch(fragments);
		break;
	case RegexCompiler::operator_concatenate:
		createConcatenation(fragments);
		break;
	}
}

void addOperator(int op, stack<StateMachine>& fragments, stack<int>& operators)
{
	while (!operators.empty() &&
		((op & RegexCompiler::operator_precedence_mask) <=
		(operators.top() & RegexCompiler::operator_precedence_mask)))
	{
		// If encountered '(', no more operations to be performed
		if (operators.top() == RegexCompiler::operator_bracket)
		{
			break;
		}

		// Apply all previous operations with a higher precedence
		applyOperator(operators.top(), fragments);
		operators.pop();
	}

	operators.push(op);
}

void addLiteral(char lit, stack<StateMachine>& fragments)
{
	createLiteralMatch(lit, fragments);
}

bool RegexCompiler::compile(const string regex, StateMachine& machine)
{
	// Shunting-yard algorithm
	stack<StateMachine> fragments;
	stack<int> operators;

	char current;
	char last;
	bool is_escaped = false;
	bool last_escaped = false;

	for (int i = 0; i < regex.length(); ++i)
	{
		last_escaped = is_escaped;

		current = regex[i];

		// Add the invisible concatenate operator
		if (i > 0 && last != '(' &&
				(last_escaped || 
				!isOperator(current) ||
				(current == '(' && !isOperator(last))
				)
			)
		{
			addOperator(operator_concatenate, fragments, operators);
		}

		if (isOperator(current) && !is_escaped)
		{
			if (current == '\\')
			{
				// Escape next one
				is_escaped = true;
			}
			else if (current == ')')
			{
				// Apply all operations before '('
				while (operators.top() != operator_bracket)
				{
					applyOperator(operators.top(), fragments);
					operators.pop();
				}

				// Pop the '('
				operators.pop();
			}
			else
			{
				addOperator(operator_map[current], fragments, operators);
			}
		}
		else
		{
			// Real literals
			addLiteral(current, fragments);

			is_escaped = false;
		}

		last = current;
	}

	// End of regex string. Apply all operations still on stack
	while (!operators.empty())
	{
		applyOperator(operators.top(), fragments);
		operators.pop();
	}

	// Add end state
	auto frag = fragments.top();
	fragments.pop();

	auto end_state = StateFactory::CreateState(State::state_match, nullptr);
	frag.patchOutput(end_state);

	machine = StateMachine(frag.getStart(), 1, vector<State*>{ end_state });

	return true;
}