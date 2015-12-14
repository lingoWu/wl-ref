#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <string>
#include <vector>

using namespace std;

//ʵ����עԪ��
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

//ѵ�����ʵ��
class CInstance
{
public:
	vector<CInstElem> seq;
};

//��עԪ��
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
