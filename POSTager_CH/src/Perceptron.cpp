#include <fstream>
#include <iostream>
#include <cmath>
#include "Perceptron.h"

CPerceptron::CPerceptron()
{
}

CPerceptron::~CPerceptron()
{
}

//��ʼ��
bool CPerceptron::Initialize(const string &model)
{
	ifstream fin(model.c_str());

	if (!fin)
	{
		cout << "Can not open file: " << model << endl;
		return false;
	}

	string line;

	while (getline(fin, line))
	{
		size_t split = line.find('\t');
		string pred = line.substr(0, split);
		string weight = line.substr(split + 1, line.length() - split - 1);
		double weightVal = atof(weight.c_str());

		m_mPredWeight[pred] = weightVal;
	}

	fin.close();
	return true;
}

//��ȡ���ƶϵķ�ֵ
double CPerceptron::PredWeight(const string &pred)
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
