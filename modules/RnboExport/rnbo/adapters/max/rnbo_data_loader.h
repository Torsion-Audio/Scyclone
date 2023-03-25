#ifndef _RNBO_DATA_LOADER_H
#define _RNBO_DATA_LOADER_H

//RNBO.h seems to have to come first in visual studio or std::numeric_limits<_>::max() breaks..
#include <RNBO.h>
#include <memory>

#include <ext.h>
#include <ext_obex.h>
#include <ext_path.h>
#include <ext_systhread.h>
#include "commonsyms.h"

extern "C" {

	typedef struct _audio_info {
		long	channels;
		long	samplerate;
		long	frames;
	} t_audio_info;

	// Simplified "loader" object that can load audio data from a file or URL
	// without using a Max buffer.
	typedef struct _rnbo_data_loader {
		t_object 				obj;
		RNBO::DataType::Type	_type;
		bool					_ready;
		t_symbol				*_last_requested;
		t_audio_info			_info;
		char					*_data;
		t_object				*_remote_resource;
		t_systhread_mutex 		_mutex;
	} t_rnbo_data_loader;

	void rnbo_data_loader_register();
	t_rnbo_data_loader *rnbo_data_loader_new(RNBO::DataType::Type type);
	bool rnbo_data_loader_ready(t_rnbo_data_loader *x);
	void rnbo_data_loader_free(t_rnbo_data_loader *x);
	void rnbo_data_loader_file(t_rnbo_data_loader *x, const char *filename);
	void rnbo_data_loader_url(t_rnbo_data_loader *x, const char *url);
	t_symbol *rnbo_data_loader_get_last_requested(t_rnbo_data_loader *x);
}

namespace RNBO {
	void DataLoaderHandoffData(
			ExternalDataIndex dataRefIndex,
			const ExternalDataRef* ref,
			t_rnbo_data_loader *loader,
			UpdateRefCallback updateDataRef,
			ReleaseRefCallback releaseDataRef
	);
};

#endif
