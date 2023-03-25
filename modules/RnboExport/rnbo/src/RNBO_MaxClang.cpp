#include "RNBO_MaxClang.h"

#if defined(USE_DYNAMIC_COMPILATION)

namespace RNBO {

std::unique_ptr<RNBO::ClangInterface> ClangInterface::create()
{
    auto clanger = make_unique<MaxClang>();
    return std::move(clanger);
}

MaxClang::MaxClang()
{
    static t_symbol* ps_clang;
    static t_symbol* ps_rnbo;
    if (!ps_clang) {
        ps_clang = gensym("clang");
        ps_rnbo = gensym("rnbo");
    }
    _maxclang = (t_object*) object_new(CLASS_NOBOX, ps_clang, ps_rnbo);
}

MaxClang::~MaxClang()
{
    if (_maxclang) {
        object_free(_maxclang);
        _maxclang = nullptr;
    }
}

void MaxClang::addPreprocessorDefinition(std::string definition)
{
    if (_maxclang) {
        static t_symbol *ps_define;
        if (!ps_define) {
            ps_define = gensym("define");
        }
        object_method(_maxclang, ps_define, gensym(definition.c_str()));
    }
}

void MaxClang::addIncludePath(std::string path)
{
    if (_maxclang) {
        static t_symbol *ps_include;
        if (!ps_include) {
            ps_include = gensym("include");
        }
        object_method(_maxclang, ps_include, gensym(path.c_str()));
    }
}

void MaxClang::addSymbol(std::string name, void* address)
{
    if (_maxclang) {
        static t_symbol *ps_addsymbol;
        if (!ps_addsymbol) {
            ps_addsymbol = gensym("addsymbol");
        }
        object_method(_maxclang, ps_addsymbol, gensym(name.c_str()), address);
    }
}

void MaxClang::setOptimizationLevel(OLevel level)
{
	if (_maxclang) {
		static t_symbol *ps_optimize;
		if (!ps_optimize) {
			ps_optimize = gensym("optimize");
		}

		t_symbol *olevel;
		switch (level) {
			case O0: olevel = gensym("O0"); break;
			case O1: olevel = gensym("O1"); break;
			case O2: olevel = gensym("O2"); break;
			case O3: olevel = gensym("O3"); break;
			case Os: olevel = gensym("Os"); break;
			case Ofast: olevel = gensym("Ofast"); break;
			case Oz: olevel = gensym("Oz"); break;
		}

		object_method(_maxclang, ps_optimize, olevel);
	}

}

bool MaxClang::compile(std::string name, const std::string& source)
{
    if (_maxclang) {
        static t_symbol *ps_compile;
        static t_symbol *ps_cpp;
        if (!ps_compile) {
            ps_compile = gensym("compile");
            ps_cpp = gensym("cpp");
        }
        t_string* strSource = string_new(source.c_str());
        if (strSource) {
            t_atom args[2];
            t_atom rv;
            atom_setobj(&args[0], strSource);
            atom_setsym(&args[1], gensym(name.c_str()));
            object_attr_setlong(_maxclang, ps_cpp, 11);
            object_method_typed(_maxclang, ps_compile, 2, &args[0], &rv);
            object_free(strSource);
            if (!atom_getlong(&rv)) {
                // success!
                return true;
            }
        }
    }
    return false;
}

void MaxClang::getLastErrors(t_dictionary **lastErrors)
{
	if (_maxclang) {
		object_method(_maxclang, gensym("getlasterrors"), lastErrors);
	}
}

void* MaxClang::getFunctionAddress(std::string name)
{
    void* address = nullptr;
    static t_symbol* ps_getfunction;
    
    if (!ps_getfunction) {
        ps_getfunction = gensym("getfunction");
    }
    
    if (_maxclang) {
        t_atom av[1];
        t_atom rv;
        atom_setsym(av, gensym(name.c_str()));
        object_method_typed(_maxclang, ps_getfunction, 1, &av[0], &rv);
        address = atom_getobj(&rv);
    }
    
    return address;
}

} // namespace RNBO

#endif // #if defined(USE_DYNAMIC_COMPILATION)
