#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "Trainer.h"

using namespace std;

int main(int argc, char *argv[])
{
	vector<string> cmds;
	int i;

	for (i = 1; i < argc; ++i)
	{
		cmds.push_back(argv[i]);
	}

	vector<string>::iterator it;
	it = find(cmds.begin(), cmds.end(), "-f");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
		cout << "Para list should be: -f corpus-file -l loop-count" << endl;
		exit(0);
	}

	string corpFile = *(it + 1);

	it = find(cmds.begin(), cmds.end(), "-l");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
		cout << "Para list should be: -f corpus-file -l loop-count" << endl;
		exit(0);
	}

	int loopCnt = atoi((it + 1)->c_str());

	CTrainer trainer;
	trainer.Initialize();
	trainer.Train(corpFile, loopCnt);
	return 0;
}
