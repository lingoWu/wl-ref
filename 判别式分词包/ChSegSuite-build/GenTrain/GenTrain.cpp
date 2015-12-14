#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>
#include <strstream>

using namespace std;

void Generate(const string &src, string &tgt, const bool &tag);

int main(int argc, char *argv[])
{
	cout << "Training corpus generator" << endl;

	if (argc != 4)
	{
		cout << "Para list should be: src-file train-file y/n(tag or not)" << endl;
		exit(1);
	}

	ifstream fin(argv[1]);
	if (!fin)
	{
		cout << "Can not open src-file: " << argv[1] << endl;
		exit(1);
	}

	ofstream fout(argv[2]);
	if (!fout)
	{
		cout << "Can not open train-file: " << argv[2] << endl;
		exit(1);
	}

	string tag = argv[3];
	bool haveTag = tag == "y" || tag == "Y";

	cout << "Start..." << endl;

	string line;
	int idx = 0;

	while (getline(fin, line))
	{
		string post;
		Generate(line, post, haveTag);

		fout << post << endl;
		
		++idx;
		if (idx % 1000 == 0)
		{
			cout << idx << " completed" << endl;
		}
	}

	cout << "Succeed" << endl;

	fin.close();
	fout.close();
	return 0;
}

//生成训练集
void Generate(const string &src, string &tgt, const bool &tag)
{
	tgt = "";

	istrstream is(src.c_str());
	string part;
	
	while (is >> part)
	{
		string word, pos;
		size_t split = part.rfind('/');

		if (split != string::npos)
		{
			word = part.substr(0, split);
			pos = part.substr(split + 1, part.length() - split - 1);
		}
		else
		{
			word = part;
		}

		vector<string> chs;
		int len = (int)word.size();
		int i = 0;

		while (i < len)
		{
			string ch;

			if (word[i] < 0)
			{
				ch = word[i];
				ch += word[i + 1];
				i += 2;
			}
			else
			{
				ch = word[i];
				++i;
			}

			chs.push_back(ch);
		}

		int cnt = (int)chs.size();

		for (i = 0; i < cnt; ++i)
		{
			if (i == 0)
			{
				if (cnt > 1)
				{
					tgt += chs[i] + "/b";
				}
				else
				{
					tgt += chs[i] + "/s";
				}
			}
			else
			{
				if (i == cnt - 1)
				{
					tgt += chs[i] + "/e";
				}
				else
				{
					tgt += chs[i] + "/m";
				}
			}

			//有无tag标记
			if (tag)
			{
				tgt += "_" + pos + "  ";
			}
			else
			{
				tgt += "  ";
			}
		}
	}
}
