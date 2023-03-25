//
//  RNBO_DynamicPatcherFactory.h
//
//  Created by Rob Sussman on 12/4/15.
//
//

#ifndef _RNBO_DynamicPatcherFactory_H_
#define _RNBO_DynamicPatcherFactory_H_

#ifdef USE_DYNAMIC_COMPILATION

#include "RNBO_Types.h"
#include "RNBO_ClangInterface.h"
#include "RNBO_String.h"
#include "RNBO_PatcherFactory.h"
#include "RNBO_DynamicPatcherFactory.h"

namespace RNBO {

	class DynamicPatcherFactory
	{
	public:

		/** DynamicPatcherFactory creates patcher instances by using clang/llvm to
			"dynamically" compile the given source. If the source changes nothing happens.
			If you want to recompile when the source changes make a new DynamicPatcherFactory() instance.
		 
			Compilation happens synchronously in the constructor. 
			You can pass in a listener to receive compilation diagnostic messages.
			Otherwise, messages are dropped.
		 
			Can check on compilation success via didCompileSuccesfully() method.
		 */
		DynamicPatcherFactory(const char* fullPathToCppSource,
                              const char* fullPathToRNBOHeaders);

		/** pass in a name and source code directly, rather than using a file on disk
		 */
		DynamicPatcherFactory(const char* name,
                              const char* source,
                              const char* fullPathToRNBOHeaders,
							  ClangInterface::OLevel oLevel);

		/** call to determine if compilation (done in constructor) was successful 
		    when not successful createInstance() is expected to return nullptr
		 */
		bool didCompileSucceed() { return _didCompileSucceed; }
		void getLastErrors(_dictionary **lastErrors);

		const char* getFullPathToCppSource();
		UniquePtr<PatcherInterface> createInstance();

        // TODO: fix debugging
		// void SetNotifyDebuggerCallback(RNBOClang::NotifyDebuggerCallback callback);

	private:
        PatcherFactoryFunctionPtr getPatcherFactoryFunction();

		void initWithNameAndSource(const char* name,
                                   const char* source,
                                   const char* fullPathToRNBOHeaders,
								   ClangInterface::OLevel oLevel);
		static String defaultFullPathToCppSource();

		String _fullPathToCppSource;
		std::shared_ptr<ClangInterface> _clanger;
		bool _didCompileSucceed;
	};

} // namespace RNBO

#endif // #ifdef USE_DYNAMIC_COMPILATION

#endif // #ifndef _RNBO_DynamicPatcherFactory_H_
