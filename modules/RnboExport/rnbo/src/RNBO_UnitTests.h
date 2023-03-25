//
//  RNBO_UnitTests.h
//  binding
//
//  Created by Stefan Brunner on 27.08.15.
//
//

#ifndef binding_RNBO_UnitTests_h
#define binding_RNBO_UnitTests_h

// for now we only want to compile unit tests in the debug version
// the general idea is, that if you want unit tests, you inlcude this file BEFORE including
// RNBO.h, this will define C74_UNIT_TESTS and therefore comoile the unit tests

// generally running unit tests needs Juce for now, so be aware that your project has to be prepared to use Juce

#if defined(DEBUG) && !defined(C74_UNIT_TESTS)
#define C74_UNIT_TESTS
#endif

#ifdef C74_UNIT_TESTS

#include "JuceHeader.h"

namespace RNBO {

	class UnitTestRunner : public juce::UnitTestRunner
	{
		// no modifications needed for now
	};

}

#endif // C74_UNIT_TESTS


#endif // binding_RNBO_UnitTests_h
