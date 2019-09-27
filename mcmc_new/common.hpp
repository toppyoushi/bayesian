#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/format.hpp>

#ifndef COMMON_HPP
#define COMMON_HPP

// min and max utility function
template <typename T>
T min(T x, T y) {
	return x < y ? x : y;
}

template <typename T>
T max(T x, T y) {
	return x > y ? x : y;
}


// random number generators
typedef boost::mt19937 Rng;
Rng rng;

double randu() {
	boost::uniform_01<double> dist;
	return dist(rng);
}
//boost::uniform_01<Rng&> randu(rng);

int randuint(int ceiling) {
	boost::uniform_int<> dist(0, ceiling - 1);
	return dist(rng);
	//boost::variate_generator<Rng&, boost::uniform_int<> > gen(rng, dist);
	//return gen();
}

/*double randu() {
	return rand() / (double) RAND_MAX;
}

int randuint(int ceiling) {
	return rand() % ceiling;
}/**/


using boost::format;
using std::string;

class Exception {
private:
	format msg_;
	
public:
	Exception(const string& msg) : msg_(msg) {
	}
	
	Exception(const format& msg) : msg_(msg) {
	}
	
	template <typename T>
	Exception operator%(const T& x) {
		return Exception(msg_ % x);
	}
	
	const char* what() {
		return str(msg_).c_str();
	}
};


template <class T>
class SquareMat {
private:
	int nNodes_;
	T* data_;
	
public:
	SquareMat(int nNodes) {
		nNodes_ = nNodes;
		data_ = new T[nNodes * nNodes];
	}

	SquareMat(const SquareMat& other) {
		nNodes_ = other.nNodes_;
		data_ = new T[nNodes_ * nNodes_];
		(*this) = other;
	}
	
	~SquareMat() {
		delete[] data_;
	}

	SquareMat& operator=(const SquareMat& other) {
		assert(other.nNodes_ == nNodes_);
		for (int i = 0; i < nNodes_ * nNodes_; ++i)
			data_[i] = other.data_[i];
	}
	
	int getNumNodes() const {
		return nNodes_;
	}
	
	void setAll(T value) {
		for (int i = 0; i < nNodes_ * nNodes_; ++i)
			data_[i] = value;
	}
	
	T& operator() (int i, int j) {
		return data_[i + j * nNodes_];
	}

	T operator() (int i, int j) const {
		return data_[i + j * nNodes_];
	}
};




#endif


