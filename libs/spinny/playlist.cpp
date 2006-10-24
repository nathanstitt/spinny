/* @(#)playlist.cpp
 */


#include "spinny/playlist.hpp"



#include "song.hpp" 
#include <vector>
#include <limits>
#include <list>
#include <iterator>

class pls_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "playlist_songs";
	};
	virtual int num_fields() const {
		return 3;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"present_order",
			"song_id",
			"playlist_id"
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"int",
			"int"
		};
		return field_types;
	};
};

static pls_desc pls_desc;


class pl_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "play_lists";
	};
	virtual int num_fields() const {
		return 2;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"bitrate",
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"string",
		};
		return field_types;
	};
};

static pl_desc table_desc;


const sqlite::table::description*
PlayList::table_description(){
	return &table_desc;
}


const sqlite::table::description*
PlayList::m_table_description() const {
	return &table_desc;
}


void
PlayList::table_insert_values( std::ostream &str ) const {
	str << bitrate_ << "," << sqlite::q(name_);
}

void
PlayList::table_update_values( std::ostream &str ) const {
	str << "bitrate=" << bitrate_ << ",name=" << sqlite::q(name_);
}


void
PlayList::initialize_from_db( const sqlite::reader *reader ) {
	bitrate_ = reader->get<int>(0);
	name_	   = reader->get<std::string>(1);
}

PlayList::PlayList() : sqlite::table(), bitrate_(0), name_("") {

}


// public methods



PlayList::ptr
PlayList::load( sqlite::id_t db_id ){
	return sqlite::db()->load<PlayList>( db_id );
}

PlayList::result_set
PlayList::all(){
	return sqlite::db()->load_all<PlayList>( "upper(name)" );
}


PlayList::ptr
PlayList::create( int bitrate, std::string name ){
	PlayList::ptr pl( new PlayList );
	pl->name_=name;
	pl->bitrate_ = bitrate;
	return pl;
}


std::string
PlayList::name() const {
	return name_;
}

int
PlayList::bitrate() const {
	return bitrate_;
}


Song::result_set
PlayList::songs() const {
	sqlite::connection *con = sqlite::db();

	*con << "select ";
	Song::table_description()->insert_fields( *con );

	*con << ",songs.rowid from songs, playlist_songs where songs.rowid "
	      << "= playlist_songs.song_id and playlist_songs.playlist_id = "
	     << this->db_id() << " order by playlist_songs.present_order";


	return con->load_stored<Song>();
}


void
PlayList::push_back( const Song &s ) {
	
	sqlite::connection *con = sqlite::db();
	*con << "insert into playlist_songs ( song_id, playlist_id, present_order ) values ( "
	     << s.db_id() << ","
	     << this->db_id() << ","
	     << "(select max(present_order) + 1 from playlist_songs where playlist_id = " 
	     << this->db_id() << ") )";

	con->exec<sqlite::none>();

}

bool
PlayList::save() const {
	return sqlite::db()->save<PlayList>(*this);
}
