#pragma once
extern "C" {
    typedef char* (__cdecl* TaskStartApiFunc)(const char* param);
    typedef char* (__cdecl* TaskStateApiFunc)(const char* param);
    typedef char* (__cdecl* TaskStopApiFunc)(const char* param);
    typedef char* (__cdecl* TaskMergeApiFunc)(const char* param);
}
