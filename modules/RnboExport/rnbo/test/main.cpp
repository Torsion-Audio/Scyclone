#include <JuceHeader.h>
#include "RNBO_UnitTests.h"
#include "RNBO.h"
#include "RNBO_MidiStreamParser.h"

int main (int /*argc*/, char** /*argv*/)
{
	RNBO::UnitTestRunner runner;
	runner.runAllTests();
	for (auto i = 0; i < runner.getNumResults(); i++) {
		auto res = runner.getResult(i);
		if (res == nullptr || res->failures > 0) {
			return -1;
		}
	}
	return 0;
}
