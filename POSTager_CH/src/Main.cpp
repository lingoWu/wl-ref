#include <iostream>
#include <cmath>
#include <fstream>
#include "Decoder.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cerr << "Usage: src-file rst-file" << endl;
		exit(1);
	}

	CDecoder decoder;

	if (!decoder.Initialize("data/pos_ch", 
							"data/word_pos_ch",
							"data/model_ch", 20))
	{
		exit(1);
	}

	ifstream fin(argv[1]);

	if (!fin)
	{
		cerr << "Can not open file: " << argv[1] << endl;
		return 1;
	}

	ofstream fout(argv[2]);

	if (!fout)
	{
		cerr << "Can not open file: " << argv[2] << endl;
		return 1;
	}

	string line;
	int cnt = 0;

	while (getline(fin, line))
	{
		if (line.empty())
		{
			continue;
		}

		string result;

		decoder.Decode(line, result);
		fout << result << endl;
		++cnt;
		
		if (cnt % 100 == 0)
		{
			cerr << cnt << endl;
		}
	}

	fin.close();
	fout.close();
	return 0;
}
