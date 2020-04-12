#define _WIN32_WINNT 0x0400
#define _WIN32_DCOM

#include <stdlib.h>
#include <windows.h>
#include <wbemidl.h>

extern void new_table(void *state, const char *name);
extern void add_entry(void *state, const char *key, const char *value);
extern void close_table(void *state);

#define get_component(component) \
{\
    VARIANT component;\
    hr = result->lpVtbl->Get(result, L#component, 0, &component, 0, 0);\
    if (hr == S_OK)\
    {\
        size_t len = wcslen(component.bstrVal) * 2;\
        char *s = (char *)malloc(len + 1);\
        wcstombs(s, component.bstrVal, len);\
        add_entry(state, #component, s);\
        free(s);\
    }\
}

void fill_systable(void *state)
{
    // result code from COM calls
    HRESULT hr = 0;

    // COM interface pointers
    IWbemLocator         *locator  = NULL;
    IWbemServices        *services = NULL;
    IEnumWbemClassObject *results  = NULL;

    // BSTR strings we'll use (http://msdn.microsoft.com/en-us/library/ms221069.aspx)
    BSTR resource = SysAllocString(L"ROOT\\CIMV2");
    BSTR language = SysAllocString(L"WQL");
    BSTR query    = SysAllocString(L"SELECT * FROM Win32_BaseBoard");

    // initialize COM
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    // connect to WMI
    hr = CoCreateInstance(&CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, &IID_IWbemLocator, (LPVOID *) &locator);
    hr = locator->lpVtbl->ConnectServer(locator, resource, NULL, NULL, NULL, 0, NULL, NULL, &services);

    // issue a WMI query
    hr = services->lpVtbl->ExecQuery(services, language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

    // list the query results
    if (results != NULL) 
    {
        IWbemClassObject *result = NULL;
        ULONG returnedCount = 0;

        new_table(state, "BaseBoard");

        // enumerate the retrieved objects
        while((hr = results->lpVtbl->Next(results, WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK) 
        {
            get_component(Caption)
            get_component(Description)
            get_component(Manufacturer)
            get_component(Model)
            get_component(OtherIdentifyingInfo)
            get_component(PartNumber)
            get_component(Product)
            get_component(SKU)
            get_component(Status)
            //get_component(Tag)
            get_component(Version)
            get_component(SerialNumber)

            // release the current result object
            result->lpVtbl->Release(result);

            break;
        }

        close_table(state);

    }

    query    = SysAllocString(L"SELECT * FROM Win32_BIOS");

    // issue a WMI query
    hr = services->lpVtbl->ExecQuery(services, language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

    // list the query results
    if (results != NULL) 
    {
        IWbemClassObject *result = NULL;
        ULONG returnedCount = 0;

        new_table(state, "BIOS");

        // enumerate the retrieved objects
        while((hr = results->lpVtbl->Next(results, WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK) 
        {
            get_component(SMBIOSBIOSVersion)
            get_component(BIOSVersion)
            get_component(BuildNumber)
            get_component(Description)
            get_component(IdentificationCode)
            get_component(Manufacturer)
            get_component(SerialNumber)
            get_component(SoftwareElementID)
            get_component(Version)

            // release the current result object
            result->lpVtbl->Release(result);

            break;
        }

        close_table(state);

    }

    query    = SysAllocString(L"SELECT * FROM Win32_MotherboardDevice");

    // issue a WMI query
    hr = services->lpVtbl->ExecQuery(services, language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

    // list the query results
    if (results != NULL) 
    {
        IWbemClassObject *result = NULL;
        ULONG returnedCount = 0;

        new_table(state, "Motherboard");

        // enumerate the retrieved objects
        while((hr = results->lpVtbl->Next(results, WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK) 
        {
            get_component(Caption)
            get_component(DeviceID)
            get_component(PNPDeviceID)
            get_component(RevisionNumber)
            get_component(SystemName)

            // release the current result object
            result->lpVtbl->Release(result);

            break;
        }

        close_table(state);

    }

    query    = SysAllocString(L"SELECT * FROM Win32_ComputerSystem");

    // issue a WMI query
    hr = services->lpVtbl->ExecQuery(services, language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

    // list the query results
    if (results != NULL) 
    {
        IWbemClassObject *result = NULL;
        ULONG returnedCount = 0;

        new_table(state, "Computer");

        // enumerate the retrieved objects
        while((hr = results->lpVtbl->Next(results, WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK) 
        {
            get_component(Caption)
            get_component(ChassisSKUNumber)
            get_component(Description)
            get_component(Manufacturer)
            get_component(Model)
            get_component(Name)
            get_component(NameFormat)
            get_component(SystemFamily)
            get_component(SystemSKUNumber)

            // release the current result object
            result->lpVtbl->Release(result);

            break;
        }

        close_table(state);

    }

    // release WMI COM interfaces
    results->lpVtbl->Release(results);
    services->lpVtbl->Release(services);
    locator->lpVtbl->Release(locator);

    // unwind everything else we've allocated
    CoUninitialize();
}
