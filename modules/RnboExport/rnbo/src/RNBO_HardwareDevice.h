//
//  RNBO_HardwareDevice.h
//

#ifndef _RNBO_HARDWAREDEVICE_H_
#define _RNBO_HARDWAREDEVICE_H_

// simple device registry

#include "RNBO_Types.h"
#include "RNBO_Vector.h"
#include "RNBO_String.h"

namespace RNBO {

	/**
	 * @brief A UniversalEvent generator
	 *
	 * UniversalEvents are simply timestamped events that can be added to RNBO's
	 * event processing queue. They usually correspond to MIDI controller input
	 * or OSC messages.
	 */
	class UniversalEventSource {

	public:

		/**
		 * @brief UniversalEventSource types
		 */
		enum Type {

			Invalid = -1,  ///< Invalid
			HardwareDevice = 0,  ///< Hardware device input
			OSCThing,  ///< OSC input
			Max_Type  ///< @private

		};

		/**
		 * @brief Construct a UniversalEventSource
		 *
		 * Default constructs with a UniversalEventSource::Invalid type with empty name and description.
		 */
		UniversalEventSource()
		: _type(Invalid)
		, _name("")
		, _description("")
		{}

		/**
		 * @brief Construct a UniversalEventSource
		 *
		 * @param type the source type
		 * @param name a string to use as a name
		 * @param description a longer description (optional)
		 */
		UniversalEventSource(Type type, String name, String description = "")
		: _type(type)
		, _name(name)
		, _description(description)
		{}

		/**
		 * @brief 
		 */
		Type getType() const {
			return _type;
		}

		/**
		 * @brief get the UniversalEventSource name
		 */
		const String& getName() const {
			return _name;
		}

		/**
		 * @brief get the UniversalEventSource description
		 */
		const String& getDescription() const {
			return _description;
		}

		// we'll possibly need some sort of pure virtual function here to get additional info, I guess

		using UniversalEventSourceList = Vector<UniversalEventSource>;

		/**
		 * @brief Get all UniversalEventSources
		 *
		 * @return a list of all UniversalEventSources as a RNBO::Vector<UniversalEventSource>
		 */
		static UniversalEventSourceList& UniversalEventSources() {
			static UniversalEventSourceList eventSourceList;
			return eventSourceList;
		}

		/**
		 * @brief Get the first matching unique ID given a source name
		 *
		 * @param sourceName the name of a UniversalEventSource
		 * @return an Index of the first matching source name, if one exists,
		 * otherwise RNBO::INVALID_INDEX
		 */
		static Index getIdForName(const String& sourceName) {
			Index count = 0;
			for (auto it = UniversalEventSources().begin(); it != UniversalEventSources().end(); it++, count++) {
				if (it->getName() == sourceName) {
					return count;
				}
			}
			return INVALID_INDEX;
		}

		/**
		 * @brief Get the string given a source's ID
		 *
		 * @param sourceId unique ID of a UniversalEventSource
		 * @return the name of a UniversalEventSource if one is found, an empty String otherwise
		 */
		static String getNameForId(const int sourceId) {
			if (sourceId >= 0 && sourceId < int(UniversalEventSources().size())) {
				return (UniversalEventSources().begin() + sourceId)->getName();
			}
			return "";
		}

	protected:

		Type 	_type;
		String	_name;
		String	_description;

	};

	/**
	 * @brief A UniversalEvent generator that contains extra hardware device info
	 */
	class HardwareDevice : public UniversalEventSource {

	public:

		/**
		 * @brief Valid HardwareDevice types
		 */
		enum HardwareType {

			Invalid = -1,  ///< Invalid hardware device type
			Unknown = 0,  ///< Unknown hardware device type
			Encoder,  ///< Endless encoder input
			Switch,  ///< On/Off switch input
			Slider,  ///< Slider device input
			Max_Type  ///< @private

		};

		/**
		 * @brief Construct a HardwareDevice
		 *
		 * Default constructs with a UniversalEventSource::Invalid type, and
		 * empty name.
		 */
		HardwareDevice()
		: UniversalEventSource(UniversalEventSource::Invalid, "")
		{}

		/**
		 * @brief Construct a HardwareDevice
		 */
		HardwareDevice(HardwareType hardwareType, const String& name, const String description = "")
		: UniversalEventSource(UniversalEventSource::HardwareDevice, name, description)
		, _hardwareType(hardwareType)
		{}

		/**
		 * @brief Get the HardwareType associated with the device
		 */
		HardwareType getHardwareType() { return _hardwareType; }

		/**
		 * @brief Register a HardwareDevice with the list of
		 * UniversalEventSources
		 *
		 * @param deviceType the HardwareDevice::HardwareType of the device
		 * @param deviceName a String identifier
		 * @param description a longer String to describe the device (optional)
		 * @return the number of registered devices if registration is
		 * successful, -1 otherwise
		 */
		static int registerHardwareDevice(HardwareType deviceType, const String& deviceName, const String description = "") {
			if (UniversalEventSource::getIdForName(deviceName) < 0) {
				UniversalEventSource::UniversalEventSources().push_back(HardwareDevice(deviceType, deviceName, description));
				return int(UniversalEventSource::UniversalEventSources().size()) - 1;
			}
			return -1;
		}

	private:

		HardwareType	_hardwareType;

	};

} // RNBO

#endif // _RNBO_HARDWAREDEVICE_H_
