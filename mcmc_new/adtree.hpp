#include <list>
#include <algorithm>

#include "data.hpp"

#ifndef ADTREE_HPP
#define ADTREE_HPP

//int indent = 0;

class ADTree : public DataView {
private:
	struct ADNode {
		int count;
		union {
			ADNode*** varyNodes;
			std::vector<int>* records;
		};
	};

	ADNode* makeADNode(int depth, int i, const DataColumns& data, const std::list<int>& records) {
//		++indent;
//		printf("| | | | | | | | "+16-indent);
//		printf("makeADNode(i = %d): count = %d\n", i, records.size());
		ADNode* adNode = new ADNode();
		adNode->count = records.size();
		if (depth == maxSetSize_) {
			adNode->varyNodes = NULL;
		} else if (adNode->count < minCount_ || depth >= maxDepth_) {
			adNode->records = new std::vector<int>(records.begin(), records.end());
		} else {
			adNode->varyNodes = new ADNode**[nVariables_ - i];
			for (int j = i; j < nVariables_; ++j)
				adNode->varyNodes[nVariables_-1-j] = makeVaryNode(depth, j, data, records);
		}
//		--indent;
		++nAdNodes;
		return adNode;
	}
	
	ADNode** makeVaryNode(int depth, int i, const DataColumns& data, const std::list<int>& records) {
//		++indent;
//		printf("| | | | | | | | |"+17-indent);
//		printf("makeVaryNode(i = %d): arity = %d\n", i, arities_[i]);
		ADNode** varyNode = new ADNode*[arities_[i]];
		std::vector<std::list<int> > childRecords(arities_[i]);
		for (std::list<int>::const_iterator it = records.begin(); it != records.end(); ++it) {
			Datum val = data(i, *it);
			childRecords[val].push_back(*it);
		}
		for (int val = 0; val < arities_[i]; ++val) {
			if (childRecords[val].empty())
				varyNode[val] = NULL;
			else
				varyNode[val] = makeADNode(depth+1, i+1, data, childRecords[val]);
		}
//		--indent;
		return varyNode;
	}
	
	void freeADNode(ADNode* adNode, int depth, int i) {
		//printf("freeADNode(i = %d)\n", i);
		if (depth == maxSetSize_) {
			// do nothing
		} else if (adNode->count < minCount_ || depth >= maxDepth_) {
			delete adNode->records;
		} else {
			for (int j = i; j < nVariables_; ++j)
				freeVaryNode(adNode->varyNodes[nVariables_-1-j], depth, j);
			delete[] adNode->varyNodes;
		}
		delete adNode;
	}
	
	void freeVaryNode(ADNode** varyNode, int depth, int i) {
		//printf("freeVaryNode(i = %d)\n", i);
		for (int val = 0; val < arities_[i]; ++val) {
			if (varyNode[val] != NULL)
				freeADNode(varyNode[val], depth+1, i+1);
		}
		delete[] varyNode;
	}
	
	void fillCounts(const ADNode* adNode, const std::vector<int>& vars,
			int i, const std::vector<int>& cumArities, int* counts) const {
		if (i >= vars.size()) {
			*counts = adNode->count;
//				printf(" set %p <- %d\n", counts, adNode->count);
			return;
		}
		assert(vars[i] >= 0 && vars[i] < nVariables_);
//		printf("var = %d, firstVar = %d\n", vars[i], firstVar);
		if (adNode->count < minCount_ || i >= maxDepth_) {
			for (std::vector<int>::iterator r = adNode->records->begin(); r != adNode->records->end(); ++r) {
				int index = 0;
				for (int j = i; j < vars.size(); ++j)
					index += (*data_)(vars[j], *r) * cumArities[j];
				++counts[index];
			}
		} else {
//			printf("%d: %d\n", i, vars[i]);
			ADNode** varyNode = adNode->varyNodes[nVariables_ - 1 - vars[i]];
			for (int val = 0; val < arities_[vars[i]]; ++val) {
				if (varyNode[val]) {
					fillCounts(varyNode[val], vars, i+1, cumArities, counts+val*cumArities[i]);
				} /*else {
					printf(" set zero counts %d times\n", cumArities[i]);
					for (int j = 0; j < cumArities[i]; ++j)
						counts[val*cumArities[i] + j] = 0;
				}*/
			}
		}
	}
	
	struct Cmp {
		const std::vector<int>& vars_;
		Cmp(const std::vector<int>& vars) : vars_(vars) {}
		bool operator()(int i, int j) {return vars_[i] < vars_[j];}
	};
	
	ADNode* root_;
	int nVariables_;
	int* arities_;
	
	const DataColumns* data_;
	int minCount_;
	int maxDepth_;
	int maxSetSize_;
	
	int nAdNodes;
	
public:
	
	ADTree(const DataColumns& data, int minCount = 0, int maxDepth = 0, int maxSetSize = 0) {
		nVariables_ = data.getNumVariables();
		minCount_ = minCount;
		maxDepth_ = (maxDepth <= 0 ? nVariables_ : maxDepth);
		maxSetSize_ = (maxSetSize <= 0 ? nVariables_ : maxSetSize);
		data_ = NULL;
		if (minCount_ > 0 || maxDepth_ < nVariables_)
			data_ = &data;
		arities_ = new int[nVariables_];
		for (int i = 0; i < nVariables_; ++i)
			arities_[i] = data.getArity(i);
		std::list<int> records;
		for (int i = 0; i < data.getNumSamples(); ++i)
			records.push_back(i);
		nAdNodes = 0;
		root_ = makeADNode(0, 0, data, records);
	}
	
	~ADTree() {
		freeADNode(root_, 0, 0);
//		if (data_)
//			delete data_;
		delete[] arities_;
	}

	int getNumSamples() const {
		return root_->count;
	}
	
	int getNumVariables() const {
		return nVariables_;
	}
	
	int getArity(int i) const {
		return arities_[i];
	}
	
	int* getCounts(const std::vector<int>& vars) const {
		int n = vars.size();
		assert(n <= maxSetSize_);
		
		// get the order of sorted variables
		std::vector<int> order(n);
		for (int i = 0; i < n; ++i)
			order[i] = i;
		Cmp cmp(vars);
		sort(order.begin(), order.end(), cmp);
		
		// compute cumulative arities
		std::vector<int> cumArities(n);
		cumArities[n-1] = 1;
		for (int i = n-1; i > 0; --i)
			cumArities[i-1] = cumArities[i] * arities_[vars[i]];
		int nValues = cumArities[0] * arities_[vars[0]];
		
		// get the sorted variables and arities
		std::vector<int> sortedVars(n);
		std::vector<int> sortedCumArities(n);
		for (int i = 0; i < n; ++i) {
			sortedVars[i] = vars[order[i]];
			sortedCumArities[i] = cumArities[order[i]];
			assert(i == 0 || sortedVars[i-1] < sortedVars[i]);
		}

//		printf("getCounts( ");
//		for (int i = 0; i < n; ++i)
//			printf("%d ", sortedVars[i]);
//		printf(")\n");
		
		
		// create the count array and initialize to zero
		int* counts = new int[nValues];
		for (int i = 0; i < nValues; ++i)
			counts[i] = 0;
		
//		printf("nValues = %d\n", nValues);
		
		// fill the array recursively
		fillCounts(root_, sortedVars, 0, sortedCumArities, counts);
		
		return counts;
	}
	
	int getNumADNodes() const {
		return nAdNodes;
	}
	
};

#endif

