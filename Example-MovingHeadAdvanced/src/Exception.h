#pragma once

#include <string>

class Exception {
public:
	Exception(const std::string& message);
	const std::string& what() const;
protected:
	const std::string message;
};

#define CATCH_TO_ALERT \
catch(const Exception& e) { \
	ofSystemAlertDialog(e.what()); \
}