#include "mysql.h"


ASQL::ASQL(const std::string path)
{
	this->ok = sqlite3_open_v2(path.c_str(), &this->db, SQLITE_OPEN_READWRITE, NULL);
	
	if (ok)
	{
		printf("Can't open the database. Use GetErrorMsg();\n");
		this->ErrMsg = (char*)sqlite3_errmsg(this->db);
		this->CloseDB();
		return;
	}
	else
	{
		printf("Database opened!\n");
		return;
	}

}


/*
@info Do not provide a ;
*/
bool ASQL::TableExists(std::string tbl)
{
	sqlite3_stmt *stmt;
	std::string checkTbl("SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ");
	/*
		Need to check if the starting character and ending character of string
		passed in begins/ends with a '.
	*/
	if (tbl.at(1) != '\'')
		tbl.insert(0, "'");

	if (tbl.at(tbl.back()) != '\'')
		tbl.insert(tbl.size(), "'");

	checkTbl.append(tbl).append(";");



	this->rc = sqlite3_prepare_v2(this->db, checkTbl.c_str(), -1, &stmt, NULL);
	if (this->checkError() != 1)
	{
		this->setError();
		return false;
	}

	//this->rc = sqlite3_exec(this->db, query.c_str(), NULL, 0, &this->ErrMsg);
	printf("%s\n", sqlite3_column_text(stmt, 0));

	return true;
}


void ASQL::createTable(std::string sqlStatement)
{

}


void ASQL::endswithappend(std::string string)
{
	if (string.at(string.back()) != ';')
		string.append(";");
}

void ASQL::setError()
{
	this->ErrMsg = (char*)sqlite3_errmsg(this->db);
	return;
}

int ASQL::callback(void * unk, int argcount, char ** argv, char ** columnName)
{
	for (int i = 0; i <= argcount; i++)
	{

	}

	return 0;
}

int ASQL::checkError()
{
	switch (this->rc)
	{
	case SQLITE_OK:
		return 1; // OK

	case SQLITE_BUSY:
		return -1;

	case SQLITE_FAIL:
		return -2;

	default:
		return -3; // cba
	}
}


void ASQL::CloseDB()
{
	sqlite3_close(this->db);
	return;
}
