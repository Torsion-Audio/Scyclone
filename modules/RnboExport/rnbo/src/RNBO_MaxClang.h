#ifndef _RNBO_MAXCLANG_H_
#define _RNBO_MAXCLANG_H_

#if defined(USE_DYNAMIC_COMPILATION)

#include "RNBO_ClangInterface.h"
#include "ext.h"

namespace RNBO {

class MaxClang : public ClangInterface
{
public: 

    MaxClang(); 
    ~MaxClang(); 

    void addPreprocessorDefinition(std::string definition) override;
    void addIncludePath(std::string path) override;
    void addSymbol(std::string name, void* address) override;
	void setOptimizationLevel(OLevel level) override;
    bool compile(std::string name, const std::string& source) override;
	void getLastErrors(t_dictionary **lastErrors) override;

    void* getFunctionAddress(std::string name) override;

private: 
    t_object* _maxclang; 

};

} // namespace RNBO

#endif // #if defined(USE_DYNAMIC_COMPILATION)

#endif // #ifndef _RNBO_MAXCLANG_H_
