//
// Created by valentin.ackva on 22.06.2023.
//

#ifndef GUI_APP_EXAMPLE_OPERATINGSYSTEM_H
#define GUI_APP_EXAMPLE_OPERATINGSYSTEM_H

struct OperatingSystem {
    enum SystemType {
        Windows_x64,
        MacOS_x64,
        MacOS_arm64,
        Unknown
    };

    static SystemType getOperatingSystem() {
#if JUCE_MAC
        #if JUCE_64BIT
            return MacOS_x64;
        #else
            return MacOS_arm64;
        #endif
#elif JUCE_WINDOWS
        return Windows_x64;
#else
        return Unknown;
#endif
    }
};



#endif //GUI_APP_EXAMPLE_OPERATINGSYSTEM_H
