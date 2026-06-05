#pragma once

#include <string>

class GenTaskThread
{
public:
    static void Run();

    static bool CancelGenTask(const std::string& task_uuid);

    static void SearchUnnormalRunningJob();

private:

    static void ProcessPendingJobs();

    static void ProcessRunningJobs();

    static void SleepMs(int ms);
};
