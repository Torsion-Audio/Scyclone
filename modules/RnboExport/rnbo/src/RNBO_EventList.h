//
//  RNBO_EventList.h
//
//  Created by Rob Sussman on 8/4/15.
//
//

#ifndef _RNBO_EventList_H_
#define _RNBO_EventList_H_

#include "RNBO_Types.h"
#include "RNBO_MidiEvent.h"
#include <utility>

//#define USE_STD_VECTOR

#ifdef USE_STD_VECTOR
#include <vector>
#include <algorithm>
#else
#include "RNBO_Vector.h"
#endif

namespace RNBO {

	/**
	 * @private
	 *
	 * @brief A container for holding Event objects sorted by time.
	 * @tparam the type to return from getTime()
	 */
	template <typename T>
	class EventList {

	public:

		// first we have some implementation details to get out of the way
#ifdef USE_STD_VECTOR
		using ListType = std::vector<T>;
#else
		using ListType = Vector<T>;
#endif
		using Iterator = typename ListType::iterator;
		using ConstIterator = typename ListType::const_iterator;
		using ReverseIterator = typename ListType::reverse_iterator;
		using ConstReverseIterator = typename ListType::const_reverse_iterator;

		// The initialCapcity reserves memory for the events early on
		// to try and avoid reallocation from the audio thread. However,
		// if there are many events then there will be a one-time reallocation each time it grows.
		EventList(size_t initialCapacity = 256)
		{
			if (initialCapacity) {
				_eventList.reserve(initialCapacity);
			}
		}

		Iterator begin()
		{
			return _eventList.begin();
		}

		Iterator end()
		{
			return _eventList.end();
		}

		ConstIterator begin() const
		{
			return _eventList.begin();
		}

		ConstIterator end() const
		{
			return _eventList.end();
		}

		ReverseIterator rbegin()
		{
			return _eventList.rbegin();
		}

		ReverseIterator rend()
		{
			return _eventList.rend();
		}

		ConstReverseIterator rbegin() const
		{
			return _eventList.rbegin();
		}

		ConstReverseIterator rend() const
		{
			return _eventList.rend();
		}

		/**
		 * @private
		 *
		 * used for ordered insertion into list
		 */
		struct EventComparator
		{
			bool operator() (T& lhs, MillisecondTime time)
			{
				return lhs.getTime() < time;
			}
			bool operator() (MillisecondTime time, T& rhs)
			{
				return time < rhs.getTime();
			}
			bool operator() (T& lhs, T& rhs)
			{
				return lhs.getTime() < rhs.getTime();
			}
		};

		/**
		 * @private
		 * @brief The actual API to use
		 * 
		 * @tparam U type of Event
		 * @param event the Event to add
		 */
		template <typename U>
		void addEvent(U&& event)
		{
			// upper_bound: iterator will point to first Event with getTime() > time
			// using lower_bound would cause second Event at same time to go before first, which would be wrong
			auto it = upper_bound(begin(), end(), event.getTime(), EventComparator());
			// and insert will put the new element *before* the item it points to
			_eventList.emplace(it, std::forward<T>(event));
		}

		Iterator getFirstEventAfterTime(MillisecondTime t)
		{
			auto it = upper_bound(_eventList.begin(), _eventList.end(), t, EventComparator());
			return it;
		}

		ConstIterator getFirstEventAfterTime(MillisecondTime t) const
		{
			auto it = upper_bound(_eventList.begin(), _eventList.end(), t, EventComparator());
			return it;
		}

		void clearEventsBeforeTime(MillisecondTime time)
		{
			auto it = upper_bound(_eventList.begin(), _eventList.end(), time, EventComparator());
			_eventList.erase(_eventList.begin(), it);
		}

		void clearEventsFromStartTime(MillisecondTime startTime)
		{
			auto first = lower_bound(_eventList.begin(), _eventList.end(), startTime, EventComparator());
			_eventList.erase(first, _eventList.end());
		}

		bool empty()
		{
			return _eventList.empty();
		}

		size_t size()
		{
			return _eventList.size();
		}

		void clear()
		{
			_eventList.clear();
		}

		void erase(ConstIterator item)
		{
			_eventList.erase(item);
		}

		void erase(ConstIterator first, ConstIterator last)
		{
			_eventList.erase(first, last);
		}

	protected:

		ListType _eventList;
#ifdef VECTOR_TESTS
		VectorTests vector_tests;
#endif

	};

	using MidiEventList = EventList<MidiEvent>;

} // namespace RNBO


#endif // #ifndef _RNBO_EventList_H_
