#include <windows.h>

HWND g_hWnd = NULL;
WNDPROC g_OrigWndProc = NULL;

// Перехватчик оконных сообщений (работает на уровне Windows)
LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Если Windows пытается забрать фокус у игры — блокируем это
    if (uMsg == WM_KILLFOCUS) {
        return 0; 
    }
    // Если игра сворачивается, подменяем статус на "Активно" (WA_ACTIVE)
    if (uMsg == WM_ACTIVATE) {
        if (LOWORD(wParam) == WA_INACTIVE) {
            return CallWindowProcA(g_OrigWndProc, hwnd, WM_ACTIVATE, WA_ACTIVE, lParam);
        }
    }
    // Аналогично для всего приложения в целом
    if (uMsg == WM_ACTIVATEAPP) {
        if (wParam == FALSE) {
            return CallWindowProcA(g_OrigWndProc, hwnd, WM_ACTIVATEAPP, TRUE, lParam);
        }
    }
    
    // Пропускаем остальные сообщения без изменений
    return CallWindowProcA(g_OrigWndProc, hwnd, uMsg, wParam, lParam);
}

// Функция для поиска окна игры
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    // Ищем окно, которое принадлежит процессу игры и является видимым
    if (pid == GetCurrentProcessId() && IsWindowVisible(hwnd)) {
        g_hWnd = hwnd;
        return FALSE; // Окно найдено, останавливаем поиск
    }
    return TRUE;
}

// Основной поток плагина
DWORD WINAPI SetupThread(LPVOID lpParam) {
    // 1. Ищем окно игры (может занять пару секунд при запуске лаунчера)
    for (int i = 0; i < 60; i++) {
        EnumWindows(EnumWindowsProc, 0);
        if (g_hWnd != NULL) {
            break;
        }
        Sleep(500);
    }

    // 2. Если окно найдено, устанавливаем системный хук на потерю фокуса
    if (g_hWnd != NULL) {
        g_OrigWndProc = (WNDPROC)SetWindowLongPtrA(g_hWnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
    }

    // 3. На всякий случай оставляем классические патчи
    DWORD oldProtect;
    if (VirtualProtect((void*)0x748A8D, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset((void*)0x748A8D, 0x90, 6);
        VirtualProtect((void*)0x748A8D, 6, oldProtect, &oldProtect);
    }
    if (VirtualProtect((void*)0x53EA88, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(BYTE*)0x53EA88 = 0xEB;
        VirtualProtect((void*)0x53EA88, 1, oldProtect, &oldProtect);
    }

    // 4. Ждем полной загрузки игры (когда игрок уже в мире)
    while (*(BYTE*)0xC8D4C0 != 9) {
        Sleep(500);
    }
    Sleep(2000); // Даем интерфейсу прогрузиться

    // 5. Выводим текст в угол (БЕЗ звука)
    typedef void(__cdecl* SetHelpMessage_t)(const char* text, bool quickMessage, bool permanent, bool addToBrief);
    SetHelpMessage_t SetHelpMessage = (SetHelpMessage_t)0x588BE0;
    
    // Защита от краша: если AMAZING изменили адрес функции вывода текста, 
    // игра просто проигнорирует команду и не вылетит.
    __try {
        SetHelpMessage("Anti-AFK loaded successfully!", true, false, false);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Ничего не делаем в случае ошибки
    }

    return 0;
}

// Точка входа
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, SetupThread, NULL, 0, NULL);
    }
    return TRUE;
}
