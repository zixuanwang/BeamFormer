using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AudioCommonManaged
{
    public interface IWavFileReader
    {
        int NumSamplesPerSec { get; }
        int NumChannels { get; }
        int NumBytesPerSample { get; }
        int NumBytesPerSec { get; }
        int BlockAlign { get; }
        int NumSamplesInFile { get; }

        byte[] GetSamplesInByte();
        byte[] GetSamplesInByte(int posStart, int numSamples);
        short[] GetSamplesInShort();
        short[] GetSamplesInShort(int poStart, int numSamples);
        int[] GetSamplesInInt();
        int[] GetSamplesInInt(int poStart, int numSamples);
        float[] GetSamplesInFloat();
        float[] GetSamplesInFloat(int poStart, int numSamples);
    }
}
