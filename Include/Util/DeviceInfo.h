#pragma once
#include <QSysInfo>
#include <QProcess>

#include <Windows.h>

#include <QNetworkAddressEntry>
#include <QNetworkInterface>

#include <QHostInfo>
#include <iostream>

const double MIN_MEMORY_REQUIRED = 16.0;

class WMIInitializer {
public:
    
    WMIInitializer() {
        
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        if (hr == RPC_E_CHANGED_MODE) {
            std::cerr << "COM already initialized with a different threading model." << std::endl;
            
        }

        if (FAILED(hr)) {
            std::cerr << "CoInitializeEx failed with error code: " << hr << std::endl;
            throw std::runtime_error("COM initialization failed");
        }
        isInitialized = true;
    }

    
    ~WMIInitializer() {
        if (isInitialized) {
            CoUninitialize();
            isInitialized = false;
        }
    }

private:
    static bool isInitialized; 
};

struct DeviceDetail
{
    bool unMatch;
    QString systemInfo;
    QString errorInfo;
};

extern DeviceDetail checkDeviceAvailable();

struct DeviceID
{
    QString mname;
    QString mid;
    QString uuid;
};

extern DeviceID getDeviceID();

