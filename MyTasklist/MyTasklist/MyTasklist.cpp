// MyTasklist.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LocalTasklist.h"
#include "RemoteTaskList.h"
#include <iostream>


int wmain(int argc, wchar_t* argv[])
{
	int local = 1;
	int i;
	// If not /S   -  No remote
	for (i = 0; i < argc; i++) {
		if (wcscmp(argv[i], L"/S") == 0) {
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
			if (wcscmp(argv[1], L"/V") == 0) {
				LocalTasklist("Verbose");
			}
			// Service Mode
			else if (wcscmp(argv[1], L"/SVC") == 0) {
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
		else if (argc >= 3 && argc <= 6) {
			std::wcout << "Need User and Password: /U User /P Password" << std::endl;
		}
		else if (argc == 7) {
			if (wcscmp(argv[1], L"/S") == 0 && wcscmp(argv[3], L"/U") == 0 && wcscmp(argv[5], L"/P") == 0) {
				// Every parameter is OK and the code should be executed
				RemoteTaskList(argv[2], argv[4], argv[6], "Standard");
			}
			else {
				std::wcout << "Need to specify Domain, User and Password: /S Domain /U User /P Password" << std::endl;
			}
		}
		else if (argc == 8) {
			if (wcscmp(argv[1], L"/S") == 0 && wcscmp(argv[3], L"/U") == 0 && wcscmp(argv[5], L"/P") == 0 && (wcscmp(argv[7], L"/V") == 0 || wcscmp(argv[7], L"/SVC") == 0)) {
				// Everything OK. All options selected.
				if (wcscmp(argv[7], L"/V") == 0) {
					// Verbose Mode
					RemoteTaskList(argv[2], argv[4], argv[6], "Verbose");
				}
				else {
					//Service Mode
					RemoteTaskList(argv[2], argv[4], argv[6], "SVC");
				}
			}
			else {
				std::wcout << "Need to specify Domain, User, Password and Output Mode (Optional): /S Domain /U User /P Password [/V OR /SVC]" << std::endl;
			}

		}
	}

	return 0;
}