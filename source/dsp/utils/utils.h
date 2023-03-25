//
//  utils.h
//  Compressor_Plugin
//
//  Created by Fares Schulz on 23.12.22.
//

#ifndef utils_h
#define utils_h

#include <cmath>

namespace utils {
    float amp2dB(float amp);
    float amp2dB(float amp, float ampRef);
    float dB2amp(float db);
    float dB2amp(float db, float ampRef);
}

#endif /* utils_h */
