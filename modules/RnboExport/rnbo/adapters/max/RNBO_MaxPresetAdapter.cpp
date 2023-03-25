#include <RNBO.h>
#include "RNBO_MaxPresetAdapter.h"
#include <ext.h>
#include <atomic>

namespace MaxPresetAdapter {
	using namespace RNBO;

	//min's max-lib doesn't export _sym_nothing, so we do it ourselves
	static t_symbol* _local_sym_nothing = gensym("");

	void toDict(const Preset& preset, t_dictionary* presetDict) {
		for (auto const& entry : preset) {
			t_symbol *key = gensym(entry.first.c_str());
			auto type = entry.second.getType();
			switch (type) {
				case ValueHolder::NUMBER: {
					number value = (number)entry.second;
					dictionary_appendfloat(presetDict, key, value);
					break;
				}
				case ValueHolder::INDEX: {
					Index value = (Index)entry.second;
					dictionary_appendfloat(presetDict, key, value);
					break;
				}
				case ValueHolder::STRING: {
					const char* str = entry.second;
					dictionary_appendstring(presetDict, key, str);
					break;
				}
				case ValueHolder::LIST: {
					const list& value = entry.second;
					t_atom *av = nullptr;
					long ac = 0;
					char alloc;
					atom_alloc_array(value.length, &ac, &av, &alloc);
					if (alloc) {
						t_atom *a = av;
						for (long i = 0; i < value.length; i++) {
							atom_setfloat(a, value[i]);
							a++;
						}
						dictionary_appendatoms(presetDict, key, ac, av);
					}
					break;
				}
				case ValueHolder::SUBSTATE: {
					t_dictionary* substate = dictionary_new();
					const Preset& preset = entry.second;
					toDict(preset, substate);
					dictionary_appenddictionary(presetDict, key, (t_object *)substate);
					break;
				}
				case ValueHolder::SUBSTATEMAP: {
					long ac = 0;
					t_atom *av = nullptr;
					char alloc;
					Index size = entry.second.getSubStateMapSize();
					if (size) {
						atom_alloc_array(size, &ac, &av, &alloc);
						for (Index i = 0; i < size; i++) {
							t_dictionary* substate = dictionary_new();
							const Preset& preset = entry.second[i];
							toDict(preset, substate);
							atom_setobj(av + i, substate);
						}
						dictionary_appendatoms(presetDict, key, ac, av);
					}

					break;
				}
				default:
					// we do only support numbers, lists and substates
					RNBO_ASSERT(false);
			}
		}
	}

	void fromDict(t_dictionary* presetDict, Preset& preset) {
		long numkeys = 0;
		t_symbol** keys;
		dictionary_getkeys(presetDict, &numkeys, &keys);
		t_atom a;
		for (long i = 0; i < numkeys; i++) {
			t_symbol* symkey = keys[i];
			const char * key = symkey->s_name;
			if (dictionary_entryisdictionary(presetDict, symkey)) {
				Preset value;
				t_dictionary *subDict = nullptr;
				dictionary_getdictionary(presetDict, symkey, (t_object **)&subDict);
				Preset& substate = preset.getSubState(key);
				fromDict(subDict, substate);
			}
			else if (dictionary_entryisatomarray(presetDict, symkey)) {
				long ac = 0;
				t_atom *av = nullptr;
				dictionary_getatoms(presetDict, symkey, &ac, &av);
				if (ac) {
					auto type = atom_gettype(av);
					if (type == A_FLOAT) {
						list value;
						for (long i = 0; i < ac; i++) {
							value.push(atom_getfloat(av));
							av++;
						}
						preset[key] = value;
					}
					else if (type == A_OBJ) {
						for (long i = 0; i < ac; i++) {
							t_dictionary* d = (t_dictionary *)atom_getobj(av);
							Preset& subPreset = preset[key][i];
							fromDict(d, subPreset);
						}
					}
				}
			}
			else if (dictionary_getatom(presetDict, symkey, &a) == 0) {
				auto t = atom_gettype(&a);
				if (t == A_SYM) {
					if (auto s = atom_getsym(&a)) {
						preset[key] = s->s_name;
					}
				}
				else if (t == A_FLOAT || t == A_LONG) {
					number value = atom_getfloat(&a);
					preset[key] = value;
				}
			}
		}
	}

	void getObjectPreset(RNBO::CoreObject& o, t_dictionary *presetDict, bool async) {
		ConstPresetPtr returnedPreset;
		if (async) {
			std::atomic_bool waiting(true);
			o.getPreset(
				[&returnedPreset, &waiting] (ConstPresetPtr preset) {
					returnedPreset = preset;
					waiting = false;
				});
			while (waiting) {
				systhread_sleep(10);
			}
		} else {
			returnedPreset = o.getPresetSync();
		}
		toDict(*returnedPreset, presetDict);
	}

	void setObjectPreset(RNBO::CoreObject& o, t_dictionary *presetDict) {
		std::unique_ptr<Preset> preset = make_unique<Preset>();
		fromDict(presetDict, *preset);
		o.setPreset(std::move(preset));
	}
}
