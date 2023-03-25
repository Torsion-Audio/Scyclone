#ifndef _RNBO_BUFFERREF_H
#define _RNBO_BUFFERREF_H

//RNBO.h seems to have to come first in visual studio or std::numeric_limits<_>::max() breaks..
#include <RNBO.h>
#include <memory>

#include <ext.h>
#include <ext_obex.h>
#include <ext_buffer.h>

extern "C" {
	// the dataref object which is used for managing all references
	// to internal Max buffers that are used by RNBO
	typedef struct _rnbo_bufferref {
		t_object		obj;
		t_buffer_ref	*r_buffer_ref;
		t_symbol 		*r_name;
		float			*r_lastKnownAddress; // To determine if memory changed
		float			*r_staleAddress; // Stale address after a deferred call to set_external_ptr
		char			r_islocked;
		char			r_pending;
	} t_rnbo_bufferref;

	void rnbo_bufferref_register();
	t_rnbo_bufferref *rnbo_bufferref_new(t_symbol *buffername);
	void rnbo_bufferref_free(t_rnbo_bufferref *x);
	t_max_err rnbo_bufferref_notify(t_rnbo_bufferref *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
	float *rnbo_bufferref_lock(t_rnbo_bufferref *x);
	char rnbo_bufferref_islocked(t_rnbo_bufferref *x);
	void rnbo_bufferref_unlock(t_rnbo_bufferref *x);
	void rnbo_bufferref_setdirty(t_rnbo_bufferref *x);
	void rnbo_bufferref_setname(t_rnbo_bufferref *x, t_symbol * name);
	t_symbol * rnbo_bufferref_getname(t_rnbo_bufferref *x);
	void rnbo_bufferref_setlastaddress(t_rnbo_bufferref *x, float *lastAddress);
	float *rnbo_bufferref_getlastaddress(t_rnbo_bufferref *x);
	bool rnbo_bufferref_buffer_exists(t_rnbo_bufferref *x);
	void rnbo_bufferref_make_pending(t_rnbo_bufferref *x, float *fromaddr);
	bool rnbo_bufferref_ispending(t_rnbo_bufferref *x);
}

namespace RNBO {
	void DataRefBindMaxBuffer(
			ExternalDataIndex dataRefIndex,
			const ExternalDataRef* ref,
			t_rnbo_bufferref *dataref,
			UpdateRefCallback updateDataRef,
			ReleaseRefCallback releaseDataRef
	);
	void DataRefUnbindMaxBuffer(
			const ExternalDataRef* externalRef,
			t_rnbo_bufferref *dataref
	);
};
#endif
