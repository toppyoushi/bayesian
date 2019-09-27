#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <time.h>
#include <string.h>
#include "common.hpp"
#include "lognum.hpp"
#include "data.hpp"
#include "stacksubset.hpp"
#include "scores.hpp"
#include "bestdagdp.hpp"
#include "timer.hpp"

using namespace std;

void generateTargets(vector<int> &swap_targets, vector<int> &order, map<int, int> &m, int n)
{
	vector<int> temp;
	random_device rd;
	std::mt19937 gen(rd());
	uniform_int_distribution<> dis(0, n - 1);
	int flag;
	do
	{
		temp.clear();
		flag = 0;
		for (int i = 0; i < swap_targets.size(); ++i)
		{
			temp.push_back(dis(gen));
		}
		for (int i = 0; i < swap_targets.size(); ++i)
		{
			for (int j = 0; j < swap_targets.size(); ++j)
			{
				if ((temp[i] == temp[j]) && (i != j))
				{
					flag = 1;
				}
			}
		}
	} while (flag);
	for (int i = 0; i < swap_targets.size(); ++i)
	{
		pair<int, int> p(order[temp[i]], temp[i]);
		m.insert(p);
	}
	map<int, int>::iterator iter;
	int i;
	for (i = 0, iter = m.begin(); iter != m.end(); ++iter, ++i)
	{
		swap_targets[i] = iter->first;
	}
}
double compute(DataColumns &datacolumns, vector<int> &order, const ScoreFun *scoreFun, int maxParentSize)
{
	int n = order.size();
	double scores = 1;
	StackSubset parents(n);
	maxParentSize = n < maxParentSize ? n : maxParentSize;
	for (int i = 0; i < n; ++i)
	{
		int node = order[i];
		double score = 0.0;
		for (int j = 0; j <= maxParentSize; ++j)
		{
			vector<int> pa(i);
			int predn = i;
			parents.clear();
			if (j > predn)
				break;
			for (int k = 0; k < j; ++k)
				pa[k] = 1;
			bool bFind = false;
			do
			{
				// getchar();
				// cout<<"node: "<<node<<" parents: ";
				for (int k = 0; k < predn; ++k)
				{
					if (pa[k])
					{
						// cout<<order[k]<<" ";
						parents.push(order[k]);
					}
				}
				// cout<<endl;
				bFind = false;
				// score=1;
				score += computeScore(&datacolumns, parents, node, scoreFun);
				// cout <<"score: "<< score << endl;
				parents.clear();
				for (int k = 0; k < predn - 1; ++k)
				{
					if (pa[k] && !pa[k + 1])
					{
						swap(pa[k], pa[k + 1]);
						bFind = true;
						if (!pa[0])
						{
							for (int a = 0, b = 0; a < k; ++a)
							{
								if (pa[a])
								{
									swap(pa[a], pa[b]);
									b++;
								}
							}
						}
						break;
					}
				}
			} while (bFind);
		}
		scores *= score;
	}
	return scores;
}
void findPartialOrder(Data &data, vector<int> &targets, vector<int> &order, const ScoreFun *scoreFun, int localMaxIndegree)
{
	int nVariables = data.nVariables;
	DataColumns dataColumns(data, targets);
	list<int> order_list;
	list<int>::iterator lit;
	findBestDAG(dataColumns, localMaxIndegree, scoreFun, order_list);
	// cout<<"order list:"<<endl;
	for (lit = order_list.begin(); lit != order_list.end(); ++lit)
	{
		// cout<<(*lit)<<" ";
		order.push_back(targets[(*lit)]);
	}
	// cout<<endl;
}
void order2dag(DataView &data, vector<int> &order, const ScoreFun *scoreFun, int maxParentSize, SquareMat<bool> &dag)
{
	int n = data.getNumVariables();
	StackSubset parents(n);
	maxParentSize = n < maxParentSize ? n : maxParentSize;
	StackSubset best_parents(n);
	// ofstream pa_outfile;
	// pa_outfile.open("./parent.dat");
	for (int i = 0; i < n; ++i)
	{
		int node = order[i];
		double best_score = -1.0 / 0.0;
		best_parents.clear();
		for (int j = 0; j <= maxParentSize; ++j)
		{
			vector<int> pa(i);
			int predn = i;
			parents.clear();
			if (j > predn)
				break;
			for (int k = 0; k < j; ++k)
				pa[k] = 1;
			bool bFind = false;
			do
			{
				for (int k = 0; k < predn; ++k)
				{
					// cout<<pa[k]<<" ";
					if (pa[k])
					{
						parents.push(order[k]);
					}
				}
				// cout<<endl;
				// score=1;
				double current_score = computeScore(&data, parents, node, scoreFun);
				// cout<<"current_score:"<<current_score<<" best_score"<<best_score<<endl;
				if (current_score > best_score)
				{
					best_parents.clear();
					best_parents.copyToEnd(parents);
					best_score = current_score;
				}
				// std::cout << score << endl;
				bFind = false;
				parents.clear();
				for (int k = 0; k < predn - 1; ++k)
				{
					if (pa[k] && !pa[k + 1])
					{
						swap(pa[k], pa[k + 1]);
						bFind = true;
						if (!pa[0])
						{
							for (int a = 0, b = 0; a < k; ++a)
							{
								if (pa[a])
								{
									swap(pa[a], pa[b]);
									b++;
								}
							}
						}
						break;
					}
				}
			} while (bFind);
		}
		// cout<<"node:"<<node<<"\tbest_parents:";
		// pa_outfile << node << ": ";
		while (!best_parents.empty())
		{
			int p = best_parents.pop();
			// cout<<p<<" ";
			// pa_outfile << p << " ";
			dag(p, node) = 1;
		}
		// cout<<endl;
		// pa_outfile << endl;
	}
	// if (pa_outfile.is_open())
	// {
	// 	pa_outfile.close();
	// }
}
// bool cmp(const pair<vector<int>, int> &a, const pair<vector<int>, int> &b)
// {
// 	return a.second > b.second;
// }
double c(int n,int m)
{
	int x=n;
	long nomi=1;
	long deno=1;
	while(x>=(n-m-1))
	{
		nomi*=x;
		x--; 
	}
	for(int i=1;i<=m;++i)
	{
		deno*=i;
	}
	cout<<nomi<<" "<<deno<<endl;
	return  (double)deno/(double)nomi;
}
void myMCMC(Data &data, vector<int> &targets, int burn_in = 1000, int maxParentSize = 3, int swap_n = 5)
{
	Timer timer;
	double equivalentSampleSize = 1;
	int n = targets.size();
	vector<int> order(targets);
	ScoreFun *scoreFun = new BDeuScore(equivalentSampleSize);
	random_device rd;
	std::mt19937 gen(rd());
	DataColumns datacolumns(data, targets);
	std::uniform_real_distribution<double> dis(0.0, 1.0);
	ofstream outfile;
	ofstream result_outfile;
	// outfile.open("order.dat", ios::out | ios::trunc);
	int temp = burn_in;
	SquareMat<int> res(n);
	SquareMat<bool> dag(n);
	res.setAll(0);
	int sample_count = 0;
	double x = compute(datacolumns, order, scoreFun, maxParentSize);
	timer.start();
	while (temp--)
	{
		vector<int> new_order = order;
		vector<int> swap_targets(swap_n);
		vector<int> swap_orders;
		map<int, int> m;
		generateTargets(swap_targets, new_order, m, n);
		// std::cout<<"swap targets:"<<endl;
		// for(int i=0;i<swap_targets.size();++i)
		// 	std::cout<<swap_targets[i]<<" ";
		// std::cout<<endl;
		// vector<int> temp;
		// for (int i = 0; i < swap_targets.size(); ++i)
		// {
		// 	temp.push_back(order[swap_targets[i]]);
		// }
		// std::cout<<"temp:"<<endl;
		// for(int i=0;i<temp.size();++i)
		// 	std::cout<<temp[i]<<" ";
		// std::cout<<endl;
		findPartialOrder(data, swap_targets, swap_orders, scoreFun, maxParentSize);
		// std::cout<<"swap orders:"<<endl;
		// for(int i=0;i<swap_orders.size();++i)
		// 	std::cout<<swap_orders[i]<<" ";
		// std::cout<<endl;
		map<int, int>::iterator mit;
		// cout<<"m:"<<endl;
		// for(mit = m.begin();mit!=m.end();++mit)
		// {
		// 	cout<<mit->first<<" "<<mit->second<<endl;
		// }
		for (int i = 0; i < swap_orders.size(); ++i)
		{
			map<int, int>::iterator mit = m.find(swap_targets[i]);
			new_order[mit->second] = swap_orders[i];
		}
		// swap(new_order[a], new_order[b]);
		// std::cout << "order:"<<endl;
		// for (int i = 0; i < n; ++i)
		// 	std::cout << order[i] << ' ';
		// std::cout << endl;
		// std::cout << "new order:"<<endl;
		// for (int i = 0; i < n; ++i)
		// 	std::cout << new_order[i] << ' ';
		// std::cout << endl;

		// timer.start();
		// double x = compute(datacolumns, order, scoreFun, maxParentSize);
		// std::cout << timer.elapsed() << endl;
		// getchar();
		double y = compute(datacolumns, new_order, scoreFun, maxParentSize)*c(n,swap_n);
		// cout << x << " " << y << endl;
		// cout<<"y/x:"<<y/x<<std::endl;
		double alpha = (y / x) < 1.0 ? y / x : 1.0;
		double beta = dis(gen);
		// std::cout << alpha << " " << beta << std::endl;
		if (alpha > beta)
		{
			order = new_order;
			x = y;
		}
		// for (int i = 0; i < n; ++i)
		// {
		// 	outfile << order[i] << ' ';
		// }
		// outfile << endl;
		if (temp % 10 == 0)
		{
			cout<<"temp: "<<temp<<endl;
			sample_count++;
			dag.setAll(false);
			order2dag(data, order, scoreFun, maxParentSize, dag);
			for (int i = 0; i < n; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					if (dag(i, j))
					{
						res(i, j)++;
					}
				}
			}
		}
		// std::cout << "--------------" << endl;
		// getchar();
	}
	// if (outfile.is_open())
	// {
	// 	outfile.close();
	// }
	// cout<<"most_frequent_order:";
	// for(int i=0;i<most_frequent_order.size();++i)
	// {
	// 	cout<<most_frequent_order[i]<<" ";
	// }
	// cout<<endl;
	// cout << "times:" << v4sort[0].second << endl;
	cout << "sample count: " << sample_count << endl;
	cout << "time elapsed: " << timer.elapsed() << endl;
	result_outfile.open("result.dat", ios::out | ios::trunc);
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			result_outfile << i << " --> " << j << " " << setprecision(3) << setiosflags(ios::fixed) << (double)res(i, j) / (double)sample_count << endl;
		}
	}
	if (result_outfile.is_open())
	{
		result_outfile.close();
	}
}
