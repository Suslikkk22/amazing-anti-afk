#include <windows.h>

// Функция для патча памяти игры (Anti-AFK)
void ApplyAntiAfkPatch() {
    DWORD oldProtect;

    // Патч 1: Отключаем автоматический выход в меню
    if (VirtualProtect((void*)0x748A8D, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset((void*)0x748A8D, 0x90, 6);
        VirtualProtect((void*)0x748A8D, 6, oldProtect, &oldProtect);
    }

    // Патч 2: Отключаем паузу игры
    if (VirtualProtect((void*)0x53EA88, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(BYTE*)0x53EA88 = 0xEB;
        VirtualProtect((void*)0x53EA88, 1, oldProtect, &oldProtect);
    }
}

// Поток, который ждет загрузки игры и выводит сообщение
DWORD WINAPI SetupThread(LPVOID lpParam) {
    // Применяем патчи сразу
    ApplyAntiAfkPatch();

    // Адрес 0xC8D4C0 хранит текущее состояние игры. 
    // Значение 9 означает, что игра полностью загружена и мы в мире.
    while (*(BYTE*)0xC8D4C0 != 9) {
        Sleep(500); // Проверяем каждые полсекунды
    }

    // Немного ждем, чтобы экран успел прогрузиться после коннекта
    Sleep(2000);

    // Объявляем оригинальную функцию GTA SA для вывода текста в левый верхний угол (Help Message)
    // Адрес функции для версии 1.0 US — 0x588BE0
    typedef void(__cdecl* SetHelpMessage_t)(const char* text, bool quickMessage, bool permanent, bool addToBrief);
    SetHelpMessage_t SetHelpMessage = (SetHelpMessage_t)0x588BE0;

    // Выводим сообщение
    SetHelpMessage("Anti-AFK loaded successfully!", true, false, false);

    // В качестве бонуса — системный звуковой сигнал (бип) при успешной загрузке
    Beep(750, 300);

    return 0;
}

// Точка входа в библиотеку
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            // Создаем поток, чтобы не заморозить игру во время инициализации
            CreateThread(NULL, 0, SetupThread, NULL, 0, NULL);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
