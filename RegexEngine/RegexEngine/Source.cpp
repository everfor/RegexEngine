#include <iostream>
#include <string>

#include "RegexEngine.h"
#include "StateMachine.h"

using namespace std;

int main(int argc, char **argv)
{
	StateMachine machine;
	auto compiler = RegexEngine::Get();

	string regex("a(bb)*a");

	compiler->compile(regex, machine);

	cout << "aa: " << compiler->match("aa", machine) << endl;
	cout << "aba: " << compiler->match("aba", machine) << endl;
	cout << "abba: " << compiler->match("abba", machine) << endl;
	cout << "abbbba: " << compiler->match("abbbba", machine) << endl;

	getchar();

	return 0;
}