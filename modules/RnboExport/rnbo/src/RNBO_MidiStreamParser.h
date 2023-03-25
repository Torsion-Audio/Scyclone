#ifndef _RNBO_MIDIStreamParser_H_
#define _RNBO_MIDIStreamParser_H_

#include <stdint.h>
#include <functional>

namespace RNBO {
	namespace {
		const size_t max_length = 3;
	}
	
	/**
	 * @brief Parser for incoming MIDI data and process valid packets
	 */
	class MidiStreamParser {
		public:
			/**
			 * @brief Processes incoming MIDI data and passes valid packets to a
			 * callback function
			 * 
			 * @param input a MIDI byte
			 * @param cb a callback function which takes a `const uint8_t *`
			 * byte array and `size_t` array index as arguments
			 */
			void process(uint8_t input, std::function<void(const uint8_t *, size_t)> cb) {
				uint8_t midibyte = static_cast<uint8_t>(input);
				switch (midibyte) {
					case 0xF8:
					case 0xFA:
					case 0xFB:
					case 0xFC:
					case 0xFE:
					case 0xFF:
						//realtime messages are handled immediately and can come inside other messages
						cb(&midibyte, 1);
						return;
					default:
						break;
				}
				if (_index >= max_length) {
					_index = 0;
				}
				if (midibyte > 0x7F && midibyte != 0xF7) {
					_buffer[0] = midibyte;
					_issysex = false; //might get updated below
					_index = 1;
				} else
					_buffer[_index++] = midibyte;
				if (ready()) {
					/*
					//zero out extra bytes
					for (auto i = _index; index < max_length+ i++)
					_buffer[i] = 0;
					*/
					cb(_buffer, _index);
					if (!_issysex) {
						_index = 1; //for running status, might be reset on next loop
					}
				}
			}
		private:
			bool ready() {
				uint8_t status = _buffer[0];
				uint8_t b1, b2;

				if (_issysex) {
					if (_index) {
						if (_buffer[_index - 1] == 0xF7) {
							_issysex = false;
							return true;
						}
						else if (_index == max_length) {
							return true;
						}
					}
					return false;
				}
				else if (status < 0x80) {	// bad byte, should never happen
					_index = 0;
					return false;
				}

				if (status < 0xF0) {	// channel message
					status = status & 0xF0;
					b1 = _buffer[1];
					b2 = _buffer[2];
					switch (status) {
						case 0x80:
						case 0x90:
						case 0xA0:
						case 0xB0:
						case 0xE0:
							if (_index == 3) {
								_buffer[1] = b1 & 0x7F;
								_buffer[2] = b2 & 0x7F;
								return true;
							} else
								return false;
							break;
						case 0xD0:
						case 0xC0:
							if (_index == 2) {
								_buffer[1] = b1 & 0x7F;
								return true;
							} else
								return false;
							break;
						default:	// should never be here
							return false;
					}
				} else {
					switch (status) {
						case 0xF0:	// system exclusive
							_issysex = true;
							if (_buffer[_index - 1] == 0xF7) {
								_issysex = false; // more or less impossible, but let's cover it
								return true;
							}
							return false;
						case 0xF7:
							//TODO C74_ASSERT(0); // something went wong, bad eox
							_index = 0;
							return false;
						case 0xF1:	// undefined
							return true;
						case 0xF2:	// song position
							if (_index == 3)
								return true;
							else
								return false;
						case 0xF3:	// song select
							if (_index == 2)
								return true;
							else
								return false;
						case 0xF4:
						case 0xF5:
						case 0xF6:
						case 0xF8:
						case 0xF9:
						case 0xFA:
						case 0xFB:
						case 0xFC:
						case 0xFD:
						case 0xFE:
						case 0xFF:
							return true;
						default:
							return false;
					}
				}
				return false;
			}
			uint8_t _buffer[max_length];
			size_t _index = 0;
			bool _issysex = false;
	};
}

#ifdef C74_UNIT_TESTS
#include <JuceHeader.h>
#include <vector>
#include <deque>
namespace MidiStreamParserTests {
	using namespace RNBO;
	class Tests : public UnitTest {
		public:
			Tests()
				: UnitTest("MidiStreamParser Tests")
			{}
			void runTest() override {
				beginTest ("Basic");

				std::deque<std::vector<int>> msg; //int requires less casting in expectEquals
				auto fill = [&msg](const uint8_t * b, size_t c) {
					std::vector<int> m;
					for (size_t i = 0; i < c; i++) {
						m.push_back(static_cast<int>(b[i]));
					}
					msg.push_back(m);
				};
				MidiStreamParser parser;

				auto popf = [&msg]() -> std::vector<int> {
					if (msg.size() == 0) {
						throw new std::runtime_error("no message");
					}
					auto m = msg.front();
					msg.pop_front();
					return m;
				};

				//note on
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				//note on with realtime message
				msg.clear();
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0xF8, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m[0], 0xF8);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				// program change
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

				beginTest ("Running Status");
				msg.clear();

				//note on
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x4, fill);
				expectEquals(msg.size(), (size_t)2, "running status note on");

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x3);
					expectEquals(m[2], 0x4);
				}

				//CC
				parser.process(0xB2, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x11, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x12, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x13, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x14, fill);
				expectEquals(msg.size(), (size_t)2, "running status CC");

				{
					auto m = popf();
					expectEquals(m[0], 0xB2);
					expectEquals(m[1], 0x11);
					expectEquals(m[2], 0x12);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0xB2);
					expectEquals(m[1], 0x13);
					expectEquals(m[2], 0x14);
				}

				//error recovery
				parser.process(0xB2, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x11, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x12, fill);
				expectEquals(msg.size(), (size_t)1);
				//partial cc message, then note
				parser.process(0x14, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m[0], 0xB2);
					expectEquals(m[1], 0x11);
					expectEquals(m[2], 0x12);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				//program change
				msg.clear();
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)2, "program change");

				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x2);
				}


				beginTest ("Sysex");

				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)1);
					expectEquals(m[0], 0xF7);
				}

				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)2);
					expectEquals(m[0], 0x3);
					expectEquals(m[1], 0xF7);
				}

				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x4, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0x3);
					expectEquals(m[1], 0x4);
					expectEquals(m[2], 0xF7);
				}

				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x4, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x5, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)3);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0x3);
					expectEquals(m[1], 0x4);
					expectEquals(m[2], 0x5);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)1);
					expectEquals(m[0], 0xF7);
				}


				beginTest ("Sysex with Realtime");
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0xF8, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0x4, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)3);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)1);
					expectEquals(m[0], 0xF8);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0x3);
					expectEquals(m[1], 0x4);
					expectEquals(m[2], 0xF7);
				}

				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0xFA, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0x3, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0x4, fill);
				expectEquals(msg.size(), (size_t)2);
				parser.process(0xF7, fill);
				expectEquals(msg.size(), (size_t)3);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)1);
					expectEquals(m[0], 0xFA);
				}
				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0x3);
					expectEquals(m[1], 0x4);
					expectEquals(m[2], 0xF7);
				}

				//note on after sysex
				msg.clear();
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				beginTest ("Sysex error Recovery");

				//two sysex bytes and then note
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				//partial sysex message and then note
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x91, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0x91);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				// program change after sysex
				msg.clear();
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);
				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

				//1 sysex byte and then program change, sysex should be ignored
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);

				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

				//two sysex bytes and then program change, sysex should be ignored
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)1);

				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

				//3 sysex bytes and then program change, should get 1 sysex message
				msg.clear();
				parser.process(0xF0, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)0);
				parser.process(0x2, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0xC0, fill);
				expectEquals(msg.size(), (size_t)1);
				parser.process(0x1, fill);
				expectEquals(msg.size(), (size_t)2);

				{
					auto m = popf();
					expectEquals(m.size(), (size_t)3);
					expectEquals(m[0], 0xF0);
					expectEquals(m[1], 0x1);
					expectEquals(m[2], 0x2);
				}

				{
					auto m = popf();
					expectEquals(m[0], 0xC0);
					expectEquals(m[1], 0x1);
				}

			}
	};

	Tests tests;
}
#endif

#endif
