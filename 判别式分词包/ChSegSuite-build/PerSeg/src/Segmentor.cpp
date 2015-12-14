#include <strstream>
#include <fstream>
#include <iostream>
#include <strstream>
#include "Segmentor.h"

CSegmentor::CSegmentor()
{
}

CSegmentor::~CSegmentor()
{
}

//��ʼ��
bool CSegmentor::Initialize()
{
	if (!m_cChType.Initialize("data/punc", "data/alph", "data/date", "data/num"))
	{
		return false;
	}

	if (!m_cPerceptron.Initialize("data/model"))
	{
		return false;
	}

	//�����ܴʳ�
	m_nScanLen = 10;

	return true;
}

//�з�
void CSegmentor::Segment(const string &src, string &tgt)
{
	tgt.clear();

	vector<CTagElem> elems;
	Split(src, elems);
	
	//��������
	vector<vector<string> > featSets;
	vector<string> nullFeats;
	featSets.push_back(nullFeats);
	featSets.push_back(nullFeats);

	int len = (int)elems.size();
	int i;
	int from = 2;
	int to = len - 3;

	for (i = from; i <= to; ++i)
	{
		vector<string> feats;
		GenFeats(elems, i, feats);
		featSets.push_back(feats);
	}

	featSets.push_back(nullFeats);
	featSets.push_back(nullFeats);

	//��ȡ����÷�
	vector<map<string, double> > scores;
	GetScores(featSets, scores);

	//�滮���·��
	BestPath(elems, scores);

	//�����зֽ��
	TraceSeg(elems, tgt);
}

//�з�Ϊ��������
void CSegmentor::Split(const string &src, vector<CTagElem> &elems)
{
	elems.clear();

	int i;
	for (i = 0; i < 2; ++i)
	{
		CTagElem tip("<s>", -1);
		elems.push_back(tip);
	}

	istrstream is(src.c_str());
	string word;

	while (is >> word)
	{
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

			int type = m_cChType.GetType(ch);
			CTagElem elem(ch, type);

			elems.push_back(elem);
		}
	}

	for (i = 0; i < 2; ++i)
	{
		CTagElem tip("</s>", -1);
		elems.push_back(tip);
	}
}

//���ɸû����µ�����
void CSegmentor::GenFeats(const vector<CTagElem> &elems, const int &cur, vector<string> &feats)
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
		sprintf(feat, "C%dC%d==%s%s", n, n + 1, elems[cur + n].ch.c_str(), elems[cur + n + 1].ch.c_str());
		feats.push_back(feat);
	}

	//C-1C1
	sprintf(feat, "C-1C1==%s%s", elems[cur - 1].ch.c_str(), elems[cur + 1].ch.c_str());
	feats.push_back(feat);

	//Pu(C0)
	sprintf(feat, "PuC0==%d", elems[cur].type == 0 ? 1 : 0);
	feats.push_back(feat);

	//T(C-2)T(C-1)T(C0)T(C1)T(C2)
	sprintf(feat, "TC-22==%d%d%d%d%d", elems[cur - 2].type, elems[cur - 1].type, elems[cur].type, elems[cur + 1].type, elems[cur + 2].type);
	feats.push_back(feat);
}

//���������ӳ�䵽tag�ĵ÷�
double CSegmentor::FeatsPred(const vector<string> &feats, const string &tag)
{
	double score = 0.0;
	int cnt = (int)feats.size();
	int k;

	for (k = 0; k < cnt; ++k)
	{
		string pred = feats[k] + "=>" + tag;
		score += m_cPerceptron.PredWeight(pred);
	}

	return score;
}

//�����Span�ķ���
void CSegmentor::GetScores(const vector<vector<string> > &featSets, vector<map<string, double> > &scores)
{
	scores.clear();

	int len = (int)featSets.size();
	int lb = 2;
	int rb = len - 3;
	int i;

	scores.resize(len);

	for (i = lb; i <= rb; ++i)
	{
		scores[i].insert(map<string, double>::value_type("b", FeatsPred(featSets[i], "b")));
		scores[i].insert(map<string, double>::value_type("m", FeatsPred(featSets[i], "m")));
		scores[i].insert(map<string, double>::value_type("e", FeatsPred(featSets[i], "e")));
		scores[i].insert(map<string, double>::value_type("s", FeatsPred(featSets[i], "s")));
	}
}

//��ע��to��β����len�Ĵ�
void CSegmentor::TagWord(vector<CTagElem> &elems, const int &to, const int &len)
{
	int from = to - len + 1;
	int p;

	//��ע�ôʶ�
	for (p = from; p <= to; ++p)
	{
		if (p == from)
		{
			if (len == 1)
			{
				elems[p].tag = "s";
			}
			else
			{
				elems[p].tag = "b";
			}
		}
		else
		{
			if (p == to)
			{
				elems[p].tag = "e";
			}
			else
			{
				elems[p].tag = "m";
			}
		}
	}
}

//��̬�滮���·��
void CSegmentor::BestPath(vector<CTagElem> &elems, const vector<map<string, double> > &scores)
{
	//׼���滮
	elems[1].best = 0.0;

	//�滮����β����
	int lb = 2;
	int rb = (int)elems.size() - 3;
	int i;

	for (i = lb; i <= rb; ++i)
	{
		bool first = true;
		int dis;

		//[i - dis, i]�Ĵ�
		for (dis = 0; dis < m_nScanLen && i - dis >= lb; ++dis)
		{

			//��ע�ôʶ�
			TagWord(elems, i, dis + 1);

			//�÷����÷�
			double score = elems[i - dis - 1].best;
			int p;

			for (p = i - dis; p <= i; ++p)
			{
				map<string, double>::const_iterator it  = scores[p].find(elems[p].tag);
				score += it->second;
			}

			//�������ֵ
			if (first || score > elems[i].best)
			{
				elems[i].best = score;
				elems[i].length = dis + 1;
			}

			first = false;
		}
	}
}

//�������·������tag����
void CSegmentor::TraceSeg(vector<CTagElem> &elems, string &seg)
{
	seg.clear();

	int from = 2;
	int p = static_cast<int>(elems.size() - 3);

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
}
