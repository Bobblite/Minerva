#include "Minerva_Exception.h"

#include <sstream>

namespace Minerva
{
	MinervaException::MinervaException(int _errorCode, const std::string& _description) :
		m_iErrCode{ _errorCode }, m_sDescription{ _description } {}

	MinervaException::MinervaException() {};

	MinervaException::~MinervaException() {}

	int MinervaException::code() const
	{
		return m_iErrCode;
	}

	const char* MinervaException::what() const throw()
	{
		std::source_location loc{ std::source_location::current() };
		std::ostringstream ss;
		//std::string s1{ "Minerva Exception thrown!\n" };
		ss << "Minerva Exception thrown!\n";
		/*ss << "File: " << loc.file_name() << "\n";
		ss << "Line: " << loc.line() << "\n";
		ss << "Function: " << loc.function_name() << "\n";*/
		//todo code to string
		std::string res = ss.str();
		const char* resC = res.c_str();

		std::string testStr{ loc.file_name() };
		return loc.file_name();
	}
}