// Compile with : g++ check_ram -o check_ram

#include <windows.h>
#include <iostream>

int main() {
    MEMORYSTATUSEX memStatus;
    
    // CRITICAL: dwLength must be initialized before calling GlobalMemoryStatusEx
    memStatus.dwLength = sizeof(memStatus);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        std::cerr << "[-] Failed to retrieve memory status. Error code: " << GetLastError() << "\n";
        return 1;
    }

    const DWORDLONG GB = 1024 * 1024 * 1024;

    std::cout << "[*] Current RAM Statistics:\n";
    std::cout << "----------------------------------------------\n";
    
    std::cout << " -> Memory Load        : " << memStatus.dwMemoryLoad << "%\n";
    std::cout << " -> Total Physical RAM : " << (memStatus.ullTotalPhys / GB) << " GB (" << memStatus.ullTotalPhys << " bytes)\n";
    std::cout << " -> Avail Physical RAM : " << (memStatus.ullAvailPhys / GB) << " GB (" << memStatus.ullAvailPhys << " bytes)\n";
    std::cout << " -> Total Pagefile     : " << (memStatus.ullTotalPageFile / GB) << " GB\n";
    std::cout << " -> Avail Pagefile     : " << (memStatus.ullAvailPageFile / GB) << " GB\n";
    std::cout << " -> Total Virtual Mem  : " << (memStatus.ullTotalVirtual / GB) << " GB\n";

    std::cout << "----------------------------------------------\n";
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}
