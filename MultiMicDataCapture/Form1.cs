using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

using AudioCaptureInterop;
using AudioCommonManaged;

namespace MultiMicDataCapture
{
    public partial class Form1 : Form
    {
        const int kNumSamplesPerSec = 16000;
        const int kNumChannels = 1;
        const string kOutputFolder = @"C:\Users\danwa\Desktop";
        bool IsBackground = false;

        Dictionary<string, string> mMicDeviceToKinectId = new Dictionary<string, string>()
        {
            //{"Microphone Array (Kinect for Windows USB Audio)", "USB\\VID_045E&PID_02C3&MI_02\\8&CAD9D38&0&0002"},
            //{"Microphone Array (2- Kinect for Windows USB Audio)", "USB\\VID_045E&PID_02C3&MI_02\\8&35ECEA53&0&0002"},
            {"Microphone Array (3- Kinect for Windows USB Audio)", "USB\\VID_045E&PID_02C3&MI_02\\8&16880585&0&0002"},
            //{"Microphone Array (4- Kinect for Windows USB Audio)", "USB\\VID_045E&PID_02C3&MI_02\\8&2FD49A23&0&0002"},
        };


        Dictionary<string, string> mMicDeviceToFilename = new Dictionary<string, string>()
        {
            //{"Microphone Array (Kinect for Windows USB Audio)", "Raw1.wav"},
            //{"Microphone Array (2- Kinect for Windows USB Audio)", "Raw2.wav"},
            {"Microphone Array (3- Kinect for Windows USB Audio)", "Raw3.wav"},
            //{"Microphone Array (4- Kinect for Windows USB Audio)", "Raw4.wav"},
        };

        Dictionary<string, string> mMicArrayDeviceToFilename = new Dictionary<string, string>()
        {
            //{"USB\\VID_045E&PID_02C3&MI_02\\8&CAD9D38&0&0002", "Array1.wav"},
            //{"USB\\VID_045E&PID_02C3&MI_02\\8&35ECEA53&0&0002", "Array2.wav"},
            {"USB\\VID_045E&PID_02C3&MI_02\\8&16880585&0&0002", "Array3.wav"},
            //{"USB\\VID_045E&PID_02C3&MI_02\\8&2FD49A23&0&0002", "Array4.wav"},
        };

        List<RawCapture> mRawCaptures;
        List<ArrayCapture> mArrayCaptures;
        double[] mArrayBackgroundEnergy = { 0.0, 0.0, 0.0, 0.0 };
        double[] mRawBackgroundEnergy = { 0.0, 0.0, 0.0, 0.0 };
        double[] mArraySignalEnergy = { 0.0, 0.0, 0.0, 0.0 };
        double[] mRawSignalEnergy = { 0.0, 0.0, 0.0, 0.0 };

        public Form1()
        {
            InitializeComponent();
            mRawCaptures = new List<RawCapture>();
            mArrayCaptures = new List<ArrayCapture>();
            AudioWaveFormatInterop arrayFormat = new AudioWaveFormatInterop(kNumSamplesPerSec, 1);
            AudioWaveFormatInterop rawFormat = new AudioWaveFormatInterop(kNumSamplesPerSec, 4);
            AudioDeviceInterop devices = new AudioDeviceInterop(AudioDeviceTypeInterop.CaptureDevice);
            for (int i = 0; i < devices.NumDevices; i++)
            {
                AudioDeviceInfoInterop info = devices.GetDeviceInfo(i);
                if (mMicDeviceToFilename.ContainsKey(info.DeviceName))
                {
                    mRawCaptures.Add(new RawCapture(info.DeviceName, info.DeviceID, rawFormat));
                    string kinectId = mMicDeviceToKinectId[info.DeviceName];
                    mArrayCaptures.Add(new ArrayCapture(kinectId, arrayFormat));
                }
            }

            if (!Directory.Exists(kOutputFolder))
            {
                Directory.CreateDirectory(kOutputFolder);
            }
        }

        private void buttonStartStop_Click(object sender, EventArgs e)
        {
            if (buttonStartStop.Text == "Start")
            {
                foreach (RawCapture d in mRawCaptures)
                {
                    d.Start();
                }
                foreach (ArrayCapture d in mArrayCaptures)
                {
                    d.Start();
                }
                
                buttonStartStop.Text = "Stop";
            }
            else
            {
                foreach (RawCapture d in mRawCaptures)
                {
                    d.Stop();
                }
                foreach (ArrayCapture d in mArrayCaptures)
                {
                    d.Stop();
                }

                foreach (RawCapture d in mRawCaptures)
                {
                    d.WriteToFile(kOutputFolder + "\\" + mMicDeviceToFilename[d.DeviceName]);
                }
                foreach (ArrayCapture d in mArrayCaptures)
                {
                    d.WriteToFile(kOutputFolder + "\\" + mMicArrayDeviceToFilename[d.DeviceId]);
                }

                buttonStartStop.Text = "Start";
                for (int i = 0; i < mMicDeviceToKinectId.Count; ++i)
                {
                    ArrayCapture a = mArrayCaptures[i];
                    RawCapture r = mRawCaptures[i];
                    TextBox aBox;
                    TextBox rBox;
                    if (IsBackground)
                    {
                        aBox = (TextBox)this.Controls.Find("textBoxArrayBackgroundEnergy" + i.ToString(), true)[0];
                        rBox = (TextBox)this.Controls.Find("textBoxRawBackgroundEnergy" + i.ToString(), true)[0];
                        mArrayBackgroundEnergy[i] = a.GetEnergy();
                        mRawBackgroundEnergy[i] = r.GetEnergy();
                        aBox.Text = mArrayBackgroundEnergy[i].ToString();
                        rBox.Text = mRawBackgroundEnergy[i].ToString();
                    }
                    else 
                    {
                        aBox = (TextBox)this.Controls.Find("textBoxArraySignalEnergy" + i.ToString(), true)[0];
                        rBox = (TextBox)this.Controls.Find("textBoxRawSignalEnergy" + i.ToString(), true)[0];
                        mArraySignalEnergy[i] = a.GetEnergy();
                        mRawSignalEnergy[i] = r.GetEnergy();
                        aBox.Text = mArraySignalEnergy[i].ToString();
                        rBox.Text = mRawSignalEnergy[i].ToString();
                    }
                    if (mArraySignalEnergy[i] != 0.0 && mArrayBackgroundEnergy[i] != 0.0) 
                    {
                        aBox = (TextBox)this.Controls.Find("textBoxArraySNR" + i.ToString(), true)[0];
                        aBox.Text = (mArraySignalEnergy[i] / mArrayBackgroundEnergy[i]).ToString();
                    }
                    if (mRawSignalEnergy[i] != 0.0 && mRawBackgroundEnergy[i] != 0.0)
                    {
                        rBox = (TextBox)this.Controls.Find("textBoxRawSNR" + i.ToString(), true)[0];
                        rBox.Text = (mRawSignalEnergy[i] / mRawBackgroundEnergy[i]).ToString();
                    }
                }
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            IsBackground = !IsBackground;
        }
    }

    public class RawCapture
    { 
        const int kCallbackIntervalInMilliSec = 100;

        AudioMicCaptureInterop mMic;
        MemoryStream mData;
        int mNumSamplesPerSec;
        short mNumChannels;
        bool mIsCapturing;

        public RawCapture(string deviceName, string deviceId, AudioWaveFormatInterop format)
        {
            mMic = new AudioMicCaptureInterop(deviceId, format, MicDataCallback, kCallbackIntervalInMilliSec);

            mNumSamplesPerSec = (int)format.nSamplesPerSec;
            mNumChannels = (short)format.nChannels;

            DeviceName = deviceName;
            mIsCapturing = false;
        }

        public void Start()
        {
            mData = new MemoryStream();
            mIsCapturing = true;
            mMic.Start();
        }

        public void Stop()
        {
            mMic.Stop();
            mIsCapturing = false;
        }

        public void WriteToFile(string fileName)
        {
            IWavFileWriter writer = WavFileWriterPCM.Create(mNumSamplesPerSec, mNumChannels);

            mData.Position = 0;
            writer.PutSamples(mData.ToArray());
            writer.Write(fileName);
        }

        public void MicDataCallback(byte[] inputBuf, int sizeBuf)
        {
            if (mIsCapturing)
            {
                mData.Write(inputBuf, 0, sizeBuf);
            }
        }

        public string DeviceName { get; private set; }
    }


    public class ArrayCapture
    {
        const int kCallbackIntervalInMilliSec = 100;

        AudioMicArrayCaptureInterop mMic;
        MemoryStream mData;
        int mNumSamplesPerSec;
        short mNumChannels;
        bool mIsCapturing;

        public ArrayCapture(string kinectId, AudioWaveFormatInterop format)
        {
            mMic = new AudioMicArrayCaptureInterop(kinectId, format, MicDataCallback, kCallbackIntervalInMilliSec);

            mNumSamplesPerSec = (int)format.nSamplesPerSec;
            mNumChannels = (short)format.nChannels;
            DeviceId = kinectId;
            mIsCapturing = false;
        }

        public void Start()
        {
            mData = new MemoryStream();
            mIsCapturing = true;
            mMic.Start();
        }

        public void Stop()
        {
            mMic.Stop();
            mIsCapturing = false;
        }

        public void WriteToFile(string fileName)
        {
            IWavFileWriter writer = WavFileWriterPCM.Create(mNumSamplesPerSec, mNumChannels);

            mData.Position = 0;
            writer.PutSamples(mData.ToArray());
            writer.Write(fileName);
        }

        public void MicDataCallback(byte[] inputBuf, int sizeBuf)
        {
            if (mIsCapturing)
            {
                mData.Write(inputBuf, 0, sizeBuf);
            }
        }

        public double GetEnergy() { return mMic.GetEnergy(); }

        public string DeviceId { get; private set; }
    }
}
