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
    // Создаем лог-файл в папке с игрой, чтобы понимать, что происходит
    std::ofstream log("AmazingAntiAFK_Log.txt");
    log << "[1/3] Плагин успешно загружен ASI Loader-ом!" << std::endl;

    // СЕКРЕТНЫЙ ШАГ: Ждем 15 секунд. 
    // Даем лаунчеру Amazing Online и мультиплееру полностью загрузиться,
    // чтобы они не смогли перезаписать наши изменения своими.
    log << "[2/3] Ожидание 15 секунд для обхода систем мультиплеера..." << std::endl;
    Sleep(15000); 

    // NOP-пустышки
    BYTE nop5[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
    BYTE nop6[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

    // Применяем патчи
    WriteMemory(0x53EA88, nop5, 5); // Отключаем CTimer::Suspend
    WriteMemory(0x748A8D, nop6, 6); // Отключаем остановку рендера

    log << "[3/3] Память успешно пропатчена! Anti-AFK должен работать." << std::endl;
    log.close(); // Закрываем лог

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
