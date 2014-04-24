#pragma once

#include "AudioCaptureCommonNative.h"

namespace AudioCaptureNative
{
    class BaseMediaBuffer : public IMediaBuffer 
    {
    public:
        BaseMediaBuffer() {}
        BaseMediaBuffer(BYTE *pData, ULONG ulSize, ULONG ulData) :
            m_pData(pData), m_ulSize(ulSize), m_ulData(ulData), m_cRef(1) {}

        STDMETHODIMP_(ULONG) AddRef()
        {
          return InterlockedIncrement((long*)&m_cRef);
        }

        STDMETHODIMP_(ULONG) Release()
        {
            long l = InterlockedDecrement((long*)&m_cRef);
            if (l == 0)
            {
                delete this;
            }
            return l;
       }

        STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
        {
            if (riid == IID_IUnknown) 
            {
                AddRef();
                *ppv = (IUnknown*)this;
                return NOERROR;
            }
            else if (riid == IID_IMediaBuffer) 
            {
                AddRef();
                *ppv = (IMediaBuffer*)this;
                return NOERROR;
            }
            else
            {
                return E_NOINTERFACE;
            }
        }
   
        STDMETHODIMP SetLength(DWORD ulLength)
        {
            m_ulData = ulLength;
            return NOERROR;
        }
        STDMETHODIMP GetMaxLength(DWORD *pcbMaxLength)
        {
            *pcbMaxLength = m_ulSize;
            return NOERROR;
        }
        STDMETHODIMP GetBufferAndLength(BYTE **ppBuffer, DWORD *pcbLength) 
        {
            if (ppBuffer)
            {
                *ppBuffer = m_pData;
            }
            if (pcbLength)
            {
                *pcbLength = m_ulData;
            }
            return NOERROR;
        }

    protected:
       BYTE *m_pData;
       ULONG m_ulSize;
       ULONG m_ulData;
       ULONG m_cRef;
    };

    class StaticMediaBuffer : public BaseMediaBuffer
    {
    public:
        STDMETHODIMP_(ULONG) AddRef()
        {
            return 2;
        }
    
        STDMETHODIMP_(ULONG) Release()
        {
            return 1;
        }
    
        void Init(BYTE *pData, ULONG ulSize, ULONG ulData) 
        {
            m_pData = pData;
            m_ulSize = ulSize;
            m_ulData = ulData;
        }
    };
}