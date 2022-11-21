#include "pch_ofApp.h"

using namespace std;

//----------
Exception::Exception(const string& message)
	: message(message)
{

}

//----------
const string &
Exception::what() const
{
	return this->message;
}
