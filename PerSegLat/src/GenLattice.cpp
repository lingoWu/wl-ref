#include <strstream>
#include <fstream>
#include <iostream>
#include <strstream>
#include <cmath>
#include "GenLattice.h"

CGenLattice::CGenLattice()
{
}

CGenLattice::~CGenLattice()
{
}

//初始化
bool CGenLattice::Initialize()
{
	InitBounds();

	if (!m_cChType.Initialize("data/punc", "data/alph", "data/date", "data/num"))
	{
		return false;
	}

	if (!m_cPerceptron.Initialize("data/model"))
	{
		return false;
	}

	//最大可能词长
	m_nScanLen = 10;
	return true;
}

//设置边界标记集合
void CGenLattice::InitBounds()
{
	m_vBound.push_back("b");
	m_vBound.push_back("m");
	m_vBound.push_back("e");
	m_vBound.push_back("s");
	m_nBndCnt = (int)m_vBound.size();
}

//切分
void CGenLattice::Generate(const string &src, 
                           string &best, 
                           const double &thres, 
                           string &latStr)
{
	vector<CTagElem> elems;

	Split(src, elems);

	vector<vector<string> > featSets;
	vector<string> nullFeats;

	featSets.push_back(nullFeats);
	featSets.push_back(nullFeats);

	int len = int(elems.size());
	int from = 2, to = len - 3;
    int i;

	for (i = from; i <= to; ++i)
	{
		vector<string> feats;

		GenFeats(elems, i, feats);
		featSets.push_back(feats);
	}

	featSets.push_back(nullFeats);
	featSets.push_back(nullFeats);

	vector<map<string, double> > scores;

	GetScores(featSets, scores);
	BestEdges(elems, scores, 5);
    best = TraceSeg(elems);
	CrossRegEdge(elems);

	vector<double> alpha;
	vector<double> beta;

	AlphaBeta(elems, alpha, beta);

    map<CSpan, double> lat;

    GenPrunedLat(elems, alpha, beta, thres, lat);
    latStr = FormLatStr(lat);
}

//切分为单字序列
void CGenLattice::Split(const string &src, vector<CTagElem> &elems)
{
	elems.clear();

	int i;
	for (i = 0; i < 2; ++i)
	{
		elems.push_back(CTagElem("<s>", -1));
	}

	istrstream is(src.c_str());
	string word;

	while (is >> word)
	{
		vector<string> chs;
		int len = int(word.size());
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
	
			elems.push_back(CTagElem(ch, m_cChType.GetType(ch)));
		}
	}

	for (i = 0; i < 2; ++i)
	{
		elems.push_back(CTagElem("</s>", -1));
	}
}

//生成该环境下的特征
void CGenLattice::GenFeats(const vector<CTagElem> &elems, 
                           const int &cur, 
                           vector<string> &feats)
{
	feats.clear();

	char feat[32];
	int n;

	//Cn
	for (n = -2; n <= 2; ++n)
	{
		sprintf(feat, "C%d==%s", n, elems[cur + n].ch.c_str());
		feats.push_back(feat);
	}

	//CnCn+1
	for (n = -2; n <= 1; ++n)
	{
		sprintf(feat, "C%dC%d==%s%s", n, 
                                      n + 1, 
                                      elems[cur + n].ch.c_str(), 
                                      elems[cur + n + 1].ch.c_str());
		feats.push_back(feat);
	}

	//C-1C1
	sprintf(feat, "C-1C1==%s%s", elems[cur - 1].ch.c_str(), 
                                 elems[cur + 1].ch.c_str());
	feats.push_back(feat);

	//Pu(C0)
	sprintf(feat, "PuC0==%d", elems[cur].type == 0 ? 1 : 0);
	feats.push_back(feat);

	//T(C-2)T(C-1)T(C0)T(C1)T(C2)
	sprintf(feat, "TC-22==%d%d%d%d%d", elems[cur - 2].type, 
                                       elems[cur - 1].type, 
                                       elems[cur].type, 
                                       elems[cur + 1].type, 
                                       elems[cur + 2].type);
	feats.push_back(feat);
}

//求该特征集映射到tag的得分
double CGenLattice::FeatsPred(const vector<string> &feats, const string &tag)
{
	double score = 0.0;
	int cnt = int(feats.size());
	int k;

	for (k = 0; k < cnt; ++k)
	{
		score += m_cPerceptron.PredWeight(feats[k] + "=>" + tag);
	}

	return score;
}

//求各个Span的分数
void CGenLattice::GetScores(const vector<vector<string> > &featSets, 
                            vector<map<string, double> > &scores)
{
	scores.clear();

	int len = int(featSets.size());

    scores.resize(len);

	int lb = 2, rb = len - 3;
	int i;

	for (i = lb; i <= rb; ++i)
	{
		int m;

		for (m = 0; m < m_nBndCnt; ++m)
		{
            scores[i][m_vBound[m]] = FeatsPred(featSets[i], m_vBound[m]);
		}
	}
}

//标注以to结尾长度len的词
void CGenLattice::MarkBnd(vector<CTagElem> &elems, const int &to, const int &len)
{
	int from = to - len + 1;
	int p;

	for (p = from; p <= to; ++p)
	{
		if (p == from)
		{
			if (len == 1)
			{
				elems[p].bnd = "s";
			}
			else
			{
				elems[p].bnd = "b";
			}
		}
		else
		{
			if (p == to)
			{
				elems[p].bnd = "e";
			}
			else
			{
				elems[p].bnd = "m";
			}
		}
	}
}

//动态规划最佳路径
void CGenLattice::BestEdges(vector<CTagElem> &elems, 
                            vector<map<string, double> > &scores, 
                            const int &degree)
{
	elems[1].best = 0.0;

	int lb = 2, rb = int(elems.size()) - 3;
	int i;

	for (i = lb; i <= rb; ++i)
	{
		bool first = true;
		int dis;

		for (dis = 0; dis < m_nScanLen && i - dis >= lb; ++dis)
		{
			MarkBnd(elems, i, dis + 1);

            double score = elems[i - dis - 1].best;
            int p;
            double incre = 0.0;

            for (p = i - dis; p <= i; ++p)
            {
                incre += scores[p][elems[p].bnd];
            }

            score += incre;

            if (first || score > elems[i].best)
            {
                elems[i].best = score;
                elems[i].length = dis + 1;
            }

            first = false;

            CLatEdge edge;

            edge.span = CSpan(i - dis, i); 
            edge.score = incre;
            edge.prior = score;
            SortInsert(edge, elems[i].bestEdges, degree);
		}
	}
}

//根据最佳路径回溯tag序列
string CGenLattice::TraceSeg(vector<CTagElem> &elems)
{
    string seg = "";
    int from = 2;
    int p = int(elems.size()) - 3;

    while (p >= from)
    {
        int len = elems[p].length;
        int k;

        for (k = 0; k < len; ++k)
        {
            seg = elems[p].ch + seg;
            --p;
        }

        if (p >= from)
        {
            seg = "  " + seg;
        }
    }

    return seg;
}

//排序插入
void CGenLattice::SortInsert(const CLatEdge &edge, 
                             vector<CLatEdge> &edgev, 
                             const int &lim)
{
	if (edgev.empty())
	{
		edgev.push_back(edge);
		return;
	}

	int sum = int(edgev.size());

	if (sum >= lim && edge.prior <= edgev[sum - 1].prior)
	{
		return;
	}

	int pos = int(edgev.size()) - 1;
	CLatEdge spc;

	edgev.push_back(spc);

	while (pos >= 0 && edge.prior > edgev[pos].prior)
	{
		edgev[pos + 1] = edgev[pos];
		--pos;
	}

	++pos;
	edgev[pos] = edge;

	while (int(edgev.size()) > lim)
	{
		edgev.pop_back();
	}

	return;
}

void CGenLattice::AlphaBeta(const vector<CTagElem> &elems, 
                            vector<double> &alpha, 
                            vector<double> &beta)
{
	alpha.clear();
	beta.clear();

	int len = int(elems.size());

	alpha.resize(len);
	beta.resize(len);

	int lb = 2, rb = len - 3;
	int i;

	for (i = lb; i <= rb; ++i)
	{
		alpha[i] = -100000000.0;

		const vector<CLatEdge> &edges = elems[i].bestEdges;
		size_t k;

		for (k = 0; k < edges.size(); ++k)
		{
			const CLatEdge &e = edges[k];

			if (e.span.to != i)
			{
				continue;
			}

			double cur = e.score + alpha[e.span.from - 1];
			
			if (cur > alpha[i])
			{
				alpha[i] = cur;
			}
		}
	}

	for (i = rb; i >= lb; --i)
	{
		beta[i] = -100000000.0;

		const vector<CLatEdge> &edges = elems[i].bestEdges;
		size_t k;

		for (k = 0; k < edges.size(); ++k)
		{
			const CLatEdge &e = edges[k];
	
			if (e.span.from != i)
			{
				continue;
			}

			double cur = e.score + beta[e.span.to + 1];

			if (cur > beta[i])
			{
				beta[i] = cur;
			}
		}
	}
}

void CGenLattice::CrossRegEdge(vector<CTagElem> &elems)
{
	int len = int(elems.size());
	vector<int> preCnt(len);
	int lb = 2, rb = len - 3;
	int i;
	
	for (i = lb; i <= rb; ++i)
	{
		preCnt[i] = int(elems[i].bestEdges.size());
	}

	for (i = lb; i <= rb; ++i)
	{
		const vector<CLatEdge> &edges = elems[i].bestEdges;
		int k;

		for (k = 0; k < preCnt[i]; ++k)
		{
			const CLatEdge &e = edges[k];

			if (e.span.from != i)
			{
				elems[e.span.from].bestEdges.push_back(e);
			}
		}
	}
}

void CGenLattice::GenPrunedLat(const vector<CTagElem> &elems, 
                               const vector<double> &alpha, 
                               const vector<double> &beta, 
                               const double &thres,
                               map<CSpan, double> &lat)
{
    lat.clear();

    int len = int(elems.size());
    int lb = 2, rb = len - 3;
    int i;

    for (i = lb; i <= rb; ++i)
    {
        int cnt = int(elems[i].bestEdges.size());
        int k;

        for (k = 0; k < cnt; ++k)
        {
            const CLatEdge &e = elems[i].bestEdges[k];

            if (e.span.to != i)
            {
                continue;
            }

            double best = beta[2];
            double cur = e.score + alpha[e.span.from - 1] + beta[e.span.to + 1];

            if (best - cur > thres)
            {
                continue;
            }

            lat[CSpan(e.span.from - 2, e.span.to - 2)] = e.score;
        }
    }
}

string CGenLattice::FormLatStr(const map<CSpan, double> &lat)
{
    string latStr = "BEGIN\n";
    map<CSpan, double>::const_iterator it;

    for (it = lat.begin(); it != lat.end(); ++it)
    {
        int from = it->first.from, to = it->first.to;
        double score = it->second;
        char buf[32];

        sprintf(buf, "[%d, %d] %lf\n", from, to, score);
        latStr += buf;
    }

    latStr += "END";
    return latStr;
}
