// KNNRG.cpp: 定义控制台应用程序的入口点。
//

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

#define ONEHOT 0
#define TERMFREQ 1
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
	return res/size;
}
double Maximum(double* data, int size)
{
	double max = data[0];
	for (int i = 1; i < size; i++)
	{
		if (data[i] > max)
		{
			max = data[i];
		}
	}
	return max;
}
double Minimum(double* data, int size)
{
	double min = data[0];
	for (int i = 1; i < size; i++)
	{
		if (data[i] < min)
		{
			min = data[i];
		}
	}
	return min;
}

class trainRow
{
public:
	int number_of_words;
	int dictSize;
	double* data;
	double* emotion;//anger,disgust,fear,joy,sad,surprise

	trainRow();
	trainRow(int dictSize);
	trainRow(const trainRow& TR);//带指针数组的类一定要有拷贝构造函数
	~trainRow();
};
trainRow::trainRow()
{
	dictSize = 0;
	number_of_words = 0;
	emotion = new double[6];
}
trainRow::trainRow(int dictSize)
{
	this->dictSize = dictSize;
	data = new double[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = 0;
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
	data = new double[TR.dictSize];
	for (int i = 0; i < TR.dictSize; i++)
	{
		data[i] = TR.data[i];
	}
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = TR.emotion[i];
	}
}
trainRow::~trainRow()
{
	delete[] data;
	delete[] emotion;
}

class trainCase
{
public:
	int dictSize;			//不重复记录单词数，矩阵的列数 
	int rowCnt;				//一共多少训练用的文章，矩阵的行数 
	vector<string> wordsVC;	//不重复记录单词 
	vector<trainRow*> matrix;//onehot矩阵 TF矩阵 感情向量

	trainCase();
	trainCase(const string &filename, int matrix_type);
	~trainCase();
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename, int matrix_type);//获取onehot矩阵，emotions数组
	friend void operator<<(ostream& os, const trainCase& TC);
};
trainCase::trainCase()
{
	dictSize = rowCnt = 0;
}
trainCase::trainCase(const string &filename, int matrix_type)
{
	get_words(filename);//Initialize dictSize/rowCnt/wordsVC
	write_matrix(filename, matrix_type);//Initialize matrix
}
trainCase::~trainCase()
{
	for (int i = 0; i < matrix.size(); i++)
	{
		delete matrix[i];
	}
	delete &matrix;
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
		int wordCnt = 0;
		string words, word;
		trainRow* TR = new trainRow();

		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		ss.str(words.c_str());
		while (ss >> word)
		{
			wordCnt++;
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//从未出现过的单词 
			{
				wordsVC.push_back(word);//记录到vector里
			}
		}
		TR->number_of_words = wordCnt;
		matrix.push_back(TR);
	}
	dictSize = wordsVC.size();
}
void trainCase::write_matrix(const string &filename, int matrix_type)
{
	//打开要读的文件 
	ifstream fin(filename);
	int currRow = 0;//当前处理的行号
	string s;//用于记录读取到的每一行的字符串 

	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		matrix[currRow]->dictSize = dictSize;
		matrix[currRow]->data = new double[dictSize];
		for (int i = 0; i < dictSize; i++)
		{
			matrix[currRow]->data[i] = 0;
		}

		double standby = 0;
		if (matrix_type == 0) { standby = 1; }
		else { standby = 1.0 / matrix[currRow]->number_of_words; }

		string words, word;
		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		istringstream Wss(words);
		while (Wss >> word)//获得当前行的记录 
		{
			//在 string_vc里找出outstr，返回索引
			int location = find_word_in_vc(word, wordsVC);
			if (location != -1)
			{
				matrix[currRow]->data[location] = standby;
			}
		}
		for (int i = 0; i < 6; i++)
		{
			string tmpEmotion;
			getline(ss,tmpEmotion, ',');
			matrix[currRow]->emotion[i] = atof(tmpEmotion.c_str());
		}
		currRow++;
	}
}
void operator<<(ostream& os, const trainCase& TC)
{
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < TC.dictSize; j++)
		{
			os << TC.matrix[i]->data[j] << ' ';
		}
		os << endl;
		for (int j = 0; j < 6; j++)
		{
			os << TC.matrix[i]->emotion[j] << ' ';
		}
		os << endl;
	}
}

class testCase
{
public:
	int number_of_words;//数据中含有的单词数
	double* data;//写出test文章的onehot或TF向量
	int dictSize;//训练集的字典大小
	vector<double> newWord;//记录测试集里出现的不在字典里的词的数据
	multimap<double, int> distPairs;//记录测试数据和所有训练数据的距离，first是距离，second是训练数据的索引

	testCase();
	testCase(const string &words, int w);
	testCase(const testCase& TC);
	~testCase();
	void getVectors(const string &words, vector<string> &vc, int type);//得到onehot向量
	double distCnt(double *trainRow, int dictSize, double distType);//返回测试数据和摸一个训练数据之间的距离
	void setDistPairs(int index, double dist);//向distPairs中添加一条距离信息
	double* distNormalize1(int k);//standard score
	double* distNormalize2(int k);//feature scaling
	double* cosNormalize(int k);//余弦相似度的归一化

	//通过记录的信息和训练集的感情数据对测试数据分类，返回感情。0是standard score；1是feature scaling；-1是cosNormalize
	double* RG(int k, trainCase& TC, int norm_type);
	void printPairs();
	void printVectors();
};
testCase::testCase()
{
	number_of_words = dictSize = 0;
	data = new double[dictSize];
}
testCase::testCase(const string &words, int w)
{
	dictSize = w;
	data = new double[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = 0;
	}
	number_of_words = 0;
	string tempword;
	istringstream iss(words);
	while (iss >> tempword)
	{
		number_of_words++;
	}
}
testCase::testCase(const testCase& TC)
{
	number_of_words = TC.number_of_words;
	dictSize = TC.dictSize;
	data = new double[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = TC.data[i];
	}
	newWord = TC.newWord;
	distPairs = TC.distPairs;
}
testCase::~testCase()
{
	delete[]data;
}

//得到onehot，TF向量
void testCase::getVectors(const string &words, vector<string> &vc, int type)
{
	string currWord;
	istringstream iss(words);
	while (iss >> currWord)
	{
		double standby;
		if (type == ONEHOT) { standby = 1; }
		else if (type == TERMFREQ) { standby = 1.0 / number_of_words; }

		int location = find_word_in_vc(currWord, vc);
		if (location != -1)
		{
			data[location] = standby;
		}
		else
		{
			newWord.push_back(standby);
		}
	}
}

//返回测试数据和一个训练数据之间的距离,-1为余弦相似度
double testCase::distCnt(double *trainRow, int dictSize, double distType)
{
	double dist = 0;
	if (distType != -1)
	{		
		for (int i = 0; i < dictSize; i++)
		{
			dist += pow(trainRow[i] - data[i], distType);
		}
		for (int i = 0; i < newWord.size(); i++)
		{
			dist += pow(0 - newWord[i], distType);
		}

		dist = pow(dist, 1.0 / distType);
	}
	else
	{
		double absTrain = 0;
		double absTest = 0;
		for (int i = 0; i < dictSize; i++)
		{
			absTrain += pow(trainRow[i], 2);
			absTest += pow(data[i], 2);
			dist += trainRow[i] * data[i];
		}
		for (int i = 0; i < newWord.size(); i++)
		{
			absTest += pow(newWord[i], 2);
		}
		dist = dist / (sqrt(absTest)*sqrt(absTrain));//越小越不相似，为了和其他距离统一，之后不用取倒数
		dist = -dist;
	}
	return dist;
}

//向distPairs中添加一条距离信息
void testCase::setDistPairs(int index, double dist)
{
	distPairs.insert(pair<double, int>(dist, index));
}

double* testCase::distNormalize1(int k)//用后记得delete
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

double* testCase::distNormalize2(int k)//用后记得delete
{
	//Feature scaling
	double* res = new double[k];
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();

		for (int i = 0; i < k; i++)
		{
			res[i] = 1.0 / (1 + it->first);
			it++;
		}

		double min = Minimum(res, k);
		double max = Maximum(res, k);

		for (int i = 0; i < k; i++)
		{
			res[i] = (res[i] - min) / (max - min);
		}
	}
	return res;
}
double* testCase::cosNormalize(int k)//用后记得delete
{
	//归一化余弦相似度
	double* res = new double[k];
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();

		for (int i = 0; i < k; i++)
		{
			res[i] = 0.5-0.5*(it->first);
			it++;
		}
	}
	return res;
}
//通过记录的信息和训练集的感情数据对测试数据分类，返回感情
double* testCase::RG(int k, trainCase& TC, int norm_type)
{
	double* testEmo = new double[6];//记录测试数据的情绪数据
	for (int i = 0; i < 6; i++)
	{
		testEmo[i] = 0;
	}
	double* weight = new double[k];
	switch (norm_type)
	{
		case -1: weight = cosNormalize(k); break;
		case 0: weight = distNormalize1(k); break;
		case 1: weight = distNormalize2(k); break;
		default: weight = distNormalize1(k); break;
	}
	if (k <= distPairs.size())
	{
		multimap<double, int>::iterator it = distPairs.begin();

		for (int i = 0; i < k; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				testEmo[j] += TC.matrix[it->second]->emotion[j]*weight[i];
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
void testCase::printVectors()
{
	for (int i = 0; i < dictSize; i++)
	{
		cout << data[i] << ' ';
	}
	cout << endl;
}

void validHandle(ostream &os, const string &infilename, trainCase &traincase, int k, int matrix_type, int dist_type, int norm_type)
{
	ifstream fin(infilename);
	string s;
	//int itemp = 0;//debug
	int testCnt = 0;//总的测试数据数量
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		//itemp++;//debug
		string words;
		double* ansEmotion;
		istringstream iss1(s);
		getline(iss1, words, ',');

		testCase testcase(words, traincase.dictSize);
		testcase.getVectors(words, traincase.wordsVC, matrix_type);

		for (int i = 0; i < traincase.rowCnt; i++)
		{
			 double dist = testcase.distCnt(traincase.matrix[i]->data, traincase.dictSize, dist_type);
			testcase.setDistPairs(i, dist);
		}

		//if (itemp > 194 && itemp < 199)
		//{
		//	testcase.printPairs();//debug
		//	system("pause");
		//}
		ansEmotion = testcase.RG(k, traincase, norm_type);
		for (int i = 0; i < 6; i++)
		{
			os << ansEmotion[i] << '\t';
		}
		os << endl;
		delete[] ansEmotion;
		//testcase.printPairs();
		//system("pause");
	}
}
void testHandle(ostream &os, const string &infilename, trainCase &traincase, int k, int matrix_type, int dist_type, int norm_type)
{
	ifstream fin(infilename);
	string s;
	int RowCnt = 1;
	int testCnt = 0;//总的测试数据数量
	getline(fin, s);//去掉说明
	os << "textid,anger,disgust,fear,joy,sad,surprise" << endl;//输出说明
	while (getline(fin, s))
	{
		string words;
		double* ansEmotion;
		istringstream iss1(s);
		getline(iss1, words, ',');//去掉数字
		getline(iss1, words, ',');

		testCase testcase(words,traincase.dictSize);
		testcase.getVectors(words, traincase.wordsVC, matrix_type);

		for (int i = 0; i < traincase.rowCnt; i++)
		{
			double dist = testcase.distCnt(traincase.matrix[i]->data, traincase.dictSize, dist_type);
			testcase.setDistPairs(i, dist);
		}

		ansEmotion = testcase.RG(k, traincase, norm_type);

		os << RowCnt;
		for (int i = 0; i < 6; i++)
		{
			os << ',' << ansEmotion[i];
		}
		os << endl;
		delete[] ansEmotion;
		//system("pause");
	}
}

int main()
{
	//user guide
	int start, end, matrix_type, dist_type, norm_type = 0;
	cout << "what kind of matrix? 0 for onehot, 1 for TF." << endl;
	cin >> matrix_type;
	cout << "what kind of distant? -1 for cos similiarity." << endl;
	cin >> dist_type;
	if (dist_type != -1)
	{
		cout << "what kind of normalization? 0 for stand score, 1 for feature scaling." << endl;
		cin >> norm_type;
	}
	else
	{
		norm_type = -1;
	}
	cout << "start?" << endl;
	cin >> start;
	cout << "end?" << endl;
	cin >> end;

	trainCase TC("train_set.csv", matrix_type);
	string validfile = "validation_set.csv";
	string testfile = "test_set.csv";
	//ofstream fout("debug_train_matrix.txt");//debug
	//fout << TC;//debug
	for (int k = start; k < end; k++)
	{
		string resfile;
		/*计算验证*/
		stringstream ss;
		ss << "fv";
		ss << k;
		ss << ".txt";
		ss >> resfile;
		ofstream fout(resfile);
		validHandle(fout, validfile, TC, k, matrix_type, dist_type, norm_type);
		/*计算测试*/
		/*resfile = "15352049_KNN_regression.csv";
		ofstream fout(resfile);
		testHandle(fout, testfile, TC, k);*/
		cout << "k=" << k << "finished" << endl;
	}

	system("pause");
	return 0;
}