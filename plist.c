#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>
#include <errhandlingapi.h>

//  Forward declarations:
BOOL GetProcessList( );
void FindProcessByID(DWORD processID);
void FindProcessByName(const char* processID);

int main(int argc, char* argv[])
{

  if (argc == 1){
    GetProcessList();
  }

  if (argc == 2) {
    const char* arg = argv[1];
    int i = 0;

    if (strcmp(argv[1], "-h") == 0) {
        printf("Usage: pslist.exe [-d][-m][-x][name|pid]\n");
        printf("-d  Show thread detail.\n");
        printf("-m  Show memory detail.\n");
        printf("-d  Show processes, memory information and threads.\n");
        return 1;
    }
    if (strcmp(argv[1], "-d") == 0) {
        ListProcessThreads();
        return 1;
    }
    if (!isdigit(arg[i])) {
      FindProcessByName(arg); 
      return 1;
    }
    FindProcessByID(atoi(arg));     
  }
  if (argc == 3){
    if (strcmp(argv[1], "-d") == 0) {
      ListProcessThreadsPerID(atoi(argv[2]));
    } 
  }
  return 0;
}

int GetHandleCount(HANDLE hprocess){
    DWORD handleCount;
    GetProcessHandleCount(hprocess, &handleCount);
    return handleCount;
}

DWORD GetProcessPrivateMemoryUsage(HANDLE hProcess) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        printf("Error getting process memory info: %lu\n", GetLastError());
        return 0;
    }

    // Return private memory usage in kilobytes
    return pmc.PrivateUsage / 1024;
}

SYSTEMTIME GetProcessCpuTime(HANDLE hprocess) {
    ULARGE_INTEGER totalCpuTime;
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    ULARGE_INTEGER cpuTime;

    GetProcessTimes(hprocess, &ftCreation, &ftExit, &ftKernel, &ftUser);

    totalCpuTime.LowPart = ftKernel.dwLowDateTime + ftUser.dwLowDateTime;
    totalCpuTime.HighPart = ftKernel.dwHighDateTime + ftUser.dwHighDateTime;
    cpuTime.QuadPart = totalCpuTime.QuadPart;
    unsigned long long cpuTimeMilliseconds = cpuTime.QuadPart / 10000;

    SYSTEMTIME cpuSystemTime;
    cpuSystemTime.wMilliseconds = cpuTimeMilliseconds % 1000; // Millisecondes restantes
    cpuTimeMilliseconds /= 1000; // Conversion en secondes
    cpuSystemTime.wSecond = cpuTimeMilliseconds % 60; // Secondes restantes
    cpuTimeMilliseconds /= 60; // Conversion en minutes
    cpuSystemTime.wMinute = cpuTimeMilliseconds % 60; // Minutes restantes
    cpuTimeMilliseconds /= 60; // Conversion en heures
    cpuSystemTime.wHour = cpuTimeMilliseconds % 24; // Heures restantes
    return cpuSystemTime;
}

SYSTEMTIME GetProcessElapsedTime(HANDLE hprocess) {
    ULARGE_INTEGER uliCreation;
    ULARGE_INTEGER uliNow;
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;
    SYSTEMTIME stNow;

    GetSystemTime(&stNow);
    GetProcessTimes(hprocess, &ftCreation, &ftExit, &ftKernel, &ftUser);
    SystemTimeToFileTime(&stNow, &ftNow);

    uliCreation.LowPart = ftCreation.dwLowDateTime;
    uliCreation.HighPart = ftCreation.dwHighDateTime;

    uliNow.LowPart = ftNow.dwLowDateTime;
    uliNow.HighPart = ftNow.dwHighDateTime;

    ULARGE_INTEGER elapsedTime;

    elapsedTime.QuadPart = uliNow.QuadPart - uliCreation.QuadPart;

    unsigned long long elapsedMilliseconds = elapsedTime.QuadPart / 10000;

    SYSTEMTIME elapsedSystemTime;
    elapsedSystemTime.wMilliseconds = elapsedMilliseconds % 1000; // Millisecondes restantes
    elapsedMilliseconds /= 1000; // Conversion en secondes
    elapsedSystemTime.wSecond = elapsedMilliseconds % 60; // Secondes restantes
    elapsedMilliseconds /= 60; // Conversion en minutes
    elapsedSystemTime.wMinute = elapsedMilliseconds % 60; // Minutes restantes
    elapsedMilliseconds /= 60; // Conversion en heures
    elapsedSystemTime.wHour = (WORD)elapsedMilliseconds;  // Heures restantes
    return elapsedSystemTime;
}

SYSTEMTIME GetThreadElapsedTime(HANDLE hThread) {
    ULARGE_INTEGER uliCreation;
    ULARGE_INTEGER uliNow;
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;
    SYSTEMTIME stNow;

    GetSystemTime(&stNow);
    GetThreadTimes(hThread, &ftCreation, &ftExit, &ftKernel, &ftUser);
    SystemTimeToFileTime(&stNow, &ftNow);

    uliCreation.LowPart = ftCreation.dwLowDateTime;
    uliCreation.HighPart = ftCreation.dwHighDateTime;

    uliNow.LowPart = ftNow.dwLowDateTime;
    uliNow.HighPart = ftNow.dwHighDateTime;

    ULARGE_INTEGER elapsedTime;

    elapsedTime.QuadPart = uliNow.QuadPart - uliCreation.QuadPart;

    unsigned long long elapsedMilliseconds =  elapsedTime.QuadPart / 10000;

    SYSTEMTIME ThreadElapsedTime;
    ThreadElapsedTime.wMilliseconds = elapsedMilliseconds % 1000; // Millisecondes restantes
    elapsedMilliseconds /= 1000; // Conversion en secondes
    ThreadElapsedTime.wSecond = elapsedMilliseconds % 60; // Secondes restantes
    elapsedMilliseconds /= 60; // Conversion en minutes
    ThreadElapsedTime.wMinute = elapsedMilliseconds % 60; // Minutes restantes
    elapsedMilliseconds /= 60; // Conversion en heures
    ThreadElapsedTime.wHour = (WORD)elapsedMilliseconds;  // Heures restantes
    return ThreadElapsedTime;
}

SYSTEMTIME GetThreadUserTime(HANDLE hThread) {
    ULARGE_INTEGER uliUser;
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;

    GetThreadTimes(hThread, &ftCreation, &ftExit, &ftKernel, &ftUser);

    uliUser.LowPart = ftUser.dwLowDateTime;
    uliUser.HighPart = ftUser.dwHighDateTime;

    unsigned long long elapsedMilliseconds =  uliUser.QuadPart / 10000;

    SYSTEMTIME ThreadElapsedTime;
    ThreadElapsedTime.wMilliseconds = elapsedMilliseconds % 1000; // Millisecondes restantes
    elapsedMilliseconds /= 1000; // Conversion en secondes
    ThreadElapsedTime.wSecond = elapsedMilliseconds % 60; // Secondes restantes
    elapsedMilliseconds /= 60; // Conversion en minutes
    ThreadElapsedTime.wMinute = elapsedMilliseconds % 60; // Minutes restantes
    elapsedMilliseconds /= 60; // Conversion en heures
    ThreadElapsedTime.wHour = (WORD)elapsedMilliseconds;  // Heures restantes
    return ThreadElapsedTime;
}

SYSTEMTIME GetThreadKernelTime(HANDLE hThread) {
    ULARGE_INTEGER uliKernel;
    FILETIME ftCreation, ftExit, ftKernel, ftUser, ftNow;

    GetThreadTimes(hThread, &ftCreation, &ftExit, &ftKernel, &ftUser);

    uliKernel.LowPart = ftKernel.dwLowDateTime;
    uliKernel.HighPart = ftKernel.dwHighDateTime;

    unsigned long long elapsedMilliseconds =  uliKernel.QuadPart / 10000;

    SYSTEMTIME ThreadElapsedTime;
    ThreadElapsedTime.wMilliseconds = elapsedMilliseconds % 1000; // Millisecondes restantes
    elapsedMilliseconds /= 1000; // Conversion en secondes
    ThreadElapsedTime.wSecond = elapsedMilliseconds % 60; // Secondes restantes
    elapsedMilliseconds /= 60; // Conversion en minutes
    ThreadElapsedTime.wMinute = elapsedMilliseconds % 60; // Minutes restantes
    elapsedMilliseconds /= 60; // Conversion en heures
    ThreadElapsedTime.wHour = (WORD)elapsedMilliseconds;  // Heures restantes
    return ThreadElapsedTime;
}

void FindProcessByID(DWORD processID) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("Error snapping process\n");
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        printf("Error getting first process\n");
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        if (pe32.th32ProcessID == processID) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
            printf("%-30s %-5s %-5s %-5s %-5s %-10s %-15s %-15s\n","Name","Pid","Pri","Thd","Hnd","Priv","CPU Time","Elapsed Time");
            if (hProcess) {
                SYSTEMTIME elapsedTime = GetProcessElapsedTime(hProcess);
                SYSTEMTIME cpuTime = GetProcessCpuTime(hProcess);
                DWORD privMem = GetProcessPrivateMemoryUsage(hProcess);
                printf("\n%-30s %-5d %-5d %-5d %-5d %-10d %d:%d:%d.%-8d %d:%d:%d.%d", pe32.szExeFile, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, GetHandleCount(hProcess), privMem, cpuTime.wHour, cpuTime.wMinute, cpuTime.wSecond, cpuTime.wMilliseconds , elapsedTime.wHour, elapsedTime.wMinute, elapsedTime.wSecond, elapsedTime.wMilliseconds);
                // Add more information as needed
                CloseHandle(hProcess);
            }
            else {
                printf("Error opening process, process are probably not authorized for you :) \n");
            }
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void FindProcessByName(const char* processName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("Error snapping process\n");
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        printf("Error getting first process\n");
        CloseHandle(hProcessSnap);
        return;
    }
    printf("%-30s %-5s %-5s %-5s %-5s %-10s %-15s %-15s\n","Name","Pid","Pri","Thd","Hnd","Priv","CPU Time","Elapsed Time");
    do {
        if (strcmp(pe32.szExeFile, processName) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                SYSTEMTIME elapsedTime = GetProcessElapsedTime(hProcess);
                SYSTEMTIME cpuTime = GetProcessCpuTime(hProcess);
                DWORD privMem = GetProcessPrivateMemoryUsage(hProcess);
                printf("\n%-30s %-5d %-5d %-5d %-5d %-10d %d:%d:%d.%-8d %d:%d:%d.%d", pe32.szExeFile, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, GetHandleCount(hProcess), privMem, cpuTime.wHour, cpuTime.wMinute, cpuTime.wSecond, cpuTime.wMilliseconds , elapsedTime.wHour, elapsedTime.wMinute, elapsedTime.wSecond, elapsedTime.wMilliseconds);
                // Add more information as needed
                CloseHandle(hProcess);
            }
            else {
                printf("Error opening process, process are probably not authorized for you :) \n");
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

BOOL GetProcessList( )
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;

  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {
    printf("Error snap process");
    return( FALSE );
  }

  pe32.dwSize = sizeof( PROCESSENTRY32 );

  if( !Process32First( hProcessSnap, &pe32 ) )
  {
    printf("First process not work"); // show cause of failure
    CloseHandle( hProcessSnap );          // clean the snapshot object
    return( FALSE );
  }

  // Now walk the snapshot of processes, and
  // display information about each process in turn
  printf("%-30s %-5s %-5s %-5s %-5s %-10s %-15s %-15s\n","Name","Pid","Pri","Thd","Hnd","Priv","CPU Time","Elapsed Time");
  do
  {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
    if (hProcess) { 
        SYSTEMTIME elapsedTime = GetProcessElapsedTime(hProcess);
        SYSTEMTIME cpuTime = GetProcessCpuTime(hProcess);
        DWORD privMem = GetProcessPrivateMemoryUsage(hProcess);

        printf("\n%-30s %-5d %-5d %-5d %-5d %-10d %d:%d:%d.%-8d %d:%d:%d.%d", pe32.szExeFile, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, GetHandleCount(hProcess), privMem, cpuTime.wHour, cpuTime.wMinute, cpuTime.wSecond, cpuTime.wMilliseconds , elapsedTime.wHour, elapsedTime.wMinute, elapsedTime.wSecond, elapsedTime.wMilliseconds);
        CloseHandle(hProcess);
    } else {
      printf("\n%-30s %-5d %-5d %-5d %-5s %-10s %-15s %-15s", pe32.szExeFile, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads, "HND", "PRIV", "CPUTIME", "ELAPSED");
    }
  } while( Process32Next( hProcessSnap, &pe32 ) );

  CloseHandle( hProcessSnap );
  return( TRUE );
}

BOOL ListProcessThreadsPerID(INT dwOwnerPID ) 
{ 
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
  THREADENTRY32 te32; 

  if (dwOwnerPID == 0) {
    printf("Error: Id is not a number\n");
    return FALSE;
  }
 
  // Take a snapshot of all running threads  
  hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
  if( hThreadSnap == INVALID_HANDLE_VALUE ) 
    return( FALSE ); 
 
  // Fill in the size of the structure before using it. 
  te32.dwSize = sizeof(THREADENTRY32); 
 
  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if( !Thread32First( hThreadSnap, &te32 ) ) 
  {
    CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
    return( FALSE );
  }

  // Now walk the thread list of the system,
  // and display information about each thread
  // associated with the specified process
  printf("%-5s %-5s %-5s %-5s %-10s %-10s %-10s\n","Tid","Pri","Cswtch","State","User Time","Kernel Time", "Elapsed Time");
  do 
  { 
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
    if( te32.th32OwnerProcessID == dwOwnerPID && hThread != NULL)
    {
      SYSTEMTIME threadKernelTime = GetThreadKernelTime(hThread);
      SYSTEMTIME threadUserTime = GetThreadUserTime(hThread);
      SYSTEMTIME threadElapsedTime = GetThreadElapsedTime(hThread);
      printf("\n%-5d %-5d %-5s %d:%d:%d.%d %d:%d:%d.%d %d:%d:%d.%d", te32.th32ThreadID, te32.tpBasePri, "Cswtch", threadUserTime.wHour, threadUserTime.wMinute, threadUserTime.wSecond, threadUserTime.wMilliseconds, threadKernelTime.wHour, threadKernelTime.wMinute, threadKernelTime.wSecond , threadKernelTime.wMilliseconds, threadElapsedTime.wHour, threadElapsedTime.wMinute, threadElapsedTime.wSecond, threadElapsedTime.wMilliseconds);
    }
  } while( Thread32Next(hThreadSnap, &te32 ) );

  _tprintf( TEXT("\n"));

//  Don't forget to clean up the snapshot object.
  CloseHandle( hThreadSnap );
  return( TRUE );
}

BOOL ListProcessThreads() 
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;
  THREADENTRY32 te32; 
  HANDLE hThreadSnap;

  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {
    printf("Error snap process");
    return( FALSE );
  }
  pe32.dwSize = sizeof( PROCESSENTRY32 );
  if( !Process32First( hProcessSnap, &pe32 ) )
  {
    printf("First process not work"); // show cause of failure
    CloseHandle( hProcessSnap );          // clean the snapshot object
    return( FALSE );
  }
  do
  {
    printf("%s %d:\n", pe32.szExeFile, pe32.th32ProcessID);
    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32); 
    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if( !Thread32First( hThreadSnap, &te32 ) ) 
    {
      CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
      return( FALSE );
    }
    printf("%-5s %-5s %-5s %-5s %-10s %-10s %-10s\n","Tid","Pri","Cswtch","State","User Time","Kernel Time", "Elapsed Time");
    do 
    { 
      HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
      //printf("%d\n", GetLastError());
      if(te32.th32OwnerProcessID == pe32.th32ProcessID && te32.th32ThreadID)
      {
        SYSTEMTIME threadKernelTime = GetThreadKernelTime(hThread);
        SYSTEMTIME threadUserTime = GetThreadUserTime(hThread);
        SYSTEMTIME threadElapsedTime = GetThreadElapsedTime(hThread);
        printf("\n%-5d %-5d %-5s %d:%d:%d.%d %d:%d:%d.%d %d:%d:%d.%d", te32.th32ThreadID, te32.tpBasePri, "Cswtch", threadUserTime.wHour, threadUserTime.wMinute, threadUserTime.wSecond, threadUserTime.wMilliseconds, threadKernelTime.wHour, threadKernelTime.wMinute, threadKernelTime.wSecond , threadKernelTime.wMilliseconds, threadElapsedTime.wHour, threadElapsedTime.wMinute, threadElapsedTime.wSecond, threadElapsedTime.wMilliseconds);
      }
    } while( Thread32Next(hThreadSnap, &te32 ) );

    _tprintf( TEXT("\n"));

  } while( Process32Next( hProcessSnap, &pe32 ) );

//  Don't forget to clean up the snapshot object.
  CloseHandle( hThreadSnap );
  return( TRUE );
}