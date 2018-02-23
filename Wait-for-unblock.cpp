
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
#pragma comment(lib,"user32.lib")
using namespace std;
int main()
{
	int num;
st:
	bool flag = true;
	SECURITY_ATTRIBUTES sa          = {0};
	STARTUPINFO         si          = {0};
	PROCESS_INFORMATION pi          = {0};
	HANDLE              hPipeOutputRead  = NULL;
	HANDLE              hPipeOutputWrite = NULL;
	HANDLE              hPipeInputRead   = NULL;
	HANDLE              hPipeInputWrite  = NULL;
	BOOL                bTest = 0;
	DWORD               dwNumberOfBytesRead = 0;
	DWORD               dwNumberOfBytesWrite = 0;
	CHAR                szMsg[100];
	CHAR                szBuffer[256];

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create pipe for standard output redirection.
	CreatePipe(&hPipeOutputRead,  // read handle
	           &hPipeOutputWrite, // write handle
	           &sa,      // security attributes
	           0      // number of bytes reserved for pipe - 0 default
	          );

	// Create pipe for standard input redirection.
	CreatePipe(&hPipeInputRead,  // read handle
	           &hPipeInputWrite, // write handle
	           &sa,      // security attributes
	           0      // number of bytes reserved for pipe - 0 default
	          );

	// Make child process use hPipeOutputWrite as standard out,
	// and make sure it does not show on screen.
	si.cb = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = hPipeInputRead;
	si.hStdOutput  = hPipeOutputWrite;
	si.hStdError   = hPipeOutputWrite;

	CreateProcess (
	    NULL, "node main",
	    NULL, NULL,
	    TRUE, 0,
	    NULL, NULL,
	    &si, &pi);

	// Now that handles have been inherited, close it to be safe.
	// You don't want to read or write to them accidentally.
	
	CloseHandle(hPipeOutputWrite);
	CloseHandle(hPipeInputRead);

	// Now test to capture DOS application output by reading
	// hPipeOutputRead.  Could also write to DOS application
	// standard input by writing to hPipeInputWrite.
	//sprintf(szMsg, "dir *.txt /b\nexit\n");
	/*WriteFile(
	      hPipeInputWrite,      // handle of the write end of our pipe
	      &szMsg,               // address of buffer that send data
	      18,                   // number of bytes to write
	      &dwNumberOfBytesWrite,// address of number of bytes read
	      NULL                  // non-overlapped.
	      );*/
	int i;
	for (i = 1; i <= 3; ++i)
	{
		if (i == 3 && flag) break;
		bTest=ReadFile(
		          hPipeOutputRead,      // handle of the read end of our pipe
		          &szBuffer,            // address of buffer that receives data
		          256,                // number of bytes to read
		          &dwNumberOfBytesRead, // address of number of bytes read
		          NULL                  // non-overlapped.
		      );
		
		// do something with data.
		szBuffer[dwNumberOfBytesRead] = 0;  // null terminate
		string S(szBuffer);
		string::size_type pos = S.find("SyntaxError");
		if (pos != string::npos)
		{
			if (flag)
			{
				cout << "Unavailable!" << "Attempt No." << num << endl; 
				num++;
				flag = false;
			}
			break;
		}
	}
	
	if (i != 3)
		system("taskkill /f /im node.exe > nul");
	else cout << "Succeed! Now Running 'main.js'!" << endl;
	// Wait for CONSPAWN to finish.
	WaitForSingleObject (pi.hProcess, INFINITE);
	
	// Close all remaining handles
	CloseHandle (pi.hProcess);
	CloseHandle (hPipeOutputRead);
	CloseHandle (hPipeInputWrite);
	goto st;
	return 0;
}

