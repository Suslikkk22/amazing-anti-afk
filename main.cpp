#include <windows.h>
#include <fstream>

// Функция для безопасной записи в память с проверкой прав доступа
bool WriteMemory(DWORD address, const void* value, int size) {
    DWORD oldProtect;
    // Проверяем, можем ли мы вообще получить права на запись к этому участку памяти
    if (VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((void*)address, value, size);
        VirtualProtect((void*)address, size, oldProtect, &oldProtect);
        return true;
    }
    return false;
}

DWORD WINAPI AntiAfkThread(LPVOID lpParam) {
    std::ofstream log("AmazingAntiAFK_Log.txt");
    if (!log.is_open()) return 0;

    log << "[1/3] Плагин запущен и ожидает..." << std::endl;

    // СЕКРЕТНЫЙ ШАГ: Долгое ожидание
    // Мы даем лаунчеру Amazing Online и античиту 30 секунд для полной загрузки
    // и прохождения начальных проверок целостности памяти.
    // Тогда мы бьем один раз, когда всё успокоилось.
    Sleep(30000); 

    log << "[2/3] Ожидание завершено. Применение патчей..." << std::endl;

    // ОРИГИНАЛЬНЫЕ ПАТЧИ ДЛЯ GTA SA v1.0
    // Если игра всё ещё крашится, вероятно, эти адреса (0x53EA88, 0x748A8D)
    // не верны для текущей версии Amazing Online!
    BYTE nop5[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
    BYTE nop6[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

    // Устраняем агрессию: применяем патчи ТОЛЬКО один раз. Это НАМНОГО стабильнее.
    bool successPause = WriteMemory(0x53EA88, nop5, 5); // Отключаем CTimer::Suspend
    bool successRender = WriteMemory(0x748A8D, nop6, 6); // RsEventHandler (рендеринг в фоне)

    log << "[3/3] Патчи применены. Статус:" << std::endl;
    log << "  Патч паузы: " << (successPause ? "УСПЕХ" : "ОШИБКА (нет доступа к памяти)") << std::endl;
    log << "  Патч рендера: " << (successRender ? "УСПЕХ" : "ОШИБКА (нет доступа к памяти)") << std::endl;
    log.close();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, AntiAfkThread, hModule, 0, nullptr);
    }
    return TRUE;
}
