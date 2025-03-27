# DFOR740Midterm

README

Overview

This project is a C++ implementation of the core functionalities of sc.exe, the Windows Service Controller command-line tool. The program allows users to manage Windows services by providing equivalent functionalities to sc.exe.

Features

This tool supports the following commands:

query - Checks the status of a specified service.

create - Creates a new service with a specified binary path.

qdescription - Retrieves the description of a specified service.

start - Starts a specified service.

stop - Stops a specified service.

delete - Deletes a specified service.

config - Modifies the configuration of an existing service.

failure - Configures service failure actions.


Usage

    sc_clone <command> <service_name> [options]


Examples

Query the status of a service:

    sc_clone query MyService

Create a new service:

    sc_clone create MyService "C:\\Path\\To\\Service.exe"

Start a service:

    sc_clone start MyService

Stop a service:

    sc_clone stop MyService

Delete a service:

    sc_clone delete MyService

Modify service configuration:

    sc_clone config MyService type= own start= auto

Configure service failure actions:

    sc_clone failure MyService reset= 86400 actions= restart/5000
    

Compilation

This project requires a Windows environment and Microsoft Visual Studio with Windows SDK installed.

Open a terminal (Developer Command Prompt for VS).

Compile the code using the following command:

    cl /std:c++17 /EHsc /Fe:sc_clone.exe sc_clone.cpp

Compiling with nmake (MSVC)

Open a Developer Command Prompt for VS.

Navigate to the directory containing the source code.

Run the following command to compile:

    nmake -f Makefile.nmake

To clean up compiled files:

    nmake clean
    

Compiling with cl (MSVC manually)

Alternatively, compile the code using the following command:

    cl /std:c++17 /EHsc /Fe:sc_clone.exe sc_clone.cpp

Compiling with GCC (manually)
        
    x86_64-w64-mingw32-g++ -o sc_clone.exe sc_clone.cpp -std=c++17 -municode -ladvapi32


Logging and Monitoring

To analyze the execution and detect the use of this tool, Sysmon and Procmon can be utilized:


Sysmon

Configure Sysmon to log process creation, service modifications, and registry changes.

Use a custom Sysmon configuration to track changes to services.


Procmon

Filter sc_clone.exe execution and service-related registry modifications.

    

Detection Methods

  Windows Event Logs
    Monitor Event IDs related to service creation (7045), modification, and execution (4697, 7036).

  Sysmon Logs
    Track process creation events and command-line arguments to detect unauthorized service modifications.

  Defensive Monitoring
    Use SIEM tools to alert suspicious modifications to services outside of administrator-approved tools.
    

Additional Notes

Administrator privileges are required to execute service modifications.

Use this tool responsibly in a controlled environment for testing and learning purposes only.
