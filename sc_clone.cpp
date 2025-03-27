#include <windows.h>
#include <iostream>
#include <string>
#include <winsvc.h>
#include <vector>
#include <sstream>

// Function to print error messages related to service control management (SCM) failures
void PrintErrorMessage(const std::wstring& message, DWORD error){
    std::wcerr << L"[SC_CLONE] OpenSCManager failed with error code: " << error << std::endl;

    // Convert error code to a human-readable message
    LPVOID errormessage;
    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, error, 0,(LPWSTR)&errormessage, 0, NULL);

    std::wcerr << L"Error Message: " << (LPWSTR)errormessage << std::endl;

    // Free allocated memory for error message
    LocalFree(errormessage);
}

// Converts service type flags into a human-readable string for display
std::wstring ServiceTypeToString(DWORD serviceType) {
    std::wstring result = L"Unknown";

    // Check the flags and append corresponding service type to the result string
    if (serviceType & SERVICE_WIN32_OWN_PROCESS) {
        result = L"SERVICE_WIN32_OWN_PROCESS";
    }
    if (serviceType & SERVICE_WIN32_SHARE_PROCESS) {
        result += L" | SERVICE_WIN32_SHARE_PROCESS";
    }
    if (serviceType & SERVICE_KERNEL_DRIVER) {
        result += L" | SERVICE_KERNEL_DRIVER";
    }
    if (serviceType & SERVICE_FILE_SYSTEM_DRIVER) {
        result += L" | SERVICE_FILE_SYSTEM_DRIVER";
    }
    if (serviceType & SERVICE_RECOGNIZER_DRIVER) {
        result += L" | SERVICE_RECOGNIZER_DRIVER";
    }
    if (serviceType & SERVICE_WIN32) {
        result += L" | SERVICE_WIN32";
    }
    if (serviceType & SERVICE_ADAPTER) {
        result += L" | SERVICE_ADAPTER";
    }
    if (serviceType & SERVICE_DRIVER) {
        result += L" | SERVICE_DRIVER";
    }
    if (serviceType & SERVICE_INTERACTIVE_PROCESS) {
        result += L" | SERVICE_INTERACTIVE_PROCESS";
    }

    // If no specific service type matches, display the numeric value
    if (result == L"Unknown") {
        result = std::to_wstring(serviceType);
    }

    return result;
}

// Queries the status of a service and prints relevant information
void QueryServiceStatus(const std::wstring& serviceName) {
    // Open the Service Control Manager
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service
    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName.c_str(), SERVICE_QUERY_STATUS);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // Query the service status
    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
        std::wcout << L"[SC_CLONE] SERVICE_NAME: " << serviceName << std::endl;
        std::wcout << L"        TYPE               : " << ServiceTypeToString(ssp.dwServiceType) << std::endl;
        std::wcout << L"        STATE              : " << ((ssp.dwCurrentState == SERVICE_RUNNING) ? L"RUNNING" :
                                                            (ssp.dwCurrentState == SERVICE_STOPPED) ? L"STOPPED" :
                                                            (ssp.dwCurrentState == SERVICE_PAUSED) ? L"PAUSED" : L"UNKNOWN") << std::endl;
    } else {
        PrintErrorMessage(L"[SC_CLONE] QueryServiceStatus failed with error code: ", GetLastError());
    }

    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Creates a new service entry in the Service Control Manager
void CreateServiceEntry(const std::wstring& serviceName, const std::wstring& executablePath) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Create the service
    SC_HANDLE hService = CreateServiceW(
        hSCManager, 
        serviceName.c_str(),
        serviceName.c_str(),
        SERVICE_ALL_ACCESS, 
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        executablePath.c_str(),
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL
    );

    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] CreateService failed with error code: ", GetLastError());
        return;
    }

    // Confirm service creation
    std::wcout << L"[SC_CLONE] Service " << serviceName << L" created successfully." << std::endl;
    
    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Starts a service entry
void StartServiceEntry(const std::wstring& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service for starting
    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_START);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // Attempt to start the service
    if (StartService(hService, 0, NULL)) {
        std::wcout << L"[SC_CLONE] StartService SUCCESS" << std::endl;
    } else {
        PrintErrorMessage(L"[SC_CLONE] StartService failed with error code: ", GetLastError());
    }

    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Stops a service entry
void StopServiceEntry(const std::wstring& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service for stopping
    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_STOP);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // Attempt to stop the service
    SERVICE_STATUS status;
    if (ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        std::wcout << L"[SC_CLONE] ControlService SUCCESS" << std::endl;
    } else {
        PrintErrorMessage(L"[SC_CLONE] ControlService failed with error code: ", GetLastError());
    }

    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Deletes a service entry from the Service Control Manager
void DeleteServiceEntry(const std::wstring& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service for deletion
    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), DELETE);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // Attempt to delete the service
    if (DeleteService(hService)) {
        std::wcout << L"[SC_CLONE] DeleteService SUCCESS" << std::endl;
    } else {
        PrintErrorMessage(L"[SC_CLONE] DeleteService failed with error code: ", GetLastError());
    }

    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Configures the service (set start type, etc.)
void ConfigureService(const std::wstring& serviceName, const std::wstring& startType = L"") {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service for configuration
    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName.c_str(), SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // If no startType specified, just query and display service configuration
    if (startType.empty()) {
        DWORD bytesNeeded = 0;
        QueryServiceConfigW(hService, NULL, 0, &bytesNeeded);  // Get required buffer size

        LPQUERY_SERVICE_CONFIGW pServiceConfig = (LPQUERY_SERVICE_CONFIGW)LocalAlloc(LPTR, bytesNeeded);
        if (!pServiceConfig) {
            std::wcerr << L"[SC_CLONE] Memory allocation failed." << std::endl;
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return;
        }

        // Query and display service configuration
        if (QueryServiceConfigW(hService, pServiceConfig, bytesNeeded, &bytesNeeded)) {
            std::wcout << L"[SC_CLONE] Service Configuration for: " << serviceName << std::endl;
            std::wcout << L"        BINARY_PATH_NAME    : " << (pServiceConfig->lpBinaryPathName ? pServiceConfig->lpBinaryPathName : L"(None)") << std::endl;
            std::wcout << L"        START_TYPE          : " << pServiceConfig->dwStartType << std::endl;
            std::wcout << L"        ERROR_CONTROL       : " << pServiceConfig->dwErrorControl << std::endl;
            std::wcout << L"        SERVICE_DEPENDENCIES: " << (pServiceConfig->lpDependencies ? pServiceConfig->lpDependencies : L"(None)") << std::endl;
            std::wcout << L"        SERVICE_START_NAME  : " << (pServiceConfig->lpServiceStartName ? pServiceConfig->lpServiceStartName : L"(None)") << std::endl;
        } else {
            PrintErrorMessage(L"[SC_CLONE] QueryServiceConfigW failed with error code: ", GetLastError());
        }

        LocalFree(pServiceConfig);
    } 
    else { 
        // Determine start type based on user input
        DWORD dwStartType = SERVICE_NO_CHANGE;
        if (startType == L"auto") {
            dwStartType = SERVICE_AUTO_START;
        } else if (startType == L"manual") {
            dwStartType = SERVICE_DEMAND_START;
        } else if (startType == L"disabled") {
            dwStartType = SERVICE_DISABLED;
        } else {
            std::wcerr << L"[SC_CLONE] Invalid start type: " << startType << L". Use 'automatic', 'manual', or 'disabled'." << std::endl;
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return;
        }

        // Apply the configuration change
        std::wcout << L"[SC_CLONE] Updating service start type to: " << startType << std::endl;
        if (!ChangeServiceConfigW(hService, 
                                  SERVICE_NO_CHANGE, 
                                  dwStartType, 
                                  SERVICE_ERROR_NORMAL,
                                  NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
            PrintErrorMessage(L"[SC_CLONE] ChangeServiceConfigW failed with error code: ", GetLastError());
        } else {
            std::wcout << L"[SC_CLONE] Service start type successfully changed to: " << startType << std::endl;
        }
    }

    // Close handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}


// Function to query and display the description and configuration details of a service
void QueryServiceDescription(const std::wstring& serviceName) {
    // Open the Service Control Manager (SCM) with the necessary access rights
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service to query its configuration details
    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName.c_str(), SERVICE_QUERY_CONFIG);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // First, query the size of the buffer required to store the service configuration
    DWORD dwBytesNeeded = 0;
    DWORD dwBufSize = 0;
    QueryServiceConfigW(hService, NULL, 0, &dwBytesNeeded);
    
    // Allocate memory for the buffer that will hold the service configuration
    dwBufSize = dwBytesNeeded;
    LPQUERY_SERVICE_CONFIGW pServiceConfig = (LPQUERY_SERVICE_CONFIGW)malloc(dwBufSize);
    
    // Query the service configuration and display relevant information
    if (QueryServiceConfigW(hService, pServiceConfig, dwBufSize, &dwBytesNeeded)) {
        std::wcout << L"[SC_CLONE] SERVICE_NAME: " << serviceName << std::endl;
        std::wcout << L"        DESCRIPTION       : " << pServiceConfig->lpDisplayName << std::endl;  // Service description
        std::wcout << L"        SERVICE_TYPE      : " << pServiceConfig->dwServiceType << std::endl;   // Service type
        std::wcout << L"        START_TYPE        : " << pServiceConfig->dwStartType << std::endl;    // Service start type
        std::wcout << L"        ERROR_CONTROL     : " << pServiceConfig->dwErrorControl << std::endl; // Error control level
        std::wcout << L"        BINARY_PATH_NAME  : " << pServiceConfig->lpBinaryPathName << std::endl; // Path to the service binary
    } else {
        PrintErrorMessage(L"[SC_CLONE] QueryServiceConfig failed with error code: ", GetLastError());
    }

    // Free the allocated memory for service configuration and close handles
    free(pServiceConfig);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Function to configure service failure actions (like restart, reboot, or run command on failure)
void ConfigureServiceFailure(const std::wstring& serviceName, const std::vector<std::wstring>& args) {
    // Open the Service Control Manager (SCM) with full access rights
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        PrintErrorMessage(L"[SC_CLONE] OpenSCManager failed with error code: ", GetLastError());
        return;
    }

    // Open the specified service to modify its failure actions
    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName.c_str(), SC_MANAGER_ALL_ACCESS);
    if (!hService) {
        PrintErrorMessage(L"[SC_CLONE] OpenService failed with error code: ", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    // Query the current service failure actions
    SERVICE_FAILURE_ACTIONS failureActions = {};
    DWORD bytesNeeded = 0;
    if (QueryServiceConfig2W(hService, SERVICE_CONFIG_FAILURE_ACTIONS, reinterpret_cast<LPBYTE>(&failureActions), sizeof(failureActions), &bytesNeeded)) {
        std::wcout << L"[SC_CLONE] Current failure actions for service: " << serviceName << std::endl;
        std::wcout << L"    Reset period (in seconds): " << failureActions.dwResetPeriod << std::endl;

        // If failure actions exist, display them
        if (failureActions.cActions > 0) {
            for (DWORD i = 0; i < failureActions.cActions; ++i) {
                SC_ACTION action = failureActions.lpsaActions[i];
                std::wcout << L"    Action " << i + 1 << L": ";
                switch (action.Type) {
                    case SC_ACTION_RESTART:
                        std::wcout << L"Restart";
                        break;
                    case SC_ACTION_REBOOT:
                        std::wcout << L"Reboot";
                        break;
                    case SC_ACTION_RUN_COMMAND:
                        std::wcout << L"Run Command";
                        break;
                    default:
                        std::wcout << L"Unknown action";
                        break;
                }
                std::wcout << L" with delay: " << action.Delay << L" seconds." << std::endl;
            }
        } else {
            std::wcout << L"    No failure actions set." << std::endl;
        }
    } else if (args.empty()) {
        // If no failure actions are found and no arguments are provided, display a message
        std::wcout << L"[SC_CLONE] No existing failure actions found or failed to query service failure actions." << std::endl;
    } else {
        // If arguments are provided, configure new failure actions based on the input
        DWORD resetPeriod = 0;
        std::vector<SC_ACTION> actions;
        std::wstring rebootMsg;
        std::wstring failureCommand;

        // Parse arguments for reset period, action types, and any additional parameters
        for (const auto& arg : args) {
            if (arg.find(L"reset=") == 0) {
                resetPeriod = std::stoi(arg.substr(6));  // Extract number after "reset="
            } else if (arg.find(L"actions=") == 0) {
                std::wstring actionList = arg.substr(8);
                std::wstringstream ss(actionList);
                std::wstring token;
                while (std::getline(ss, token, L'/')) {
                    SC_ACTION action = {};
                    if (token == L"restart") {
                        action.Type = SC_ACTION_RESTART;
                    } else if (token == L"run") {
                        action.Type = SC_ACTION_RUN_COMMAND;
                    } else if (token == L"reboot") {
                        action.Type = SC_ACTION_REBOOT;
                    } else {
                        action.Delay = std::stoi(token);  // Delay after action type
                        if (!actions.empty()) {
                            actions.back().Delay = action.Delay;
                        }
                        continue;
                    }
                    actions.push_back(action);
                }
            } else if (arg.find(L"reboot=") == 0) {
                rebootMsg = arg.substr(7);  // Custom reboot message
            } else if (arg.find(L"command=") == 0) {
                failureCommand = arg.substr(8);  // Command to run upon failure
            }
        }

        // Dynamically allocate memory for the new failure actions
        SERVICE_FAILURE_ACTIONS newFailureActions = {};
        newFailureActions.dwResetPeriod = resetPeriod;
        newFailureActions.cActions = static_cast<DWORD>(actions.size());
        newFailureActions.lpsaActions = actions.empty() ? nullptr : &actions[0];  // Points to actions vector
        newFailureActions.lpRebootMsg = rebootMsg.empty() ? nullptr : const_cast<LPWSTR>(rebootMsg.c_str());
        newFailureActions.lpCommand = failureCommand.empty() ? nullptr : const_cast<LPWSTR>(failureCommand.c_str());

        // Apply the new failure actions to the service
        if (ChangeServiceConfig2W(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &newFailureActions)) {
            std::wcout << L"[SC_CLONE] Service failure actions configured for: " << serviceName << std::endl;
        } else {
            PrintErrorMessage(L"[SC_CLONE] ChangeServiceConfig2 failed with error code: ", GetLastError());
        }
    }

    // Close service and SCM handles
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

// Main function which handles command-line arguments and invokes relevant service management functions
int wmain(int argc, wchar_t* argv[]) {
    // Ensure enough arguments are provided
    if (argc < 3) {
        std::wcerr << L"[SC_CLONE] Usage: sc_clone <command> <service_name> [options]" << std::endl;
        return 1;
    }
    
    std::wstring command = argv[1];
    std::wstring serviceName = argv[2];
    
    // Parse the command and invoke the corresponding function
    if (command == L"query") {
        QueryServiceStatus(serviceName);
    } else if (command == L"create" && argc == 4) {
        std::wstring executablePath = argv[3];
        CreateServiceEntry(serviceName, executablePath);
    } else if (command == L"start") {
        StartServiceEntry(serviceName);
    } else if (command == L"stop") {
        StopServiceEntry(serviceName);
    } else if (command == L"delete") {
        DeleteServiceEntry(serviceName);
    } else if (command == L"config") {
        if (argc > 3) {
            std::wstring startType = argv[3];
            ConfigureService(serviceName, startType);
        } else {
            ConfigureService(serviceName);
        }
    } else if (command == L"qdescription") {
        QueryServiceDescription(serviceName);
    } else if (command == L"failure") {
        std::vector<std::wstring> failureArgs;
        for (int i = 3; i < argc; ++i) {
            failureArgs.push_back(argv[i]);
        }

        ConfigureServiceFailure(serviceName, failureArgs);
    } else {
        std::wcerr << L"[SC_CLONE] Unsupported or incorrect command usage." << std::endl;
    }
    
    return 0;
}
