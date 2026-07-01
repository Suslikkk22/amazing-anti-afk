#include <windows.h>

void WriteMemory(DWORD address, const void* value, int size) {
    DWORD oldProtect;
    VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)address, value, size);
    VirtualProtect((void*)address, size, oldProtect, &oldProtect);
}

void InitAntiAFK() {
    BYTE nop5[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
    BYTE nop6[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

    // Патчи для отключения паузы GTA SA при сворачивании
    WriteMemory(0x53EA88, nop5, 5);
    WriteMemory(0x748A8D, nop6, 6);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        InitAntiAFK();
    }
    return TRUE;
}
