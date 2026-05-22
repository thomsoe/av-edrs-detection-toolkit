#include <windows.h>
#include <iostream>

int main() {
    // Get the number of milliseconds elapsed since the system started
    ULONGLONG uptimeMS = GetTickCount64();

    // Convert milliseconds to hours
    // 1 Hour = 60 minutes * 60 seconds * 1000 milliseconds
    ULONGLONG uptimeHours = uptimeMS / (3600 * 1000);

    std::cout << "[*] System Uptime: " << uptimeHours << " hours (" << uptimeMS << " ms)\n";

    if (uptimeHours < 1) {
        std::cout << "[-] Short system uptime detected.\n";
    } else {
        std::cout << "[+] System uptime verification passed.\n";
    }

    return 0;
}
