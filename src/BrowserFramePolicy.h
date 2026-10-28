#pragma once

// Platform-free scheduling policy; DOM observation belongs to the browser adapter.
class BrowserFramePolicy
{
public:
    struct Decision
    {
        bool tick;
        bool reset;
    };

    Decision NextFrame(double now, bool hidden, bool focused, bool activityChanged)
    {
        const bool active = !hidden && focused;
        const bool reset = activityChanged || !wasActive || !active ||
            (previousTime >= 0.0 && now - previousTime > 250.0);
        previousTime = now;
        wasActive = active;
        return {active, reset};
    }

private:
    double previousTime = -1.0;
    bool wasActive = false;
};
