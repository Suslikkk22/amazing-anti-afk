#include <windows.h>
#include <fstream> // Библиотека для создания лог-файла

// Функция записи в память
void WriteMemory(DWORD address, const void* value, int size) {
    DWORD oldProtect;
    VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)address, value, size);
    VirtualProtect((void*)address, size, oldProtect, &oldProtect);
}

// Наш рабочий поток, который будет работать независимо от запуска игры
DWORD WINAPI AntiAfkThread(LPVOID lpParam) {
    std::ofstream log("AmazingAntiAFK_Log.txt");
    if (log.is_open()) {
        log << "[1/3] Плагин успешно загружен ASI Loader-ом!" << std::endl;
        log << "[2/3] Ожидание 15 секунд для полной загрузки игры..." << std::endl;
        log.close();
    }

    // Ждем 15 секунд, пока игра и мультиплеер полностью загрузятся
    Sleep(15000); 

    // NOP-пустышки
    BYTE nop5[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
    BYTE nop6[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    BYTE jmpByte = 0xEB; // Байт безусловного перехода JMP вместо условного JZ

    std::ofstream logAppend("AmazingAntiAFK_Log.txt", std::ios_base::app);
    if (logAppend.is_open()) {
        logAppend << "[3/3] Агрессивный режим запущен. Патчим память каждые 500мс." << std::endl;
        logAppend.close();
    }

    // АГРЕССИВНЫЙ ЦИКЛ: противодействуем восстановлению памяти со стороны лаунчера
    while (true) {
        // 1. Отключаем CTimer::Suspend (остановка игрового времени/таймеров деморгана)
        WriteMemory(0x53EA88, nop5, 5); 

        // 2. Отключаем остановку рендера RsEventHandler (игра продолжает крутиться в фоне)
        WriteMemory(0x748A8D, nop6, 6); 

        // 3. Блокируем автоматический вызов меню ESC при потере фокуса окна
        WriteMemory(0x53F415, &jmpByte, 1);

        // Спим полсекунды и заново перезаписываем байты памяти
        Sleep(500);
    }

    return 0;
}

// Точка входа в плагин
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        // Создаем отдельный процесс (поток), чтобы игра не зависла при ожидании 15 секунд
        CreateThread(nullptr, 0, AntiAfkThread, hModule, 0, nullptr);
    }
    return TRUE;
}
