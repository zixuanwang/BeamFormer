#include "AudioCaptureCommonNative.h"
#include "AudioDeviceNative.h"

#pragma warning(push)
#pragma warning(disable : 6011 6244 6387)
#define UNICODE

#include "strsafe.h"
#include "functiondiscoverykeys.h"

#pragma warning(pop)

#include <cstdlib>
#include <stdexcept>
#include <atlstr.h>

using namespace AudioCaptureNative;

AudioDeviceNative::AudioDeviceNative(EDataFlow flow) :
    mDeviceInfo(NULL),
    mNumDevices(0),
    mIsInitialized(false)
{
    HRESULT hr;
    UINT uDevCount = 0;
    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;    

    // Initialization
    CoInitialize(NULL);

    hr = spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    if (FAILED(hr))
    {
        throw std::exception("Error from CoCreateInstance()");
    }

    hr = spEnumerator->EnumAudioEndpoints(flow,
                                          DEVICE_STATE_ACTIVE, 
                                          &spEndpoints);
    if (FAILED(hr))
    {
        throw std::exception("Error from EnumAudioEndpoints()");
    }

    hr = spEndpoints->GetCount(&uDevCount);
    if (FAILED(hr))
    {
        throw std::exception("Error from GetCount()");
    }

    mNumDevices = (int)uDevCount;
    mDeviceInfo = new AUDIO_DEVICE_INFO[mNumDevices];
    memset(mDeviceInfo, 0, sizeof(AUDIO_DEVICE_INFO) * mNumDevices);

    // get info for each device
    for (int i = 0; i < mNumDevices; i++)
    {
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;
        WCHAR* pszDeviceId = NULL;
        PROPVARIANT value;

        hr = spEndpoints->Item(i, &spDevice);
        if (FAILED(hr))
        {
            throw std::exception("Error from Item()");
        }
        
        // device id
        hr = spDevice->GetId(&pszDeviceId);
        if (FAILED(hr))
        {
            throw std::exception("Error from GetId()");
        }

        StringCchCopy(mDeviceInfo[i].szDeviceID, 
                      MAX_STR_LEN - 1, 
                      pszDeviceId);

        CoTaskMemFree(pszDeviceId);
        pszDeviceId = NULL;

        hr = spDevice->OpenPropertyStore(STGM_READ, &spProperties);
        if (FAILED(hr))
        {
            throw std::exception("Error from OpenPropertyStore()");
        }

        // device friendly name
        PropVariantInit(&value);
        hr = spProperties->GetValue(PKEY_Device_FriendlyName, &value);
        if (FAILED(hr))
        {
            throw std::exception("Error from GetValue()");
        }

        if (value.pwszVal != NULL)
        {
            StringCchCopy(mDeviceInfo[i].szDeviceName, 
                          MAX_STR_LEN - 1, 
                          value.pwszVal);
        }
        PropVariantClear(&value);

        // device enumerator name
        PropVariantInit(&value);
        hr = spProperties->GetValue(PKEY_Device_EnumeratorName, &value);
        if (FAILED(hr))
        {
            throw std::exception("Error from GetValue()");
        }

        if (value.pwszVal != NULL)
        {
            StringCchCopy(mDeviceInfo[i].szDeviceEnumerator, 
                          MAX_STR_LEN - 1, 
                          value.pwszVal);
        }

        PropVariantClear(&value);
    }

    mIsInitialized = true;
}

AudioDeviceNative::~AudioDeviceNative()
{
    if (mDeviceInfo)
    {
        delete mDeviceInfo;
        mDeviceInfo = NULL;
    }
}

int AudioDeviceNative::NumDevices() const
{
    return mNumDevices;
}

AUDIO_DEVICE_INFO AudioDeviceNative::GetDeviceInfo(int index) const
{
    if (!mIsInitialized)
    {
        throw std::logic_error("This instance is not properly initialized");
    }
    if (index < 0 || index >= mNumDevices)
    {
        throw std::invalid_argument("index is out of range");
    }

    return mDeviceInfo[index];
}
