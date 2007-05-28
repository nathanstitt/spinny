
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

void
CompletePlayList::set_order( std::string order, bool descending ){
	order_ = "upper(";
	order_ += sqlite::q( order, false );
	if ( descending ){
		order_ += ") DESC";
	} else {
		order_ += ") ASC";
	}
}


Song::result_set
CompletePlayList::songs( unsigned int start, unsigned int limit ) const {
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ",(select name from artists where artists.rowid=songs.artist_id) as artist"
	     << ",(select name from albums where albums.rowid=songs.album_id) as album"
	     << ",rowid from songs order by ";
	if ( order_.empty() ){
		*con << "songs.title";
	} else {
		*con << order_;
	}

	if ( limit ){
		*con << " limit " << start << ',' << limit;
	}
	return con->load_stored<Song>();
}


unsigned int
CompletePlayList::size() const {
	return sqlite::db()->exec<unsigned int>("select count(*) from songs");
}


Song::ptr
CompletePlayList::at( unsigned int pos ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ",(select name from artists where artists.rowid=songs.artist_id) as artist"
	     << ",(select name from albums where albums.rowid=songs.album_id) as album"
	     << ",rowid from songs order by ";
	if ( order_.empty() ){
		*con << "songs.title";
	} else {
		*con << order_;
	}

	*con << " limit " << pos << ",1" ;

	return con->load_one<Song>();
}



void
CompletePlayList::set_song_order( sqlite::id_t, int  ){

}

void
CompletePlayList::insert( Spinny::Song::result_set, int){

}

void
CompletePlayList::insert( Song::ptr , int ){

}

void
CompletePlayList::insert( PlayList::ptr, int ){

}

void
CompletePlayList::insert( Spinny::MusicDir::ptr, int ){

}

void
CompletePlayList::insert( Spinny::Artist::ptr, int ){

}

void
CompletePlayList::insert( boost::shared_ptr<Album>, int ){

}

void
CompletePlayList::clear(){

}

void
CompletePlayList::remove( sqlite::id_t ){

}


} // namespace Spinny
