#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <string>
#include <vector>

using namespace std;

//实例标注元素
class CInstElem
{
public:
	string ch;
	string tag;
	int type;

	CInstElem(const string &chVal, const string &tagVal, const int &typeVal)
	{
		ch = chVal;
		tag = tagVal;
		type = typeVal;
	}
};

//训练语句实例
class CInstance
{
public:
	vector<CInstElem> seq;
};

//标注元素
class CTagElem
{
public:
	string ch;
	int type;
	string tag;

	double best;
	int length;
};

#endif
