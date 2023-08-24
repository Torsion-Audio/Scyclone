//
// Created by schee on 22/08/2023.
//

#include "SystemSpecs.h"

#if JUCE_WINDOWS
    #include "windows.h"

    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors;
    static HANDLE self;

    SystemSpecs::SystemSpecs() {
        SYSTEM_INFO sysInfo;
        FILETIME ftime, fsys, fuser;

        GetSystemInfo(&sysInfo);
        numProcessors = sysInfo.dwNumberOfProcessors;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));

        self = GetCurrentProcess();
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
    }
    SystemSpecs::~SystemSpecs() {}

    double SystemSpecs::getCPULoad() {

        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;
        double percent;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));

        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));
        percent = (sys.QuadPart - lastSysCPU.QuadPart) +
                  (user.QuadPart - lastUserCPU.QuadPart);
        percent /= (now.QuadPart - lastCPU.QuadPart);
        percent /= numProcessors;
        lastCPU = now;
        lastUserCPU = user;
        lastSysCPU = sys;

        return percent * 100;
    }

#elif JUCE_LINUX
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "sys/times.h"
    #include "sys/vtimes.h"

    static clock_t lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors;

    SystemSpecs::SystemSpecs(){
        FILE* file;
        struct tms timeSample;
        char line[128];

        lastCPU = times(&timeSample);
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;

        file = fopen("/proc/cpuinfo", "r");
        numProcessors = 0;
        while(fgets(line, 128, file) != NULL){
            if (strncmp(line, "processor", 9) == 0) numProcessors++;
        }
        fclose(file);
    }

    double SystemSpecs::getCPULoad(){
        struct tms timeSample;
        clock_t now;
        double percent;

        now = times(&timeSample);
        if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
            timeSample.tms_utime < lastUserCPU){
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else{
            percent = (timeSample.tms_stime - lastSysCPU) +
                (timeSample.tms_utime - lastUserCPU);
            percent /= (now - lastCPU);
            percent /= numProcessors;
            percent *= 100;
        }
        lastCPU = now;
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;

        return percent;
    }

#elif JUCE_MAC
    #include <mach/mach_init.h>
    #include <mach/mach_error.h>
    #include <mach/mach_host.h>
    #include <mach/vm_map.h>

    static unsigned long long _previousTotalTicks = 0;
    static unsigned long long _previousIdleTicks = 0;

    SystemSpecs::getCPULoad() {
       host_cpu_load_info_data_t cpuinfo;
       mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
       if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS)
       {
          unsigned long long totalTicks = 0;
          for(int i=0; i<CPU_STATE_MAX; i++) totalTicks += cpuinfo.cpu_ticks[i];
          return calculateCPULoad(cpuinfo.cpu_ticks[CPU_STATE_IDLE], totalTicks);
       }
        else return -1.0f;
    }

    SystemSpecs::calculateCPULoad() {
        unsigned long long totalTicksSinceLastTime = totalTicks-_previousTotalTicks;
        unsigned long long idleTicksSinceLastTime  = idleTicks-_previousIdleTicks;
        float ret = 1.0f-((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime)/totalTicksSinceLastTime : 0);
        _previousTotalTicks = totalTicks;
        _previousIdleTicks  = idleTicks;
        return ret;
    }
#endif


