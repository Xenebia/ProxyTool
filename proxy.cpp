#include "proxy.h"
#include "config.h"

#include <windows.h>
#include <wininet.h>
#include <string>

#pragma comment(lib, "wininet.lib")

bool setProxy()
{
    HKEY key = nullptr;

    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        0,
        KEY_SET_VALUE,
        &key
    );

    if (result != ERROR_SUCCESS)
        return false;

    DWORD enabled = 1;

    if (RegSetValueExA(
        key,
        "ProxyEnable",
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&enabled),
        sizeof(enabled)
    ) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return false;
    }

    std::string proxy =
        std::string(PROXY_ADDRESS) + ":" + PROXY_PORT;

    if (RegSetValueExA(
        key,
        "ProxyServer",
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(proxy.c_str()),
        static_cast<DWORD>(proxy.size() + 1)
    ) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return false;
    }

    RegCloseKey(key);

    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);

    return true;
}

bool disableProxy()
{
    HKEY key = nullptr;

    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        0,
        KEY_SET_VALUE,
        &key
    );

    if (result != ERROR_SUCCESS)
        return false;

    DWORD disabled = 0;

    bool ok =
        RegSetValueExA(
            key,
            "ProxyEnable",
            0,
            REG_DWORD,
            reinterpret_cast<const BYTE*>(&disabled),
            sizeof(disabled)
        ) == ERROR_SUCCESS;

    RegCloseKey(key);

    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);

    return ok;
}
