//
//  RNBO_ParamNameHash.h
//
//

#ifndef _RNBO_ParamNameHash_H_
#define _RNBO_ParamNameHash_H_

#include "RNBO_Types.h"

#ifndef RNBO_NOSTDLIB
#include <map>
#include <string>
#endif

namespace RNBO {

	class ParamNameHash {
	public:
		void update(const PatcherInterface *patcher) {
#ifndef RNBO_NOSTDLIB
			_paramNames.clear();
			for (ParameterIndex i = 0; i < patcher->getNumParameters(); i++) {
				const char* paramid = patcher->getParameterId(i);
				_paramNames.emplace(std::make_pair(paramid, i));
			}
#endif
		}

		ParameterIndex get(const char* paramid) const {
#ifdef RNBO_NOSTDLIB
			// not supported without standard library
			RNBO_ASSERT(false);
			return INVALID_INDEX;
#else
			auto match = _paramNames.find(paramid);
			if (match != _paramNames.end()) {
				return match->second;
			}

			return INVALID_INDEX;
#endif
		}

	private:
#ifndef RNBO_NOSTDLIB
		std::map<std::string, ParameterIndex> _paramNames;
#endif
	};

} // namespace RNBO


#endif // #ifndef _RNBO_ParamNameHash_H_
