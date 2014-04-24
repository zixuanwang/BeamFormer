using System;
using System.IO;

namespace AudioCommonManaged
{
    public interface IWavFileWriter : IDisposable
    {
        void PutSamples(byte[] data);
        void PutSamples(short[] data);
        void PutSamples(int[] data);
        void PutSamples(float[] data);

        byte[] ToArray();
        void Write(BinaryWriter writer);
        void Write(string filename);
    }
}
