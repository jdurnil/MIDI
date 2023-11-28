// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "stdafx.h"

#define SVCNAME TEXT("MidiSrv")
#define SVCDISPLAYNAME TEXT("Microsoft MIDI Service")

//#ifdef _DEBUG
//#define SVCDESCRIPTION TEXT("Windows MIDI Services core service (Debug Build)")
//#else
//#define SVCDESCRIPTION TEXT("Windows MIDI Services core service for MIDI 2.0 and MIDI 1.0 protocols")
//#endif


#define SVC_ERROR       ((DWORD)0xC0020001L)

// wait up to 30 seconds for the service to stop for uninstall
#define SERVICE_STOP_TIMEOUT 30000

SERVICE_STATUS          g_SvcStatus; 
SERVICE_STATUS_HANDLE   g_SvcStatusHandle; 
wil::unique_handle      g_SvcStopEvent;
std::unique_ptr<CMidiSrv> g_MidiService;

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
//
VOID ReportSvcStatus(
    _In_ DWORD dwCurrentState,
    _In_ DWORD dwWin32ExitCode,
    _In_ DWORD dwWaitHint)
{
    static DWORD checkPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    g_SvcStatus.dwCurrentState = dwCurrentState;
    g_SvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_SvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        g_SvcStatus.dwControlsAccepted = 0;
    }
    else
    {
        g_SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || 
        (dwCurrentState == SERVICE_STOPPED))
    {
        g_SvcStatus.dwCheckPoint = 0;
    }
    else
    {
        g_SvcStatus.dwCheckPoint = checkPoint++;
    }

    // Report the status of the service to the SCM.
    if (!SetServiceStatus(g_SvcStatusHandle, &g_SvcStatus))
    {
        LOG_LAST_ERROR_MSG("Failed to set the service status");
    }
}

//
// Purpose: 
//   Installs a service in the SCM database
//
VOID SvcInstall()
{
    wil::unique_schandle serviceControlManager;
    wil::unique_schandle service;
    TCHAR unquotedPath[MAX_PATH] = {0};
    TCHAR quotedPath[MAX_PATH] = {0};
    HRESULT hr = S_OK;

    if (!GetModuleFileName(NULL, unquotedPath, MAX_PATH))
    {
        LOG_LAST_ERROR_MSG("Cannot install service, failed to retrieve module file name");
        return;
    }

    // In case the path contains a space, it must be quoted
    hr = StringCbPrintf(quotedPath, MAX_PATH, TEXT("\"%s\""), unquotedPath);
    if (FAILED(hr))
    {
        LOG_HR_MSG(hr, "Cannot install service, failed to compose quoted path");
        return;
    }

    // Get a handle to the SCM database. 
    serviceControlManager.reset(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
    if (!serviceControlManager)
    {
        LOG_LAST_ERROR_MSG("Cannot install service, OpenSCManager failed");
        return;
    }

    // Create the service
    service.reset(CreateService( 
        serviceControlManager.get(),
        SVCNAME,
        SVCDISPLAYNAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        quotedPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL));
    if (!service)
    {
        LOG_LAST_ERROR_MSG("Cannot install service, CreateService failed");
        return;
    }

    if (!StartService(service.get(), 0, nullptr))
    {
        LOG_LAST_ERROR_MSG("Starting the service failed during service install");
        return;
    }
    //// Set the service description so it's not just blank. This should be localized
    //// in the future with "@[path]dllname,-strID"

    //SERVICE_DESCRIPTION info;
    //info.lpDescription = SVCDESCRIPTION;

    //if (!ChangeServiceConfig2(service.get(), SERVICE_CONFIG_DESCRIPTION, &info))
    //{
    //    // non-fatal
    //    LOG_LAST_ERROR_MSG("Changing service description failed");
    //    return;
    //}
}

//
// Purpose: 
//   Stops and uninstalls midisrv
//
VOID SvcUninstall()
{
    wil::unique_schandle serviceControlManager;
    wil::unique_schandle service;
    SERVICE_STATUS_PROCESS status{0};
    DWORD size_needed{0};

    // Get a handle to the SCM database. 
    serviceControlManager.reset(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
    if (!serviceControlManager)
    {
        LOG_LAST_ERROR_MSG("Cannot uninstall service, OpenSCManager failed");
        return;
    }

    service.reset(OpenService(serviceControlManager.get(), SVCNAME, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE));
    if (!service)
    {
        LOG_LAST_ERROR_MSG("Cannot uninstall service, OpenService failed");
        return;
    }

    if (FALSE == QueryServiceStatusEx(service.get(), SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(status), &size_needed))
    {
        LOG_LAST_ERROR_MSG("Cannot uninstall service, QueryServiceStatusEx failed");
        return;
    }

    // if the service is already stopped, all that is needed is to delete.
    if (status.dwCurrentState != SERVICE_STOPPED)
    {
        SERVICE_STATUS ss{0};

        if (FALSE == ControlService(service.get(), SERVICE_CONTROL_STOP, &ss))
        {
            LOG_LAST_ERROR_MSG("Cannot uninstall service, service is not stopped and SERVICE_CONTROL_STOP failed");
            return;
        }

        ULONGLONG startTime = GetTickCount64();
        ULONGLONG currentTime = startTime;
        do
        {
            Sleep(500);

            currentTime = GetTickCount64();

            memset(&status, 0, sizeof(status));
            size_needed = 0;
            if (FALSE == QueryServiceStatusEx(service.get(), SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(status), &size_needed))
            {
                LOG_LAST_ERROR_MSG("Cannot uninstall service, QueryServiceStatusEx after SERVICE_CONTROL_STOP failed");
                return;
            }
        }
        while (status.dwCurrentState != SERVICE_STOPPED && (currentTime - startTime < SERVICE_STOP_TIMEOUT));
        
        if (status.dwCurrentState != SERVICE_STOPPED)
        {
            LOG_LAST_ERROR_MSG("Service did not stop during uninstall, calling DeleteService anyway, but deletion will pend.");
        }
    }

    // Service is stopped, delete it from the database
    if (FALSE == DeleteService(service.get()))
    {
        LOG_LAST_ERROR_MSG("Cannot uninstall service, DeleteService failed");
        return;
    }
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
VOID SvcInit()
{
    // service control event
    g_SvcStopEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
    if (NULL == g_SvcStopEvent)
    {
        LOG_LAST_ERROR_MSG("Cannot init service, stop event creation failed");
        ReportSvcStatus( SERVICE_STOPPED, GetLastError(), 0 );
        return;
    }

    g_MidiService = std::make_unique<CMidiSrv>();
    if (NULL == g_MidiService.get())
    {
        LOG_IF_NULL_ALLOC(g_MidiService);
        ReportSvcStatus( SERVICE_STOPPED, ERROR_NOT_ENOUGH_MEMORY, 0 );
        return;
    }

    HRESULT hr = g_MidiService->Initialize();
    if (FAILED(hr))
    {
        LOG_IF_FAILED(hr);
        ReportSvcStatus( SERVICE_STOPPED, hr, 0 );
        return;
    }

    // Report running status when initialization is complete.
    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

    // Check whether to stop the service.
    WaitForSingleObject(g_SvcStopEvent.get(), INFINITE);

    hr = g_MidiService->Cleanup();
    LOG_IF_FAILED(hr);
    ReportSvcStatus( SERVICE_STOPPED, hr, 0 );

    return;
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
VOID SvcReportEvent(_In_z_ LPCSTR )
{ 
// TODO: create service providers using public api's rather than
// event logs.
#if 0
    wil::unique_any_handle_null<decltype(&::DeregisterEventSource), ::DeregisterEventSource> 
        eventSource(RegisterEventSource(NULL, SVCNAME));

    LOG_LAST_ERROR_MSG(Operation);

    if (!eventSource)
    {
        LOG_LAST_ERROR_MSG("Failed to register event source");
    }
    else
    {
        LPCTSTR lpszStrings[2];
        TCHAR Buffer[80];

        StringCchPrintf(Buffer, 80, TEXT("%S failed with %d"), Operation, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(eventSource.get(), EVENTLOG_ERROR_TYPE, 0, SVC_ERROR, NULL, 2, 0, lpszStrings, NULL);
    }
#endif
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
//
VOID WINAPI SvcCtrlHandler(_In_ DWORD Ctrl)
{
   // Handle the requested control code. 
   switch(Ctrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.
         SetEvent(g_SvcStopEvent.get());

         ReportSvcStatus(g_SvcStatus.dwCurrentState, NO_ERROR, 0);
         return;
      case SERVICE_CONTROL_INTERROGATE:
         break;
      default:
         break;
   }
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
VOID WINAPI SvcMain(_In_ DWORD , _In_ LPTSTR *)
{
    // Register the handler function for the service
    g_SvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
    if (!g_SvcStatusHandle)
    {
        SvcReportEvent("RegisterServiceCtrlHandler");
        return; 
    } 

    // These SERVICE_STATUS members remain as set here
    g_SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    g_SvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.
    SvcInit();
}

int __cdecl _tmain(_In_ int ArgC, _In_reads_(ArgC) TCHAR *ArgV[]) 
{ 
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.
    wil::SetResultTelemetryFallback(MidiSrvTelemetryProvider::FallbackTelemetryCallback);

    winrt::init_apartment();

    if (ArgC >= 2 && lstrcmpi(ArgV[1], TEXT("install")) == 0)
    {
        SvcInstall();
        return 0;
    }

    if (ArgC >= 2 && lstrcmpi(ArgV[1], TEXT("uninstall")) == 0)
    {
        SvcUninstall();
        return 0;
    }

    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.
    if (!StartServiceCtrlDispatcher(DispatchTable)) 
    { 
        SvcReportEvent("StartServiceCtrlDispatcher");
    }
    return 0;
} 

