//
//  RNBO_EventQueue.h
//  Created by Timothy Place on 11.08.15.
//

#ifndef _RNBO_EventQueue_h
#define _RNBO_EventQueue_h

RNBO_PUSH_DISABLE_WARNINGS
#include "3rdparty/readerwriterqueue/readerwriterqueue.h"
RNBO_POP_DISABLE_WARNINGS

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

// disable this on iOS for now as it requires thread-local-storage which iOS seems to not support
// this is a bit of a hack, but fixes build for now
#if TARGET_OS_IPHONE
#define RNBO_MULTIPRODUCER_QUEUE_SUPPORTED 0
#else
#define RNBO_MULTIPRODUCER_QUEUE_SUPPORTED 1
#endif

#if RNBO_MULTIPRODUCER_QUEUE_SUPPORTED
RNBO_PUSH_DISABLE_WARNINGS
#include "3rdparty/concurrentqueue/concurrentqueue.h"
RNBO_POP_DISABLE_WARNINGS
#endif

namespace RNBO {

	/**
	 * @brief A lock-free queue that may have one producer thread, one consumer thread
	 *
	 * @tparam T type of event to be queued
	 */
	template <class T>
	class EventQueueBase
	{
	public:
		virtual ~EventQueueBase() {}
		/** Add an event of any kind to the back of the queue.
			@param	ev	An event to push onto the queue.
		 */
		virtual void enqueue(const T& ev) = 0;

		/** Fetch the front-most event off of the queue.
			@param	ev	A dummy event to be filled-in upon return.
			@return		True if an event was successfully retrieved.
			Will return false if there are no events left on the queue.
		 */
		virtual bool dequeue(T& ev) = 0;
	};

	// TODO: could parameterize with e.g. queue size
	/**
	 * @brief A lock-free queue that may have one producer thread, one consumer thread
	 *
	 * @tparam T type of event to be queued
	 * @tparam Q type of queue
	 */
	template <class T, class Q>
	class EventQueue : public EventQueueBase<T>
	{
	public:
		EventQueue()
		: _queue(128)		// for now setting inital size to avoid malloc on audio thread at startup
		{

		}

		~EventQueue() override {}

		/**
		 * @brief Add an event of any kind to the back of the queue.
		 *
		 * @param ev An event to push onto the queue.
		 */
		void enqueue(const T& ev) override {
			_queue.enqueue(ev);
		}

		/*
		 * @brief Fetch the front-most event off of the queue.
		 *
		 * @param ev A dummy event to be filled-in upon return.
		 *
		 * @return True if an event was successfully retrieved.
		 * @return False if there are no events left on the queue.
		 */
		bool dequeue(T& ev) override {
			return _queue.try_dequeue(ev);
		}
	private:
		Q	_queue;
	};

} // namespace rnbo

#endif // _RNBO_EventQueue_h
