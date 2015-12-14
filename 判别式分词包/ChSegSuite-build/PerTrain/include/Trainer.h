#ifndef TRAINER
#define TRAINER

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "DataStruct.h"
#include "ChType.h"

using namespace std;

class CTrainer
{
public:
	CTrainer();
	~CTrainer();
	bool Initialize();
	void Train(const string &corpFile, const int &loopCnt);
	
private:
	bool Load(const string &corpFile);
	void Round();
	void RandSeq(vector<int> &seq);
	int FitInst(const CInstance &inst);
	int Adjust(const vector<CInstElem> &ans, const vector<CTagElem> &best, const vector<vector<string> > &featSets);
	void BestPath(vector<CTagElem> &elems, const vector<map<string, double> > &scores);
	void TagWord(vector<CTagElem> &elems, const int &to, const int &len);
	void TraceTag(vector<CTagElem> &elems);
	void GenFeats(const vector<CTagElem> &elems, const int &cur, vector<string> &feats);
	double FeatsPred(const vector<string> &feats, const string &tag);
	void GetScores(const vector<vector<string> > &featSets, vector<map<string, double> > &scores);

	double GetNonAvg(const string &pred);
	double GetAvgSum(const string &pred);
	void Update(const map<string, double> &updates);
	void OutputNonAvg(const string &fWeight);
	void OutputAvg(const string &fWeight);

	fstream m_fOut;
	int m_nTotalCh;
	int m_nScanLen;

	int m_nInstCnt;
	int m_nCurRound;
	int m_nCurIndex;

	vector<CInstance> m_vInstance;
	CChType m_cChType;

	map<string, double> m_mPredWeight;
	map<string, double> m_mSumForAvg;
	map<string, pair<int, int> > m_mLastModify;

};

#endif
