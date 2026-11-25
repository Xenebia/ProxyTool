#include <windows.h>
#include <shellapi.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "config.h"
#include "proxy.h"

namespace fs = std::filesystem;

// ------------------------------------------------------------
// IDs des contrôles
// ------------------------------------------------------------

constexpr int ID_PASSWORD = 1001;
constexpr int ID_LOGIN    = 1002;
constexpr int ID_ENABLE   = 1003;
constexpr int ID_DISABLE  = 1004;
constexpr int ID_LOG      = 1005;
constexpr int ID_STATUS   = 1006;
constexpr int ID_OPEN_LOG = 1007;

// ------------------------------------------------------------
// Variables globales
// ------------------------------------------------------------

HWND g_password = nullptr;
HWND g_status   = nullptr;
HWND g_enable   = nullptr;
HWND g_disable  = nullptr;
HFONT g_font     = nullptr;
HFONT g_titleFont = nullptr;

// ------------------------------------------------------------
// Journal
// ------------------------------------------------------------

std::string getDateTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

std::filesystem::path getAuditFilePath()
{
    wchar_t localAppData[MAX_PATH]{};
    DWORD length = GetEnvironmentVariableW(
        L"LOCALAPPDATA", localAppData, MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH)
        return std::filesystem::path(L"ProxyTool\\audit.log");

    return std::filesystem::path(localAppData) / L"ProxyTool" / L"audit.log";
}

void writeAudit(const std::string& action)
{
    try
    {
        const auto auditPath = getAuditFilePath();
        fs::create_directories(auditPath.parent_path());

        std::ofstream file(auditPath, std::ios::app);
        if (!file)
            return;

        file << "[" << getDateTime() << "] " << action << "\n";
    }
    catch (...)
    {
    }
}

// ------------------------------------------------------------
// UI
// ------------------------------------------------------------

void setStatus(const wchar_t* text, bool success)
{
    SetWindowTextW(g_status, text);

    if (success)
        EnableWindow(g_disable, TRUE);
}

void updateButtons()
{
    EnableWindow(g_enable, TRUE);
    EnableWindow(g_disable, TRUE);
}

void authenticate(HWND hwnd)
{
    wchar_t buffer[256]{};

    GetWindowTextW(
        g_password,
        buffer,
        static_cast<int>(std::size(buffer))
    );

    std::wstring entered(buffer);

    // Le mot de passe est défini en wchar_t dans config.h :
    // comparaison directe, sans conversion de caractères.
    if (entered != PROGRAM_PASSWORD)
    {
        SetWindowTextW(
            g_status,
            L"Mot de passe incorrect."
        );

        writeAudit("Tentative avec un mot de passe incorrect.");

        MessageBoxW(
            hwnd,
            L"Le mot de passe est incorrect.",
            L"ProxyTool",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    writeAudit("Authentification reussie.");

    EnableWindow(g_password, FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_LOGIN), FALSE);

    EnableWindow(g_enable, TRUE);
    EnableWindow(g_disable, TRUE);

    SetWindowTextW(
        g_status,
        L"Authentification reussie. Vous pouvez utiliser le proxy."
    );

    SetWindowTextW(
        g_password,
        L""
    );
}

void enableProxy(HWND hwnd)
{
    if (!setProxy())
    {
        SetWindowTextW(g_status, L"Erreur lors de l'activation du proxy.");

        writeAudit(
            "ERREUR : impossible de configurer le proxy."
        );

        MessageBoxW(
            hwnd,
            L"Windows n'a pas pu modifier la configuration du proxy.",
            L"ProxyTool",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    std::string audit =
        "Proxy active : " +
        std::string(PROXY_ADDRESS) +
        ":" +
        std::string(PROXY_PORT);

    writeAudit(audit);

    SetWindowTextW(
        g_status,
        L"Proxy active."
    );

    MessageBoxW(
        hwnd,
        L"Le proxy a ete active.",
        L"ProxyTool",
        MB_OK | MB_ICONINFORMATION
    );
}

void disableProxyGUI(HWND hwnd)
{
    if (!disableProxy())
    {
        SetWindowTextW(g_status, L"Erreur lors de la desactivation.");

        writeAudit(
            "ERREUR : impossible de desactiver le proxy."
        );

        MessageBoxW(
            hwnd,
            L"Windows n'a pas pu desactiver le proxy.",
            L"ProxyTool",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    writeAudit("Proxy desactive.");

    SetWindowTextW(
        g_status,
        L"Proxy desactive."
    );

    MessageBoxW(
        hwnd,
        L"Le proxy a ete desactive.",
        L"ProxyTool",
        MB_OK | MB_ICONINFORMATION
    );
}

// ------------------------------------------------------------
// Fenêtre
// ------------------------------------------------------------

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            g_font = CreateFontW(
                -16, 0, 0, 0,
                FW_NORMAL,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Segoe UI"
            );

            g_titleFont = CreateFontW(
                -26, 0, 0, 0,
                FW_SEMIBOLD,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Segoe UI"
            );

            HWND title = CreateWindowW(
                L"STATIC",
                L"ProxyTool",
                WS_CHILD | WS_VISIBLE,
                30, 25, 500, 40,
                hwnd,
                nullptr,
                nullptr,
                nullptr
            );

            SendMessageW(title, WM_SETFONT, (WPARAM)g_titleFont, TRUE);

            HWND description = CreateWindowW(
                L"STATIC",
                L"Configuration du proxy Windows",
                WS_CHILD | WS_VISIBLE,
                32, 70, 500, 25,
                hwnd,
                nullptr,
                nullptr,
                nullptr
            );

            SendMessageW(description, WM_SETFONT, (WPARAM)g_font, TRUE);

            HWND passwordLabel = CreateWindowW(
                L"STATIC",
                L"Mot de passe",
                WS_CHILD | WS_VISIBLE,
                32, 115, 200, 25,
                hwnd,
                nullptr,
                nullptr,
                nullptr
            );

            SendMessageW(passwordLabel, WM_SETFONT, (WPARAM)g_font, TRUE);

            g_password = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"EDIT",
                L"",
                WS_CHILD | WS_VISIBLE |
                ES_PASSWORD | ES_AUTOHSCROLL,
                32, 145, 330, 34,
                hwnd,
                (HMENU)ID_PASSWORD,
                nullptr,
                nullptr
            );

            SendMessageW(g_password, WM_SETFONT, (WPARAM)g_font, TRUE);

            HWND login = CreateWindowW(
                L"BUTTON",
                L"Se connecter",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                375, 145, 150, 34,
                hwnd,
                (HMENU)ID_LOGIN,
                nullptr,
                nullptr
            );

            SendMessageW(login, WM_SETFONT, (WPARAM)g_font, TRUE);

            g_enable = CreateWindowW(
                L"BUTTON",
                L"Activer le proxy",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                32, 210, 235, 45,
                hwnd,
                (HMENU)ID_ENABLE,
                nullptr,
                nullptr
            );

            SendMessageW(g_enable, WM_SETFONT, (WPARAM)g_font, TRUE);

            g_disable = CreateWindowW(
                L"BUTTON",
                L"Desactiver le proxy",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                290, 210, 235, 45,
                hwnd,
                (HMENU)ID_DISABLE,
                nullptr,
                nullptr
            );

            SendMessageW(g_disable, WM_SETFONT, (WPARAM)g_font, TRUE);

            g_status = CreateWindowW(
                L"STATIC",
                L"Entrez le mot de passe pour commencer.",
                WS_CHILD | WS_VISIBLE,
                32, 275, 500, 55,
                hwnd,
                (HMENU)ID_STATUS,
                nullptr,
                nullptr
            );

            SendMessageW(g_status, WM_SETFONT, (WPARAM)g_font, TRUE);

            const std::wstring logLabel =
                L"Journal : " + getAuditFilePath().wstring();

            HWND logInfo = CreateWindowW(
                L"STATIC",
                logLabel.c_str(),
                WS_CHILD | WS_VISIBLE,
                32, 345, 520, 25,
                hwnd,
                (HMENU)ID_LOG,
                nullptr,
                nullptr
            );

            SendMessageW(logInfo, WM_SETFONT, (WPARAM)g_font, TRUE);

            HWND openLog = CreateWindowW(
                L"BUTTON",
                L"Ouvrir le journal",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                32, 380, 180, 34,
                hwnd,
                (HMENU)ID_OPEN_LOG,
                nullptr,
                nullptr
            );
            SendMessageW(openLog, WM_SETFONT, (WPARAM)g_font, TRUE);

            EnableWindow(g_enable, FALSE);
            EnableWindow(g_disable, FALSE);

            writeAudit("Programme demarre.");

            return 0;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_LOGIN:
                    authenticate(hwnd);
                    return 0;

                case ID_ENABLE:
                    enableProxy(hwnd);
                    return 0;

                case ID_DISABLE:
                    disableProxyGUI(hwnd);
                    return 0;

                case ID_OPEN_LOG:
                {
                    const auto path = getAuditFilePath();
                    fs::create_directories(path.parent_path());
                    if (!fs::exists(path))
                        writeAudit("Journal initialise.");
                    ShellExecuteW(hwnd, L"open", L"notepad.exe", path.c_str(), nullptr, SW_SHOWNORMAL);
                    return 0;
                }
            }

            break;
        }

        case WM_DESTROY:
        {
            writeAudit("Programme ferme.");

            if (g_font)
                DeleteObject(g_font);

            if (g_titleFont)
                DeleteObject(g_titleFont);

            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ------------------------------------------------------------
// WinMain
// ------------------------------------------------------------

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"ProxyToolWindow";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc))
        return 1;

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"ProxyTool",
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        590,
        490,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hwnd)
        return 1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};

    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
