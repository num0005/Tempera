/*
	Project: haloforge
	File: addlog.h
	Copyright � 2009 SilentK, Abyll

	This file is part of haloforge.

    haloforge is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    haloforge is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with haloforge.  If not, see <http://www.gnu.org/licenses/>.

	No addresses needed


*/ 

#ifndef ADDLOG_H
#define ADDLOG_H

#include <windows.h>
#include <time.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#define LOGNAME "live_projekt.log"

void __cdecl DEBUG(const char* fmt, ...);
char *GetDirectoryFile(char* filename);
void InitAddLog(HMODULE hModule);
void ExitAddLog();

/*class CLogger
{
private:
	ofstream ofile;
	
public:
	CLogger(HMODULE hModule);
	~CLogger();

	AddLog(char* fmt, ...);
};*/

#endif /* ADDLOG_H */