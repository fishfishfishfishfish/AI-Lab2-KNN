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
	int dictSize;	//���ظ���¼����������������� 
	int rowCnt;		//һ������ѵ���õ����£���������� 
	vector<string> wordsVC;//���ظ���¼���� 
	bool** onehotMatrix;	//onehot���� 
	string* emotions; 		//��¼ÿһ��/ÿһƪ���µĸ������� 

	~trainCase();
	void get_words(const string &filename);	//��ȡwordVC����,ͬʱ�䵱�˹��캯��
	void write_matrix(const string &filename);//��ȡonehot����emotions���� 
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

//�������Ҫ�ĵ������� 
void trainCase::get_words(const string &filename)
{
	rowCnt = 0;
	ifstream fin(filename.c_str());
	string s;
	getline(fin, s);//ȥ��˵�� 
	while (getline(fin, s))
	{
		rowCnt++;
		string words, word;

		istringstream ss(s);
		getline(ss, words, ',');//��õ�������
		ss.str(words.c_str());
		while (ss >> word)
		{
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//��δ���ֹ��ĵ��� 
			{
				wordsVC.push_back(word);//��¼��vector��
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

//�� string_vc���ҳ�str����������
int find_word_in_vc(string &str, vector<string> &string_vc)
{
	vector<string>::iterator it = find(string_vc.begin(), string_vc.end(), str);
	return it - string_vc.begin();
}

//�Ѿ����������д����� 
void trainCase::write_matrix(const string &filename)
{
	//��Ҫ��д���ļ� 
	ifstream fin(filename.c_str());

	string s;//���ڼ�¼��ȡ����ÿһ�е��ַ��� 

	int row_num = 0;//��ǵ�ǰ������к�

	getline(fin, s);//ȥ��˵�� 
	while (getline(fin, s))
	{
		string words, word;
		istringstream ss(s);
		getline(ss, words, ',');//��õ�������
		getline(ss, emotions[row_num], '\n');//��ø���
		ss.clear();
		ss.str(words.c_str());
		while (ss >> word)//��õ�ǰ�еļ�¼ 
		{
			//�� string_vc���ҳ�outstr����������
			int location = find_word_in_vc(word, wordsVC);
			onehotMatrix[row_num][location] = true;
		}
		row_num++;
	}
}

class testCase
{
public:
	bool* onehot;//д��test���µ�onehot����
	int wordNum;//ѵ�������ֵ��С
	int newWord;//���Լ���û��ѵ�����г��ֵĴʵ���
	multimap<double, int> distPairs;//��¼�������ݺ�����ѵ�����ݵľ��룬first�Ǿ��룬second��ѵ�����ݵ�����

	testCase(int w);
	~testCase();
	void getOnehot(const string &words, const vector<string> &vc);//�õ�onehot����
	double distCnt(bool *trainRow, int dictSize, double distType);//���ز������ݺ���һ��ѵ������֮��ľ���
	void setDistPairs(int index, double dist);//��distPairs�����һ��������Ϣ
	string classify(int k, string* emotions);//ͨ����¼����Ϣ��ѵ�����ĸ������ݶԲ������ݷ��࣬���ظ���
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

//�õ�onehot����
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

//���ز������ݺ���һ��ѵ������֮��ľ���
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

//��distPairs�����һ��������Ϣ
void testCase::setDistPairs(int index, double dist)
{
	distPairs.insert(pair<double, int>(dist, index));
}

//ͨ����¼����Ϣ��ѵ�����ĸ������ݶԲ������ݷ��࣬���ظ���
string testCase::classify(int k, string *emotions)
{
	map<string, int> emotionsCnt;//����Ϊ����
	//����ǰk�������и������������
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

	//�ҵ�ǰk��������������ĸ���
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
	//����ѵ�����ݵ�onehot����
	string trainFilename = "train_set.csv";
	trainCase traincase;

	traincase.get_words(trainFilename);
	traincase.write_matrix(trainFilename);

	//����֤�����ݽ��з���
	cout << "������kֵ" << endl;
	int k;
	cin >> k;

	string testFilename = "validation_set.csv";
	ifstream fin(testFilename);
	ofstream fout("classify_res.txt");//���д����ļ�
	
	string s;
	int rightCnt = 0;//������ȷ����������
	int testCnt = 0;//�ܵĲ�����������
	getline(fin, s);//ȥ��˵�� 
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

	cout << "������ȷ�ʣ�" << 100.0*rightCnt/testCnt << '%' << endl;
	system("pause");
	return 0;
}