#include <strstream>
#include <fstream>
#include <iostream>
#include <strstream>
#include <ctime>
#include "Trainer.h"

CTrainer::CTrainer()
{
}

CTrainer::~CTrainer()
{
}

//��ʼ��
bool CTrainer::Initialize()
{
	if (!m_cChType.Initialize("data/punc", "data/alph", "data/date", "data/num"))
	{
		return false;
	}

	//�������������
	m_nScanLen = 10;

	m_fOut.open("train_log.txt", ios::out);
	m_fOut << "--------------- Perseg train log file ---------------" << endl;

	return true;
}

//��ȡ����
bool CTrainer::Load(const string &corpFile)
{
	fstream fin(corpFile.c_str());
	if (!fin)
	{
		cout << "Can not open file: " << corpFile << endl;
		return false;
	}

	//ѵ������������
	m_nTotalCh = 0;

	//��������ʵ��
	string line;

	while (getline(fin, line))
	{
		CInstance inst;

		//���ױ��
		int i;
		for (i = 0; i < 2; ++i)
		{
			CInstElem tip("<s>", "s", -1);
			inst.seq.push_back(tip);
		}

		//����ע��Ԫ
		istrstream is(line.c_str());
		string part;

		while (is >> part)
		{
			size_t split = part.rfind('/');
			string ch = part.substr(0, split);
			string tag = part.substr(split + 1, part.length() - split - 1);
			int type = m_cChType.GetType(ch);

			CInstElem elem(ch, tag, type);
			inst.seq.push_back(elem);
		}

		//��β���
		for (i = 0; i < 2; ++i)
		{
			CInstElem tip("</s>", "s", -1);
			inst.seq.push_back(tip);
		}

		m_vInstance.push_back(inst);

		//�ۼ�������
		m_nTotalCh += (int)inst.seq.size() - 4;
	}
	
	//ѵ��ʵ����Ŀ
	m_nInstCnt = (int)m_vInstance.size();

	cout << "Total instances: " << m_nInstCnt << " , total characters: " << m_nTotalCh <<  endl;
	m_fOut << "Total instances: " << m_nInstCnt << " , total characters: " << m_nTotalCh <<  endl;

	fin.close();
	return true;
}

//ѵ��
void CTrainer::Train(const string &corpFile, const int &loopCnt)
{
	//����ѵ������
	if (!Load(corpFile))
	{
		cout << "Read corpus file fail" << endl;
		return;
	}

	//��ʼ��ʱ
	long begin = clock();

	int i;

	for (i = 0; i < loopCnt; ++i)
	{
		cout << "\nThe " << i + 1 << " th training round" << endl;
		m_fOut << "\nThe " << i + 1 << " th training round" << endl;

		m_nCurRound = i;
		Round();

		char name[32];
		sprintf(name, "work/weight_after%d.txt", i + 1);
		cout << "Output non_avg weight...";
		flush(cout);
		OutputNonAvg(name);
		cout << "succeed" << endl;

		sprintf(name, "work/avg_weight_after_%d.txt", i + 1);
		cout << "Output avg weight...";
		flush(cout);
		OutputAvg(name);
		cout << "succeed" << endl;
	}

	//��ʱ����
	long end = clock();

	//ѵ����ʱ
	double sec = (double)(end - begin) / (double)CLOCKS_PER_SEC;
	cout << "\nTime cost: " << sec << " secs" << endl;
	m_fOut << "\nTime cost: " << sec << " secs" << endl;
}

//һ��ѵ��
void CTrainer::Round()
{
	vector<int> idxSeq;
	RandSeq(idxSeq);

	int i;
	int diffSen = 0;
	int diffCh = 0;

	for (i = 0; i < m_nInstCnt; ++i)
	{
		m_nCurIndex = i;
		int diff = FitInst(m_vInstance[idxSeq[i]]);

		diffCh += diff;
		diffSen += diff > 0 ? 1 : 0;

		if (i % 100 == 0)
		{
			cout << i << endl;
		}
	}

	//ͳ����ȷ��
	float pCh = float(m_nTotalCh - diffCh) / float(m_nTotalCh);
	float pSen = float(m_nInstCnt - diffSen) / float(m_nInstCnt);

	cout << "Round finish" << endl;
	cout << "P(S) = " << pSen * 100 << "% , P(C) = " << pCh * 100 << "%" << endl;
	m_fOut << "P(S) = " << pSen * 100 << "% , P(C) = " << pCh * 100 << "%" << endl;
}

//�������ѵ������
void CTrainer::RandSeq(vector<int> &seq)
{
	seq.clear();

	//˳������
	int i;

	for (i = 0; i < m_nInstCnt; ++i)
	{
		seq.push_back(i);
	}

	//�������
	for (i = m_nInstCnt - 1; i > 0; --i)
	{
		int r = rand() % (i + 1);

		int tmp = seq[i];
		seq[i] = seq[r];
		seq[r] = tmp;
	}
}

//ʹ�ø�ʵ��ѵ��
int CTrainer::FitInst(const CInstance &inst)
{
	//����ʵ��
	int len = (int)inst.seq.size();
	vector<CTagElem> elems(len);
	int i;

	for (i = 0; i < len; ++i)
	{
		elems[i].ch = inst.seq[i].ch;
		elems[i].type = inst.seq[i].type;
	}

	//��������
	vector<vector<string> > featSets;
	vector<string> nullFeats;
	featSets.push_back(nullFeats);
	featSets.push_back(nullFeats);

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

	//����·������tag����
	TraceTag(elems);

	//���ݲ������Ȩ��
	int diff = Adjust(inst.seq, elems, featSets);

	return diff;
}

//�����Span�ķ���
void CTrainer::GetScores(const vector<vector<string> > &featSets, vector<map<string, double> > &scores)
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

//���ɸû����µ�����
void CTrainer::GenFeats(const vector<CTagElem> &elems, const int &cur, vector<string> &feats)
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

//��ע��to��β����len�Ĵ�
void CTrainer::TagWord(vector<CTagElem> &elems, const int &to, const int &len)
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
void CTrainer::BestPath(vector<CTagElem> &elems, const vector<map<string, double> > &scores)
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

//���������ӳ�䵽tag�ĵ÷�
double CTrainer::FeatsPred(const vector<string> &feats, const string &tag)
{
	double score = 0.0;
	int cnt = (int)feats.size();
	int k;

	for (k = 0; k < cnt; ++k)
	{
		string pred = feats[k] + "=>" + tag;
		score += GetNonAvg(pred);
	}

	return score;
}

//�������·������tag����
void CTrainer::TraceTag(vector<CTagElem> &elems)
{
	int from = 2;
	int p = (int)elems.size() - 3;

	while (p >= from)
	{
		int len = elems[p].length;
		TagWord(elems, p, len);

		p -= len;
	}
}

//������ѱ�ע�ʹ𰸵Ĳ������Ȩ��
int CTrainer::Adjust(const vector<CInstElem> &ans, const vector<CTagElem> &best, const vector<vector<string> > &featSets)
{
	//����µ�pred�б�
	map<string, double> updates;

	int diff = 0;
	int from = 2;
	int to = (int)ans.size() - 3;
	int i;

	//���bestÿ�����ֵ�tag��ans����ͬ
	for (i = from; i <= to; ++i)
	{
		if (best[i].tag == ans[i].tag)
		{
			continue;
		}

		++diff;

		//���ɵ�ǰ�ֵĻ�������
		const vector<string> &feats = featSets[i];
		int cnt = (int)feats.size();
		int k;

		for (k = 0; k < cnt; ++k)
		{
			//�𰸼�Ȩ��
			string pred = feats[k] + "=>" + ans[i].tag;
			updates[pred] += 1.0;

			//�����Ȩ��
			pred = feats[k] + "=>" + best[i].tag;
			updates[pred] -= 1.0;
		}
	}

	//���¸�֪��Ȩ��
	Update(updates);

	return diff;
}

//�����ƽ��Ȩ������
void CTrainer::OutputNonAvg(const string &fWeight)
{
	ofstream fout(fWeight.c_str());
	if (!fout)
	{
		cout << "Can not open file: " << fWeight << endl;
		return;
	}

	map<string, double>::iterator it;

	//������ʽΪ��������tab����ֵ
	for (it = m_mPredWeight.begin(); it != m_mPredWeight.end(); ++it)
	{
		fout << it->first << "\t" << it->second << endl;
	}

	fout.close();
}

//���ƽ��Ȩ������
void CTrainer::OutputAvg(const string &fWeight)
{
	ofstream fout(fWeight.c_str());
	if (!fout)
	{
		cout << "Can not open file: " << fWeight << endl;
		return;
	}

	double deno = (double)(m_nCurRound + 1) * (double)m_nInstCnt;
	map<string, double>::iterator it;

	//������ʽΪ��������tab����ֵ
	for (it = m_mSumForAvg.begin(); it != m_mSumForAvg.end(); ++it)
	{
		fout << it->first << "\t" << it->second / deno << endl;
	}

	fout.close();
}

//��ȡ���ƶϵķ�ƽ����֪���ķ�ֵ
double CTrainer::GetNonAvg(const string &pred)
{
	map<string, double>::iterator it = m_mPredWeight.find(pred);

	if (it != m_mPredWeight.end())
	{
		return it->second;
	}
	else
	{
		return 0.0;
	}
}

//��ȡ���ƶϵ�ƽ����֪���ķ�ֵ
double CTrainer::GetAvgSum(const string &pred)
{
	map<string, double>::iterator it = m_mSumForAvg.find(pred);

	if (it != m_mSumForAvg.end())
	{
		return it->second;
	}
	else
	{
		return 0.0;
	}
}

//����Ȩ��
void CTrainer::Update(const map<string, double> &updates)
{
	//�������Ȩ��
	map<string, double>::const_iterator it;

	for (it = updates.begin(); it != updates.end(); ++it)
	{
		int lastRnd = m_mLastModify[it->first].first;
		int lastIdx = m_mLastModify[it->first].second;
		int incTime = m_nInstCnt * (m_nCurRound - lastRnd) + (m_nCurIndex - lastIdx);
		double incAvg = (double)incTime * GetNonAvg(it->first) + it->second;
		m_mSumForAvg[it->first] +=  incAvg;

		m_mPredWeight[it->first] += it->second;

		m_mLastModify[it->first].first = m_nCurRound;
		m_mLastModify[it->first].second = m_nCurIndex;
	}

	//���һ��ѵ������ʱˢ��ƽ��Ȩ��
	if (m_nCurIndex == m_nInstCnt - 1)
	{
		map<string, double>::iterator it;

		for (it = m_mSumForAvg.begin(); it != m_mSumForAvg.end(); ++it)
		{
			int lastRnd = m_mLastModify[it->first].first;
			int lastIdx = m_mLastModify[it->first].second;
			int incTime = m_nInstCnt * (m_nCurRound - lastRnd) + (m_nCurIndex - lastIdx);
			double incAvg = (double)incTime * GetNonAvg(it->first);
			m_mSumForAvg[it->first] +=  incAvg;

			m_mLastModify[it->first].first = m_nCurRound;
			m_mLastModify[it->first].second = m_nCurIndex;
		}
	}
}
