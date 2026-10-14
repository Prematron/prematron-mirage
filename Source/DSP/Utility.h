#pragma once
#include <cmath>

namespace mirage
{
    // Linear below the threshold, then a smooth tanh knee up to +/-1.
    // Quiet loops stay perfectly clean (and freeze holds at unity), hot loops compress instead of clipping.
    inline float softClip (float x)
    {
        constexpr float t = 0.5f;
        const float a = std::fabs (x);
        if (a <= t) return x;
        const float s = t + (1.0f - t) * std::tanh ((a - t) / (1.0f - t));
        return x < 0.0f ? -s : s;
    }

    class DcBlocker
    {
    public:
        void prepare (double sampleRate) { r = 1.0f - 6.2831853f * 5.0f / (float) sampleRate; x1 = y1 = 0.0f; }
        float process (float x)
        {
            const float y = x - x1 + r * y1;
            x1 = x; y1 = y;
            return y;
        }
    private:
        float r = 0.999f, x1 = 0.0f, y1 = 0.0f;
    };
}
