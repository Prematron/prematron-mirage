#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace mirage
{
    // Schroeder allpass: y[n] = -g*x[n] + w[n-N], w[n] = x[n] + g*w[n-N]
    class Allpass
    {
    public:
        void prepare (int length) { len = std::max (1, length); buf.assign ((size_t) len, 0.0f); idx = 0; }
        int length() const { return len; }
        float process (float x, float g)
        {
            const float d = buf[(size_t) idx];
            const float w = x + g * d;
            buf[(size_t) idx] = w;
            if (++idx >= len) idx = 0;
            return d - g * w;
        }
    private:
        std::vector<float> buf;
        int len = 1, idx = 0;
    };

    // Octave-up pitch shifter: two crossfaded read heads sweeping through a short window.
    // Used inside the feedback loop, so repeats climb in octaves for the shimmer effect.
    class OctaveShifter
    {
    public:
        void prepare (double sampleRate)
        {
            window = std::max (64, (int) (0.04 * sampleRate));
            int size = 1;
            while (size < 2 * window + 8) size <<= 1;
            buf.assign ((size_t) size, 0.0f);
            mask = size - 1;
            widx = 0;
            phase = 0.0f;
        }

        float process (float x)
        {
            buf[(size_t) widx] = x;
            float out = 0.0f;
            for (int h = 0; h < 2; ++h)
            {
                float p = phase + 0.5f * (float) h;
                if (p >= 1.0f) p -= 1.0f;
                const float delay = (1.0f - p) * (float) window + 1.0f;
                const int di = (int) delay;
                const float frac = delay - (float) di;
                const float a = buf[(size_t) ((widx - di) & mask)];
                const float b = buf[(size_t) ((widx - di - 1) & mask)];
                const float s = a + (b - a) * frac;
                const float sn = std::sin (3.14159265f * p);
                out += sn * sn * s;                       // Hann crossfade (amplitude complementary)
            }
            widx = (widx + 1) & mask;
            phase += 1.0f / (float) window;
            if (phase >= 1.0f) phase -= 1.0f;
            return out;
        }

    private:
        std::vector<float> buf;
        int window = 64, mask = 0, widx = 0;
        float phase = 0.0f;
    };
}
