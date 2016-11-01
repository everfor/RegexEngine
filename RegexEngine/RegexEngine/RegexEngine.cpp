#include "RegexEngine.h"
#include "StateMachine.h"
#include "StateFactory.h"
#include "State.h"

#include <stack>
#include <sstream>

using namespace std;

unique_ptr<RegexEngine> RegexEngine::compiler;
map<char, int> RegexEngine::operator_map;

RegexEngine::RegexEngine()
{
}

RegexEngine::~RegexEngine()
{
}

RegexEngine* RegexEngine::Get()
{
	if (compiler == nullptr)
	{
		compiler.reset(new RegexEngine());
		InitializeOperatorMap();
	}

	return compiler.get();
}

void RegexEngine::InitializeOperatorMap()
{
	operator_map['|'] = RegexEngine::operator_alternation;
	operator_map['+'] = RegexEngine::operator_oneormore;
	operator_map['?'] = RegexEngine::operator_oneorzero;
	operator_map['*'] = RegexEngine::operator_zeroormore;
	operator_map['('] = RegexEngine::operator_bracket;
}

/* Helper functions */
void createLiteralMatch(int literal_char, stack<StateMachine>& fragments)
{
	// Literal match
	auto curr_state = StateFactory::CreateState(literal_char, nullptr);

	// Create new state machine on stack
	fragments.push(StateMachine(curr_state, 1, vector<State*>{ curr_state }));
}

void createCharacterClassMatch(const string ranges, stack<StateMachine>& fragments)
{
	// Character class literal match
	auto curr_state = StateFactory::CreateState(State::state_range, nullptr);

	int start = 0, end = 0;
	while (true)
	{
		end = ranges.find('-', start);

		if (end == string::npos)
		{
			break;
		}

		end += 1;

		curr_state->addRange(ranges[start], ranges[end]);

		start = end + 1;
	}

	// Create state machine on stack
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
	case RegexEngine::operator_alternation:
		createAlernation(fragments);
		break;
	case RegexEngine::operator_oneormore:
		createOneOrMoreMatch(fragments);
		break;
	case RegexEngine::operator_oneorzero:
		createZeroOrOneMatch(fragments);
		break;
	case RegexEngine::operator_zeroormore:
		createZeroOrMoreMatch(fragments);
		break;
	case RegexEngine::operator_concatenate:
		createConcatenation(fragments);
		break;
	}
}

void addOperator(int op, stack<StateMachine>& fragments, stack<int>& operators)
{
	while (!operators.empty() &&
		((op & RegexEngine::operator_precedence_mask) <=
		(operators.top() & RegexEngine::operator_precedence_mask)))
	{
		// If encountered '(', no more operations to be performed
		if (operators.top() == RegexEngine::operator_bracket)
		{
			break;
		}

		// Apply all previous operations with a higher precedence
		applyOperator(operators.top(), fragments);
		operators.pop();
	}

	operators.push(op);
}

void addLiteral(int lit, stack<StateMachine>& fragments)
{
	createLiteralMatch(lit, fragments);
}

void addCharacterClassLiteral(const string ranges, stack<StateMachine>& fragments)
{
	createCharacterClassMatch(ranges, fragments);
}

string RegexEngine::preCompile(const string regex)
{
	// Pre compile the counted repititions

	// Store partial strings incorporated as (...) or [...]
	stack<string> partials;
	// Store the poisitions of '(' and '['
	stack<int> left_bracket_pos;

	stringstream ss;

	bool is_escaped = false;
	string last;

	for (int i = 0; i < regex.size(); i++)
	{
		if ((regex[i] == '(' || regex[i] == '[') && !is_escaped)
		{
			// If there is only 1 partial left and a new partial is coming up
			// The old partial is complete without a counted repitition
			if (left_bracket_pos.empty() && !partials.empty())
			{
				if (partials.top().find('{') == string::npos)
				{
					ss << partials.top();
				}
				
				partials.pop();
			}

			left_bracket_pos.push(i);
		}
		else if ((regex[i] == ')' || regex[i] == ']') && !is_escaped)
		{
			// Generate partials
			int last_left_bracket = left_bracket_pos.top();
			left_bracket_pos.pop();

			// Add additional brackets for submatch extraction
			partials.push("(" + regex.substr(last_left_bracket, i - last_left_bracket + 1) + ")");
		}
		else if (regex[i] == '{' && !is_escaped)
		{
			// Get the expression to apply counted reference
			string expression;
			// Count modifier - if the expression is a single char, then it is already included once in the ss
			int count_modifier = 0;
			if (partials.empty())
			{
				expression = last;
				count_modifier = -1;
			}
			else
			{
				expression = partials.top();
				partials.pop();
			}

			// Find '}'
			int right = regex.find('}', i);

			// Get substring
			string counts = regex.substr(i + 1, right - i - 1);

			// Find the separator ','
			size_t separator_pos = counts.find(',');

			if (separator_pos == string::npos)
			{
				// Exact counts, {n}
				int count = stoi(counts) + count_modifier;

				while (count > 0)
				{
					ss << expression;
					--count;
				}
			}
			else
			{
				// Range
				int lower = stoi(counts.substr(0, separator_pos)) + count_modifier;
				
				if (separator_pos == counts.length() - 1)
				{
					// Case {n,}
					while (lower > 0)
					{
						ss << expression;
						--lower;
					}

					ss << "+";
				}
				else
				{
					// Case {n,m}
					int higher = stoi(counts.substr(separator_pos + 1)) + count_modifier;

					while (lower > 0)
					{
						ss << expression;
						--lower;
						--higher;
					}

					while (higher > 0)
					{
						ss << expression << "?";
						--higher;
					}
				}
			}

			// Update i
			i = right;
			last = string(1, regex[right]);
		}
		else if (regex[i] == '\\' && !is_escaped)
		{
			is_escaped = true;
			ss << string(1, regex[i]);
		}
		else
		{
			// Literals

			// Last partial is complete without counted repititions
			if (!partials.empty() && left_bracket_pos.empty())
			{
				ss << partials.top();
				partials.pop();
			}

			// Current char is not inside a partial
			// Update it into the ss directly
			if (left_bracket_pos.empty())
			{
				// Add additional brackets for submatch extraction
				ss << (isOperator(regex[i]) ? string(1, regex[i]) : "(" + string(1, regex[i]) + ")");
			}

			is_escaped = false;
		}

		// Add additional brackets for submatch extraction
		last = (isOperator(regex[i]) ? string(1, regex[i]) : "(" + string(1, regex[i]) + ")");
	}

	// Include the rest of the partials
	while (!partials.empty())
	{
		if (partials.top().find('{') == string::npos)
		{
			ss << partials.top();
		}
		
		partials.pop();
	}

	return ss.str();
}

void RegexEngine::compile(const string init_regex, StateMachine& machine)
{
	string regex = preCompile(init_regex);

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
				current == '('
				)
			)
		{
			addOperator(operator_concatenate, fragments, operators);
		}

		// If encountered "(", add a save mark
		if (current == '(' && !is_escaped)
		{
			addLiteral(State::state_save, fragments);
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
			if (current == '.' && !is_escaped)
			{
				// Match anything
				addLiteral(State::state_any, fragments);
			}
			else if (current == '[' && !is_escaped)
			{
				// Range matches

				// Find position of next ']'
				auto right_angle = regex.find(']', i);

				// Create range literal
				addCharacterClassLiteral(regex.substr(i + 1, right_angle - i - 1), fragments);

				// Update current to ']', and set i correspondingly
				current = regex[right_angle];
				i = right_angle;
			}
			else
			{
				addLiteral(current, fragments);
			}

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
}

void addState(State *curr_state, vector<State*>& states)
{
	if (curr_state == nullptr)
	{
		return;
	}
	else if (curr_state->getCondition() == State::state_split)
	{
		addState(curr_state->getNext(), states);
		addState(curr_state->getAltNext(), states);
	}
	else if (curr_state->getCondition() == State::state_save)
	{
		// Ignore save states
		addState(curr_state->getNext(), states);
	}
	else
	{
		states.push_back(curr_state);
	}
}

bool matchState(int match_char, State* state)
{
	return state->isMatch(match_char);
}

bool matchStepBFS(int match_char, vector<State*>& current, vector<State*>& next)
{
	bool is_match = false;

	for (int i = 0; i < current.size(); ++i)
	{
		if (matchState(match_char, current[i]))
		{
			is_match = true;
			addState(current[i]->getNext(), next);
			addState(current[i]->getAltNext(), next);
		}
	}

	return is_match;
}

bool RegexEngine::match(const std::string input, StateMachine& machine)
{
	// Current and next set of states
	vector<State*> current;
	vector<State*> next;

	// Initialize current states
	addState(machine.getStart(), current);

	for (int i = 0; i < input.length(); ++i)
	{
		if (!matchStepBFS(input[i], current, next))
		{
			return false;
		}

		current.clear();
		current = move(next);
		next.clear();
	}

	return matchStepBFS(State::state_match, current, next);
}

bool matchDFS(const std::string input, State* state, size_t pos, size_t last_pos, vector<string>& submatches)
{
	if (state->getCondition() == State::state_match)
	{
		// Match

		// Add the remaining submatch
		if (last_pos < input.length())
		{
			submatches.push_back(input.substr(last_pos));
		}


		return true;
	}

	if (pos < input.length() && state != nullptr)
	{
		if (state->getCondition() == State::state_split)
		{
			// Branch state
			// Try both branches
			if (matchDFS(input, state->getNext(), pos, last_pos, submatches))
			{
				return true;
			}
			else if (matchDFS(input, state->getAltNext(), pos, last_pos, submatches))
			{
				return true;
			}
		}
		else if (state->getCondition() == State::state_save)
		{
			// Save state
			if (pos > last_pos)
			{
				// Encountered save state. Save current submatch
				submatches.push_back(input.substr(last_pos, pos - last_pos));

				// Further macthes
				if (matchDFS(input, state->getNext(), pos, pos, submatches))
				{
					return true;
				}

				// Step back if not successful
				submatches.pop_back();
			}
			else
			{
				// Save state encountered but no submatches yet. Go to next state directly
				return matchDFS(input, state->getNext(), pos, last_pos, submatches);
			}
		}
		else if (state->isMatch(input[pos]))
		{
			// Literal match

			if (matchDFS(input, state->getNext(), pos + 1, last_pos, submatches))
			{
				return true;
			}
			else if (state->altNextEnabled() && matchDFS(input, state->getAltNext(), pos + 1, last_pos, submatches))
			{
				return true;
			}
		}
	}

	return false;
}

bool RegexEngine::matchWithSubmatchExtraction(const std::string input, StateMachine& machine, std::vector<string>& submatches)
{
	return matchDFS(input, machine.getStart(), 0, 0, submatches);
}
