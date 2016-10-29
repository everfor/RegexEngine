#include <iostream>
#include <string>

#include "RegexCompiler.h"
#include "StateMachine.h"

using namespace std;

int main(int argc, char **argv)
{
	StateMachine machine;
	auto compiler = RegexCompiler::Get();

	string regex("a(bb)+a");

	compiler->compile(regex, machine);

	cout << "Not crashed!" << endl;
	getchar();

	return 0;
}