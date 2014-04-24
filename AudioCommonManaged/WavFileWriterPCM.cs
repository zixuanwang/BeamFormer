using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace AudioCommonManaged
{
    // this is a copy of the mono file writer. That class needs to subclass from this
    // and set the number of channels via a class variable.
    public class WavFileWriterPCM : IWavFileWriter
    {
        const short kRiffFormatPCM = 1;
        const short kDefaultNumChannels = 1;
        const short kNumBytesPerSample = 2;

        int mNumDataBytes;
        int mStreamPositionTotalBytes;
        int mStreamPositionDataBytes;

        short mNumChannels = 2;
        MemoryStream mWavData;
        BinaryWriter mWavWriter;

        /// <summary>
        /// Private constructor
        /// </summary>
        /// <param name="numSamplesPerSec"># of samples per sec</param>
        /// <param name="numChannels"></param>
        protected WavFileWriterPCM(int numSamplesPerSec, short numChannels)
        {
            mNumChannels = numChannels;
            if (numSamplesPerSec < 0)
            {
                throw new ArgumentOutOfRangeException("Sampling rate must be positive");
            }
            mWavData = new MemoryStream();
            mWavWriter = new BinaryWriter(mWavData);
            mNumDataBytes = 0;

            short shortVal;
            int intVal;

            // put wav header into the stream

            mWavWriter.Write("RIFF".ToCharArray(), 0, 4);

            mStreamPositionTotalBytes = (int)mWavData.Position;

            intVal = 0;   // total number of bytes
            mWavWriter.Write(intVal);  // will be overwritten when close

            mWavWriter.Write("WAVE".ToCharArray(), 0, 4);
            mWavWriter.Write("fmt ".ToCharArray(), 0, 4);

            intVal = 16;  // header size
            mWavWriter.Write(intVal);
            mWavWriter.Write(kRiffFormatPCM);
            mWavWriter.Write(mNumChannels);
            mWavWriter.Write(numSamplesPerSec);

            intVal = numSamplesPerSec * mNumChannels * kNumBytesPerSample;  // Avg. bytes per second 
            mWavWriter.Write(intVal);

            shortVal = (short)(mNumChannels * kNumBytesPerSample);  // Block align
            mWavWriter.Write(shortVal);

            shortVal = kNumBytesPerSample * 8;  // Bits per sample
            mWavWriter.Write(shortVal);

            mWavWriter.Write("data".ToCharArray(), 0, 4);

            mStreamPositionDataBytes = (int)mWavData.Position;

            intVal = 0;  // number of data bytes
            mWavWriter.Write(intVal); // will be overwritten when close
        }

        /// <summary>
        /// Static creator
        /// </summary>
        /// <param name="numSamplesPerSec"># of samples per sec</param>
        /// <returns></returns>
        public static IWavFileWriter Create(int numSamplesPerSec)
        {
            return new WavFileWriterPCM(numSamplesPerSec, kDefaultNumChannels) as IWavFileWriter;
        }

        /// <summary>
        /// Static creator
        /// </summary>
        /// <param name="numSamplesPerSec"># of samples per sec</param>
        /// <returns></returns>
        public static IWavFileWriter Create(int numSamplesPerSec, short numChannels)
        {
            return new WavFileWriterPCM(numSamplesPerSec, numChannels) as IWavFileWriter;
        }

        /// <summary>
        /// Update wav header with the current data size
        /// </summary>
        void UpdateWavHeader()
        {
            int numBytes;

            mWavWriter.Seek(mStreamPositionTotalBytes, SeekOrigin.Begin);
            numBytes = mNumDataBytes + (mStreamPositionDataBytes - mStreamPositionTotalBytes);//8 + 16 + 12;
            mWavWriter.Write(numBytes);  // total number of bytes

            mWavWriter.Seek(mStreamPositionDataBytes, SeekOrigin.Begin);
            numBytes = mNumDataBytes;  // number of data bytes
            mWavWriter.Write(numBytes);

            // ready to accept new data
            mWavWriter.Seek(0, SeekOrigin.End);
        }

        /// <summary>
        /// Put data samples into stream
        /// </summary>
        /// <param name="data">data in byte</param>
        public void PutSamples(byte[] data)
        {
            if (data == null)
            {
                throw new ArgumentNullException("Must supply input data");
            }
            mWavWriter.Write(data);
            mNumDataBytes += data.Length;
        }

        /// <summary>
        /// Put data samples into stream
        /// </summary>
        /// <param name="data">data in short</param>
        public void PutSamples(short[] data)
        {
            if (data == null)
            {
                throw new ArgumentNullException("Must supply input data");
            }
            foreach (short d in data)
            {
                mWavWriter.Write(d);
            }
            mNumDataBytes += data.Length * 2;
        }

        /// <summary>
        /// Put data samples into stream
        /// (data is clipped if it's outside of short range)
        /// </summary>
        /// <param name="data">data in int</param>
        public void PutSamples(int[] data)
        {
            if (data == null)
            {
                throw new ArgumentNullException("Must supply input data");
            }
            foreach (int d in data)
            {
                short dd;
                if (d > short.MaxValue)
                {
                    dd = short.MaxValue;
                }
                else if (d < short.MinValue)
                {
                    dd = short.MinValue;
                }
                else
                {
                    dd = (short)d;
                }
                mWavWriter.Write(dd);
            }
            mNumDataBytes += data.Length * 2;
        }

        /// <summary>
        /// Put data samples into stream
        /// (data is clipped if it's outside of short range)
        /// </summary>
        /// <param name="data">data in float</param>
        public void PutSamples(float[] data)
        {
            if (data == null)
            {
                throw new ArgumentNullException("Must supply input data");
            }
            foreach (float d in data)
            {
                short dd;
                if (d > short.MaxValue)
                {
                    dd = short.MaxValue;
                }
                else if (d < short.MinValue)
                {
                    dd = short.MinValue;
                }
                else
                {
                    dd = Convert.ToInt16(d);
                }
                mWavWriter.Write(dd);
            }
            mNumDataBytes += data.Length * 2;
        }

        /// <summary>
        /// Return entire byte data in stream
        /// </summary>
        /// <returns>byte data</returns>
        public byte[] ToArray()
        {
            UpdateWavHeader();
            return mWavData.ToArray();
        }

        /// <summary>
        /// Write wav data into BinaryWriter
        /// </summary>
        /// <param name="writer">BinaryWriter</param>
        public void Write(BinaryWriter writer)
        {
            if (writer == null)
            {
                throw new ArgumentNullException("Must supply writer to write wav data");
            }
            UpdateWavHeader();
            writer.Write(ToArray());
        }

        /// <summary>
        /// Write wav data into file
        /// </summary>
        /// <param name="filename">file name</param>
        public void Write(string filename)
        {
            if (String.IsNullOrWhiteSpace(filename))
            {
                throw new ArgumentNullException("Must supply filename to write wav data");
            }
            using (FileStream fs = new FileStream(filename, FileMode.Create))
            {
                using (BinaryWriter writer = new BinaryWriter(fs))
                {
                    Write(writer);
                }
            }
        }

        /// <summary>
        /// Dispose resources
        /// </summary>
        public void Dispose()
        {
            mWavWriter.Dispose();
            mWavData.Dispose();
        }
    }

}
