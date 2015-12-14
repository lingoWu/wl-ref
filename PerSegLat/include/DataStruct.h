#ifndef DATASTRUCT_H
#define DATASTRUCT_H

class CSpan
{
public:
    int from;
    int to;

    const CSpan(const int &f = 0, const int &t = 0)
    {
        from = f;
        to = t;
    }

    bool operator < (const CSpan &sp2) const
    {
        int len = to - from, len2 = sp2.to - sp2.from;

        return len < len2 || (len == len2 && from < sp2.from);
    }
};

class CLatEdge
{
public:
    double prior;
    CSpan span;
    double score;
};

//±ê×¢ÔªËØ
class CTagElem
{
public:
    string ch;
    int type;
    string tag;
    string bnd;
    double best;
    int length;

    vector<CLatEdge> bestEdges;

    CTagElem(const string &chVal, const int &typeVal)
    {
        ch = chVal;
        type = typeVal;
    }
};

#endif
