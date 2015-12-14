#ifndef DECODER_H
#define DECODER_H

#include <string>
#include <vector>
#include <map>
#include "DataStruct.h"
#include "Perceptron.h"

using namespace std;

class CDecoder
{
public:
	CDecoder();
	~CDecoder();
	bool Initialize(const string &POSFile,
					const string &wordPOSFile,
					const string &modelFile,
					const int &histo);
	void Decode(const string &seq, string &result);

private:
	bool LoadPOS(const string &POSFile);
	bool LoadWordPOS(const string &wordPOSFile);
	void GenWordSeq(const string &seq);
	double EvalPOSTag(const int &idx,
					  const CTagNode &preNode,
					  const string &curPOS);
	double ItemScore(const string &item);
	void Search();
	void SortInsert(const CTagNode &cand, vector<CTagNode> &cands);
	void TraceBack(string &result);
	set<string> *GetIncPOSs(const string &word);

	vector<string> m_vWord;
	vector<vector<CTagNode> > m_vvPOSTag;
	int m_nHistogram;
	map<string, double> m_mItemBuf;
	CPerceptron m_cModel;
	map<string, set<string> > m_mWordPOS;
	set<string> m_sPOS;
};

#endif
