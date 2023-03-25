//
//  RNBO_Logger.cpp
//  Created: 25 Jan 2016 4:07:31pm
//  Author:  stb
//
//

#include "RNBO_Logger.h"
#include "RNBO_DynamicSymbolRegistry.h"

namespace RNBO {

	static Logger s_logger_instance;
	LoggerInterface* console = &s_logger_instance;
	static DynamicSymbolRegistrar ConsoleRegisteredSymbol("console" , reinterpret_cast<void*>(&console));

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
		Platform::get()->printMessage(formattedMessage.c_str());
	}

	Logger& Logger::getInstance()
	{
		return s_logger_instance;
	}

}  // namespace RNBO
