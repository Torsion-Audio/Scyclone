//
//  _RNBO_PatcherInterfaceImpl_H_
//
//  Created by Rob Sussman on 1/11/18.
//
//

#ifndef _RNBO_PatcherInterfaceImpl_H_
#define _RNBO_PatcherInterfaceImpl_H_

#include "RNBO_Types.h"
#include "RNBO_PatcherInterface.h"

namespace RNBO {

	class PatcherInterfaceImpl : public PatcherInterface
	{
	public:

		void destroy() override
		{
			delete this;
		}

	};

} // namespace RNBO


#endif // #ifndef _RNBO_PatcherInterfaceImpl_H_
