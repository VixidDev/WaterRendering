#pragma once

#include <string>
#include <exception>

// Credit to Markus Billeter (COMP3811 module) for Error class

// Class used for exceptions. Unlike e.g. std::runtime_error, which only
// accepts a "fixed" string, Error provides std::printf()-like formatting.
// Example:
//
//	throw Error( "glGetError() returned %d", glerr );
//
class Error : public std::exception
{
public:
	explicit Error(char const*, ...);

public:
	char const* what() const noexcept override;

private:
	std::string mMsg;
};
