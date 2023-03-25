// RNBO_ClangInterface.h

#ifndef _RNBO_CLANGINTERFACE_H_
#define _RNBO_CLANGINTERFACE_H_

#include <memory>
#include <string>

class _dictionary;

namespace RNBO {

/**
 * @private
 */
struct ClangInterface
{
    class Listener;

    static std::unique_ptr<ClangInterface> create();
    virtual ~ClangInterface() = default;

	/** Append a preprocessor define at the back of the list of defines.
		@param definition	A string for the define, e.g. "DEBUG=1" or "USE_CATS"
	 */
    virtual void addPreprocessorDefinition(std::string definition) = 0;

	/** Append an include path at the back of the list of include paths.
		@param path	A string for the path.
		@remarks	Paths may be absolute or relative to the working directory.
					Absolute paths are recommended as the working directory changes based on how the app is launched.
	 */
    virtual void addIncludePath(std::string path) = 0;

    /**    Add searchable symbol/value pair to the LLVM execution engine.
        This functions permanently adds the symbol 'name' with the value at 'address'.
        These symbols are searched before any libraries.
        @param    name    The symbol associated with the value.
        @param    address    The address of the value.
     */
    virtual void addSymbol(std::string name, void* address) = 0;

	/** Set the optimization level */
	enum OLevel {
		O0,
		O1,
		O2,
		O3,
		Os,
		Ofast,
		Oz
	};

	virtual void setOptimizationLevel(OLevel level) = 0;

    /** Create an LLVM module by compiling C++ source code.
		@param	name	A unique string to identify the created module.
		@param	source		The entire source code listing to be compiled.
	    @param	listener	Optional: object to receive any errors or warnings during compilation
		@return				true on success, false on failure
	 */
	virtual bool compile(std::string name,
                         const std::string& source) = 0;

	/**	Fetches the function pointer for a function of a given name.
		@param	name	The name of the function or method (including C++ mangling).
		@return			A pointer to the function, or nullptr if the call failed.
	 */
    virtual void* getFunctionAddress(std::string name) = 0;

	/** Gets a dictionary filled with the most recent errors and warnings
	 */
	virtual void getLastErrors(_dictionary **lastErrors) = 0;

	/**
	 * @private
	 */
    class Listener
	{
	public:
		virtual ~Listener() {}

		enum Type {
			Warning,
			Error,
			Other
		};

		/** Listener is called for each diagnostic message reported by compiler.
			@param	message		the error message
		    @param	type		Warning, Error, or Other
			@param	line		line number of source location for error, or -1 if invalid
			@param	column		column of source location for error, or -1 if invalid
			@param	fileName	file name of source being compiled
		 */
		virtual void onDiagnosticMessage(std::string message,
										 Type type = Listener::Other,
										 int line = -1,
										 int column = -1,
										 std::string filename = ""
										 ) = 0;

		/** utility method to convert diagnostic message to a string in a reasonable way
		 */
		std::string formatDiagnosticMessage(std::string message,
											Type type,
											int line, int column,
											std::string filename)
		{
			std::string msg = filename;
			if (line > 0) {
				msg += std::string(":") + std::to_string(line);
			}
			if (column > 0) {
				msg += std::string(":") + std::to_string(column);
			}
			if (msg.length() > 0) {
				msg += " ";
			}
			if (type == ClangInterface::Listener::Error) {
				msg += "error: ";
			}
			else if (type == ClangInterface::Listener::Warning) {
				msg += "warning: ";
			}
			msg += message;
			msg += "\n";
			return msg;
		}
	};

	/**
	 * @private
	 */
	class StringBufferListener : public Listener
	{
	public:

		void onDiagnosticMessage(std::string message,
								 Type type,
								 int line,
								 int column,
								 std::string filename
								 ) override
		{
			_diagnosticMessages += formatDiagnosticMessage(message, type, line, column, filename);
		}

		std::string getMessages() { return _diagnosticMessages; }

	private:
		std::string _diagnosticMessages;
	};



};

} // namespace RNBO

#endif // #ifndef _RNBO_CLANGINTERFACE_H_
