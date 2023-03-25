#ifndef _RNBO_PROBINGINTERFACE_H_
#define _RNBO_PROBINGINTERFACE_H_

#include "RNBO_Types.h"

namespace RNBO {

	/**
	* @private
	*/
	class ProbingInterface {

	protected:

		~ProbingInterface() { }

	public:

		virtual ParameterIndex getProbingChannels(MessageTag outletId) const = 0;
	};

} // namespace RNBO

#endif // #ifndef _RNBO_PROBINGINTERFACE_H_
