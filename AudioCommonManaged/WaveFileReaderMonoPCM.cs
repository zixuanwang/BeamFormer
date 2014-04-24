using System;
using System.Linq;
using System.IO;

namespace AudioCommonManaged
{
    public class WavFileReaderMonoPCM : IWavFileReader
    {
        const ushort kRiffFromatPCM = 0x0001;

        int mNumChannels;
        int mNumSamplesPerSec;
        int mNumBytesPerSample;
        int mNumBytesPerSec;
        int mBlockAlign;
        byte[] dataBuf;

        /// <summary>
        /// Get the number of samples per second
        /// </summary>
        public int NumSamplesPerSec
        {
            get { return mNumSamplesPerSec; }
        }

        /// <summary>
        /// Get the number of channels
        /// </summary>
        public int NumChannels
        {
            get { return mNumChannels; }
        }

        /// <summary>
        /// Get the number of bytes per sample
        /// </summary>
        public int NumBytesPerSample
        {
            get { return mNumBytesPerSample; }
        }

        /// <summary>
        /// Get the number of bytes per second
        /// </summary>
        public int NumBytesPerSec
        {
            get { return mNumBytesPerSec; }
        }

        /// <summary>
        /// Get block alignment
        /// </summary>
        public int BlockAlign
        {
            get { return mBlockAlign; }
        }

        /// <summary>
        /// Get the number of samples in file
        /// </summary>
        public int NumSamplesInFile
        {
            get { return (dataBuf.Length / mNumChannels / 2); }
        }

        /// <summary>
        /// Private constructor of WavFileReaderMonoPCM
        /// </summary>
        /// <param name="readerWav">binary reader</param>
        private WavFileReaderMonoPCM(BinaryReader readerWav)
        {
            try
            {
                string chunkName;

                chunkName = new string(readerWav.ReadChars(4));
                if (!chunkName.Equals("RIFF"))
                {
                    throw new WavFileException("Invalid wav format");
                }

                long fileSize = (long)readerWav.ReadUInt32();

                chunkName = new string(readerWav.ReadChars(4));
                if (!chunkName.Equals("WAVE"))
                {
                    throw new WavFileException("Invalid wav format");
                }

                uint formatTag = 0;
                ushort numChannels = 0;
                uint numSamplesPerSec = 0;
                ushort numBitsPerSample = 0;
                uint numBytesPerSec = 0;
                ushort blockAlign = 0;

                chunkName = new string(readerWav.ReadChars(4));
                while (!chunkName.Equals("data"))
                {
                    if (chunkName.Equals("fmt "))
                    {
                        long chunkPosition = readerWav.BaseStream.Position;
                        long nextChunkOffset = (long)readerWav.ReadUInt32();
                        formatTag = readerWav.ReadUInt16();
                        numChannels = readerWav.ReadUInt16();
                        numSamplesPerSec = readerWav.ReadUInt32();
                        numBytesPerSec = readerWav.ReadUInt32();
                        blockAlign = readerWav.ReadUInt16();
                        numBitsPerSample = readerWav.ReadUInt16();
                        readerWav.BaseStream.Seek(chunkPosition, SeekOrigin.Begin);
                    }

                    long nextOffset = (long)readerWav.ReadUInt32();
                    readerWav.BaseStream.Seek(nextOffset, SeekOrigin.Current);

                    chunkName = new string(readerWav.ReadChars(4));
                }

                if (formatTag != kRiffFromatPCM)
                {
                    throw new WavFileException("Can support PCM format only");
                }
                if (numBitsPerSample != 16)
                {
                    throw new WavFileException("Can support only 16-bit per sample");
                }
                if (numChannels != 1)
                {
                    throw new WavFileException("Can support only mono channel");
                }

                mNumSamplesPerSec = Convert.ToInt32(numSamplesPerSec);
                mNumChannels = Convert.ToInt32(numChannels);
                mNumBytesPerSample = Convert.ToInt32(numBitsPerSample / 8);
                mNumBytesPerSec = Convert.ToInt32(numBytesPerSec);
                mBlockAlign = Convert.ToInt32(blockAlign);

                uint dwChunkSize = readerWav.ReadUInt32();
                dataBuf = new byte[dwChunkSize];
                readerWav.Read(dataBuf, 0, (int)dwChunkSize);
            }
            catch (EndOfStreamException ex)
            {
                // EndOfStreamException is mapped to WavFileException,
                // since it 
                throw new WavFileException("Invalid wav file: " + ex.Message);
            }
        }

        /// <summary>
        /// Creates a wave file reader from a byte array
        /// </summary>
        /// <param name="bytes">The byte array.</param>
        /// <returns>a new IWavFileReader</returns>
        public static IWavFileReader CreateFromBytes(byte[] bytes)
        {
            using (BinaryReader readerWav = new BinaryReader(new MemoryStream(bytes)))
            {
                return new WavFileReaderMonoPCM(readerWav);
            }
        }

        /// <summary>
        /// Creates a wave file reader from filename.
        /// </summary>
        /// <param name="fileName">the filename.</param>
        /// <returns>a new IWavFileReader</returns>
        public static IWavFileReader CreateFromFile(string fileName)
        {
            using (BinaryReader readerWav = new BinaryReader(new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read)))
            {
                return new WavFileReaderMonoPCM(readerWav);
            }
        }

        /// <summary>
        /// Return byte buffer of entire waveform data
        /// </summary>
        /// <returns>byte array with waveform data</returns>
        public byte[] GetSamplesInByte()
        {
            return dataBuf.ToArray();
        }

        /// <summary>
        /// Return byte buffer with a specified start position and the number of samples
        /// </summary>
        /// <param name="posStart">start position in sample</param>
        /// <param name="numSamples">number of samples</param>
        /// <returns>byte array with waveform data</returns>
        public byte[] GetSamplesInByte(int posStart, int numSamples)
        {
            byte[] outBuffer = new byte[Math.Min(numSamples * 2, dataBuf.Length - posStart * 2)];
            Array.ConstrainedCopy(dataBuf, posStart * 2, outBuffer, 0, outBuffer.Length);
            return outBuffer;
        }

        /// <summary>
        /// Return short buffer of entire waveform data
        /// </summary>
        /// <returns>short array with waveform data</returns>
        public short[] GetSamplesInShort()
        {
            short[] outBuffer = new short[dataBuf.Length / 2];
            for (int i = 0; i < dataBuf.Length / 2; i++)
            {
                outBuffer[i] = EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }

        /// <summary>
        /// Return short buffer with a specified start position and the number of samples.
        /// </summary>
        /// <param name="posStart">start position</param>
        /// <param name="numSamples">number of samples</param>
        /// <returns>short array with waveform data</returns>
        public short[] GetSamplesInShort(int posStart, int numSamples)
        {
            short[] outBuffer = new short[Math.Min(numSamples, dataBuf.Length / 2 - posStart)];
            for (int i = posStart; i < Math.Min(dataBuf.Length / 2, posStart + numSamples); i++)
            {
                outBuffer[i - posStart] = EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }

        /// <summary>
        /// Return short buffer of entire waveform data
        /// </summary>
        /// <returns>int array with waveform data</returns>
        public int[] GetSamplesInInt()
        {
            int[] outBuffer = new int[dataBuf.Length / 2];
            for (int i = 0; i < dataBuf.Length / 2; i++)
            {
                outBuffer[i] = (int)EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }

        /// <summary>
        /// Return short buffer with a specified start position and the number of samples.
        /// </summary>
        /// <param name="posStart">start position</param>
        /// <param name="numSamples">number of samples</param>
        /// <returns>int array with waveform data</returns>
        public int[] GetSamplesInInt(int posStart, int numSamples)
        {
            int[] outBuffer = new int[Math.Min(numSamples, dataBuf.Length / 2 - posStart)];
            for (int i = posStart; i < Math.Min(dataBuf.Length / 2, posStart + numSamples); i++)
            {
                outBuffer[i - posStart] = (int)EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }

        /// <summary>
        /// Return float buffer of entire waveform data
        /// </summary>
        /// <returns>float array with waveform data</returns>
        public float[] GetSamplesInFloat()
        {
            float[] outBuffer = new float[dataBuf.Length / 2];
            for (int i = 0; i < dataBuf.Length / 2; i++)
            {
                outBuffer[i] = (float)EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }

        /// <summary>
        /// Return short buffer with a specified start position and the number of samples.
        /// </summary>
        /// <param name="posStart">start position</param>
        /// <param name="numSamples">number of samples</param>
        /// <returns>float array with waveform data</returns>
        public float[] GetSamplesInFloat(int posStart, int numSamples)
        {
            float[] outBuffer = new float[Math.Min(numSamples, dataBuf.Length / 2 - posStart)];
            for (int i = posStart; i < Math.Min(dataBuf.Length / 2, posStart + numSamples); i++)
            {
                outBuffer[i - posStart] = (float)EndianBitConverter.ToInt16(dataBuf, i * 2, true);
            }
            return outBuffer;
        }
    }
}
