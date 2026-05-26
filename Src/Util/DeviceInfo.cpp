#include "Util/DeviceInfo.h"
#include <QSysInfo>
#include <QProcess>
#include <QObject>
#include <comdef.h>
#include <wbemidl.h>
#include <Windows.h>
#include <vector>
#pragma comment(lib, "wbemuuid.lib")





bool WMIInitializer::isInitialized = false;

bool QueryWMI(const wchar_t* query, LPCWSTR Key, std::vector<std::wstring>& result)
{
    try {
        WMIInitializer initializer;  

        
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;
        HRESULT hr;

        
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hr)) {
            qDebug() << L"Failed to create IWbemLocator object. Error code =" << hr;
            return false;
        }

        
        hr = pLoc->ConnectServer(
            BSTR(L"ROOT\\CIMV2"),  
            nullptr,               
            nullptr,               
            0,                     
            0,                     
            0,                     
            0,                     
            &pSvc                  
        );

        if (FAILED(hr)) {
            qDebug() << L"Failed to connect to ROOT\\CIMV2. Error code = " << hr;
            pLoc->Release();
            return false;
        }
        
        hr = CoSetProxyBlanket(
            pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

        if (FAILED(hr))
        {
            qDebug() << L"Failed to set proxy blanket. Error code = " << hr;
            pSvc->Release();
            pLoc->Release();
            return false;
        }

        
        IEnumWbemClassObject* pEnumerator = nullptr;
        hr = pSvc->ExecQuery(
            _bstr_t(L"WQL"), _bstr_t(query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

        if (FAILED(hr))
        {
            qDebug() << L"Query for data failed. Error code = " << hr;
            pSvc->Release();
            pLoc->Release();
            return false;
        }

        
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (pEnumerator)
        {
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0)
                break;

            VARIANT vtProp;

            
            hr = pclsObj->Get(Key, 0, &vtProp, 0, 0);

            if (SUCCEEDED(hr))
            {
                std::wstring value = vtProp.bstrVal;
                
                result.push_back(value);
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }

        
        pEnumerator->Release();
        pSvc->Release();
        pLoc->Release();
    }
    catch (const std::exception& e) {
        qDebug() << "Exception: " << e.what();
        return false;
    }

    return true;
}

DeviceDetail checkDeviceAvailable() {
    boolean systemUnmatch = false;
    
    QString errMsg = "Installation environment mismatch\n";

    
    QString memoryInfo = "Memory Information:\n";

    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        double currentMemory = memoryStatus.ullTotalPhys / (1024 * 1024 * 1024);
        
        memoryInfo += QString("System memory: %1 %2\n").arg(currentMemory).arg("GB");
        if (currentMemory < MIN_MEMORY_REQUIRED) {
            systemUnmatch = true;
            
            errMsg += QString("System memory: %1 %2, less than required %3 GB\n").arg(currentMemory).arg("GB").arg(MIN_MEMORY_REQUIRED);
        }
        
    }
    else {
        
        memoryInfo += QString("Unable to get memory usage information.\n");
    }


    
    QString cpuInfo = "";
    

    
    QString gpuInfo = "GPU Information:\n";
    QProcess gpuProcess;
    gpuProcess.start("wmic path win32_VideoController get Name");
    gpuProcess.waitForFinished();
    QString gpuResult = gpuProcess.readAllStandardOutput();
    QStringList gpuList = gpuResult.split("\n", QString::SkipEmptyParts);
    bool findNividaGPU = false;
    for (int i = 1; i < gpuList.size(); i++) {
        QString gpuName = gpuList.at(i).trimmed();
        if (gpuName != "") {
            gpuInfo += "GPU " + QString::number(i) + ": " + gpuName + "\n";
            if (gpuInfo.contains("NVIDIA", Qt::CaseInsensitive)) {
                findNividaGPU = true;
            }
        }
    }
    if (!findNividaGPU) {
        systemUnmatch = true;
        
        errMsg += "No compatible Nvidia graphics card found. The current graphics card is:";
        errMsg += gpuInfo;
    }

    
    QString systemInfo = memoryInfo + "\n" + cpuInfo + "\n" + gpuInfo + +"\n";
    

    DeviceDetail detail;
    detail.unMatch = systemUnmatch;
    detail.systemInfo = systemInfo;
    detail.errorInfo = errMsg;

    return detail;
}

DeviceID getDeviceID() {
    QString hostName = QHostInfo::localHostName();
    

    QList<QNetworkInterface>list = QNetworkInterface::allInterfaces();
    QString mac = "";
    foreach(QNetworkInterface inter, list)
    {
        
        
        
        if (inter.flags().testFlag(QNetworkInterface::IsUp) && inter.flags().testFlag(QNetworkInterface::IsRunning) && !inter.flags().testFlag(QNetworkInterface::IsLoopBack)) { 
            
            mac = inter.hardwareAddress();
            
            break;
        }
        

    }
    std::vector<std::wstring> idResult;
    idResult.clear();
    QString uuid;
    bool bmem = QueryWMI(L"SELECT * FROM Win32_ComputerSystemProduct", L"UUID", idResult);
    if (bmem) {
        bool found = false;
        for (int i = 0; i < idResult.size(); i++) {
            std::wstring item = idResult[i];
            uuid = QString::fromStdWString(item);
        }
    }
    

    DeviceID deviceId;
    deviceId.mid = mac;
    deviceId.mname = hostName;
    deviceId.uuid = uuid;

    return deviceId;
}