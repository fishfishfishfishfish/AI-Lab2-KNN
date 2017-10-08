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
#include <stack>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
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
void normalize_6(double* x)
{
	double sum = 0;
	for (int i = 0; i < 6; i++)
	{
		sum += x[i];
	}
	for (int i = 0; i < 6; i++)
	{
		x[i] = x[i] / sum;
	}
}
double mean(double* data, int size)
{
	double res = 0;
	for (int i = 0; i < size; i++)
	{
		res += data[i];
	}
	res = res / size;
	return res;
}
double variance(double* data, int size)
{
	double m = mean(data, size);
	double res = 0;
	for (int i = 0; i < size; i++)
	{
		res += pow(data[i] - m, 2);
	}
	return res;
}

class trainRow
{
public:
	int dictSize;
	bool* onehot;
	double* emotion;//anger,disgust,fear,joy,sad,surprise

	trainRow();
	trainRow(int dictSize);
	trainRow(const trainRow& TR);//带指针数组的类一定要有拷贝构造函数
	~trainRow();
};
trainRow::trainRow()
{
	dictSize = 0;
	emotion = new double[6];
}
trainRow::trainRow(int dictSize)
{
	this->dictSize = dictSize;
	onehot = new bool[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		onehot[i] = false;
	}
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = 0;
	}
}
trainRow::trainRow(const trainRow& TR)
{
	dictSize = TR.dictSize;
	onehot = new bool[TR.dictSize];
	for (int i = 0; i < TR.dictSize; i++)
	{
		onehot[i] = TR.onehot[i];
	}
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = TR.emotion[i];
	}
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
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);//获取onehot矩阵，emotions数组
	friend void operator<<(ostream& os, const trainCase& TC);
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
		ss.str(words.c_str());
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
		//trainRow TR(dictSize);
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
			string tmpEmotion;
			getline(ss,tmpEmotion, ',');
			TR->emotion[i] = atof(tmpEmotion.c_str());
		}
		matrix.push_back(*TR);
	}
}
void operator<<(ostream& os, const trainCase& TC)
{
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < TC.dictSize; j++)
		{
			os << TC.matrix[i].onehot[j] << ' ';
		}
		os << endl;
		for (int j = 0; j < 6; j++)
		{
			os << TC.matrix[i].emotion[j] << ' ';
		}
		os << endl;
	}
}

class testCase
{
public:
	bool* onehot;//写出test文章的onehot向量
	int dictSize;//训练集的字典大小
	vector<int> newWord;//记录测试集里出现的不在字典里的词的数据
	multimap<double, int> distPairs;//记录测试数据和所有训练数据的距离，first是距离，second是训练数据的索引

	testCase();
	testCase(int w);
	testCase(const testCase& TC);
	~testCase();
	void getOnehot(const string &words, vector<string> &vc);//得到onehot向量
	double distCnt(bool *trainRow, int dictSize, double distType);//返回测试数据和摸一个训练数据之间的距离
	void setDistPairs(int index, double dist);//向distPairs中添加一条距离信息
	double* distNormalize(int k);
	double* RG(int k, trainCase& TC);//通过记录的信息和训练集的感情数据对测试数据分类，返回感情
	void printPairs();
	void printOnehot();
};
testCase::testCase()
{
	dictSize = 0;
	onehot = new bool[dictSize];
}
testCase::testCase(int w)
{
	dictSize = w;
	onehot = new bool[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		onehot[i] = false;
	}
}
testCase::testCase(const testCase& TC)
{
	dictSize = TC.dictSize;
	onehot = new bool[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		onehot[i] = TC.onehot[i];
	}
	newWord = TC.newWord;
	distPairs = TC.distPairs;
}
testCase::~testCase()
{
	delete[]onehot;
}

//得到onehot向量
void testCase::getOnehot(const string &words, vector<string> &vc)
{
	string currWord;
	istringstream iss(words);
	while (iss >> currWord)
	{
		int location = find_word_in_vc(currWord, vc);
		if (location != -1)
		{
			onehot[location] = true;
		}
		else
		{
			newWord.push_back(1);
		}
	}
}

//返回测试数据和一个训练数据之间的距离
double testCase::distCnt(bool *trainRow, int dictSize, double distType)
{
	double dist = 0;
	for (int i = 0; i < dictSize; i++)
	{
		dist += pow(trainRow[i] - onehot[i], distType);
	}
	for (int i = 0; i < newWord.size(); i++)
	{
		dist += pow(0 - newWord[i], distType);
	}

	dist = pow(dist, 1.0 / distType);
	return dist;
}

//向distPairs中添加一条距离信息
void testCase::setDistPairs(int index, double dist)
{
	distPairs.insert(pair<double, int>(dist, index));
}

double* testCase::distNormalize(int k)//用后记得delete
{
	//standard score
	double* res = new double[k];
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();

		for (int i = 0; i < k; i++)
		{
			res[i] = 1.0/(1+it->first);
			it++;
		}

		double m = mean(res, k);
		double v = variance(res, k);

		for (int i = 0; i < k; i++)
		{
			res[i] = abs(res[i] - m) / sqrt(v);
		}
	}
	return res;
}
//通过记录的信息和训练集的感情数据对测试数据分类，返回感情
double* testCase::RG(int k, trainCase& TC)
{
	double* testEmo = new double[6];//记录测试数据的情绪数据
	for (int i = 0; i < 6; i++)
	{
		testEmo[i] = 0;
	}
	double* weight = new double[k];
	weight = distNormalize(k);
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();

		for (int i = 0; i < k; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				testEmo[j] += TC.matrix[it->second].emotion[j]*weight[i];
			}	
			
			it++;
		}
		normalize_6(testEmo);
	}
	delete[] weight;
	return testEmo;	
}

void testCase::printPairs()
{
	for (multimap<double, int>::iterator it = distPairs.begin(); it != distPairs.end(); it++)
	{
		cout << it->first << '\t' << it->second << endl;
	}
}
void testCase::printOnehot()
{
	for (int i = 0; i < dictSize; i++)
	{
		cout << onehot[i] << ' ';
	}
	cout << endl;
}

void validHandle(ostream &os, const string &infilename, trainCase &traincase, int k)
{
	ifstream fin(infilename);
	string s;
	int testCnt = 0;//总的测试数据数量
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		string words;
		double* ansEmotion;
		istringstream iss1(s);
		getline(iss1, words, ',');

		testCase testcase(traincase.dictSize);
		testcase.getOnehot(words, traincase.wordsVC);

		for (int i = 0; i < traincase.rowCnt; i++)
		{
			double dist = testcase.distCnt(traincase.matrix[i].onehot, traincase.dictSize, 2);
			testcase.setDistPairs(i, dist);
		}

		//testcase.printPairs();//debug
		ansEmotion = testcase.RG(k, traincase);

		for (int i = 0; i < 6; i++)
		{
			os << ansEmotion[i] << '\t';
		}
		os << endl;
		delete[] ansEmotion;
		//system("pause");
	}
}

int main() 
{
	trainCase TC("train_set.csv");
	string validfile = "validation_set.csv";
	int start, end;
	cout << "start?" << endl;
	cin >> start;
	cout << "end?" << endl;
	cin >> end;
	for (int k = start; k < end; k++)
	{
		string resfile;
		stringstream ss;
		ss << "fv";
		ss << k;
		ss << ".txt";
		ss >> resfile;
		ofstream fout(resfile);
		validHandle(fout, validfile, TC, k);
		cout << "k=" << k << "finished" << endl;
	}
	
	system("pause");
	return 0;
}

