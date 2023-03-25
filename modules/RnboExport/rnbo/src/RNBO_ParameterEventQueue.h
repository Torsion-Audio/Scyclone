//
//  RNBO_ParameterEventQueue.h
//  Created by Timothy Place on 11.08.15.
//

#ifndef _RNBO_ParameterEventQueue_h
#define _RNBO_ParameterEventQueue_h

#include "RNBO_EventQueue.h"
#include "RNBO_ServiceNotification.h"

namespace RNBO {

	using ParameterEventQueue = EventQueue<EventVariant, moodycamel::ReaderWriterQueue<EventVariant>>;
	using ParameterEventQueuePtr = std::unique_ptr<EventQueueBase<EventVariant>>;
#if RNBO_MULTIPRODUCER_QUEUE_SUPPORTED
	using SafeParameterEventQueue = EventQueue<EventVariant, moodycamel::ConcurrentQueue<EventVariant>>;
#else
	using SafeParameterEventQueue = ParameterEventQueue;  // hack, TODO fix this!
#endif
	using ServiceNotificationQueue = EventQueue<ServiceNotification, moodycamel::ReaderWriterQueue<ServiceNotification>>;

	using MidiEventQueue = EventQueue<MidiEvent, moodycamel::ReaderWriterQueue<MidiEvent>>;

} // namespace RNBO

#endif // _RNBO_ParameterEventQueue_h
