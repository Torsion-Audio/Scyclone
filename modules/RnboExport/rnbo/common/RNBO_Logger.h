//
//    RNBO_Logger.h
//    Created: 25 Jan 2016 2:42:32pm
//    Author:  stb
//
//

#ifndef _RNBO_Logger_h_
#define _RNBO_Logger_h_

#include "RNBO_Types.h"
#include "RNBO_String.h"
#include "RNBO_List.h"

namespace RNBO {

class LoggerInterface;

extern "C" LoggerInterface* console;		// enables console->log() usage

// interface class enables binary compatibility from dynamically compiled clang/llvm code
// templated functions just simplify usage of the interface, but don't really change the class
/**
 * @private
 */
class LoggerInterface {
public:
	virtual ~LoggerInterface() {}
	virtual void log(LogLevel level, const char* message) = 0;

	// can send as many args to log() as you like, similar to javascript
	template <typename...ARGS>
	void log(ARGS...args) {
		String message;

		// template magic to handle arbitrary args
		appendArgsToString(message, args...);

		log(Info, message.c_str());
	}

	template <typename...ARGS>
	void log(LogLevel level, ARGS...args) {
		String message;

		// template magic to handle arbitrary args
		appendArgsToString(message, args...);

		log(level, message.c_str());
	}
private:
	// implementation details below
	// necessary in header for templates to work

	void appendArgument(String& message, const char *str)
	{
		message += str;
	}

	template <class T, int N=256>
	void appendArgument(String& message, T val)
	{
		char buff[N];
		Platform::get()->toString(buff, N, val);
		message += buff;
	}

	void appendArgument(String& message, const list& l)
	{
		for (size_t i = 0; i < l.length; i++) {
			if (i > 0) {
				message += " ";
			}
			appendArgument(message, l[i]);
		}
	}

	/** empty argument handling (required for recursive variadic templates)
	 */
	void appendArgsToString(String& message)
	{
		RNBO_UNUSED(message);
	}

	/** handle N arguments of any type by recursively working through them
		and matching them to the type-matched routine above.
	 */
	template <typename FIRST_ARG, typename ...REMAINING_ARGS>
	void appendArgsToString(String& message, FIRST_ARG const& first, REMAINING_ARGS const& ...args)
	{
		appendArgument(message, first);
		bool hasArgs = sizeof...(args) > 0 ? true : false;
		if (hasArgs) {
			message += " ";
			appendArgsToString(message, args...);	// recurse
		}
	}
};

/**
 * @private
 */
class Logger : public LoggerInterface {
public:

	Logger();
	virtual ~Logger() override;

	static Logger& getInstance();

	// platform can set callback to take over handling of log messages
	using OutputCallback = void(LogLevel level, const char* message);
	void setLoggerOutputCallback(OutputCallback* callback);

	void log(LogLevel level, const char* message) override
	{
		_outputCallback(level, message);
	}

	// defaultLogOutputFunction
	static void defaultLogOutputFunction(LogLevel level, const char* message);

private:

	OutputCallback*		_outputCallback;
};

}

#endif  // _RNBO_Logger_h_
