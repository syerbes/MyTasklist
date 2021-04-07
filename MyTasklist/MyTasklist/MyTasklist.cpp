// MyTasklist.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LocalTasklist.h"
#include <iostream>




int main(int argc, char* argv[])
{
    int local = 1;
    int i;
    // Si no esta /S   -  No remote
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "/S") == 0) {
            local = 0;
        }
    }

    // We are in local mode
    if (local == 1) {

        // Standard tasklist
        if (argc == 1) {
            LocalTasklist("Standard");
        }
        // Verbose or services modes
        else if (argc == 2) {
            //Verbose mode
            if (strcmp(argv[1], "/V") == 0) {
                LocalTasklist("Verbose");
            }
            // Service Mode
            else if (strcmp(argv[1], "/SVC") == 0) {
                LocalTasklist("SVC");
            }
            // Wrong input
            else {
                std::wcout << "Incorrect arguments" << std::endl;
            }
        }
        // Wrong input
        else {
            std::wcout << "Incorrect arguments" << std::endl;
        }
    }

    // Remote Mode
    else {
        if (argc == 2) {
            std::wcout << "Need to specify Domain Name: /S Domain /U User /P Password" << std::endl;
        }
        else if (argc >= 3 && argc <=6){
            std::wcout << "Need User and Password: /U User /P Password" << std::endl;
        }
        else if (argc == 7){
            if (strcmp(argv[1], "/S") == 0 && strcmp(argv[3], "/U") == 0 && strcmp(argv[5], "/P")==0) {
                // Every parameter is OK and the code should be executed
                //RemoteTasklist();
            }
            else {
                std::wcout << "Need to specify Domain, User and Password: /S Domain /U User /P Password" << std::endl;
            }
        }
        else if (argc == 8) {
            if (strcmp(argv[1], "/S") == 0 && strcmp(argv[3], "/U") == 0 && strcmp(argv[5], "/P")==0 && (strcmp(argv[7], "/V")==0 || strcmp(argv[7], "/SVC")==0)) {
                // Everything OK. All options selected.
                if (strcmp(argv[7], "/V")==0) {
                    // Verbose Mode
                    //RemoteTasklistVerbose();
                }
                else{
                    //Service Mode
                    //RemoteTaskListService();
                }
            }
           
        }
    }

    return 0;
        




        // IF arguments > 2 -  Too many arguments for local mode
        // Else If arguments = 1   -   Standard tasklist
        // Else (arguments = 2 )
            // If argument = /V  -  Verbose Mode
            // Else If argument = /SVC  -  Weird Mode
            // Else  -   Incorrect argument

    // Else () - Remote Mode
        // If arguments = 2 - Need User and Password
        // Else If arguments = 3  - Need to specify both /u 'User' and /p 'Password'
        // Else If arguments = 4  
            // If arguments = 4 and /u and /p - Remote tasklist
            // Else If arguments = 4 and only /u - Need to specify a /p
            // Else If arguments = 4 and only /p - Need to specify a /u
            // Else - Need to specify both /u and /p, Incorrect argument
        // Else If arguments = 5
            //If no /v or /svc, /s, /u, /r - Incorrect arguments, please, specify /s, /u, /p and /v or /SVC, if needed
        // Else  - Incorrect arguments, please, specify /s, /u, /p and /v or /SVC, if needed
            
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
