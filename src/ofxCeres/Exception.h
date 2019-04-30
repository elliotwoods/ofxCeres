#pragma once

#include <exception>
#include <string>

namespace ofxCeres {
	class Exception : public std::exception {
	public:
		Exception(const std::string & errorMessage);
		const char * what() const throw() override;
	protected:
		const std::string errorMessage;
	};
}