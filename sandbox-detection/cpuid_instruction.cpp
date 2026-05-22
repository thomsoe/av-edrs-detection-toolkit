// Compile with gcc or Visual Studio

#include <iostream>
#include <string>
#include <cstring>

#if defined(_MSC_VER)
    #include <intrin.h>
#elif defined(__GNUC__)
    #include <cpuid.h>
#endif

int main() {
    // Array to store the 4 registers returned by CPUID (EAX, EBX, ECX, EDX)
    int cpuInfo[4] = { 0 };

    // Call CPUID with EAX = 0 to query the vendor ID string
#if defined(_MSC_VER)
    __cpuid(cpuInfo, 0);
#elif defined(__GNUC__)
    __get_cpuid(0, (unsigned int*)&cpuInfo[0], (unsigned int*)&cpuInfo[1], (unsigned int*)&cpuInfo[2], (unsigned int*)&cpuInfo[3]);
#endif

    // Reconstruct the 12-character manufacturer string from EBX, EDX, and ECX registers
    char manufacturer[13];
    std::memcpy(manufacturer,     &cpuInfo[1], 4); // EBX
    std::memcpy(manufacturer + 4, &cpuInfo[3], 4); // EDX
    std::memcpy(manufacturer + 8, &cpuInfo[2], 4); // ECX
    manufacturer[12] = '\0';

    std::string vendor(manufacturer);
    std::cout << "[*] CPU Vendor Detected: " << vendor << "\n";

    if (vendor == "GenuineIntel") {
        std::cout << "[+] Verified Intel Processor.\n";
    }
    else if (vendor == "AuthenticAMD") {
        std::cout << "[+] Verified AMD Processor.\n";
    }
    else {
        std::cout << "[-] Non-standard or emulated CPU detected.\n";
    }

    return 0;
}
