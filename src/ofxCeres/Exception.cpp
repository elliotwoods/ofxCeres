#include "pch_ofxCeres.h"
#include "Exception.h"

using namespace std;

namespace ofxCeres {
	//---------
	Exception::Exception(const string & errorMessage)
		: errorMessage(errorMessage) {

	}

	//---------
	const char * Exception::what() const throw() {
		return this->errorMessage.c_str();
	}
}