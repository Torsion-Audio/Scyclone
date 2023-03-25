#include <inttypes.h>

extern "C" {
	const char * JuceStatic_Plugin_Name() {
		return PLUGIN_NAME;
	}
	uint32_t JuceStatic_Plugin_Code() {
		return PLUGIN_CODE_HEX;
	}
	uint32_t JuceStatic_Plugin_VersionCode() {
		return PLUGIN_VERSION_NUM;
	}
	const char * JuceStatic_Plugin_VersionString() {
		return PLUGIN_VERSION;
	}

	const char * JuceStatic_Plugin_Manufacturer() {
		return PLUGIN_MANUFACTURER_NAME;
	}
	const char * JuceStatic_Plugin_ManufacturerWebsite() {
		return PLUGIN_MANUFACTURER_WEBSITE;
	}
	const char * JuceStatic_Plugin_ManufacturerEmail() {
		return PLUGIN_MANUFACTURER_EMAIL;
	}
	uint32_t JuceStatic_Plugin_ManufacturerCode() {
		return PLUGIN_MANUFACTURER_CODE_HEX;
	}

}
