// g++ main.cpp ressource.o -o Word_setup.exe -lversion -Os -static -static-libgcc -static-libstdc++
// g++ main.cpp -o Word_setup.exe -lversion -Os -static -static-libgcc -static-libstdc++
 

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <iostream>
#include <vector>
#include <tlhelp32.h>
#include <iomanip>
#include <string>
#include <sddl.h>


BOOL EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Open current process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        std::cerr << "[-] OpenProcessToken failed. Error: " << GetLastError() << std::endl;
        return FALSE;
    }

    // Get LUID of SeDebugPrivilege
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid)) {
        std::cerr << "[-] LookupPrivilegeValue failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Set privileges to the token
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
        std::cerr << "[-] AdjustTokenPrivileges failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}


void ListModules(HANDLE hProcess, DWORD processID){
    HMODULE tabModules[1024];
    DWORD bytesNeeded;

    printf("--- DLLs loaded ---\n");
    // 2. Enumérer les modules (LIST_MODULES_ALL permet de voir les DLL 32 et 64 bits)
    if (EnumProcessModulesEx(hProcess, tabModules, sizeof(tabModules), &bytesNeeded, LIST_MODULES_ALL)) {
        int count = bytesNeeded / sizeof(HMODULE);
        std::cout << "[+] " << count << " modules found in the PID " << processID << ":" << std::endl;

        for (int i = 0; i < count; i++) {
            char moduleName[MAX_PATH];

            // 3. Obtenir le chemin de chaque DLL
            if (GetModuleFileNameExA(hProcess, tabModules[i], moduleName, MAX_PATH)) {
			printf("  [%u] %s (Base address: %p)\n", i, moduleName, tabModules[i]);
            }
        }
    } else {
        std::cerr << "[-] Erreur EnumProcessModulesEx: " << GetLastError() << std::endl;
    }

}

void ListSelfModules() {
    HANDLE hProcess = GetCurrentProcess();
    DWORD myPid = GetCurrentProcessId();
    ListModules(hProcess,myPid);
    CloseHandle(hProcess);
}

std::string GetProcessDescription(const char* fullPath) {
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(fullPath, &handle);
    if (size == 0) return "N/A";

    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoA(fullPath, 0, size, data.data())) return "N/A";

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    UINT cbTranslate;
    if (VerQueryValueA(data.data(), "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate)) {
        char subBlock[50];
        sprintf_s(subBlock, "\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
        char* description = NULL;
        UINT len = 0;
        if (VerQueryValueA(data.data(), subBlock, (LPVOID*)&description, &len)) return description;
    }
    return "N/A";
}

std::string GetProcessUser(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) return "Access denied";

    HANDLE hToken;
    std::string userName = "Unknown";
    if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        DWORD len = 0;
        GetTokenInformation(hToken, TokenUser, NULL, 0, &len);
        std::vector<BYTE> buffer(len);
        if (GetTokenInformation(hToken, TokenUser, buffer.data(), len, &len)) {
            PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());
            char name[256], domain[256];
            DWORD nameLen = 256, domainLen = 256;
            SID_NAME_USE snu;
            if (LookupAccountSidA(NULL, pTokenUser->User.Sid, name, &nameLen, domain, &domainLen, &snu)) {
                userName = std::string(domain) + "\\" + name;
            }
        }
        CloseHandle(hToken);
    }
    CloseHandle(hProcess);
    return userName;
}

void ListProcesses() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

    std::cout << std::left << std::setw(25) << "NAME" << std::setw(8) << "PID" 
              << std::setw(25) << "USER" << "DESCRIPTION" << "\n";
    std::cout << std::string(90, '-') << "\n";

    if (Process32First(hSnap, &pe32)) {
        do {
            std::string path = "Unknown";
            HANDLE hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            if (hModSnap != INVALID_HANDLE_VALUE) {
                MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
                if (Module32First(hModSnap, &me32)) path = me32.szExePath;
                CloseHandle(hModSnap);
            }

            std::cout << std::left << std::setw(25) << pe32.szExeFile
                      << std::setw(8) << pe32.th32ProcessID
                      << std::setw(25) << GetProcessUser(pe32.th32ProcessID)
                      << GetProcessDescription(path.c_str()) << "\n";

        } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
}

void ListRemoteModules(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (NULL == hProcess) {
        std::cerr << "[-] Can't open the process. Error: " << GetLastError() << std::endl;
        std::cerr << "[-] Check your privs, you may have tried to open a process which requires admin rights !" << std::endl;
        return;
    }

    ListModules(hProcess,processID);
    CloseHandle(hProcess);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " self|remote" << std::endl;
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "self"){
        ListSelfModules();
    }
    else if (mode == "remote"){
        if (!EnableDebugPrivilege()) {
            std::cout << "[-] Failed to enable debug privilege" << std::endl;
            return false;
        }

        ListProcesses();

        while (true){
            DWORD pid; 
            std::cout << "[?] Type the remote pid (press Ctrl+C to exit) : "; // Type a number and press enter
            std::cin >> pid; 
            ListRemoteModules(pid);
        }
    }
    else {
        std::cout << "[-] Unknown mode" << std::endl;
        return false;
    }

    return 0;

}