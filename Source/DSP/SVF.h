#pragma once
#include <cmath>

namespace mirage
{
    // Zero-delay-feedback state-variable filter (Simper / Cytomic topology).
    // Stable under fast cutoff modulation; resonance can reach self-oscillation as k -> 0.
    class SVF
    {
    public:
        // g = tan(pi * fc / fs); k = 1 / Q  (1.414 = Butterworth, ~0.02 = near self-oscillation)
        void set (float g, float k_)
        {
            k = k_;
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }

        void reset() { ic1 = ic2 = 0.0f; }

        float lowpass (float v0)  { return run (v0, 0); }
        float highpass (float v0) { return run (v0, 1); }

    private:
        float run (float v0, int which)
        {
            const float v3 = v0 - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            return which == 0 ? v2 : v0 - k * v1 - v2;
        }

        float k = 1.414f, a1 = 1, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
    };
}
