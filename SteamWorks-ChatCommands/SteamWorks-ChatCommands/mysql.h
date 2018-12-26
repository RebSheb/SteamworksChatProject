#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "sqlite3.h"

#pragma comment(lib, "H:\\Visual Studio Libraries\\Network\\SQLiteCpp\\build\\sqlite3\\RelWithDebInfo\\sqlite3.lib")


class ASQL
{
public:

	ASQL(const std::string);

	void createTable(std::string);




	sqlite3 * _getDBHandle() { return this->db; }
	int GetReturnCode() { return this->rc; }
	std::string GetErrorMsg() { return std::string(this->ErrMsg); }
	bool TableExists(std::string);

	static int callback(void *unk, int argcount, char**argv, char** columnName);


private:
	sqlite3 *db;
	char *ErrMsg = 0;
	int ok;
	int rc;

	char *authCreate = "CREATE TABLE AUTH(ID INT PRIMARY KEY NOT NULL,STEAMID VARCHAR(32) NOT NULL, NAME VARCHAR(32) NOT NULL);";


	void endswithappend(std::string);
	void setError();
	int checkError();
	void CloseDB();
};

