#include "stdafx.h"
#include "libuvccam.h"

#ifdef WIN32
#include <comdef.h>
TString errmsg(HRESULT hr) {
    TChar sz[MAX_PATH] = { 0 };
    _com_error err(hr);
    _stprintf_s(sz, L"[0x%X]%s", hr, err.ErrorMessage());
    return sz;
}
#else
TString errmsg(HRESULT hr) {
    TChar sz[32] = { 0 };
    _stprintf_s(sz, L"0x%X", hr);
    return sz;
}
#endif // WIN32

#define SAFE_RELEASE(ptr) if (ptr != NULL) { ptr->Release(); ptr = NULL; }

namespace libuvccam {

    TString UVCCamera::Property::DumpStr() {
        TChar sz[256] = { 0 };
        _stprintf_s(sz, L"{min:%d, max:%d, step:%d, default:%d, val:%d}", min, max, step, default, val);
        return sz;
    }

    UVCCamera::UVCCamera()
        : m_pCreateDevEnum(NULL), m_pEnumMoniker(NULL), m_pDeviceFilter(NULL) {
        // initialize COM
        CoInitialize(0);
    }

    UVCCamera::~UVCCamera() {
        if (IsConnected()) {
            Disconnect();
        }
        Release();
        // finalize COM
        CoUninitialize();
    }

    void UVCCamera::Release() {
        // release directshow class instances
        SAFE_RELEASE(m_pEnumMoniker);
        SAFE_RELEASE(m_pCreateDevEnum);
    }

    void UVCCamera::EnumerateDevices(EnumerateFunc func) {
        Release();

        // Create CreateDevEnum to list device
        CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (PVOID *)&m_pCreateDevEnum);

        // Create EnumMoniker to list VideoInputDevice
        m_pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &m_pEnumMoniker, 0);
        if (m_pEnumMoniker == NULL) {
            LOG_ERROR(L"no device found");
            return;
        }
        // reset EnumMoniker
        m_pEnumMoniker->Reset();

        IMoniker *pMoniker = NULL;
        ULONG nFetched = 0;
        int index = 0;
        while (m_pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {
            //real name of the camera without suffix
            TString cameraRealNames[256];

            //unique name with suffix 
            CameraInfo info;

            // bind to IPropertyBag
            IPropertyBag *pPropertyBag;
            pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropertyBag);

            // get FriendlyName, CLSID, DevicePath
            VARIANT var;
            var.vt = VT_BSTR;
            pPropertyBag->Read(L"FriendlyName", &var, 0);//FriendlyName
            info.deviceName = var.bstrVal;
            pPropertyBag->Read(L"CLSID", &var, 0);//CLSID
            info.clsid = var.bstrVal;
            pPropertyBag->Read(L"DevicePath", &var, 0);//DevicePath
            info.devicePath = var.bstrVal;
            VariantClear(&var);

            int nSameNamedDevices = 0;
            for (int j = 0; j < index; j++) {
                if (cameraRealNames[j] == info.deviceName)
                    nSameNamedDevices++;
            }
            cameraRealNames[index] = info.deviceName;
            //if there are the same type of cameras 
            //need to add some suffixes to identify the camera
            //first camera has no prefix
            //suffix [name] #[index] (e.g. PTZOptics Camera #1)
            if (nSameNamedDevices > 0) {
                TChar devname[256] = { 0 };
                _stprintf_s(devname, L"%s #%d", info.deviceName, nSameNamedDevices);
                info.deviceName = devname;
            }

            // stop enumerate if func(...) return false
            auto ret = true;
            if (func != NULL) {
                ret = func(index, info, pMoniker);
            }

            pPropertyBag->Release();
            pMoniker->Release();

            index++;

            if (!ret) {
                break;
            }
        }
    }

    int UVCCamera::ListDevices(std::map<int, CameraInfo>& devices) {
        EnumerateDevices([&](int index, const CameraInfo& deviceInfo, IMoniker *pMoniker)->bool {
            devices[index] = deviceInfo;
            devices[index].index = index;
            return true; // continue enumerate
        });
        return devices.size();
    }

    int UVCCamera::Connect(const TChar* name) {
        HRESULT hr = S_OK;
        if (name == m_connectedDevice.deviceName) {
            return hr;
        }

        Disconnect();

        EnumerateDevices([&](int index, const CameraInfo& deviceInfo, IMoniker *pMoniker)->bool {
            if (deviceInfo.deviceName == name) {
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pDeviceFilter);
                if (m_pDeviceFilter != NULL) {
                    m_connectedDevice = deviceInfo;
                }
                return false; // stop enumerate
            }
            return true; // continue enumerate
        });
        return hr;
    }

    bool UVCCamera::IsConnected() {
        return m_pDeviceFilter != NULL;
    }

    CameraInfo UVCCamera::GetConnectedDeviceInfo() {
        return m_connectedDevice;
    }

    void UVCCamera::Disconnect() {
        m_connectedDevice.clear();
        //release directshow filter
        SAFE_RELEASE(m_pDeviceFilter);
    }

    int UVCCamera::GetProperty(KSPROPERTY_VIDCAP_CAMERACONTROL prop, __out Property& data) {
        IAMCameraControl *pCameraControl = 0;
        HRESULT hr = m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
        if (FAILED(hr)) {
            // The device does not support IAMCameraControl
            LOG_ERROR(L"This device does not support IAMCameraControl. hr:%s", errmsg(hr).c_str());
        } else {
            // Get the range and default values 
            hr = pCameraControl->GetRange(prop, &data.min, &data.max, &data.step, &data.default, &data.flags);
            if (hr != S_OK) {
                LOG_ERROR(L"GetRange fail. hr:%s", errmsg(hr).c_str());
            }
            hr = pCameraControl->Get(prop, &data.val, &data.flags);
            if (hr != S_OK) {
                LOG_ERROR(L"Get fail. hr:%s", errmsg(hr).c_str());
            }
            LOG_DEBUG(L"GetProperty. prop:%d, data:%s", prop, data.DumpStr().c_str());
        }
        if (pCameraControl != NULL)
            pCameraControl->Release();
        return hr;
    }

    int UVCCamera::MoveCamera(KSPROPERTY_VIDCAP_CAMERACONTROL prop, int step) {
        IAMCameraControl *pCameraControl = 0;
        HRESULT hr = m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
        if (FAILED(hr)) {
            // The device does not support IAMCameraControl
            LOG_ERROR(L"This device does not support IAMCameraControl. hr:%s", errmsg(hr).c_str());
        } else {
            hr = pCameraControl->Set(prop, step, CameraControl_Flags_Manual);
            LOG_DEBUG(L"Set. prop:%d, step:%d", prop, step);
            if (FAILED(hr)) {
                LOG_ERROR(L"Set fail. prop:%d, step:%d, hr:%s", prop, step, errmsg(hr).c_str());
            }
        }
        if (pCameraControl != NULL)
            pCameraControl->Release();
        return hr;
    }

    int UVCCamera::AbsoluteMove(EMoveAction action) {
        KSPROPERTY_VIDCAP_CAMERACONTROL prop;
        if (action == MoveLeft || action == MoveRight) {
            prop = KSPROPERTY_CAMERACONTROL_PAN;
        } else if (action == MoveUp || action == MoveDown) {
            prop = KSPROPERTY_CAMERACONTROL_TILT;
        } else if (action == ZoomIn || action == ZoomOut) {
            prop = KSPROPERTY_CAMERACONTROL_ZOOM;
        }
        auto forward = (action == MoveRight || action == MoveUp || action == ZoomIn);

        Property data;
        GetProperty(prop, data);

        auto step = ceil((double)(data.max - data.min) / 10);
        long val = forward ? (data.val + step) : (data.val - step);
        val = min(max(val, data.min), data.max);
        return MoveCamera(prop, val);
    }

    int UVCCamera::RelativeMove(EMoveAction action) {
        KSPROPERTY_VIDCAP_CAMERACONTROL prop;
        if (action == MoveLeft || action == MoveRight) {
            prop = KSPROPERTY_CAMERACONTROL_PAN_RELATIVE;
        } else if (action == MoveUp || action == MoveDown) {
            prop = KSPROPERTY_CAMERACONTROL_TILT_RELATIVE;
        } else if (action == ZoomIn || action == ZoomOut) {
            prop = KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE;
        }
        auto forward = (action == MoveRight || action == MoveUp || action == ZoomIn);

        Property data;
        GetProperty(prop, data);

        auto val = forward ? data.step : -data.step;
        return MoveCamera(prop, val);
    }

    int UVCCamera::RelativeStop() {
        int ret = MoveCamera(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, 0);
        ret |= MoveCamera(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, 0);
        ret |= MoveCamera(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, 0);
        return ret;
    }

    int UVCCamera::Reset() {
        Property data;
        auto prop = KSPROPERTY_CAMERACONTROL_PAN;
        GetProperty(prop, data);
        auto ret = MoveCamera(prop, data.default);

        prop = KSPROPERTY_CAMERACONTROL_TILT;
        GetProperty(prop, data);
        ret |= MoveCamera(prop, data.default);

        prop = KSPROPERTY_CAMERACONTROL_ZOOM;
        GetProperty(prop, data);
        ret |= MoveCamera(prop, data.default);

        return ret;
    }

    HRESULT _FindExtensionNode(IKsTopologyInfo* pKsTopologyInfo, DWORD* node) {
        if (!pKsTopologyInfo || !node)
            return E_POINTER;

        HRESULT hr = E_FAIL;
        DWORD dwNumNodes = 0;
        GUID guidNodeType;
        IKsControl* pKsControl = NULL;
        ULONG ulBytesReturned = 0;

        // Retrieve the number of nodes in the filter
        hr = pKsTopologyInfo->get_NumNodes(&dwNumNodes);
        if (!SUCCEEDED(hr))
            return hr;
        if (dwNumNodes == 0)
            return E_FAIL;


        // Find the extension unit node that corresponds to the given GUID
        for (unsigned int i = 0; i < dwNumNodes; i++) {
            hr = E_FAIL;
            pKsTopologyInfo->get_NodeType(i, &guidNodeType);
            if (IsEqualGUID(guidNodeType, KSNODETYPE_DEV_SPECIFIC)) {
                *node = i;
                return S_OK;
            }
        }

        return E_FAIL;
    }

    int UVCCamera::XUOperate(EXUOP op, GUID guid, ULONG cs, __inout LPVOID data, ULONG len, __out ULONG* readCount) {
        IUnknown* unKnown;
        IKsControl* ks_control = NULL;
        IKsTopologyInfo* pKsTopologyInfo = NULL;
        KSP_NODE kspNode;
        HRESULT hr = S_OK;

        do {
            hr = m_pDeviceFilter->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);
            if (hr != S_OK) break;

            DWORD dwNodeId = 0;
            hr = _FindExtensionNode(pKsTopologyInfo, &dwNodeId);
            if (hr != S_OK) break;

            hr = pKsTopologyInfo->CreateNodeInstance(dwNodeId, IID_IUnknown, (LPVOID *)&unKnown);
            if (hr != S_OK) break;

            hr = unKnown->QueryInterface(__uuidof(IKsControl), (void **)&ks_control);
            if (hr != S_OK) break;

            kspNode.Property.Set = guid;            // XU GUID
            kspNode.NodeId = (ULONG)dwNodeId;       // XU Node ID
            kspNode.Property.Id = cs;               // XU control ID
            // Set/Get request
            if (op == SET) {
                kspNode.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
            } else {
                kspNode.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
            }

            hr = ks_control->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), (PVOID)data, len, readCount);
            if (hr != S_OK) break;
        } while (0);

        SAFE_RELEASE(ks_control);
        return hr;
    }
}
