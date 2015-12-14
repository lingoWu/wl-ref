#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include "Segmentor.h"

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
	it = find(cmds.begin(), cmds.end(), "-s");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
		cout << "Para list should be: -s src-file -t tgt-file" << endl;
		exit(0);
	}

	string srcFile = *(it + 1);

	it = find(cmds.begin(), cmds.end(), "-t");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
		cout << "Para list should be: -s src-file -t tgt-file" << endl;
		exit(0);
	}

	string tgtFile = *(it + 1);

	ifstream fin(srcFile.c_str());
	if (!fin)
	{
		cout << "Can not open src-file: " << srcFile << endl;
		exit(0);
	}

	ofstream fout(tgtFile.c_str());
	if (!fout)
	{
		cout << "Can not open tgt-file: " << tgtFile << endl;
		exit(0);
	}

	CSegmentor segmentor;

	cout << "Initialize segmentor...";
	flush(cout);

	if (!segmentor.Initialize())
	{
		cout << "Segmentor initialize fail" << endl;
		exit(0);
	}

	cout << "succeed" << endl;

	//计时开始
	long begin = clock();

	string line;
	int cnt = 0;

	while (getline(fin, line))
	{
		string seg;
		segmentor.Segment(line, seg);

		fout << seg << endl;
		
		++cnt;
		if (cnt % 10 == 0)
		{
			cout << cnt << endl;
		}
	}
	
	//计时结束
	long end = clock();

	//切分耗时
	double sec = (double)(end - begin) / (double)CLOCKS_PER_SEC;
	cout << "\nSucceed, time cost: " << sec << " secs" << endl;

	fin.close();
	fout.close();
	return 0;
}
