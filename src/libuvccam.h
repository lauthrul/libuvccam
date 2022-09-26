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
    enum EMoveAction {
        None,
        MoveLeft,
        MoveUp,
        MoveRight,
        MoveDown,
        ZoomIn,
        ZoomOut,
    };

    enum EXUOP {
        GET,
        SET,
    };

    struct CameraInfo {
        int         index;
        TString     clsid;
        TString     deviceName;
        TString     devicePath;
        void clear() {
            index = -1;
            clsid.clear();
            deviceName.clear();
            devicePath.clear();
        }
    };

    class LIBUVCCAM_API UVCCamera {
    public:
        UVCCamera();
        ~UVCCamera();

    public:
        int ListDevices(std::map<int, CameraInfo>& devices);
        int Connect(const TChar* deviceName);
        bool IsConnected();
        CameraInfo GetConnectedDeviceInfo();
        void Disconnect();

        int AbsoluteMove(EMoveAction action);
        int RelativeMove(EMoveAction action);
        int RelativeStop();

        int Reset();

        int XUOperate(EXUOP op, GUID guid, ULONG cs, __inout LPVOID data, ULONG len, __out ULONG* readCount);

    private:
        void Release();
        typedef std::function<bool(int index, const CameraInfo& deviceInfo, IMoniker* pMoniker)> EnumerateFunc;
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

        CameraInfo m_connectedDevice;
    };
}
