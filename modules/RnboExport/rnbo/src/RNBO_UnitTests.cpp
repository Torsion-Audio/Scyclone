//a place for for small misc tests

#ifdef C74_UNIT_TESTS

#include "RNBO.h"
#include <JuceHeader.h>
#include <stdexcept>

namespace RNBOUnitTests {
	using namespace RNBO;
	class ArrayTests : public UnitTest {
		public:
			ArrayTests()
				: UnitTest("Array Tests")
			{}
			void runTest() override {
				beginTest("bounds");
				RNBO::array<number, 1> l = { 1.0 };
				expectDoesNotThrow(l[0]);
				expectThrowsType(l[1], std::out_of_range);
				expectThrowsType(l[100], std::out_of_range);
			}
	};
	ArrayTests tests;
}
#endif

