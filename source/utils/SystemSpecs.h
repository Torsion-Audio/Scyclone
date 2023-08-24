//
// Created by schee on 22/08/2023.
//

#ifndef SCYCLONE_SYSTEMSPECS_H
#define SCYCLONE_SYSTEMSPECS_H
#include "JuceHeader.h"

class SystemSpecs {
    public:
        SystemSpecs();
        ~SystemSpecs();

        double getCPULoad();
        double calculateCPULoad();
    private:
        juce::SystemStats::OperatingSystemType os;
};




#endif //SCYCLONE_SYSTEMSPECS_H
