//
//  _RNBO_PatcherInterface_H_
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_PatcherEventTarget_H_
#define _RNBO_PatcherEventTarget_H_

#include "RNBO_Types.h"
#include "RNBO_EventTarget.h"
#include "RNBO_List.h"

namespace RNBO {

	/**
	 * @private
	 */
	class PatcherEventTarget : public EventTarget {

	public:

		virtual void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) = 0;
		virtual void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) = 0;
		virtual void processOutletEvent(EngineLink* sender, OutletIndex index, ParameterValue value,  MillisecondTime time) = 0;
		virtual void processOutletAtCurrentTime(EngineLink* sender, OutletIndex index, ParameterValue value) = 0;
		virtual void processDataViewUpdate(DataRefIndex index, MillisecondTime time) = 0;
		virtual void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) = 0;
		virtual void processListMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, const list& payload) = 0;
		virtual void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) = 0;
		virtual void processTempoEvent(MillisecondTime time, Tempo tempo) = 0;
		virtual void processTransportEvent(MillisecondTime time, TransportState state) = 0;
		virtual void processBeatTimeEvent(MillisecondTime time, BeatTime beatTime) = 0;
		virtual void processTimeSignatureEvent(MillisecondTime time, int numerator, int denominator) = 0;

	protected:

		~PatcherEventTarget() { }

	};

} // namespace RNBO


#endif // #ifndef _RNBO_PatcherEventTarget_H_
