#include <cstddef>
#include <exception>
#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "common.hpp"

#ifndef DATA_HPP
#define DATA_HPP

typedef unsigned char Datum;

class DataView
{
public:
	virtual ~DataView(){};
	//	virtual int nVariables() const = 0;
	virtual int getNumSamples() const = 0;
	virtual int getNumVariables() const = 0;
	virtual int getArity(int i) const = 0;
	virtual int *getCounts(const std::vector<int> &vars) const = 0;
};

class Data : public DataView
{
private:
	Data(const Data &);			   // disable copying
	Data &operator=(const Data &); // disable copying

	void computeArities()
	{
		assert(arities == NULL);
		arities = (int *)malloc(sizeof(int) * nVariables);
		for (int v = 0; v < nVariables; ++v)
		{
			int arity = 0;
			for (int i = 0; i < nSamples; ++i)
			{
				if ((*this)(v, i) >= arity)
					arity = (*this)(v, i) + 1;
			}
			arities[v] = arity;
		}
	}

public:
	int nVariables;
	int nSamples;
	Datum *data;
	int *arities;

	Data()
	{
		nVariables = 0;
		nSamples = 0;
		data = NULL;
		arities = NULL;
	}

	void clear()
	{
		nVariables = 0;
		nSamples = 0;
		if (data)
			free(data);
		data = NULL;
		if (arities)
			free(arities);
		arities = NULL;
	}

	~Data()
	{
		clear();
	}

	int getNumSamples() const
	{
		return nSamples;
	}

	int getNumVariables() const
	{
		return nVariables;
	}

	Datum &operator()(int v, int i)
	{
		//return data[v * nSamples + i];
		return data[i * nVariables + v];
	}

	Datum operator()(int v, int i) const
	{
		//return data[v * nSamples + i];
		return data[i * nVariables + v];
	}

	void read(std::istream &file, int nVars, int nSamps)
	{
		nVariables = nVars;
		nSamples = nSamps;
		data = (Datum *)malloc(sizeof(Datum) * nVariables * nSamples);

		// load the data
		std::string row;
		for (int i = 0; i < nSamples; ++i)
		{
			if (file.eof())
				throw Exception("Not enough rows (%d while %d expected).") % i % nSamples;
			getline(file, row);
			std::istringstream rowStream(row);
			for (int v = 0; v < nVariables; ++v)
			{
				int tmp;
				//file >> tmp;
				rowStream >> tmp;
				if (rowStream.fail())
					throw Exception("Could not read value on row %d column %d") % (i + 1) % (v + 1);
				(*this)(v, i) = (Datum)tmp;
			}
		}

		computeArities();
	}

	void read(std::istream &file)
	{
		nVariables = 1;
		nSamples = 1;
		data = (Datum *)malloc(sizeof(Datum) * nVariables * nSamples);

		std::string row;

		// read the first row (and get the number of variables)
		getline(file, row);
		std::istringstream rowStream(row);
		int v = 0;
		rowStream >> std::ws;
		while (!rowStream.eof())
		{
			if (v >= nVariables)
			{
				nVariables *= 2;
				data = (Datum *)realloc(data, sizeof(Datum) * nVariables * nSamples);
			}
			int tmp;
			rowStream >> tmp;
			if (rowStream.fail())
				throw Exception("Could not read value on row 1 column %d.") % (v + 1);
			data[v] = (Datum)tmp;
			rowStream >> std::ws;
			++v;
		}
		file >> std::ws;
		nVariables = v;
		data = (Datum *)realloc(data, sizeof(Datum) * nVariables * nSamples);

		// load the data rest of the data
		int i = 1;
		while (!file.eof())
		{
			if (i >= nSamples)
			{
				nSamples *= 2;
				data = (Datum *)realloc(data, sizeof(Datum) * nVariables * nSamples);
			}
			getline(file, row);
			std::istringstream rowStream(row);
			for (int v = 0; v < nVariables; ++v)
			{
				int tmp;
				rowStream >> tmp;
				if (rowStream.fail())
					throw Exception("Could not read %dth value on row %d") % (v + 1) % (i + 1);
				data[i * nVariables + v] = (Datum)tmp;
			}
			file >> std::ws;
			++i;
		}
		nSamples = i;
		data = (Datum *)realloc(data, sizeof(Datum) * nVariables * nSamples);

		computeArities();
	}

	int getArity(int v) const
	{
		return arities[v];
	}

	int *getCounts(const std::vector<int> &vars) const
	{
		// create count table and initialize to zero
		int nValues = 1;
		for (size_t i = 0; i < vars.size(); ++i)
			nValues *= getArity(vars[i]);
		int *counts = new int[nValues];
		for (int i = 0; i < nValues; ++i)
			counts[i] = 0;

		// fill counts
		for (int j = 0; j < nSamples; ++j)
		{
			int index = 0;
			for (size_t i = 0; i < vars.size(); ++i)
				index = index * getArity(vars[i]) + (*this)(vars[i], j);
			++counts[index];
		}
		return counts;
	}
};

class DataColumns : public DataView
{
private:
	const Data &data_;
	std::vector<int> variables_;

public:
	DataColumns(const Data &data, const std::vector<int> &vars)
		: data_(data), variables_(vars) {}

	DataColumns(const DataColumns &dataCols)
		: data_(dataCols.data_), variables_(dataCols.variables_) {}

	DataColumns(const Data &data)
		: data_(data), variables_(data.getNumVariables())
	{
		for (size_t i = 0; i < variables_.size(); ++i)
			variables_[i] = i;
	}

	int getNumSamples() const
	{
		return data_.getNumSamples();
	}

	int getNumVariables() const
	{
		return variables_.size();
	}

	int getArity(int v) const
	{
		return data_.getArity(variables_[v]);
	}

	Datum operator()(int v, int i) const
	{
		return data_(variables_[v], i);
	}

	int *getCounts(const std::vector<int> &vars) const
	{
		std::vector<int> transVars(vars.size());
		for (size_t i = 0; i < vars.size(); ++i)
			transVars[i] = variables_[vars[i]];
		return data_.getCounts(transVars);
	}
}; /**/

#endif
