
#include "boost/filesystem/operations.hpp"
 

struct FooDoggy {
	int age;
	std::string name;
};

namespace sqlite{
	template <> inline
	FooDoggy 
	reader::get( int index ){
		FooDoggy f;
		f.age=this->get<int>(0);
		f.name=this->get<std::string>(1);
		return f;
	}
	template <> inline
	void
	command::bind( const FooDoggy &fd ){
		this->bind( 0, fd.age );
		this->bind<const std::string&>( 1, fd.name );
	};
}

inline
bool
populate_db( sqlite::connection &con ) {
	int count=con.exec<int>("select count(*) from sqlite_master where name='people_test';");
	if( count == 0 ) {
		con.exec<sqlite::none>("create table people_test (number,string);");
	} else {
		con.exec<sqlite::none>("delete from people_test");
	}
	sqlite::transaction trans(con);

	sqlite::command cmd(con, "insert into people_test values(?,?);");
	cmd.bind(2, "Bar" );
	for(int i=1; i<10000; i++ ) {
		cmd.bind(1, i);
		cmd.exec<sqlite::none>();
	}
	trans.commit();
	return true;
}


inline
bool
remove_db( sqlite::connection &con ){
	con.close();
	boost::filesystem::remove("test.db");
	return true;
}
