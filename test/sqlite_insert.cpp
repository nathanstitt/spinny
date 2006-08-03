/*
	Copyright (C) 2004-2005 Cory Nelson

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
	
	CVS Info :
		$Author: phrostbyte $
		$Date: 2005/06/16 20:46:40 $
		$Revision: 1.1 $
*/

#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;

#include "sqlite/sqlite.hpp"
#include "boost/filesystem/operations.hpp"
using namespace sqlite;

int
sqlite_insert( int argc, char *argv[] )
{
	bool failed=false;
	try
	{
		connection con("test.db");
		int count=con.exec<int>("select count(*) from sqlite_master where name='t_test';");
		if( count == 0 ) {
			con.exec<none>("create table t_test(number,string);");
		} else {
			con.exec<none>("delete from t_test");
		}

		transaction trans(con);
		{
			command cmd(con, "insert into t_test values(?,?);");
			cmd.bind(2, "foobar", 6);
			for(int i=0; i<10000; i++) {
				cmd.bind(1, i);
				cmd.exec<none>();
			}
		}

		trans.commit();
		{
			command cmd(con, "select count(*) from t_test;");
			reader r=cmd.exec<reader>();
			r.read();
			failed=( r.get<int>() != 10000 );
		}
		con.close();
	}
	catch(exception &ex) {
		cerr << "Exception Occured: " << ex.what() << endl;
		failed = true;
	}
	boost::filesystem::remove("test.db");
	return failed;
}
