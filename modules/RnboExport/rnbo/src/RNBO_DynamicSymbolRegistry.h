#ifndef _RNBO_DYNAMICSYMBOLREGISTRY_H_
#define _RNBO_DYNAMICSYMBOLREGISTRY_H_

#ifndef RNBO_DISABLE_CODEGEN

#include <vector>

namespace RNBO {

	/**
	 * @private 
	 */
	class DynamicSymbolRegistry {

	public:

		/**
		 * @private
		 */
		struct DynamicSymbol {
			char	 _name[256];
			void	*_location;
		};

		using DynamicSymbolList = std::vector<DynamicSymbol>;

		static DynamicSymbolList& getRegisteredSymbols() {
			static 	DynamicSymbolList dynamicSymbolRegistry;
			return dynamicSymbolRegistry;
		}

		static void registerSymbol(const char *name, void *location) {
			DynamicSymbol sym;
			char *n = sym._name;

			while ((*n++ = *name++) != 0) ;
			sym._location = location;

			getRegisteredSymbols().push_back(sym);
		}

	};

	/**
	 * @private
	 */
	class DynamicSymbolRegistrar {

	public:

		DynamicSymbolRegistrar(const char *name, void *location) {
			DynamicSymbolRegistry::registerSymbol(name, location);
		}

	};

} // RNBO

#	define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#	define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#	define REGISTER_RNBO_EXTERNAL(ExternalName) extern "C" RNBO::ExternalBase * CAT(ExternalName, Factory) () \
	{ return new ExternalName (); } \
	static RNBO::DynamicSymbolRegistrar ExternalName ## RegisteredSymbol( #ExternalName "Factory" , (void *) CAT(ExternalName, Factory) );

#else // RNBO_DISABLE_CODEGEN

namespace RNBO {

	class DynamicSymbolRegistry {

	};

	class DynamicSymbolRegistrar {

	public:

		DynamicSymbolRegistrar(const char *name, void *location) {}

	};

} // RNBO

#	define REGISTER_RNBO_EXTERNAL(ExternalName)

#endif // RNBO_DISABLE_CODEGEN

#endif // _RNBO_DYNAMICSYMBOLREGISTRY_H_
