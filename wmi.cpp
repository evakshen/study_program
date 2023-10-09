#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#define _WIN32_DCOM
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

class system_info
{
public:
    ~system_info(void);
    system_info(const char *WMI_namespace = "ROOT\\CIMV2");
    int ExecQuery(const char *strQuery, const char *strProperty, int in_property, VARIANT *vtProp);

private:
    HRESULT hres;
    IWbemServices *pSvc;
    IWbemLocator *pLoc;
};

system_info::~system_info(void)
{
    // Cleanup
    // ========

    if (SUCCEEDED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
    }
}

system_info::system_info(const char *WMI_namespace) : hres(E_FAIL), pSvc(nullptr), pLoc(nullptr)
{
    // Step 1: --------------------------------------------------
    // Initialize COM -------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        printf("Failed to initialize COM library. Errno = 0x%lX\n", hres);
        return;
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,                        // The access permissions
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    );
    if (FAILED(hres))
    {
        printf("Failed to initialize security. Errno = 0x%lX\n", hres);
        CoUninitialize();
        return;
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    hres = CoCreateInstance(
        CLSID_WbemLocator,    // The CLSID associated with the data.
        0,                    // Not as part of an aggregate.
        CLSCTX_INPROC_SERVER, // Execution contexts.
        IID_IWbemLocator,     // A reference to the identifier.
        (LPVOID *)&pLoc       // receives the interface pointer requested in riid.
    );
    if (FAILED(hres))
    {
        printf("Failed to create IWbemLocator object. Errno = 0x%lX\n", hres);
        CoUninitialize();
        return;
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    hres = pLoc->ConnectServer(
        _bstr_t(WMI_namespace), // Object path of WMI namespace
        NULL,                   // User name. NULL = current user
        NULL,                   // User password. NULL = current
        0,                      // Locale. NULL indicates current
        NULL,                   // Security flags.
        0,                      // Authority (for example, Kerberos)
        0,                      // Context object
        &pSvc                   // pointer to IWbemServices proxy
    );
    if (FAILED(hres))
    {
        printf("Could not connect IWbemServices. Errno = 0x%lX\n", hres);
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
    );
    if (FAILED(hres))
    {
        printf("Could not set proxy blanket. Errno = 0x%lX\n", hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }
}

int system_info::ExecQuery(const char *strQuery, const char *strProperty, int in_property, VARIANT *vtProp)
{
    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    HRESULT QueryRes;
    IEnumWbemClassObject *pEnumerator = nullptr;

    if (FAILED(hres))
    {
        return 0;
    }

    QueryRes = pSvc->ExecQuery(
        _bstr_t("WQL"),                                        // Query languages : WQL
        _bstr_t(strQuery),                                     // The text of the query
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, // Flags
        NULL,                                                  // A pointer to an IWbemContext
        &pEnumerator                                           // The instances in the result
    );
    if (FAILED(QueryRes))
    {
        printf("Query for %s failed. Errno = 0x%lX\n", strQuery, QueryRes);
        return 0;
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    int out_property = 0;
    IWbemClassObject *pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        // Retrieve data objects
        QueryRes = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (FAILED(QueryRes))
        {
            printf("Retrieve data objects failed. Errno = 0x%lX\n", QueryRes);
        }

        if (0 == uReturn)
        {
            break;
        }

        // Get the value of the desired property
        QueryRes = pclsObj->Get(_bstr_t(strProperty), 0, &vtProp[out_property], 0, 0);
        if (FAILED(QueryRes))
        {
            printf("Get property %s failed. Errno = 0x%lX\n", strProperty, QueryRes);
        }
        else
        {
            out_property++;
            if (out_property > in_property)
            {
                printf("Property %s array %d isn't enough.\n", strProperty, in_property);
                break;
            }
        }

        pclsObj->Release();
        uReturn = 0;
    }

    pEnumerator->Release();

    return out_property;
}

// Needed runas administrator
void pc_temperature_read(int *temp_cpu0, int *temp_cpu1)
{
    int n_core = 0, sumTemp = 0;
    VARIANT temperature[64];
    static system_info sys("ROOT\\WMI");

    n_core = sys.ExecQuery("SELECT * FROM MSAcpi_ThermalZoneTemperature", "CurrentTemperature", 64, temperature);

    // Get temperature for APP
    if (2 == n_core)
    {
        *temp_cpu0 = temperature[0].intVal - 2732;
        *temp_cpu1 = temperature[1].intVal - 2732;
    }
    else
    {
        if (n_core > 0)
        {
            for (int i = 0; i < n_core; i++)
            {
                sumTemp += temperature[i].intVal - 2732;
            }
            *temp_cpu0 = sumTemp / n_core;
            *temp_cpu1 = sumTemp / n_core;
        }
    }
}

#elif defined __linux

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void pc_temperature_read(int *temp_cpu0, int *temp_cpu1)
{
    int fd;
    int i, read_ret[2] = {0};
    char buff[128];
    const char *thermal[2] = {
        "/sys/devices/virtual/thermal/thermal_zone0/temp",
        "/sys/devices/virtual/thermal/thermal_zone1/temp"};

    for (i = 0; i < 2; i++)
    {
        fd = open(thermal[i], O_RDONLY);
        if (fd < 0)
        {
            printf("Open %s %s\n", thermal[i], strerror(errno));
            continue;
        }

        read_ret[i] = read(fd, buff, 128);
        if (read_ret[i] < 0)
        {
            printf("Read %s %s\n", thermal[i], strerror(errno));
        }
        else
        {
            read_ret[i] = atoi(buff);
        }

        close(fd);
    }

    if (read_ret[0] > 0)
    {
        *temp_cpu0 = read_ret[0] / 100;
    }

    if (read_ret[1] > 0)
    {
        *temp_cpu1 = read_ret[1] / 100;
    }
}

#endif

int main(int argc, char **argv)
{
    int temp_cpu0 = 0, temp_cpu1 = 0;

    pc_temperature_read(&temp_cpu0, &temp_cpu1);
    printf("Temperature: %d %d\n", temp_cpu0, temp_cpu1);
    return 0;
}
