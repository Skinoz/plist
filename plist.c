#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

//  Forward declarations:
BOOL GetProcessList( );

int main( void )
{
  GetProcessList( );
  return 0;
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
  _tprintf(TEXT("%-20s %-5s %-5s %-5s %-5s %-8s %-15s %-15s\n"),TEXT("Name"), TEXT("Pid"), TEXT("Pri"), TEXT("Thd"), TEXT("Hnd"), TEXT("Priv"), TEXT("CPU Time"), TEXT("Elapsed Time"));
  do
  {
    _tprintf(TEXT("\n%-20s %-5d %-5d %-5d"), pe32.szExeFile, pe32.th32ProcessID, pe32.pcPriClassBase, pe32.cntThreads);

  } while( Process32Next( hProcessSnap, &pe32 ) );

  CloseHandle( hProcessSnap );
  return( TRUE );
}