#pragma once
#include <cstdio>
class logger {
public:
	template<typename... Args>
	static void log(unsigned int loglvl, Args ... args) {
		if (loglvl <= mloglvl) {
			std::printf(args ...);
			std::fflush(stdout);
		}
	}
	static void setloglvl(unsigned int inloglvl) {
		inloglvl <= 9 ? mloglvl = inloglvl : mloglvl = 9;
	}
private:
	static unsigned int mloglvl;
};