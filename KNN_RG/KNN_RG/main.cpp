// KNNRG.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
using namespace std;

//在 string向量里找出字符串str，返回索引
int find_word_in_vc(string &str, vector<string> &string_vc)
{
	vector<string>::iterator it = find(string_vc.begin(), string_vc.end(), str);
	if (it == string_vc.end())
	{
		return -1;
	}
	return it - string_vc.begin();
}

class trainRow
{
public:
	bool* onehot;
	double* emotion;//anger,disgust,fear,joy,sad,surprise

	trainRow();
	trainRow(int dictSize);
	~trainRow();
};
trainRow::trainRow()
{
	emotion = new double[6];
}
trainRow::trainRow(int dictSize)
{
	onehot = new bool[dictSize];
	emotion = new double[6];
}
trainRow::~trainRow()
{
	delete[] onehot;
	delete[] emotion;
}
class trainCase
{
public:
	int dictSize;			//不重复记录单词数，矩阵的列数 
	int rowCnt;				//一共多少训练用的文章，矩阵的行数 
	vector<string> wordsVC;	//不重复记录单词 
	vector<trainRow> matrix;//onehot矩阵 

	trainCase();
	trainCase(const string &filename);
	~trainCase();
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);//获取onehot矩阵，emotions数组 
};
trainCase::trainCase()
{
	dictSize = rowCnt = 0;
}
trainCase::trainCase(const string &filename)
{
	get_words(filename);//Initialize dictSize/rowCnt/wordsVC
	write_matrix(filename);//Initialize matrix
}
void trainCase::get_words(const string &filename)
{
	rowCnt = 0;
	ifstream fin(filename.c_str());
	string s;
	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		rowCnt++;
		string words, word;

		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		ss.str(words.c_str);
		while (ss >> word)
		{
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//从未出现过的单词 
			{
				wordsVC.push_back(word);//记录到vector里
			}
		}
	}
	dictSize = wordsVC.size();
}
void trainCase::write_matrix(const string &filename)
{
	//打开要读的文件 
	ifstream fin(filename.c_str());

	string s;//用于记录读取到的每一行的字符串 

	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		string words, word;
		istringstream ss(s);
		trainRow* TR = new trainRow(dictSize);
		getline(ss, words, ',');//获得单词序列
		istringstream Wss(words);
		while (Wss >> word)//获得当前行的记录 
		{
			//在 string_vc里找出outstr，返回索引
			int location = find_word_in_vc(word, wordsVC);
			if (location != -1)
			{
				TR->onehot[location] = true;
			}
		}
		for (int i = 0; i < 6; i++)
		{
			getline(ss, TR->emotion[i], ',');
		}
		matrix.push_back(*TR);
	}
}

int main()
{
    return 0;
}

