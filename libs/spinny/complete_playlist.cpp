
#include "spinny/complete_playlist.hpp"

namespace Spinny {

CompletePlayList::ptr instance_;

CompletePlayList::ptr
CompletePlayList::instance(){
	if ( ! instance_ ){
		instance_.reset( new CompletePlayList );
	}
	return instance_;
}

Song::result_set
CompletePlayList::songs( std::string order, unsigned int start, unsigned int limit ) const {
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ",rowid from songs order by ";
	if ( order.empty() ){
		*con << "songs.title";
	} else {
		*con << sqlite::q( order );
	}
	if ( start && limit ){
		*con << " limit " << start << ',' << limit;
	}
	return con->load_stored<Song>();
}


unsigned int
CompletePlayList::size() const {
	return sqlite::db()->exec<unsigned int>("select count(*) from songs");
}



} // namespace Spinny
