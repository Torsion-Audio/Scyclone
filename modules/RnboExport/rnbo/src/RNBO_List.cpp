#include "RNBO_List.h"

#ifdef C74_UNIT_TESTS
#include <JuceHeader.h>
#include <stdexcept>

namespace ListTests {
	using namespace RNBO;
	class Tests : public UnitTest {
		public:
			Tests()
				: UnitTest("List Tests")
			{}
			void runTest() override {
				beginTest("listbase bounds");
				listbase<number> l = { 1.0 };
				expectDoesNotThrow(l[0]);
				expectThrowsType(l[1], std::out_of_range);
				expectThrowsType(l[1] = 20.0, std::out_of_range);
				expectThrowsType(l[100], std::out_of_range);

				//shift
				expectEquals(l.shift(), 1.0);
				expectThrowsType(l.shift(), std::out_of_range);

				//pop
				l.push(20.0);
				l.push(10.0);
				expectEquals(l.pop(), 10.0);
				expectEquals(l.pop(), 20.0);
				expectEquals(l.pop(), 0.0);
				expectEquals(l.pop(), 0.0);
			}
	};
	Tests tests;
}
#endif

