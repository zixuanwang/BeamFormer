#pragma once

namespace AudioCaptureInterop
{
    public delegate void DataReadyCallback(array<System::Byte>^ inputBuf,
                                           int sizeBuf);

    public interface class IAudioCaptureInterop
    {
        void Start();
        void Stop();
        property bool IsCapturing { bool get(); }
    };
}
