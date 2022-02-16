#pragma once

#include <string>
#include <exception>
#include <source_location>

namespace Minerva
{
	class MinervaException : public std::exception
	{
	public:

		enum class MINERVA_EXCEPTION
		{
			DEFAULT_ERROR = 0
		};

		MinervaException(int _errorCode, const std::string& _description);

		MinervaException();

		~MinervaException();

		virtual int code() const;

		virtual const char* what() const throw();

	private:
		int m_iErrCode { 0 };
		std::string m_sDescription{"Minerva: Exception thrown but no error code or description given."};
	};
}