#ifndef _RNBO_ExternalBase_h_
#define _RNBO_ExternalBase_h_

#include "RNBO_ProcessInterface.h"
#include "RNBO_EngineLink.h"
#include "RNBO_EventTarget.h"

namespace RNBO {

	/**
	 * @private
	 */
	class ExternalBase : public ProcessInterface, public EngineLink, public EventTarget {

	public:

		virtual ~ExternalBase() { }

	};

} // namespace RNBO

#endif  // #ifndef _RNBO_ExternalBase_h_
