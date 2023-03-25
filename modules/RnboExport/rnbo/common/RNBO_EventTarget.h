//
//  RNBO_EventTarget.h
//
//  Created by Rob Sussman on 1/14/16.
//
//

#ifndef _RNBO_EventTarget_h_
#define _RNBO_EventTarget_h_

#include "RNBO_Types.h"
#include "RNBO_EngineLink.h"

namespace RNBO {

	/**
	 * @private 
	 */
	class EventTarget {

	public:

		virtual void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) = 0;
		virtual void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) = 0;

	protected:

		~EventTarget() { }

	};

} // namespace RNBO

#endif // #ifndef _RNBO_EventTarget_h_
