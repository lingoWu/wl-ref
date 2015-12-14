#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <vector>
#include <fstream>
#include "Perceptron.h"
#include "ChType.h"
#include "DataStruct.h"

using namespace std;

class CGenLattice
{
public:
	CGenLattice();
	~CGenLattice();
	bool Initialize();
    void Generate(const string &src, 
                  string &best, 
                  const double &thres, 
                  string &latStr);

private:
	void InitBounds();
	void Split(const string &src, vector<CTagElem> &elems);
	void CrossRegEdge(vector<CTagElem> &elems);
	void BestEdges(vector<CTagElem> &elems, 
                   vector<map<string, double> > &scores, 
                   const int &degree);
	string TraceSeg(vector<CTagElem> &elems);
    void GenPrunedLat(const vector<CTagElem> &elems, 
                      const vector<double> &alpha, 
                      const vector<double> &beta, 
                      const double &thres,
                      map<CSpan, double> &lat);
    string FormLatStr(const map<CSpan, double> &lat);
	void SortInsert(const CLatEdge &edge, vector<CLatEdge> &edgev, const int &lim);
	void MarkBnd(vector<CTagElem> &elems, const int &to, const int &len);
	void GenFeats(const vector<CTagElem> &elems, const int &cur, vector<string> &feats);
	double FeatsPred(const vector<string> &feats, const string &tag);
	void GetScores(const vector<vector<string> > &featSets, vector<map<string, double> > &scores);
	void AlphaBeta(const vector<CTagElem> &elems, vector<double> &alpha, vector<double> &beta);

	int m_nScanLen;
	int m_nBndCnt;
	int m_nPOSCnt;

	CPerceptron m_cPerceptron;
	CChType m_cChType;
	vector<string> m_vPOS;
	vector<string> m_vBound;
};

#endif

