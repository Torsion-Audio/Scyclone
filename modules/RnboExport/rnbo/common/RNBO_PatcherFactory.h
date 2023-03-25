//
//  RNBO_PatcherFactory.h
//
//  Created by DDZ on 11/08/15.
//
//

#ifndef _RNBO_PatcherFactory_h
#define _RNBO_PatcherFactory_h

namespace RNBO {

    class PlatformInterface;
	class PatcherInterface;

	/** The patcher factory is just a function pointer to a function that
		allocates a PatcherInterface derived object and returns the PatcherInterface pointer
		Note, PatcherInterface should be freed via the PatcherInterface destroy() method.
	 */
	using PatcherFactoryFunctionPtr = PatcherInterface*(*)();

	/** the patcher factory can be obtained via a staticaly linked in function named GetPatcherFactoryFunction
	 	or a DLL can export the function named GetPatcherFactoryFunction in which case the function
	 	should have the following signature.
	*/
	using GetPatcherFactoryFunctionPtr = PatcherFactoryFunctionPtr(*)(PlatformInterface*);
	
}  // namespace RNBO

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" RNBO::PatcherFactoryFunctionPtr GetPatcherFactoryFunction(RNBO::PlatformInterface*);
#endif

#endif // ifndef _RNBO_PatcherFactory_h
