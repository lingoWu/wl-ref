#include <iostream>
#include <fstream>
#include <strstream>
#include "Decoder.h"

CDecoder::CDecoder()
{
}

CDecoder::~CDecoder()
{
}

bool CDecoder::Initialize(const string &POSFile,
						  const string &wordPOSFile,
						  const string &modelFile,
						  const int &histo)
{
	m_nHistogram = histo;
	return LoadPOS(POSFile)
		   && LoadWordPOS(wordPOSFile)
		   && m_cModel.Initialize(modelFile);
}

bool CDecoder::LoadPOS(const string &POSFile)
{
	ifstream fin(POSFile.c_str());

	if (!fin)
	{
		cerr << "Can not open file: " << POSFile << endl;
		return false;
	}

	string line;

	while (getline(fin, line))
	{
		if (!line.empty())
		{
			m_sPOS.insert(line);
		}
	}

	fin.close();
	return true;
}

bool CDecoder::LoadWordPOS(const string &wordPOSFile)
{
	ifstream fin(wordPOSFile.c_str());
	
	if (!fin)
	{
		cerr << "Can not open file: " << wordPOSFile << endl;
		return false;
	}

	string line;

	while (getline(fin, line))
	{
		istrstream is(line.c_str());
		string word;
		string POS;

		is >> word;

		while (is >> POS)
		{
			m_mWordPOS[word].insert(POS);
		}
	}

	fin.close();
	return true;
}

void CDecoder::Decode(const string &seq, string &result)
{
	m_mItemBuf.clear();
	GenWordSeq(seq);
	Search();
	TraceBack(result);
}

void CDecoder::TraceBack(string &result)
{
	result = "";

	int to = int(m_vWord.size()) - 3;
	int i;
	int best = 0;

	for (i = to; i >= 2; --i)
	{
		result = m_vWord[i] + "/" + m_vvPOSTag[i][best].POS + " " + result;
		best = m_vvPOSTag[i][best].prior;
	}
}

void CDecoder::GenWordSeq(const string &seq)
{
	m_vWord.clear();
	m_vWord.push_back("<s>");
	m_vWord.push_back("<s>");

	istrstream is(seq.c_str());
	string word;
	
	while (is >> word)
	{
		m_vWord.push_back(word);
	}

	m_vWord.push_back("</s>");
	m_vWord.push_back("</s>");
}

double CDecoder::EvalPOSTag(const int &idx,
							const CTagNode &preNode,
							const string &curPOS)
{
	const string &w_2 = m_vWord[idx - 2];
	const string &w_1 = m_vWord[idx - 1];
	const string &w = m_vWord[idx];
	const string &w1 = m_vWord[idx + 1];
	const string &w2 = m_vWord[idx + 2];
	const string &t_2 = preNode.POS_1;
	const string &t_1 = preNode.POS;
	const string &t = curPOS;
	double score = 0.0;

	score += ItemScore("w_2t="+w_2+"|"+t);
	score += ItemScore("w_1t="+w_1+"|"+t);
	score += ItemScore("wt="+w+"|"+t);
	score += ItemScore("w1t="+w1+"|"+t);
	score += ItemScore("w2t="+w2+"|"+t);
	score += ItemScore("t_1t="+t_1+"|"+t);
	score += ItemScore("t_2t_1t="+t_2+"|"+t_1+"|"+t);
	return score;
}

double CDecoder::ItemScore(const string &item)
{
	map<string, double>::const_iterator it = m_mItemBuf.find(item);

	if (it != m_mItemBuf.end())
	{
		return it->second;
	}

	double score = m_cModel.PredWeight(item);

	m_mItemBuf[item] = score;
	return score;
}

void CDecoder::Search()
{
	m_vvPOSTag.clear();
	m_vvPOSTag.resize(m_vWord.size());
	m_vvPOSTag[1].push_back(CTagNode("<s>", "<s>", 0.0, -1));

	int from = 2;
	int to = int(m_vWord.size()) - 3;
	int i;

	for (i = from; i <= to; ++i)
	{
		set<string> *pIncPOSs = GetIncPOSs(m_vWord[i]);
		const vector<CTagNode> &preTags = m_vvPOSTag[i - 1];
		vector<CTagNode> &curTags = m_vvPOSTag[i];
		int p;

		for (p = 0; p < int(preTags.size()); ++p)
		{
			const CTagNode &preNode = preTags[p];
			set<string>::const_iterator q;

			for (q = pIncPOSs->begin(); q != pIncPOSs->end(); ++q)
			{
				const string &curPOS = *q;
				double score = preNode.score + EvalPOSTag(i, preNode, curPOS);
				CTagNode curNode(preNode.POS, curPOS, score, p);

				SortInsert(curNode, curTags);
			}
		}
	}
}

set<string> *CDecoder::GetIncPOSs(const string &word)
{
	map<string, set<string> >::iterator it = m_mWordPOS.find(word);

	if (it != m_mWordPOS.end())
	{
		return &(it->second);
	}
	else
	{
		return &m_sPOS;
	}
}

void CDecoder::SortInsert(const CTagNode &cand, vector<CTagNode> &cands)
{
	if (cands.empty())
	{
		cands.push_back(cand);
		return;
	}

	int sum = int(cands.size());
	int p = sum - 1;

	if (sum == m_nHistogram && cand.score <= cands[p].score)
	{
		return;
	}

	while (p >= 0 && !cand.Equival(cands[p]))
	{
		--p;
	}

	if (p >= 0)
	{
		if (cand.score <= cands[p].score)
		{
			return;
		}
		else
		{
			--p;
		}
	}
	else
	{
		p = sum - 1;

		CTagNode spc;

		cands.push_back(spc);
	}

	while (p >= 0 && cand.score > cands[p].score)
	{
		cands[p + 1] = cands[p];
		--p;
	}

	++p;
	cands[p] = cand;

	while (int(cands.size()) > m_nHistogram)
	{
		cands.pop_back();
	}
}
