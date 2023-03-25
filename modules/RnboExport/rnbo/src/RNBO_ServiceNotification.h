//
//  RNBO_ServiceEvent.h
//
//  Created by Stefan Brunner on 9/21/15.
//
//

#ifndef _RNBO_ServiceEvent_H_
#define _RNBO_ServiceEvent_H_

#include "RNBO_Types.h"

namespace RNBO {

	// an event representing service message on our dedicated serive queue, note that it does NOT impement getTime()
	// so it cannot be used inside an EventVariant
	using ServiceNotificationPayload = void*;

	/**
	 * @private
	 */
	class ServiceNotification {
	public:

		enum ServiceNotificationType
		{
			ServiceNotificationUndefined,
			ParameterInterfaceCreated,
			ParameterInterfaceDeleted
		};

		ServiceNotification(const ServiceNotification& other) = default;
		ServiceNotification& operator = (const ServiceNotification& other) = default;

		ServiceNotification()
		: _type(ServiceNotificationUndefined)
		, _payload(nullptr)
		{
		}

		ServiceNotification(ServiceNotificationType serviceType, ServiceNotificationPayload payload)
		: _type(serviceType)
		, _payload(payload)
		{
		}

		bool operator==(const ServiceNotification& rhs) const
		{
			return rhs.getType() == getType()
			&& rhs.getPayload() == getPayload();
		}

		ServiceNotificationType getType() const { return _type; }
		ServiceNotificationPayload getPayload() const { return _payload; }

		// debugging
		void dumpEvent() const { fprintf(stdout, "ServiceNotification: type=%d payload=%payload\n", _type, _payload); }

	private:

		ServiceNotificationType			_type;
		ServiceNotificationPayload		_payload;

	};

} // namespace RNBO

#endif // #ifndef _RNBO_ServiceEvent_H_
