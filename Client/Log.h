#pragma once
#include <iostream>
#include <ostream>
#include <streambuf>

namespace clog {
	extern char endl;

	extern std::wstring logs;

	class FileOut : private std::streambuf, public std::ostream {
	private:
		std::string buffer;
		int overflow(int c) override;
		bool badge_file;
		bool work;
		std::string badge;
		std::wstring file;
	public:
		FileOut(std::string badge, std::wstring file, bool add_badge_to_file = false, bool work = true);
	};

	extern FileOut info;
	extern FileOut warn;
	extern FileOut error;
	extern FileOut debug;
};