//
//  utils.c
//  Compressor
//
//  Created by Fares Schulz on 23.12.22.
//

#include "utils.h"

float utils::amp2dB(float amp){
    return 20*std::log10(amp);
}

float utils::amp2dB(float amp, float ampRef){
    return 20*std::log10(amp/ampRef);
}

float utils::dB2amp(float db){
    return std::pow(10.f, (db/20.f));
}

float utils::dB2amp(float db, float ampRef){
    return std::pow(10.f, (db/20.f))*ampRef;
}
