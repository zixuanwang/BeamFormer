#pragma once

namespace AudioCaptureInterop
{
    public ref struct AudioWaveFormatInterop
    {
    public:
        AudioWaveFormatInterop(unsigned long samplesPerSec,
                               unsigned short channels)
        {
            nSamplesPerSec = samplesPerSec;
            nChannels = channels;

            wFormatTag = 1; //PCM
            wBitsPerSample = 16;
            nBlockAlign = (wBitsPerSample / 8) * nChannels;
            nAvgBytesPerSec = nBlockAlign * nSamplesPerSec;
            cbSize = 0;
        }

        unsigned short wFormatTag;
        unsigned short nChannels;
        unsigned long nSamplesPerSec;
        unsigned long nAvgBytesPerSec;
        unsigned short nBlockAlign;
        unsigned short wBitsPerSample;
        unsigned short cbSize;
     };
}
