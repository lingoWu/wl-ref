#ifndef DATASTRUCT_H
#define DATASTRUCT_H

//±ê×¢ÔªËØ
class CTagElem
{
public:
	string ch;
	int type;
	string tag;

	double best;
	int length;

	CTagElem(const string &chVal, const int &typeVal)
	{
		ch = chVal;
		type = typeVal;
	}
};

#endif
