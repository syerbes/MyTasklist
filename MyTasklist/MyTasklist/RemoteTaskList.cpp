#define _WIN32_DCOM
#define UNICODE
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "credui.lib")
#pragma comment(lib, "comsuppw.lib")
#include <wincred.h>
#include <strsafe.h>
#include <String>
#include <iomanip>



int RemoteTaskList(wchar_t * domain, wchar_t * user, wchar_t * password, const char* mode) {
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return 1;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IDENTIFY,    // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;                    // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices* pSvc = NULL;

    // Get the user name and password for the remote computer from parameters
    bool useToken = false;
    bool useNTLM = true;
    wchar_t pszName[CREDUI_MAX_USERNAME_LENGTH + 1] = { 0 };
    wcscpy_s(pszName, user);
    wchar_t pszPwd[CREDUI_MAX_PASSWORD_LENGTH + 1] = { 0 };
    wcscpy_s(pszPwd, password);
    wchar_t pszDomain[CREDUI_MAX_USERNAME_LENGTH + 1];
    wcscpy_s(pszDomain, domain);
    wchar_t pszUserName[CREDUI_MAX_USERNAME_LENGTH + 1];
    wchar_t pszAuthority[CREDUI_MAX_USERNAME_LENGTH + 1];
    BOOL fSave;
    DWORD dwErr;


    // Change the computerName strings below to the full computer name
    // of the remote computer
    if (!useNTLM)
    {
        StringCchPrintf(pszAuthority, CREDUI_MAX_USERNAME_LENGTH + 1, L"kERBEROS:%s", L"COMPUTERNAME");
    }

    // Connect to the remote root\cimv2 namespace
    // and obtain pointer pSvc to make IWbemServices calls.
    //---------------------------------------------------------

    // Parsing my Domain
    std::wstring auxDomain = L"\\\\";
    // Length for Domain
    std::wstring auxLength = auxDomain.append(pszDomain);
    auxDomain.append(L"\\root\\cimv2");
    wchar_t* pszDomainAux = const_cast<wchar_t*>(auxDomain.c_str());
    wchar_t pszDomain2[CREDUI_MAX_USERNAME_LENGTH + 1] = { 0 };
    wcscpy_s(pszDomain2, pszDomainAux);

    // Connection to the specified system

    hres = pLoc->ConnectServer(
        _bstr_t(useToken ? NULL : pszDomain2),
        _bstr_t(useToken ? NULL : pszName),    // User name
        _bstr_t(useToken ? NULL : pszPwd),     // User password
        NULL,                              // Locale             
        NULL,                              // Security flags
        _bstr_t(useNTLM ? NULL : pszAuthority),// Authority        
        NULL,                              // Context object 
        &pSvc                              // IWbemServices proxy
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

    // step 5: --------------------------------------------------
    // Create COAUTHIDENTITY that can be used for setting security on proxy

    COAUTHIDENTITY* userAcct = NULL;
    COAUTHIDENTITY authIdent;

    if (!useToken)
    {
        memset(&authIdent, 0, sizeof(COAUTHIDENTITY));
        authIdent.PasswordLength = wcslen(pszPwd);
        authIdent.Password = (USHORT*)pszPwd;

        // Conversion to include \\ operator
        std::wstring auxName = L"\\";
        auxName.append(pszName);
        wchar_t* pszNameAux = const_cast<wchar_t*>(auxName.c_str());
        wchar_t pszName2[CREDUI_MAX_USERNAME_LENGTH + 1] = { 0 };
        wcscpy_s(pszName2, pszNameAux);

        LPWSTR slash = wcschr(pszName2, L'\\');

        if (slash == NULL)
        {
            cout << "Could not create Auth identity. No domain specified\n";
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }

        StringCchCopy(pszUserName, CREDUI_MAX_USERNAME_LENGTH + 1, slash + 1);
        authIdent.User = (USHORT*)pszUserName;
        authIdent.UserLength = wcslen(pszUserName);

        StringCchCopyN(pszDomain, CREDUI_MAX_USERNAME_LENGTH + 1, pszName2, slash - pszName2);
        authIdent.Domain = (USHORT*)pszDomain;
        authIdent.DomainLength = slash - pszName2;
        authIdent.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        userAcct = &authIdent;

    }

    // Step 6: --------------------------------------------------
    // Set security levels on a WMI connection ------------------

    hres = CoSetProxyBlanket(
        pSvc,                           // Indicates the proxy to set
        RPC_C_AUTHN_DEFAULT,            // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_DEFAULT,            // RPC_C_AUTHZ_xxx
        COLE_DEFAULT_PRINCIPAL,         // Server principal name 
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE,    // RPC_C_IMP_LEVEL_xxx
        userAcct,                       // client identity
        EOAC_NONE                       // proxy capabilities 
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

    // Step 7: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----
    //Create context to call GetUser Function for Verbose Mode

    BSTR ClassName = SysAllocString(L"Win32_Process");

    _bstr_t MethodName = (L"GetOwner");

    IWbemClassObject* pClass = NULL;

    hres = pSvc->GetObject(ClassName, 0, NULL, &pClass, NULL);

    if (FAILED(hres))
    {
        cout << "Could not get the object. Error code = 0x"
            << hex << hres << endl;
    }

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

    // Query for all processes
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

    // Security Context for pEnumerator function call
    hres = CoSetProxyBlanket(
        pEnumerator,                           // Indicates the proxy to set
        RPC_C_AUTHN_DEFAULT,            // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_DEFAULT,            // RPC_C_AUTHZ_xxx
        COLE_DEFAULT_PRINCIPAL,         // Server principal name 
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE,    // RPC_C_IMP_LEVEL_xxx
        userAcct,                       // client identity
        EOAC_NONE                       // proxy capabilities 
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

    IWbemClassObject* pclsObj;
    ULONG uReturn = 0;

    // Output design for standard tasklist
    if (mode == "Standard") {
        std::wcout << "Hemos llegado adonde hacía falta" << std::endl;
        int TAB = 20;
        std::cout
            << std::left << std::setw(30) << "Process"
            << std::setw(TAB) << "ID"
            << std::setw(TAB) << "Session Name"
            << std::setw(TAB) << "Session ID"
            << std::setw(TAB) << "Mem Usage (MB)"
            << std::endl;

        std::cout
            << std::left << "-------------------------------------------------------------------------------------------------------------"
            << std::endl;

        // Iterate processes
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

            // Get the value of the properties
            hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            hres = pclsObj->Get(L"ProcessID", 0, &vtProp2, 0, 0);
            hres = pclsObj->Get(L"SessionId", 0, &vtProp3, 0, 0);
            hres = pclsObj->Get(L"PageFileUsage", 0, &vtProp4, 0, 0);

            // Setting sessionName according to SessionId
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
                << std::setw(TAB) << vtProp4.uintVal / 1024
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

        // Layout
        int TAB = 20;

        std::cout
            << std::left << std::setw(30) << "Process"
            << std::setw(TAB) << "ID"
            << std::setw(TAB) << "Session Name"
            << std::setw(TAB*0.9) << "Session ID"
            << std::setw(TAB*0.9) << "Mem Usage (MB)"
            << std::setw(TAB*0.9) << "Status"
            << std::setw(TAB*1.5) << "User Name"
            << std::setw(TAB) << "CPU Time (Min)"
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
                cout << "mal";
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
            hres = pclsObj->Get(L"PageFileUsage", 0, &vtProp4, 0, 0);
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
            // Truncation of Names
            if (SysStringLen(vtProp.bstrVal) > 25) {
                std::wstring a(vtProp.bstrVal, 25);
                vtProp.bstrVal = SysAllocStringLen(a.data(), a.size());
            }

            if (SysStringLen(vtProp8.bstrVal) > 25) {
                std::wstring a(vtProp8.bstrVal, 25);
                vtProp8.bstrVal = SysAllocStringLen(a.data(), a.size());
            }

            // Translating execution state from int to String

            wchar_t* executionState;
            if (vtProp5.uintVal == 3) {
                executionState = const_cast <wchar_t*>(L"Running");
            }
            else {
                executionState = const_cast <wchar_t*>(L"Unkwown");
            }

            bool userFlag = FALSE;
            wstring domainUser;

            // Executing the GetOwner Method
            hres = pSvc->ExecMethod(vtPath.bstrVal, MethodName, 0, NULL, NULL, &pOutMethod, NULL);
            if (FAILED(hres))
            {
                userFlag = TRUE;
                domainUser = L"N/A";
            }
            // Formatting output of the previous method
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
                << std::setw(TAB*0.9) << vtProp3.uintVal
                << std::setw(TAB*0.9) << vtProp4.uintVal / 1024
                << std::setw(TAB*0.9) << executionState
                << std::setw(TAB*1.5) << domainUser
                << std::setw(TAB) << CPUUsage
                << std::setw(TAB) << "N/A"
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
        // Layout for SVC Mode
        int TAB = 30;
        std::cout
            << std::left << std::setw(TAB) << "Name"
            << std::setw(TAB*0.5) << "PID"
            << std::setw(TAB*1.5) << "Services"
            << std::endl;

        std::cout
            << std::left << "-------------------------------------------------------------------------------------------------------------"
            << std::endl;

        // Iterate through processes
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


            // Get the value of the properties
            hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            hres = pclsObj->Get(L"ProcessID", 0, &vtProp2, 0, 0);

            IEnumWbemClassObject* serviceQuery = NULL;
            IWbemClassObject* serviceQueryObject;

            std::wstring processId = std::to_wstring(vtProp2.uintVal);
            std::wstring query = L"SELECT * FROM Win32_Service WHERE ProcessId = " + processId;

            // Idle Process, too many services. Skipped
            if (processId == L"0") {
                std::wcout
                    << std::left << std::setw(TAB) << vtProp.bstrVal
                    << std::setw(TAB*0.5) << vtProp2.uintVal
                    << std::setw(TAB*1.5) << "N/A"
                    << std::endl;
            }
            else {
                int counter = 0;
                // Create security context for new query
                hres = CoSetProxyBlanket(
                    pSvc,                    // Indicates the proxy to set
                    RPC_C_AUTHN_DEFAULT,            // RPC_C_AUTHN_xxx
                    RPC_C_AUTHZ_DEFAULT,            // RPC_C_AUTHZ_xxx
                    COLE_DEFAULT_PRINCIPAL,         // Server principal name 
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // RPC_C_AUTHN_LEVEL_xxx 
                    RPC_C_IMP_LEVEL_IMPERSONATE,    // RPC_C_IMP_LEVEL_xxx
                    userAcct,                       // client identity
                    EOAC_NONE                       // proxy capabilities 
                );

                if (FAILED(hres))
                {
                    cout << "Could not set proxy blanket on enumerator. Error code = 0x"
                        << hex << hres << endl;
                    pEnumerator->Release();
                    pSvc->Release();
                    pLoc->Release();
                    CoUninitialize();
                    return 1;               // Program has failed.
                }

                hres = pSvc->ExecQuery(
                    bstr_t("WQL"),
                    bstr_t(query.c_str()),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &serviceQuery);

                // Create security context for posterior iteration
                hres = CoSetProxyBlanket(
                    serviceQuery,                    // Indicates the proxy to set
                    RPC_C_AUTHN_DEFAULT,            // RPC_C_AUTHN_xxx
                    RPC_C_AUTHZ_DEFAULT,            // RPC_C_AUTHZ_xxx
                    COLE_DEFAULT_PRINCIPAL,         // Server principal name 
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // RPC_C_AUTHN_LEVEL_xxx 
                    RPC_C_IMP_LEVEL_IMPERSONATE,    // RPC_C_IMP_LEVEL_xxx
                    userAcct,                       // client identity
                    EOAC_NONE                       // proxy capabilities 
                );

                if (FAILED(hres))
                {
                    cout << "Could not set proxy blanket on enumerator. Error code = 0x"
                        << hex << hres << endl;
                    pEnumerator->Release();
                    pSvc->Release();
                    pLoc->Release();
                    CoUninitialize();
                    return 1;               // Program has failed.
                }

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
                        << std::setw(TAB*0.5) << vtProp2.uintVal
                        << std::setw(TAB*1.5) << vtProp3.bstrVal
                        << std::endl;
                    counter++;
                }
                // If there are not services associated with a PID
                if (counter == 0) {
                    std::wcout
                        << std::left << std::setw(TAB) << vtProp.bstrVal
                        << std::setw(TAB*0.5) << vtProp2.uintVal
                        << std::setw(TAB*1.5) << "N/A"
                        << std::endl;
                }
            }


            VariantClear(&vtProp);
            VariantClear(&vtProp2);
            VariantClear(&vtProp3);
            VariantClear(&vtProp4);

            pclsObj->Release();
            pclsObj = NULL;
        }

    }

    // Deleting credentials

    SecureZeroMemory(pszName, sizeof(pszName));
    SecureZeroMemory(pszPwd, sizeof(pszPwd));
    SecureZeroMemory(pszUserName, sizeof(pszUserName));
    SecureZeroMemory(pszDomain, sizeof(pszDomain));


    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();


    CoUninitialize();

    return 0;   // Program successfully completed.

}