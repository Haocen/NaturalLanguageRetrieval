#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <Windows.h>

//Code by Haocen.xu@gmail.com

using namespace std;

class Term
{
private:
	string term;
	unsigned int TF;

public:
	Term(string term, unsigned int TF)
	{
		this->term = term;
		this->TF = TF;
	}

	const unsigned int getTF() const
	{
		return TF;
	}

	const string getTerm() const
	{
		return term;
	}
};

class DocIndex
{
private:
	vector<Term> terms;
	string doc;
	unsigned int total;

	double getProbability(string term)
	{
		for (auto it = terms.cbegin(); it != terms.cend(); ++it)
		{
			if (it->getTerm() == term)
			{
				return (double(double(it->getTF()) / double(total)));
			}
		}
		return 0;
	}

public:
	DocIndex(string doc)
	{
		this->doc = doc;
		total = 0;
	}

	void putTerm(Term term)
	{
		terms.push_back(term);
		total = total + term.getTF();
	}

	void clear()
	{
		doc = string();
		terms.clear();
		total = 0;
	}

	double getProbability(vector<string>& query_terms, unsigned long total_terms)
	{
		double probability = 1;
		for (auto it = query_terms.cbegin(); it != query_terms.cend(); ++it)
		{
			if (getProbability(*it) != 0)
			{
				probability = probability * getProbability(*it);
			}
			else
			{
				probability = probability / ((double)total_terms);
			}
		}
		return probability;
	}

	string getDoc()
	{
		return doc;
	}
};

class Index
{
private:
	vector<shared_ptr<DocIndex>> index;
	unsigned long total_terms;

public:
	Index()
	{
		total_terms = 0;
	}

	void addTerm(string doc, string term, unsigned int tf)
	{
		bool found = false;
		for (auto it = index.cbegin(); it != index.cend(); ++it)
		{
			if ((*it)->getDoc() == doc)
			{
				found = true;
				(*it)->putTerm(Term(term, tf));
				total_terms = total_terms + tf;
				break;
			}
		}
		if (!found)
		{
			index.push_back(make_shared<DocIndex>(doc));
			index.back()->putTerm(Term(term, tf));
			total_terms = total_terms + tf;
		}
	}

	vector<shared_ptr<DocIndex>>& retrieve()
	{
		return index;
	}

	void clear()
	{
		total_terms = unsigned long();
		index.clear();
	}

	unsigned int size()
	{
		return index.size();
	}

	unsigned long getTotalTerms()
	{
		return total_terms;
	}
};

class doc_result
{
public:
	string doc;
	double probability;

	doc_result(string doc, double probability)
	{
		this->doc = doc;
		this->probability = probability;
	}

	bool operator==(doc_result& obj)
	{
		if (probability == obj.probability)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator>(doc_result& obj)
	{
		if (probability > obj.probability)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator>=(doc_result& obj)
	{
		if ((*this) > obj || (*this) == obj)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator<(doc_result& obj)
	{
		return !((*this) >= obj);
	}

	bool operator<=(doc_result& obj)
	{
		return !((*this) > obj);
	}
};

int main()
{
	string path;
	cout << "Welcome to Natural Language Retrieval." << endl << "Please ensure the index is already built properly." << endl;
	cout << "Path to the index: ";
	getline(cin, path);
	cin.clear();
	if (path.find(".txt") == string::npos)
	{
		path = path + ".txt";
	}
	ifstream check(path);
	if (!check.good())
	{
		check.close();
		cout << "File not found, will try default index file." << endl;
		ifstream default_check("TF_IDF_INDEX.txt");
		if (default_check.good())
		{
			default_check.close();
			path = "TF_IDF_INDEX.txt";
		}
		else
		{
			default_check.close();
			cout << "Cannot find index. Program will exit." << endl;
			system("pause");
			return 0;
		}
	}
	else
	{
		check.close();
	}
	ifstream load;
	load.open(path);
	if (!load.is_open())
	{
		cout << "Index read error." << endl;
		return 0;
	}
	else
	{
		cout << "Loading index..." << endl;
		string line;
		Index index;
		string current_doc;
		while (getline(load, line))
		{
			string doc, term, tf;
			if (!line.empty() && line.find(' ') == string::npos)
			{
				cout << "Following line have format error." << endl << line << endl;
				system("pause");
				continue;
			}
			else
			{
				doc = line.substr(0, line.find(' '));
				line = line.substr(line.find(' ') + 1);
				//system("pause");
				if (!line.empty() && line.find(' ') == string::npos)
				{
					cout << "Following line have format error." << endl << line << endl;
					system("pause");
					continue;
				}
				else
				{
					term = line.substr(0, line.find(' '));
					line = line.substr(line.find(' ') + 1);
					if (!line.empty() && line.find(' ') == string::npos)
					{
						cout << "Following line have format error." << endl << line << endl;
						system("pause");
						continue;
					}
					else
					{
						tf = line.substr(0, line.find(' '));
						//cout << "Term: " << term << " TF: " << tf << " Doc: " << doc << endl;
						//system("pause");
						if (doc.size() != 0 && term.size() != 0 && tf.size() != 0)
						{
							index.addTerm(doc, term, stol(tf));
							//cout << "SUCCESS!" << index.size() << endl;
						}
						else
						{
							/*cout << "Argument missing:" << endl;
							cout << "Term: " << term << " TF: " << tf << " Doc: " << doc << endl;
							system("pause");*/
						}
						//cout << "Term: " << term << " in Doc: " << doc << " is added." << endl;
					}
				}
			}
			if (doc != string() && current_doc != doc)
			{
				cout << "Index for " << doc << " processed." << endl;
				current_doc = doc;
			}
		}
		cout << endl << "Index parsing complete." << endl;
		unsigned long limit = 0;
		while (true)
		{
			string query;
			vector<string> query_terms;
			cin.sync();
			cin.clear();
			cout << "You may input your query now, input nothing to go to menu: ";
			getline(cin, line);
			//cin.ignore(numeric_limits<streamsize>::max(),'\n');
			if (line.empty())
			{
				cout <<endl << "Please choose an action." << endl;
				cout << "1. Go to query mode.(default)" << endl;
				cout << "2. Configure search scheme." << endl;
				cout << "3. Set index searching limit." << endl;
				cout << "0. EXIT." << endl;
				cout << endl << "Input number in front of your choice: ";
				getline(cin, line);
				if (line.empty())
				{
					continue;
				}
				else if (line.front() == '0')
				{
					break;
				}
				else if (line.front() == '2')
				{
					cout << "This function is only available in full version. Please support developer!" << endl;
					system("pause");
					continue;
				}
				else if (line.front() == '3')
				{
					cout << "Set limitation of results displayed, will reset after next query: ";
					while (!(cin >> limit))
					{
						cin.sync();
						cin.clear();
						cout << "Please input positive number only: ";
					}
					continue;
				}
				else
				{
					continue;
				}
			}
			DWORD start = GetTickCount();
			while (line.find(' ') != string::npos)
			{
				query_terms.push_back(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
			}
			if (!line.empty() && line.find(' ') == string::npos)
			{
				query_terms.push_back(line);
			}
			vector<doc_result> result;
			for (auto it = index.retrieve().cbegin(); it != index.retrieve().cend(); ++it)
			{
				//cout << "Doc:" << (*it)->getDoc() << " Prob: " << (*it)->getProbability(query_terms) << endl;
				result.push_back(doc_result((*it)->getDoc(), (*it)->getProbability(query_terms, index.getTotalTerms())));
			}
			sort(result.begin(), result.end());
			reverse(result.begin(), result.end());
			//DWORD end = GetTickCount();
			for (auto it = result.cbegin(); it != result.cend(); ++it)
			{
				if ((*it).probability < double(1/(pow(index.getTotalTerms(), query_terms.size() - 1)*double(double(index.getTotalTerms())/double(index.size())))))
				{
					break;
				}
				cout << endl << "Rank: " << (it - result.cbegin() + 1) << " Probability: " << (*it).probability << endl;
				cout << "Doc: " << (*it).doc << endl;
				if (limit != 0)
				{
					--limit;
					if (limit == 0)
					{
						break;
					}
				}
				//cout << "Limit: " << double(pow(index.getTotalTerms(), query_terms.size() - 1)*double(double(index.getTotalTerms()) / double(index.size()))) << endl;
			}
			limit = 0;
			cout << endl << "====Query completed====" << endl << endl;
			DWORD end = GetTickCount();
			cout << "Query took " << (end - start) << " milliseconds." << endl;
		}
		cout << "Thanks for using." << endl << "Code by Haocen.xu@gmail.com" << endl;
		system("pause");
	}
}