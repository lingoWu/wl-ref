#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include "GenLattice.h"

using namespace std;

int main(int argc, char *argv[])
{
	vector<string> cmds;
	int i;

	for (i = 1; i < argc; ++i)
	{
		cmds.push_back(argv[i]);
	}

	vector<string>::iterator it = find(cmds.begin(), cmds.end(), "-i");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
        cerr << "Example: -i input.txt -o output.txt" << endl;
		exit(1);
	}

	string srcFile = *(it + 1);

	it = find(cmds.begin(), cmds.end(), "-o");

	if (it == cmds.end() || it + 1 == cmds.end())
	{
		cerr << "Example: -i input.txt -o output.txt" << endl;
		exit(1);
	}

	string tgtFile = *(it + 1);
    ifstream fin(srcFile.c_str());

    if (!fin)
    {
        cerr << "Can not open file: " << srcFile << endl;
        exit(1);
    }

	ofstream fout(tgtFile.c_str());
	CGenLattice generator;

	cerr << "Initialize generator...";
	flush(cerr);

	if (!generator.Initialize())
	{
		cout << "generator initialize fail" << endl;
		exit(1);
	}

	cerr << "succeed" << endl;

	//计时开始
	long begin = clock();

	string line;
	int cnt = 0;
	while (getline(fin, line))
	{
        ++cnt;

		string best, lat;
        // threshold取0.1我用来表示分数0，如果用真的0，会是里面的浮点比较不可靠
        // 好像用100的话，能够产生较好的lattice
		generator.Generate(line, best, 40.0, lat);
        fout << best << endl;
        //fout << cnt << " " << lat << endl;
		
		if (cnt % 10 == 0)
		{
			cerr << cnt << endl;
		}
	}
	
	//计时结束
	long end = clock();

	//切分耗时
	double sec = (double)(end - begin) / (double)CLOCKS_PER_SEC;
	cerr << "\nSucceed, time cost: " << sec << " secs" << endl;

    fin.close();
	fout.close();
	return 0;
}
