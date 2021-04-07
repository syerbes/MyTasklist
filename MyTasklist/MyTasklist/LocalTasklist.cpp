#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <iomanip>
#include <String>
#include <comutil.h>

#pragma comment(lib, "wbemuuid.lib")

int LocalTasklist(const char * mode)
    {
        HRESULT hres;

        // Initialize COM.
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres))
        {
            cout << "Failed to initialize COM library. "
                << "Error code = 0x"
                << hex << hres << endl;
            return 1;              // Program has failed.
        }

        // Initialize 
        hres = CoInitializeSecurity(
            NULL,
            -1,      // COM negotiates service                  
            NULL,    // Authentication services
            NULL,    // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,    // authentication
            RPC_C_IMP_LEVEL_IMPERSONATE,  // Impersonation
            NULL,             // Authentication info 
            EOAC_NONE,        // Additional capabilities
            NULL              // Reserved
        );


        if (FAILED(hres))
        {
            cout << "Failed to initialize security. "
                << "Error code = 0x"
                << hex << hres << endl;
            CoUninitialize();
            return 1;          // Program has failed.
        }

        // Obtain the initial locator to Windows Management
        // on a particular host computer.
        IWbemLocator* pLoc = 0;

        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);

        if (FAILED(hres))
        {
            cout << "Failed to create IWbemLocator object. "
                << "Error code = 0x"
                << hex << hres << endl;
            CoUninitialize();
            return 1;       // Program has failed.
        }

        IWbemServices* pSvc = 0;

        // Connect to the root\cimv2 namespace with the
        // current user and obtain pointer pSvc
        // to make IWbemServices calls.

        hres = pLoc->ConnectServer(

            _bstr_t(L"ROOT\\CIMV2"), // WMI namespace
            NULL,                    // User name
            NULL,                    // User password
            0,                       // Locale
            NULL,                    // Security flags                 
            0,                       // Authority       
            0,                       // Context object
            &pSvc                    // IWbemServices proxy
        );

        if (FAILED(hres))
        {
            cout << "Could not connect. Error code = 0x"
                << hex << hres << endl;
            pLoc->Release();
            CoUninitialize();
            return 1;                // Program has failed.
        }

        cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

        // Set the IWbemServices proxy so that impersonation
        // of the user (client) occurs.
        hres = CoSetProxyBlanket(

            pSvc,                         // the proxy to set
            RPC_C_AUTHN_WINNT,            // authentication service
            RPC_C_AUTHZ_NONE,             // authorization service
            NULL,                         // Server principal name
            RPC_C_AUTHN_LEVEL_CALL,       // authentication level
            RPC_C_IMP_LEVEL_IMPERSONATE,  // impersonation level
            NULL,                         // client identity 
            EOAC_NONE                     // proxy capabilities     
        );

        if (FAILED(hres))
        {
            cout << "Could not set proxy blanket. Error code = 0x"
                << hex << hres << endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }




        //Create context to call GetUser Function for Verbose Mode



        BSTR ClassName = SysAllocString(L"Win32_Process");
  

        _bstr_t MethodName = (L"GetOwner");

        IWbemClassObject* pClass = NULL;

        hres = pSvc->GetObject(ClassName, 0, NULL, &pClass, NULL);

        IWbemClassObject* pOutMethod = NULL;

        hres = pClass->GetMethod(MethodName, 0,
            NULL, &pOutMethod);

        if (FAILED(hres))
        {
            cout << "Could not get the method. Error code = 0x"
                << hex << hres << endl;
        }

        IWbemClassObject* pInInst = NULL;
        hres = pOutMethod->SpawnInstance(0, &pInInst);
        if (FAILED(hres))
        {
            printf("SpawnInstance hres = %08x\n", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;
        }

        // Use the IWbemServices pointer to make requests of WMI. 
        // Make requests here:

        // For example, query for all the running processes
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_Process"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator);

        if (FAILED(hres))
        {
            cout << "Query for processes failed. "
                << "Error code = 0x"
                << hex << hres << endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }
        IWbemClassObject* pclsObj;
        ULONG uReturn = 0;

        // Output design for standard tasklist
        if (mode == "Standard") {
            int TAB = 20;
            std::cout
                << std::left << std::setw(30) << "Process"
                << std::setw(TAB) << "ID"
                << std::setw(TAB) << "Session Name"
                << std::setw(TAB) << "Session ID"
                << std::setw(TAB) << "Mem Usage"
                << std::endl;

            std::cout
                << std::left << "-------------------------------------------------------------------------------------------------------------"
                << std::endl;

            while (pEnumerator)
            {
                hres = pEnumerator->Next(WBEM_INFINITE, 1,
                    &pclsObj, &uReturn);

                if (0 == uReturn)
                {
                    break;
                }

                VARIANT vtProp;
                VARIANT vtProp2;
                VARIANT vtProp3;
                VARIANT vtProp4;
                wchar_t* sessionName;


                // Get the value of the Name property
                hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                hres = pclsObj->Get(L"ProcessID", 0, &vtProp2, 0, 0);
                hres = pclsObj->Get(L"SessionId", 0, &vtProp3, 0, 0);
                hres = pclsObj->Get(L"WorkingSetSize", 0, &vtProp4, 0, 0);
                if (vtProp3.uintVal == 0) {
                    sessionName = const_cast<wchar_t*>(L"Services");
                }
                else {
                    sessionName = const_cast <wchar_t*>(L"Console");
                }
                if (SysStringLen(vtProp.bstrVal) > 25) {
                    std::wstring a(vtProp.bstrVal, 25);
                    vtProp.bstrVal = SysAllocStringLen(a.data(), a.size());
                }

                std::wcout
                    << std::left << std::setw(30) << vtProp.bstrVal
                    << std::setw(TAB) << vtProp2.uintVal
                    << std::setw(TAB) << sessionName
                    << std::setw(TAB) << vtProp3.uintVal
                    << std::setw(TAB) << vtProp4.ulVal / 1024
                    << std::endl;

                VariantClear(&vtProp);
                VariantClear(&vtProp2);
                VariantClear(&vtProp3);
                VariantClear(&vtProp4);

                pclsObj->Release();
                pclsObj = NULL;
            }
        }
        // Output mode /V
        else if (mode == "Verbose") {

            int TAB = 20;

            std::cout
                << std::left << std::setw(30) << "Process"
                << std::setw(TAB) << "ID"
                << std::setw(TAB) << "Session Name"
                << std::setw(TAB) << "Session ID"
                << std::setw(TAB) << "Mem Usage"
                << std::setw(TAB) << "Status"
                << std::setw(TAB) << "User Name"
                << std::setw(TAB) << "CPU Time"
                << std::setw(TAB) << "Window Title"
                << std::endl;

            std::cout
                << std::left << "-------------------------------------------------------------------------------------------------------------"
                << std::endl;
            int i = 0;

            while (pEnumerator)
            {
                hres = pEnumerator->Next(WBEM_INFINITE, 1,
                    &pclsObj, &uReturn);

                if (0 == uReturn)
                {
                    break;
                }

                VARIANT vtPath;
                VARIANT vtProp;
                VARIANT vtProp2;
                VARIANT vtProp3;
                VARIANT vtProp4;
                VARIANT vtProp5;
                VARIANT vtProp6;
                VARIANT vtProp7;
                VARIANT vtProp8;
                wchar_t* sessionName;


                // Get the value of the properties
                hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                hres = pclsObj->Get(L"ProcessID", 0, &vtProp2, 0, 0);
                hres = pclsObj->Get(L"SessionId", 0, &vtProp3, 0, 0);
                hres = pclsObj->Get(L"WorkingSetSize", 0, &vtProp4, 0, 0);
                hres = pclsObj->Get(L"ExecutionState", 0, &vtProp5, 0, 0);
                hres = pclsObj->Get(L"UserModeTime", 0, &vtProp7, 0, 0);
                hres = pclsObj->Get(L"Description", 0, &vtProp8, 0, 0);
                if (FAILED(hres))
                {
                    cout << "Query for processes failed. "
                        << "Error code = 0x"
                        << hex << hres << endl;
 
                    return 1;               // Program has failed.
                }

                // Obtain the Path for getting User and Domain

                hres = pclsObj->Get(L"__PATH", 0, &vtPath, 0, 0);


                if (vtProp3.uintVal == 0) {
                    sessionName = const_cast<wchar_t*>(L"Services");
                }
                else {
                    sessionName = const_cast <wchar_t*>(L"Console");
                }
                if (SysStringLen(vtProp.bstrVal) > 25) {
                    std::wstring a(vtProp.bstrVal, 25);
                    vtProp.bstrVal = SysAllocStringLen(a.data(), a.size());
                }

                // Translating execution state from int to String.

                wchar_t* executionState;
                if (vtProp5.uintVal == 3) {
                    executionState = const_cast < wchar_t*>(L"Running");
                }
                else {
                    executionState = const_cast <wchar_t*>(L"Unkwown");
                }

                bool userFlag = FALSE;
                wstring domainUser;
                
                hres = pSvc->ExecMethod(vtPath.bstrVal, MethodName, 0, NULL, NULL, &pOutMethod, NULL);
                if (FAILED(hres))
                {
                    userFlag = TRUE;
                    domainUser = L"N/A";
                }
                else {
                    BSTR Text;
                    hres = pOutMethod->GetObjectText(0, &Text);
                    wstring text = Text;
                    wstring domain;
                    wstring user;
                    if (text.find(L"Domain") != -1) {
                        wstring::size_type posDomain = text.find(L"Domain");
                        wstring::size_type posAfterDomain = text.find(L";");
                        domain = text.substr(posDomain + 10, posAfterDomain - posDomain - 11).c_str();
                        wstring::size_type posUser = text.find(L"User");
                        wstring::size_type posAfterUser = text.find(L";", posUser);
                        user = text.substr(posUser + 8, posAfterUser - posUser - 9).c_str();
                        domainUser = domain + L"\\" + user;
                    }
                    else {
                        domainUser = L"N/A";
                    }
                }

                // Condition for Usage Time
                std::wstring CPUUsage;
                uint64_t i = 0;
                if (vtProp7.uintVal == i) {
                    CPUUsage = L"N/A";
                }
                else {
                    CPUUsage = std::to_wstring(vtProp7.uintVal / (10000000 * 60));
                }
                    
                std::wcout
                    << std::left << std::setw(30) << vtProp.bstrVal
                    << std::setw(TAB) << vtProp2.uintVal
                    << std::setw(TAB) << sessionName
                    << std::setw(TAB) << vtProp3.uintVal
                    << std::setw(TAB) << vtProp4.ulVal / 1024
                    << std::setw(TAB) << executionState
                    << std::setw(TAB) << domainUser
                    << std::setw(TAB) << CPUUsage
                    << std::setw(TAB) << vtProp8.bstrVal
                    << std::endl;

                VariantClear(&vtProp);
                VariantClear(&vtProp2);
                VariantClear(&vtProp3);
                VariantClear(&vtProp4);
                VariantClear(&vtProp5);
                VariantClear(&vtProp6);

            }
        }

        // Mode SVC
        else {

            int TAB = 40;
            std::cout
                << std::left << std::setw(30) << "Name"
                << std::setw(TAB) << "Services"
                << std::endl;

            std::cout
                << std::left << "-------------------------------------------------------------------------------------------------------------"
                << std::endl;

            while (pEnumerator)
            {
                hres = pEnumerator->Next(WBEM_INFINITE, 1,
                    &pclsObj, &uReturn);

                if (0 == uReturn)
                {
                    break;
                }


                VARIANT vtProp;
                VARIANT vtProp2;
                VARIANT vtProp3;
                VARIANT vtProp4;
                wchar_t* sessionName;
                

                // Get the value of the Name property
                hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                hres = pclsObj->Get(L"ProcessID", 0, &vtProp2, 0, 0);
                
                IEnumWbemClassObject* serviceQuery = NULL;
                IWbemClassObject* serviceQueryObject;

                std::wstring processId = std::to_wstring(vtProp2.uintVal);
                std::wstring query = L"SELECT * FROM Win32_Service WHERE ProcessId = " + processId;

                // Idle Process, too many services.
                if (processId == L"0") {
                    std::wcout
                        << std::left << std::setw(TAB) << vtProp.bstrVal
                        << std::setw(TAB) << vtProp2.uintVal
                        << std::right
                        << std::setw(TAB) << "N/A"
                        << std::endl;
                }
                else {
                    int counter = 0;

                    hres = pSvc->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t(query.c_str()),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &serviceQuery);

                    while (serviceQuery) {
                        hres = serviceQuery->Next(WBEM_INFINITE, 1,
                            &serviceQueryObject, &uReturn);

                        if (0 == uReturn)
                        {
                            break;
                        }

                        hres = serviceQueryObject->Get(L"DisplayName", 0, &vtProp3, 0, 0);
                        std::wcout
                            << std::left << std::setw(TAB) << vtProp.bstrVal
                            << std::setw(TAB) << vtProp2.uintVal
                            << std::right
                            << std::setw(TAB) << vtProp3.bstrVal
                            << std::endl;
                        counter++;

                    }

                    if (counter == 0) {
                        std::wcout
                            << std::left << std::setw(TAB) << vtProp.bstrVal
                            << std::setw(TAB) << vtProp2.uintVal
                            << std::right
                            << std::setw(TAB) << "N/A"
                            << std::endl;
                    }
                }


                //std::wcout
                  //  << std::left << std::setw(TAB) << vtProp.bstrVal
                    //<< std::setw(TAB) << vtProp3.bstrVal
                    //<< std::endl;

                VariantClear(&vtProp);
                VariantClear(&vtProp2);
                VariantClear(&vtProp3);
                VariantClear(&vtProp4);

                pclsObj->Release();
                pclsObj = NULL;
            }

        }

        // Cleanup
        // ========

        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();

        CoUninitialize();

        //std::getchar();

        return 0;   // Program successfully completed.
}