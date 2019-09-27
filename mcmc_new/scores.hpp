#include <cmath>

#include "data.hpp"
#include "stacksubset.hpp"


#ifndef SCORES_HPP
#define SCORES_HPP


class ScoreFun {
public:
	virtual double compute(int nValues, int nParentValues, int* counts) const = 0;
	virtual ~ScoreFun() {};
};


/**
 * BDeu score function.
 */
class BDeuScore : public ScoreFun {
private:
	double ess_;
public:
	BDeuScore(double ess) {
		ess_ = ess;
	}
	double compute(int nValues, int nParentValues, int* counts) const {
		double score = 0;
		double pseudocount = ess_ / (nValues * nParentValues);
		double parentPseudocount = ess_ / nParentValues;
		for (int pv = 0; pv < nParentValues; ++pv) {
			int cumCount = 0;
			for (int v = 0; v < nValues; ++v) {
				int c = counts[pv * nValues + v];
				score += lgamma(c + pseudocount) - lgamma(pseudocount);
				cumCount += c;
			}
			score += lgamma(parentPseudocount) - lgamma(cumCount + parentPseudocount);
		}
		return score;
	}
};

/**
 * BDeu score function with precomputed log gammas.
 */
class BDeuScoreCached : public ScoreFun {
private:
	double ess_;
	double* logGammas_;
	int maxCount_;
public:
	BDeuScoreCached(double ess, int maxCount, int maxNumValues) {
		ess_ = ess;
		maxCount_ =  maxCount;
		//printf("ess = %g, maxcount = %d, maxnumvalues = %d\n", ess, maxCount, maxNumValues);
		//exit(1);
		logGammas_ = new double[(maxCount_ + 1) * maxNumValues];
		for (int i = 0; i <= maxCount_; ++i)
			for (int j = 0; j < maxNumValues; ++j)
				logGammas_[i + j * (maxCount_ + 1)] = lgamma(i + ess_ / (j+1));
	}
	
	~BDeuScoreCached() {
		delete[] logGammas_;
	}
	
	double compute(int nValues, int nParentValues, int* counts) const {
		double score = 0;
		int pseudoTermIndex = (nParentValues * nValues - 1) * (maxCount_ + 1);
		int parentPseudoTermIndex = (nParentValues - 1) * (maxCount_ + 1);
		
		for (int pv = 0; pv < nParentValues; ++pv) {
			int cumCount = 0;
			for (int v = 0; v < nValues; ++v) {
				int c = counts[pv * nValues + v];
				score += logGammas_[c + pseudoTermIndex] - logGammas_[pseudoTermIndex];
				cumCount += c;
			}
			score += logGammas_[parentPseudoTermIndex] - logGammas_[cumCount + parentPseudoTermIndex];
		}
		return score;
	}
};


/**
 * K2 score function.
 */
class K2Score : public ScoreFun {
public:
	double compute(int nValues, int nParentValues, int* counts) const {
		double score = 0;
		for (int pv = 0; pv < nParentValues; ++pv) {
			int cumCount = 0;
			for (int v = 0; v < nValues; ++v) {
				int c = counts[pv * nValues + v];
				score += lgamma(c + 1);
				cumCount += c;
			}
			score += lgamma(nValues) - lgamma(cumCount + nValues);
		}
		return score;
	}
};

/**
 * K2 score function with precomputed log gammas.
 */
class K2ScoreCached : public ScoreFun {
private:
	double* logGammas_;
public:
	K2ScoreCached(int maxCount) {
		logGammas_ = new double[maxCount + 1];
		for (int i = 0; i <= maxCount; ++i)
			logGammas_[i] = lgamma(i);
	}
	
	~K2ScoreCached() {
		delete[] logGammas_;
	}
	
	double compute(int nValues, int nParentValues, int* counts) const {
		double score = 0;
		for (int pv = 0; pv < nParentValues; ++pv) {
			int cumCount = 0;
			for (int v = 0; v < nValues; ++v) {
				int c = counts[pv * nValues + v];
				score += logGammas_[c + 1];
				cumCount += c;
			}
			score += logGammas_[nValues] - logGammas_[cumCount + nValues];
		}
		return score;
	}
};



double computeLLScore(int nValues, int nParentValues, int* counts) {
	double score = 0;
	for (int pv = 0; pv < nParentValues; ++pv) {
		int cumCount = 0;
		for (int v = 0; v < nValues; ++v) {
			int c = counts[pv * nValues + v];
			if (c > 0)
				score += c * log(c);
			cumCount += c;
		}
		if (cumCount > 0)
			score -= cumCount * log(cumCount);
	}
	return score;
}

/**
 * LL (log-likelihood) score function.
 */
class LLScore : public ScoreFun {
public:
	double compute(int nValues, int nParentValues, int* counts) const {
		return computeLLScore(nValues, nParentValues, counts);
	}
};


/**
 * MDL (minimum description length) score function.
 */
class MDLScore : public ScoreFun {
private:
	int nSamples_;
public:
	MDLScore(int nSamples) {
		nSamples_ = nSamples;
	}
	double compute(int nValues, int nParentValues, int* counts) const {
		return computeLLScore(nValues, nParentValues, counts) - .5 * log(nSamples_) * (nValues - 1) * nParentValues;
	}
};


/**
 * AIC (Akaike Information Criterion) score function.
 */
class AICScore : public ScoreFun {
public:
	double compute(int nValues, int nParentValues, int* counts) const {
		return computeLLScore(nValues, nParentValues, counts) - (nValues - 1) * nParentValues;
	}
};


/*double* logGammas = NULL;
void computeLogGammas(int n) {
	logGammas = new double[n + 1];
	for (int i = 0; i <= n; ++i)
		logGammas[i] = lgamma(i);
}

void freeLogGammas() {
	assert(logGammas != NULL);
	delete[] logGammas;
}

double BDELogScore(int nValues, int nParentValues, int* counts, int pseudocount) {
	double score = 0;
	for (int pv = 0; pv < nParentValues; ++pv) {
		int cumCount = 0;
		for (int v = 0; v < nValues; ++v) {
			int c = counts[pv * nValues + v];
			//score += lgamma(c + pseudocount) - lgamma(pseudocount);
			score += logGammas[c + pseudocount] - logGammas[pseudocount];
			cumCount += c;
		}
		//score += lgamma(nValues * pseudocount) - lgamma(cumCount + nValues * pseudocount);
		score += logGammas[nValues * pseudocount] - logGammas[cumCount + nValues * pseudocount];
	}
	return score;
}/**/


/*
class CountTable {
private:
	int* counts_;

public:
	int nParentConfs;
	int nNodeConfs;
	
//	CountTable(int _nParentConfs, int _nNodeConfs)
//			: nParentConfs(_nParentConfs), nNodeConfs(_nNodeConfs) {
//		counts_ = new int[nParentConfs * nNodeConfs];
//	}
	
	CountTable(const DataView& dataView, const StackSubset& parents, int node) {
		// compute numbers of configurations
		nParentConfs = 1;
		for (int i = 0; i < parents.size(); ++i)
			nParentConfs *= dataView.getArity(parents[i]);
		nNodeConfs = dataView.getArity(node);
		
		counts_ = new int[nParentConfs * nNodeConfs];
		// get counts
		std::vector<int> vars(parents.size() + 1);
		for (int i = 0; i < parents.size(); ++i)
			vars[i] = parents[i];
		vars[parents.size()] = node;
		int* counts = dataView->getCounts(vars);
	}
	
	~CountTable() {
		delete[] counts_;
	}
	
//	void setCounts(int* counts) {
//		for (int i = 0; i < nParentConfs * nNodeConfs; ++i)
//			counts_[i] = counts[i];
//	}
	
	int operator()(int pa, int i) const {
		return counts_[pa * nNodeConfs + i];
	}

	int& operator()(int pa, int i) {
		return counts_[pa * nNodeConfs + i];
	}
};/**/



/*double computeScore(const Data& data, const StackSubset& parents, int node, const ScoreFun* scoreFun) {
	int nParentValues = 1;
	for (int i = 0; i < parents.size(); ++i)
		nParentValues *= data.arities[parents[i]];
	int nNodeValues = data.arities[node];
	int* counts = new int[nParentValues * nNodeValues];
	for (int i = 0; i < nParentValues * nNodeValues; ++i)
		counts[i] = 0;
	//memset(counts, 0, nParentValues * nNodeValues * sizeof(int));
	
	for (int j = 0; j < data.nSamples; ++j) {
		int index = 0;
		for (int i = 0; i < parents.size(); ++i)
			index = index * data.arities[parents[i]] + data(parents[i], j);
		index = index * data.arities[node] + data(node, j);
		++counts[index];
	}
	
	//for (int i = 0; i < nParentValues * nNodeValues; ++i)
	//	printf("%d ", counts[index]);
	//printf("\n");
	
	//double score = BDELogScore(nNodeValues, nParentValues, counts, 1);
	double score = scoreFun->compute(nNodeValues, nParentValues, counts);
	delete[] counts;
	return score;
}/**/

/*double computeScore(const ADTree& adTree, const StackSubset& parents, int node, const ScoreFun* scoreFun) {
	int nParentValues = 1;
	for (int i = 0; i < parents.size(); ++i)
		nParentValues *= adTree.getArity(parents[i]);
	int nNodeValues = adTree.getArity(node);
	int nVars = parents.size() + 1;
	std::vector<int> vars(nVars);
	for (int i = 0; i < nVars - 1; ++i)
		vars[i] = parents[i];
	vars[nVars - 1] = node;
	int* counts = adTree.getCounts(vars);
	
//	printf("nnv*npv = %d\n", nNodeValues * nParentValues);
//	printf("%p\n", counts);
//	for (int i = 0; i < nParentValues * nNodeValues; ++i)
//		printf("%d ", counts[i]);
//	printf("\n");
	
	//double score = BDELogScore(nNodeValues, nParentValues, counts, 1);
	double score = scoreFun->compute(nNodeValues, nParentValues, counts);
	delete[] counts;
	return score;
}/**/

double computeScore(const DataView* dataView, const StackSubset& parents, int node, const ScoreFun* scoreFun) {
	// get counts
	// std::cout<<"-------"<<std::endl;
	// std::cout<<"node:"<<node<<std::endl;
	// std::cout<<"parents:";
	// for (int i = 0; i < parents.size(); ++i)
	// 	std::cout<<parents[i]<<" ";
	// std::cout<<std::endl;
	std::vector<int> vars(parents.size() + 1);
	for (int i = 0; i < parents.size(); ++i)
		vars[i] = parents[i];
	vars[parents.size()] = node;
	int* counts = dataView->getCounts(vars);
	// compute score
	int nParentValues = 1;
	for (int i = 0; i < parents.size(); ++i)
		nParentValues *= dataView->getArity(parents[i]);
	int nNodeValues = dataView->getArity(node);
	// std::cout<<nNodeValues<<" "<<nParentValues<<std::endl;
	double score = scoreFun->compute(nNodeValues, nParentValues, counts);
	delete[] counts;
	return score;
}/**/


#endif

