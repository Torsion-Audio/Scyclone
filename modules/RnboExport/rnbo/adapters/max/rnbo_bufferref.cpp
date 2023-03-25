#include "rnbo_bufferref.h"

namespace {
	t_class *s_rnbo_bufferref_class = nullptr;
}

extern "C" {

	void rnbo_bufferref_register()
	{
		if (s_rnbo_bufferref_class)
			return;
		auto c = class_new("rnbo_bufferref", (method)rnbo_bufferref_new, (method)rnbo_bufferref_free, (long)sizeof(t_rnbo_bufferref), 0L, A_CANT, 0);

		class_addmethod(c, (method)rnbo_bufferref_notify, "notify", A_CANT, 0);

		class_register(CLASS_NOBOX, c);
		s_rnbo_bufferref_class = c;
	}

	t_rnbo_bufferref *rnbo_bufferref_new(t_symbol *buffername)
	{
		t_rnbo_bufferref *x = (t_rnbo_bufferref *)object_alloc(s_rnbo_bufferref_class);
		if (x) {
			x->r_buffer_ref = buffer_ref_new((t_object *)x, buffername);
			x->r_name = buffername;
			x->r_lastKnownAddress = NULL;
			x->r_staleAddress = NULL;
			x->r_islocked = false;
			x->r_pending = false;
		}

		return x;
	}

	void rnbo_bufferref_free(t_rnbo_bufferref *x)
	{
		if (x->r_buffer_ref) object_free(x->r_buffer_ref);
	}

	t_max_err rnbo_bufferref_notify(t_rnbo_bufferref *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
	{
		return buffer_ref_notify(x->r_buffer_ref, s, msg, sender, data);
	}

	float *rnbo_bufferref_lock(t_rnbo_bufferref *x)
	{
		t_buffer_obj *b = buffer_ref_getobject(x->r_buffer_ref);
		float *smps = buffer_locksamples(b);
		x->r_islocked = smps != nullptr;
		if (x->r_pending)
			x->r_pending = smps == x->r_staleAddress;
		return smps;
	}

	char rnbo_bufferref_islocked(t_rnbo_bufferref *x)
	{
		return x->r_islocked;
	}

	void rnbo_bufferref_unlock(t_rnbo_bufferref *x)
	{
		t_buffer_obj *b = buffer_ref_getobject(x->r_buffer_ref);
		buffer_unlocksamples(b);
	}

	void rnbo_bufferref_setdirty(t_rnbo_bufferref *x)
	{
		t_buffer_obj *b = buffer_ref_getobject(x->r_buffer_ref);
		buffer_setdirty(b);
	}

	void rnbo_bufferref_setname(t_rnbo_bufferref *x, t_symbol * name)
	{
		x->r_name = name;
		buffer_ref_set(x->r_buffer_ref, name);
	}

	t_symbol * rnbo_bufferref_getname(t_rnbo_bufferref *x)
	{
		return x->r_name;
	}

	void rnbo_bufferref_setlastaddress(t_rnbo_bufferref *x, float *lastAddress)
	{
		x->r_lastKnownAddress = lastAddress;
	}

	float *rnbo_bufferref_getlastaddress(t_rnbo_bufferref *x)
	{
		return x->r_lastKnownAddress;
	}

	bool rnbo_bufferref_buffer_exists(t_rnbo_bufferref *x)
	{
		return buffer_ref_exists(x->r_buffer_ref);
	}

	void rnbo_bufferref_make_pending(t_rnbo_bufferref *x, float *fromaddr)
	{
		x->r_pending = true;
		x->r_staleAddress = fromaddr;
	}

	bool rnbo_bufferref_ispending(t_rnbo_bufferref *x)
	{
		return x->r_pending;
	}

}

namespace RNBO {
	void DataRefBindMaxBuffer(
			ExternalDataIndex dataRefIndex,
			const ExternalDataRef* ref,
			t_rnbo_bufferref *dataref,
			UpdateRefCallback updateDataRef,
			ReleaseRefCallback releaseDataRef
	)
	{
		static t_symbol *set_external_sampleptr = NULL;
		if (!set_external_sampleptr) set_external_sampleptr = gensym("set_external_sampleptr");

		if (dataref) {
			float *smps = rnbo_bufferref_lock(dataref); // might not actually lock if there is no buffer
			bool didlock = rnbo_bufferref_islocked(dataref);
			bool pending = rnbo_bufferref_ispending(dataref); // is a deferred call to set_external_sampleptr pending
			if (!ref->getData() && !smps) return; // don't bother if there's no data to bind

			Index channelcount = 0;
			double samplerate = 44100;
			SampleIndex framecount = 0;
			t_buffer_obj *b = buffer_ref_getobject(dataref->r_buffer_ref);

			if (b) {
				channelcount = static_cast<Index>(buffer_getchannelcount(b));
				samplerate = buffer_getsamplerate(b);
				framecount = static_cast<SampleIndex>(buffer_getframecount(b));
			}

			size_t sizeInBytes = framecount * channelcount * sizeof(float);
			auto type = ref->getType();

			// Max buffers are all 32-bit floats. If the RNBO buffer doesn't have the same type,
			// then we're not going to be able to sync them.
			if (type.type != RNBO::DataType::Float32AudioBuffer) {
				object_warn(b, "Can't sync RNBO buffer to Max buffer: RNBO buffer is not 32-bit");
				return;
			}

			if (ref->getData() != (char *)smps
				|| ref->getSizeInBytes() != sizeInBytes
				|| type.audioBufferInfo.samplerate != samplerate
				|| type.audioBufferInfo.channels != channelcount
			) {
				Float32AudioBuffer newType(channelcount, samplerate);

				// If the Max buffer has updated but not the RNBO buffer
				// or if the Max buffer has bytes but not the RNBO buffer,
				// then use the Max buffer
				float *lastAddress = rnbo_bufferref_getlastaddress(dataref);
				if ((lastAddress != smps && lastAddress == (float *)ref->getData())
					||
					!ref->getData())
				{
					// It should be okay if smps is null. I can't actually think
					// of a situation that would cause this, but it shouldn't cause
					// a problem.
					if (!smps) sizeInBytes = 0;

					// Don't do anything if you're still waiting for set_external_sampleptr
					if (!pending) {
						updateDataRef(dataRefIndex, (char *)smps, sizeInBytes, newType);
						rnbo_bufferref_setlastaddress(dataref, smps);
					}
				} else {
					t_atom_long rnbo_framecount = ref->getSizeInBytes() / (type.audioBufferInfo.channels * sizeof(float));

					if (!object_getmethod(b, set_external_sampleptr)) {
						object_warn(b, "Can't sync RNBO buffer to Max buffer: Max buffer does not support external samples");
						return;
					}

					object_method(b, set_external_sampleptr, ref->getData(), rnbo_framecount, type.audioBufferInfo.channels, !didlock);
					rnbo_bufferref_make_pending(dataref, smps);
					rnbo_bufferref_setlastaddress(dataref, (float *) ref->getData());
				}
			}

			return;
		}

		if (ref->getData()) {
			releaseDataRef(dataRefIndex);
		}
	}
	void DataRefUnbindMaxBuffer(
			const ExternalDataRef* externalRef,
			t_rnbo_bufferref *dataref
	)
	{
			if (dataref) {
				if (rnbo_bufferref_islocked(dataref)) {
					rnbo_bufferref_unlock(dataref);
				}

				if (externalRef->getTouched()) {
					rnbo_bufferref_setdirty(dataref);
				}
			}
	}
}
