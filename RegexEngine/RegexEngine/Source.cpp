#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "RegexEngine.h"
#include "StateMachine.h"

using namespace std;

int main(int argc, char **argv)
{
	StateMachine machine;
	auto compiler = RegexEngine::Get();

	string regex("a{2,}(bb){2,5}[a-e]{2}");

	compiler->compile(regex, machine);

	cout << "aa: " << compiler->match("aa", machine) << endl;
	cout << "aba: " << compiler->match("aba", machine) << endl;
	cout << "aabbaa: " << compiler->match("aabbaa", machine) << endl;
	cout << "aabbbbea: " << compiler->match("aabbbbea", machine) << endl;
	//cout << "aaaaaaaaaaabbbbbbbbcc: " << compiler->match("aaaabbbbbbbbcc", machine) << endl;
	vector<string> submatches;
	cout << "aaaaaaaaaaabbbbbbbbcc: " << compiler->matchWithSubmatchExtraction("aaaaaaaaaaabbbbbbbbcc", machine, submatches) << endl;
	cout << "Submatches: " << endl;
	for_each(begin(submatches), end(submatches), [](const string& s) {cout << s << endl; });

	getchar();

	return 0;
}