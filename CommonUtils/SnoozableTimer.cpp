#include "SnoozableTimer.h"

SnoozableTimer::SnoozableTimer(std::function<void ()> ahFunciont, int anSnoozePeriodMs)
    : mhFunction(ahFunciont)
    , mnSnoozePeriodMs(anSnoozePeriodMs)
    , mbIsRunning(false)
{

}

SnoozableTimer::~SnoozableTimer()
{
    stop();
}

void SnoozableTimer::start()
{
    if (mbIsRunning) return;

    mcExecutionTime = std::chrono::high_resolution_clock::now() +
                      std::chrono::milliseconds(mnSnoozePeriodMs);

    mbIsRunning = true;
    mcThread = std::thread([this]()
    {
        std::unique_lock<std::mutex> lock(mcMutex);

        while (mbIsRunning)
        {
            // Wait until execution time or until notified
            if (mcCv.wait_until(lock, mcExecutionTime) == std::cv_status::timeout && mbIsRunning)
            {
                lock.unlock();
                mhFunction();
                lock.lock();
                // This should prevent reexecution until we hit the snooze!
                mcExecutionTime = std::chrono::high_resolution_clock::now() +
                                  std::chrono::hours(87600);
            }
        }
    });
}

void SnoozableTimer::stop()
{
    {
        std::lock_guard<std::mutex> lock(mcMutex);
        mbIsRunning = false;
    }

    mcCv.notify_all();

    if (mcThread.joinable())
    {
        mcThread.join();
    }
}

void SnoozableTimer::snooze()
{
    std::lock_guard<std::mutex> lock(mcMutex);
    auto lcTimeNow = std::chrono::high_resolution_clock::now() +
            std::chrono::milliseconds(mnSnoozePeriodMs);
    mcExecutionTime = lcTimeNow;
    mcCv.notify_all(); // Notify to re-evaluate the wait condition
}

void SnoozableTimer::updateSnoozePeriod(int anSnoozePeriodMs)
{
    {
        std::lock_guard<std::mutex> lock(mcMutex);
        mnSnoozePeriodMs = anSnoozePeriodMs;
    }
    snooze();
}
