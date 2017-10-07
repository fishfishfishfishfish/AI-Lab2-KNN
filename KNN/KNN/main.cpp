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

class trainCase
{
public:
	int dictSize;	//不重复记录单词数，矩阵的列数 
	int rowCnt;		//一共多少训练用的文章，矩阵的行数 
	vector<string> wordsVC;//不重复记录单词 
	bool** onehotMatrix;	//onehot矩阵 
	string* emotions; 		//记录每一行/每一篇文章的感情数据 

	~trainCase();
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);//获取onehot矩阵，emotions数组 
};

trainCase::~trainCase()
{
	for (int i = 0; i < rowCnt; i++)
	{
		delete[] onehotMatrix[i];
	}
	delete[]onehotMatrix;
	delete[]emotions;
}

//计算出需要的单词向量 
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
	onehotMatrix = new bool*[rowCnt];
	for (int i = 0; i < rowCnt; i++)
	{
		onehotMatrix[i] = new bool[dictSize];
		for (int j = 0; j < dictSize; j++)
		{
			onehotMatrix[i][j] = false;
		}
	}
	emotions = new string[dictSize];
}

//在 string_vc里找出str，返回索引
int find_word_in_vc(string &str, vector<string> &string_vc)
{
	vector<string>::iterator it = find(string_vc.begin(), string_vc.end(), str);
	return it - string_vc.begin();
}

//把矩阵结果算出并写入矩阵 
void trainCase::write_matrix(const string &filename)
{
	//打开要读写的文件 
	ifstream fin(filename.c_str());

	string s;//用于记录读取到的每一行的字符串 

	int row_num = 0;//标记当前处理的行号

	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		string words, word;
		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		getline(ss, emotions[row_num], '\n');//获得感情
		ss.clear();
		ss.str(words.c_str());
		while (ss >> word)//获得当前行的记录 
		{
			//在 string_vc里找出outstr，返回索引
			int location = find_word_in_vc(word, wordsVC);
			onehotMatrix[row_num][location] = true;
		}
		row_num++;
	}
}

class testCase
{
public:
	bool* onehot;//写出test文章的onehot向量
	int wordNum;//训练集的字典大小
	int newWord;//测试集中没在训练集中出现的词的数
	multimap<double, int> distPairs;//记录测试数据和所有训练数据的距离，first是距离，second是训练数据的索引

	testCase(int w);
	~testCase();
	void getOnehot(const string &words, const vector<string> &vc);//得到onehot向量
	double distCnt(bool *trainRow, int dictSize, double distType);//返回测试数据和摸一个训练数据之间的距离
	void setDistPairs(int index, double dist);//向distPairs中添加一条距离信息
	string classify(int k, string* emotions);//通过记录的信息和训练集的感情数据对测试数据分类，返回感情
	void printPairs();
	void printOnehot();
};

testCase::testCase(int w)
{
	newWord = 0;
	wordNum = w;
	onehot = new bool[wordNum];
	for (int i = 0; i < wordNum; i++)
	{
		onehot[i] = false;
	}
}

testCase::~testCase()
{
	delete[]onehot;
}

//得到onehot向量
void testCase::getOnehot(const string &words, const vector<string> &vc)
{
	string currWord;
	istringstream iss(words);
	while (iss >> currWord)
	{
		if (find(vc.begin(), vc.end(), currWord) != vc.end())
		{
			int location = find(vc.begin(), vc.end(), currWord) - vc.begin();
			onehot[location] = 1;
		}
		else
		{
			newWord++;
		}
	}
}

//返回测试数据和摸一个训练数据之间的距离
double testCase::distCnt(bool *trainRow, int dictSize, double distType)
{
	double dist=0;
	for (int i = 0; i < dictSize; i++)
	{
		dist += pow(trainRow[i] - onehot[i], distType);
	}
	for (int i = 0; i < newWord; i++)
	{
		dist += pow(0 - 1, distType);
	}

	dist = pow(dist, 1 / distType);
	return dist;
}

//向distPairs中添加一条距离信息
void testCase::setDistPairs(int index, double dist)
{
	distPairs.insert(pair<double, int>(dist, index));
}

//通过记录的信息和训练集的感情数据对测试数据分类，返回感情
string testCase::classify(int k, string *emotions)
{
	map<string, int> emotionsCnt;//索引为感情
	//计数前k个数据中各个感情的数量
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();
		for (int i = 0; i < k; i++)
		{
			if (emotionsCnt.find(emotions[it->second]) != emotionsCnt.end())
			{
				emotionsCnt[emotions[it->second]]++;
			}
			else
			{
				emotionsCnt[emotions[it->second]] = true;
			}
			it++;
		}
	}

	//找到前k近数据里出现最多的感情
	int most_num = 0;
	string most_emotion = "";
	for (map<string, int>::iterator iter = emotionsCnt.begin(); iter != emotionsCnt.end(); iter++)
	{
		if (iter->second >= most_num)
		{
			most_num = iter->second;
			most_emotion = iter->first;
		}
	}

	return most_emotion;
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
	for (int i = 0; i < wordNum; i++)
	{
		cout << onehot[i] << ' ';
	}
	cout << endl;
}

int main()
{
	//构建训练数据的onehot矩阵
	string trainFilename = "train_set.csv";
	trainCase traincase;

	traincase.get_words(trainFilename);
	traincase.write_matrix(trainFilename);

	//对验证集数据进行分类
	cout << "请输入k值" << endl;
	int k;
	cin >> k;

	string testFilename = "validation_set.csv";
	ifstream fin(testFilename);
	ofstream fout("classify_res.txt");//结果写入的文件
	
	string s;
	int rightCnt = 0;//分类正确的数据数量
	int testCnt = 0;//总的测试数据数量
	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		string words, ansEmotion;
		istringstream iss(s);
		getline(iss, words, ',');
		testCase testcase(traincase.dictSize);
		testcase.getOnehot(words, traincase.wordsVC);
		getline(iss, ansEmotion, '\n');
		for (int i = 0; i < traincase.rowCnt; i++)
		{
			double dist = testcase.distCnt(traincase.onehotMatrix[i], traincase.dictSize, 2);
			testcase.setDistPairs(i, dist);
		}

		string res_emotion = testcase.classify(k, traincase.emotions);
		
		testCnt++;
		if (res_emotion == ansEmotion)
		{
			rightCnt++;
		}
		
		cout << res_emotion << endl;
		fout << res_emotion << endl;
		//system("pause");
	}

	cout << "分类正确率：" << 100.0*rightCnt/testCnt << '%' << endl;
	system("pause");
	return 0;
}