#include <fstream>
#include <iostream>
#include "Lexicon.h"

CLexicon::CLexicon()
{
}

CLexicon::~CLexicon()
{
}

//��ʼ��
bool CLexicon::Initialize(const string &lex)
{
	if (!Load(lex))
	{
		cout << "Load lexicon fail" << endl;
		return false;
	}

	return true;
}

//���شʵ��ļ�
bool CLexicon::Load(const string &lex)
{
	ifstream fin(lex.c_str());
	if (!fin)
	{
		cout << "Can not open lexicon file: " << lex << endl;
		return false;
	}

	string line;

	while (getline(fin, line))
	{
		set<string>::iterator it = m_sLexicon.find(line);

		if (it == m_sLexicon.end())
		{
			m_sLexicon.insert(line);
		}
	}

	fin.close();
	return true;
}

//�ô��Ƿ������ʵ�
bool CLexicon::Exist(const string &word)
{
	set<string>::iterator it = m_sLexicon.find(word);
	return it != m_sLexicon.end();
}
