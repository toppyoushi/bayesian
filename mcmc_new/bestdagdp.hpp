#include <list>
//#include <algorithm>

#include "stdlib.h"
#include "common.hpp"
#include "lognum.hpp"
#include "data.hpp"
#include "scores.hpp"

#ifndef BESTDAGDP_HPP
#define BESTDAGDP_HPP
using std::list;
typedef Lognum<double> Real;

template <class T>
class ParentsetMap
{
public:
	const int nNodes;

private:
	T **data_;

	ParentsetMap(const ParentsetMap &);			   // disable copying
	ParentsetMap &operator=(const ParentsetMap &); // disable copying

public:
	ParentsetMap(int n) : nNodes(n)
	{
		size_t nSubsets = 1 << n;
		data_ = new T *[nNodes];
		for (int i = 0; i < nNodes; ++i)
		{
			data_[i] = new T[nSubsets];
		}
	}

	~ParentsetMap()
	{
		for (int i = 0; i < nNodes; ++i)
			delete[] data_[i];
		delete[] data_;
	}

	T &operator()(size_t i, const StackSubset &subset)
	{
		assert(0 <= i && i < nNodes);
		size_t j = getParentsetIndex(subset);
		return data_[i][j];
	}

	T operator()(size_t i, const StackSubset &subset) const
	{
		assert(0 <= i && i < nNodes);
		size_t j = getParentsetIndex(subset);
		return data_[i][j];
	}

	T &operator()(size_t i, size_t j)
	{
		assert(0 <= i && i < nNodes);
		assert(0 <= j && j < (1 << nNodes));
		return data_[i][j];
	}

	T operator()(size_t i, size_t j) const
	{
		assert(0 <= i && i < nNodes);
		assert(0 <= j && j < (1 << nNodes));
		return data_[i][j];
	}

	size_t getParentsetIndex(const StackSubset &subset) const
	{
		assert(subset.size() <= nNodes);
		size_t index = 0;
		for (int i = 0; i < subset.size(); ++i)
			index |= (1 << subset[i]);
		return index;
	}
};

void computeScores(const DataView &dataView, int maxParents, const ScoreFun *scoreFun, ParentsetMap<Real> &scores)
{
	int n = dataView.getNumVariables();
	StackSubset parents(n);
	// printf("  compute scores\n");
	for (int i = 0; i < n; ++i)
	{
		int node = i;
		for (size_t pa = 0; pa < ((size_t)1 << n); ++pa)
		{
			parents.clear();
			for (int j = 0; j < n; ++j)
				if (pa & ((size_t)1 << j))
					parents.push(j);
			if (parents.contains(node))
				continue;
			double logScore = (parents.size() > maxParents) ? -1.0 / 0.0 : computeScore(&dataView, parents, node, scoreFun);
			// printf("%g ", logScore);
			Lognum<double> tmp;
			tmp.setLog(logScore);
			scores(i, pa) = to<Real>(tmp);
		}
	}
}

void findBestParents(int n, const ParentsetMap<Real> &scores, ParentsetMap<size_t> &bestPa)
{
	for (int i = 0; i < n; ++i)
	{
		for (size_t ppa = 0; ppa < ((size_t)1 << n); ++ppa)
		{
			size_t pa = ppa;
			for (int j = 0; j < n; ++j)
			{
				size_t subpa = ppa & ~((size_t)1 << j);
				if (j != i && ppa != subpa && scores(i, pa) < scores(i, bestPa(i, subpa)))
				{
					pa = bestPa(i, subpa);
				}
			}
			bestPa(i, ppa) = pa;
		}
	}
}

void findOrder(int n, const ParentsetMap<Real> &scores, const ParentsetMap<size_t> &bestPa, list<int> &order)
{
	Real *bestScore = new Real[1 << n];
	int *bestSink = new int[1 << n];

	// dynamic programming
	bestScore[0] = 1;
	for (size_t ss = 1; ss < ((size_t)1 << n); ++ss)
	{
		Real score = 0;
		int sink = -1;
		for (int i = 0; i < n; ++i)
		{
			size_t sss = ss & ~((size_t)1 << i);
			Real newScore = bestScore[sss] * scores(i, bestPa(i, sss));
			if (ss != sss && newScore > score)
			{
				score = newScore;
				sink = i;
			}
		}
		bestScore[ss] = score;
		bestSink[ss] = sink;
	}

	// backtracking
	size_t ss = ((size_t)1 << n) - 1;
	while (ss) {
		int i = bestSink[ss];
		order.push_front(i);
		ss &= ~((size_t)1 << i);
		// size_t pa = bestPa(i, ss);
		// for (int j = 0; j < n; ++j)
		// 	if (pa & ((size_t)1 << j))
		// 		printf("%d ",j);
		// printf("\n");
	}

	delete[] bestScore;
	delete[] bestSink;
}

void findBestDAG(const DataView &data, int maxParents, const ScoreFun *scoreFun, list<int> &order)
{

	int n = data.getNumVariables();
	ParentsetMap<Real> scores(n);
	ParentsetMap<size_t> bestPa(n);
	computeScores(data, maxParents, scoreFun, scores);
	findBestParents(n, scores, bestPa);
	findOrder(n, scores, bestPa, order);
}

#endif
