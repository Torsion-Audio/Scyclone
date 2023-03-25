//
//  RNBO_Config.h
//
//  Created by Rob Sussman on 1/12/16.
//
//

#ifndef _RNBO_Config_h_
#define _RNBO_Config_h_

// include your own RNBO_LocalConfig.h before including "RNBO.h"
// if you want to override the settings in this file

//////////////////////////////////////////////////////////////////////////////////

// dynamic compilation watches the patcher source file and
// uses clang/llvm to recompile dynamically when it changes
// can enable or disable here for local testing, but primary configuration should be via project file
// #define USE_DYNAMIC_COMPILATION
// #undef USE_DYNAMIC_COMPILATION

// determines if we should init the file watcher that checks for changes in the
// default test file and triggers re-compilation
// #define USE_TEST_FILEWATCHER
// #undef USE_TEST_FILEWATCHER

//////////////////////////////////////////////////////////////////////////////////

#if defined(USE_DYNAMIC_COMPILATION) && defined(RNBO_NO_CLANG)

// disable clang/llvm for projects that don't need/support it
#undef USE_DYNAMIC_COMPILATION

#endif // defined(USE_DYNAMIC_COMPILATION) && defined(RNBO_NO_CLANG)

//////////////////////////////////////////////////////////////////////////////////

// Default place to write output cpp code
#ifdef RNBO_DEFAULT_TEST_FILE
#define RNBO_TEST_FILE RNBO_QUOTE(RNBO_DEFAULT_TEST_FILE)
#endif

// Default place to write output js code
#ifdef RNBO_DEFAULT_WEB_FILE
#define RNBO_WEB_FILE RNBO_QUOTE(RNBO_DEFAULT_WEB_FILE)
#endif

// Switch off the default Patcher Interface creation, necessary when you want to
// use multiple CoreObjects in one Project
//#define RNBO_NO_PATCHERFACTORY

//////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef _RNBO_Config_h_
