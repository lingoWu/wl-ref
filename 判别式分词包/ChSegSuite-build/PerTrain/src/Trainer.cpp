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

//初始化
bool CTrainer::Initialize()
{
	if (!m_cChType.Initialize("data/punc", "data/alph", "data/date", "data/num"))
	{
		return false;
	}

	//规则最长词语字数
	m_nScanLen = 10;

	m_fOut.open("train_log.txt", ios::out);
	m_fOut << "--------------- Perseg train log file ---------------" << endl;

	return true;
}

//读取语料
bool CTrainer::Load(const string &corpFile)
{
	fstream fin(corpFile.c_str());
	if (!fin)
	{
		cout << "Can not open file: " << corpFile << endl;
		return false;
	}

	//训练集合总字数
	m_nTotalCh = 0;

	//读入各语句实例
	string line;

	while (getline(fin, line))
	{
		CInstance inst;

		//句首标记
		int i;
		for (i = 0; i < 2; ++i)
		{
			CInstElem tip("<s>", "s", -1);
			inst.seq.push_back(tip);
		}

		//各标注单元
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

		//句尾标记
		for (i = 0; i < 2; ++i)
		{
			CInstElem tip("</s>", "s", -1);
			inst.seq.push_back(tip);
		}

		m_vInstance.push_back(inst);

		//累加总字数
		m_nTotalCh += (int)inst.seq.size() - 4;
	}
	
	//训练实例数目
	m_nInstCnt = (int)m_vInstance.size();

	cout << "Total instances: " << m_nInstCnt << " , total characters: " << m_nTotalCh <<  endl;
	m_fOut << "Total instances: " << m_nInstCnt << " , total characters: " << m_nTotalCh <<  endl;

	fin.close();
	return true;
}

//训练
void CTrainer::Train(const string &corpFile, const int &loopCnt)
{
	//加载训练语料
	if (!Load(corpFile))
	{
		cout << "Read corpus file fail" << endl;
		return;
	}

	//开始计时
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

	//计时结束
	long end = clock();

	//训练耗时
	double sec = (double)(end - begin) / (double)CLOCKS_PER_SEC;
	cout << "\nTime cost: " << sec << " secs" << endl;
	m_fOut << "\nTime cost: " << sec << " secs" << endl;
}

//一轮训练
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

	//统计正确率
	float pCh = float(m_nTotalCh - diffCh) / float(m_nTotalCh);
	float pSen = float(m_nInstCnt - diffSen) / float(m_nInstCnt);

	cout << "Round finish" << endl;
	cout << "P(S) = " << pSen * 100 << "% , P(C) = " << pCh * 100 << "%" << endl;
	m_fOut << "P(S) = " << pSen * 100 << "% , P(C) = " << pCh * 100 << "%" << endl;
}

//生成随机训练序列
void CTrainer::RandSeq(vector<int> &seq)
{
	seq.clear();

	//顺序序列
	int i;

	for (i = 0; i < m_nInstCnt; ++i)
	{
		seq.push_back(i);
	}

	//随机排列
	for (i = m_nInstCnt - 1; i > 0; --i)
	{
		int r = rand() % (i + 1);

		int tmp = seq[i];
		seq[i] = seq[r];
		seq[r] = tmp;
	}
}

//使用该实例训练
int CTrainer::FitInst(const CInstance &inst)
{
	//复制实例
	int len = (int)inst.seq.size();
	vector<CTagElem> elems(len);
	int i;

	for (i = 0; i < len; ++i)
	{
		elems[i].ch = inst.seq[i].ch;
		elems[i].type = inst.seq[i].type;
	}

	//生成特征
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

	//求取所需得分
	vector<map<string, double> > scores;
	GetScores(featSets, scores);

	//规划最佳路径
	BestPath(elems, scores);

	//根据路径回溯tag序列
	TraceTag(elems);

	//根据差异调节权重
	int diff = Adjust(inst.seq, elems, featSets);

	return diff;
}

//求各个Span的分数
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

//生成该环境下的特征
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

//标注以to结尾长度len的词
void CTrainer::TagWord(vector<CTagElem> &elems, const int &to, const int &len)
{
	int from = to - len + 1;
	int p;
	
	//标注该词段
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

//动态规划最佳路径
void CTrainer::BestPath(vector<CTagElem> &elems, const vector<map<string, double> > &scores)
{
	//准备规划
	elems[1].best = 0.0;

	//规划非首尾部分
	int lb = 2;
	int rb = (int)elems.size() - 3;
	int i;

	for (i = lb; i <= rb; ++i)
	{
		bool first = true;
		int dis;

		//[i - dis, i]的串
		for (dis = 0; dis < m_nScanLen && i - dis >= lb; ++dis)
		{
			
			//标注该词段
			TagWord(elems, i, dis + 1);

			//该方案得分
			double score = elems[i - dis - 1].best;
			int p;

			for (p = i - dis; p <= i; ++p)
			{
				map<string, double>::const_iterator it  = scores[p].find(elems[p].tag);
				score += it->second;
				
			}

			//更新最佳值
			if (first || score > elems[i].best)
			{
				elems[i].best = score;
				elems[i].length = dis + 1;
			}

			first = false;
		}
	}
}

//求该特征集映射到tag的得分
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

//根据最佳路径回溯tag序列
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

//根据最佳标注和答案的差异调整权重
int CTrainer::Adjust(const vector<CInstElem> &ans, const vector<CTagElem> &best, const vector<vector<string> > &featSets)
{
	//需更新的pred列表
	map<string, double> updates;

	int diff = 0;
	int from = 2;
	int to = (int)ans.size() - 3;
	int i;

	//检查best每个汉字的tag与ans的异同
	for (i = from; i <= to; ++i)
	{
		if (best[i].tag == ans[i].tag)
		{
			continue;
		}

		++diff;

		//生成当前字的环境特征
		const vector<string> &feats = featSets[i];
		int cnt = (int)feats.size();
		int k;

		for (k = 0; k < cnt; ++k)
		{
			//答案加权重
			string pred = feats[k] + "=>" + ans[i].tag;
			updates[pred] += 1.0;

			//结果减权重
			pred = feats[k] + "=>" + best[i].tag;
			updates[pred] -= 1.0;
		}
	}

	//更新感知机权重
	Update(updates);

	return diff;
}

//输出非平均权重向量
void CTrainer::OutputNonAvg(const string &fWeight)
{
	ofstream fout(fWeight.c_str());
	if (!fout)
	{
		cout << "Can not open file: " << fWeight << endl;
		return;
	}

	map<string, double>::iterator it;

	//特征格式为：特征名tab特征值
	for (it = m_mPredWeight.begin(); it != m_mPredWeight.end(); ++it)
	{
		fout << it->first << "\t" << it->second << endl;
	}

	fout.close();
}

//输出平均权重向量
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

	//特征格式为：特征名tab特征值
	for (it = m_mSumForAvg.begin(); it != m_mSumForAvg.end(); ++it)
	{
		fout << it->first << "\t" << it->second / deno << endl;
	}

	fout.close();
}

//获取该推断的非平均感知机的分值
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

//获取该推断的平均感知机的分值
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

//更新权重
void CTrainer::Update(const map<string, double> &updates)
{
	//更新相关权重
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

	//最后一个训练例子时刷新平均权重
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
