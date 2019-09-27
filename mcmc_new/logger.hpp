#include <cstdarg>

#ifndef LOGGER_HPP
#define LOGGER_HPP

class Logger {
private:
	int verbosity_;
	
public:
	void setVerbosity(int v) {
		verbosity_ = v;
	}
	
	void printf(int minVerbosity, const char* fmt, ...) {
		if (verbosity_ < minVerbosity) return;
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
	
	void printfln(int minVerbosity, const char* fmt, ...) {
		if (verbosity_ < minVerbosity) return;
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		std::cerr << std::endl;
		//fprintf(stderr, "\n");
		//std::cerr << std::flush;
	}

	void println(int minVerbosity) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << std::endl;
	}
	
	template <class A>
	void print(int minVerbosity, A a) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a;
	}

	template <class A>
	void println(int minVerbosity, A a) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a << std::endl;
	}

	template <class A, class B>
	void print(int minVerbosity, A a, B b) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a << b;
	}

	template <class A, class B>
	void println(int minVerbosity, A a, B b) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a << b << std::endl;
	}

	template <class A, class B, class C>
	void print(int minVerbosity, A a, B b, C c) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a << b << c;
	}

	template <class A, class B, class C>
	void println(int minVerbosity, A a, B b, C c) {
		if (verbosity_ < minVerbosity) return;
		std::cerr << a << b << c << std::endl;
	}
	
	void flush() {
		std::cerr << std::flush;
	}
};

#endif
