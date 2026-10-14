#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace mirage
{
    // Circular delay buffer with 4-point cubic Hermite interpolation.
    // Call read() BEFORE write() each sample: read(d) then returns the input from exactly d samples ago.
    class DelayLine
    {
    public:
        void prepare (int minSamples)
        {
            int size = 1;
            while (size < minSamples) size <<= 1;
            buf.assign ((size_t) size, 0.0f);
            mask = size - 1;
            widx = 0;
        }

        int capacity() const { return mask + 1; }
        void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }

        void write (float x)
        {
            buf[(size_t) widx] = x;
            widx = (widx + 1) & mask;
        }

        // delay in samples, valid range [2, capacity - 4]
        float read (float delay) const
        {
            delay = std::min (std::max (delay, 2.0f), (float) (mask - 4));
            const int di = (int) delay;
            const float frac = delay - (float) di;
            const int b = widx - di;                       // index of the sample `di` back
            const float t = 1.0f - frac;                   // position between b-1 and b
            const float ym1 = buf[(size_t) ((b - 2) & mask)];
            const float y0  = buf[(size_t) ((b - 1) & mask)];
            const float y1  = buf[(size_t) (b & mask)];
            const float y2  = buf[(size_t) ((b + 1) & mask)];
            const float c1 = 0.5f * (y1 - ym1);
            const float c2 = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
            const float c3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
            return ((c3 * t + c2) * t + c1) * t + y0;
        }

    private:
        std::vector<float> buf;
        int mask = 0, widx = 0;
    };
}
