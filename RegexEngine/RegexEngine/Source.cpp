#include <iostream>
#include <string>

#include "RegexEngine.h"
#include "StateMachine.h"

using namespace std;

int main(int argc, char **argv)
{
	StateMachine machine;
	auto compiler = RegexEngine::Get();

	string regex("a(bb)*[a-e]");

	compiler->compile(regex, machine);

	cout << "aa: " << compiler->match("aa", machine) << endl;
	cout << "aba: " << compiler->match("aba", machine) << endl;
	cout << "abba: " << compiler->match("abba", machine) << endl;
	cout << "abbbba: " << compiler->match("abbbba", machine) << endl;
	cout << "abbbbs: " << compiler->match("abbbbs", machine) << endl;

	getchar();

	return 0;
}