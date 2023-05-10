#pragma once

#ifndef _BAYES_FILTER_EXCEPTION
#define _BAYES_FILTER_EXCEPTION

#include <exception>


namespace BAYESIAN_FILTER
{
	class FilterException : virtual public std::exception
	{
	public:
		const char* what() const throw()
		{
			return error_description;
		}
	protected:
		FilterException(const char* description)
		{
			error_description = description;
		}
	private:
		const char* error_description;
	};

	class LogicException : virtual public FilterException
	{
	public:
		LogicException(const char* description) : FilterException(description)
		{
		}
	};

	class NumericException : virtual public FilterException
	{
	public:
		NumericException(const char* description) : FilterException(description)
		{
		}
	};
}

#endif 
