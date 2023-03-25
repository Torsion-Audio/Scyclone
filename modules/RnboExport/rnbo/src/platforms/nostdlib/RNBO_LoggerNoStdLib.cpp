//
//    RNBO_LoggerNoStdLib.cpp
//    Created: 12 July 2016 2:42:32pm
//    Author:  jb
//
//

#include "RNBO_Common.h"

namespace RNBO {

	Logger consoleInstance;
	LoggerInterface* console = &consoleInstance;

	Logger::Logger()
	: _outputCallback(&Logger::defaultLogOutputFunction)
	{
	}

	Logger::~Logger()
	{
	}

	void Logger::setLoggerOutputCallback(OutputCallback* callback)
	{
		_outputCallback = callback ? callback : defaultLogOutputFunction;
	}

	void Logger::defaultLogOutputFunction(LogLevel level, const char* message)
	{
		const static char* levelStr[] = { "[INFO]", "[WARNING]", "[ERROR]" };
		String formattedMessage = levelStr[level];
		formattedMessage += "\t";
		formattedMessage += message;
		formattedMessage += "\n";
		platform->printMessage(formattedMessage.c_str());
	}

	Logger& Logger::getInstance()
	{
		return consoleInstance;
	}

}
