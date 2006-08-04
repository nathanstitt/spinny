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

#include <iostream>
#include <stdexcept>
using namespace std;

#include "sqlite/sqlite.hpp"

#include "sqlite_db_utils.hpp"

using namespace sqlite;


int
sqlite_select( int argc, char *argv[] )
{
	bool failed=false;
	sqlite::connection con("test.db");
	populate_db( con );
	try {
		{
			command cmd(con, "select * from t_test;");
			reader r=cmd.exec<reader>();
			int age=1;
			while(r.read()){
				FooDoggy fd=r.get<FooDoggy>();
				assert( fd.age == age++ );
			}
				
		}
		con.close();
	}
	catch(exception &ex) {
		cerr << "Exception Occured: " << ex.what() << endl;
	}
	remove_db( con );
	return failed;
}
