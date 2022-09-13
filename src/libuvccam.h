#pragma once

#include "util.h"
#include <tchar.h>
#include <windows.h>
#include <dshow.h>
#include <vidcap.h>
#include <ks.h>
#include <ksproxy.h>
#include <Ksmedia.h>
#include <map>
#include <functional>

namespace libuvccam {
    enum EMoveAction{
        None,
        MoveLeft,
        MoveUp,
        MoveRight,
        MoveDown,
        ZoomIn,
        ZoomOut,
    };

    class LIBUVCCAM_API UVCCamera {
    public:
        UVCCamera();
        ~UVCCamera();

    public:
        int ListDevices(map<int, TString>& devices);
        bool Connect(const TChar* deviceName);
        bool IsConnected();
        TString GetConnectedDeviceName();
        void Disconnect();

        int AbsoluteMove(EMoveAction action);
        int RelativeMove(EMoveAction action);
        int RelativeStop();

        int Reset();

    private:
        void Release();
        typedef std::function<bool(int index, const TChar* deviceName, IMoniker* pMoniker)> EnumerateFunc;
        void EnumerateDevices(EnumerateFunc func);
        struct Property {
            long min, max, step, default, val, flags;
            TString DumpStr();
        };
        int GetProperty(KSPROPERTY_VIDCAP_CAMERACONTROL prop, __out Property& data);
        int MoveCamera(KSPROPERTY_VIDCAP_CAMERACONTROL prop, int step);

    private:
        // to select a video input device
        ICreateDevEnum* m_pCreateDevEnum = NULL;
        IEnumMoniker* m_pEnumMoniker = NULL;

        //base directshow filter
        IBaseFilter* m_pDeviceFilter = NULL;

        TString m_connectedDevice;
    };
}
