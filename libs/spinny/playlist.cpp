/* @(#)playlist.cpp
 */


//#include "streaming/stream.hpp"
#include "spinny/playlist.hpp"
#include "spinny/complete_playlist.hpp"


namespace Spinny {


class pl_song_desc : public sqlite::table::description {
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

static pl_song_desc pls_desc;


class pl_desc : public sqlite::table::description {
public:
	void checked_callback( bool ) {
		if ( ! sqlite::db()->exec<int>( "select count(*) from playlists") ){
			PlayList::ptr pl = PlayList::create( "All Songs","All the songs in your library" );
			pl->save();
		}

	}
	virtual const char* table_name() const {
		return "playlists";
	};
	virtual int num_fields() const {
		return 4;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"name",
			"description",
			"order_by",
			"present_order",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"string",
			"string",
			"string",
			"int",
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
	str << sqlite::q(name_) << ',' << sqlite::q(description_) << ',' << sqlite::q(order_by_) << ',' << present_order_;
}

void
PlayList::table_update_values( std::ostream &str ) const {
	str << "name=" << sqlite::q(name_) 
	    << ",description=" << sqlite::q(description_) << ",order_by=" << sqlite::q(order_by_) << ",present_order=" << present_order_;
}


void
PlayList::initialize_from_db( const sqlite::reader *reader ) {
	name_	   = reader->get<std::string>(0);
	description_ = reader->get<std::string>(1);
	order_by_ = reader->get<std::string>(2);
	present_order_ = reader->get<int>(3);
}

PlayList::PlayList() : sqlite::table(), order_by_("present_order desc"),present_order_(0) {

}


// public methods



PlayList::ptr
PlayList::load( sqlite::id_t db_id ){
	if ( db_id == 1 ){
		return CompletePlayList::instance();
	} else {
		return sqlite::db()->load<PlayList>( db_id );
	}
}

PlayList::result_set
PlayList::all(){
	return sqlite::db()->load_all<PlayList>( "present_order,upper(name)" );
}


PlayList::ptr
PlayList::create( const std::string &name, const std::string &description ){
	PlayList::ptr pl( new PlayList );
	pl->name_=name;
	pl->description_ = description;
	pl->present_order_=sqlite::db()->exec<int>( "select max(present_order)+1 from playlists" );
	return pl;
}


void
PlayList::set_order( sqlite::id_t db_id, int order ){
	sqlite::connection *con = sqlite::db();
	*con << "update "
	     << PlayList::table_description()->table_name()
	     << " set present_order = " << order << " where rowid = " << db_id;
	con->exec<sqlite::none>();
}



std::string
PlayList::name() const {
	return name_;
}

std::string
PlayList::set_name(  const std::string& name ){
	return name_=name;
}

std::string
PlayList::description() const {
	return description_;
}

std::string
PlayList::set_description(  const std::string& desc ){
	return description_=desc;
}


Song::ptr
PlayList::at( unsigned int pos ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ", playlist_songs.rowid from songs, playlist_songs where songs.rowid "
	      << "= playlist_songs.song_id and playlist_songs.playlist_id = "
	     << this->db_id() << " order by " << sqlite::q(order_by_,false)
	     << " limit " << pos << ",1" ;
	return con->load_one<Song>();
}

Song::ptr
PlayList::load_song( sqlite::id_t song_id ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ", songs.rowid from songs, playlist_songs where songs.rowid"
	     << "= playlist_songs.song_id and playlist_songs.rowid = " << song_id;
	return con->load_one<Song>();
}


int
PlayList::present_order() const {
	return present_order_;
}

int
PlayList::set_present_order( int present_order ){
	return present_order_=present_order;
}

void
PlayList::set_order( std::string order, bool descending ){
	std::string new_order = "upper(";
	new_order += order;
	if ( descending ){
		new_order += ") desc";
	} else {
		new_order += ") asc";
	}
	if ( new_order != order_by_ ){
		this->order_by_ = new_order;
		this->save();
	}
}

Song::result_set
PlayList::songs( unsigned int start, unsigned int limit ) const {
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << ",(select name from artists where artists.rowid=songs.artist_id) as artist"
	     << ",(select name from albums where albums.rowid=songs.album_id) as album"
	     << ",songs.rowid from songs, playlist_songs where songs.rowid "
	      << "= playlist_songs.song_id and playlist_songs.playlist_id = "
	     << this->db_id() << " order by "
	     << sqlite::q(order_by_,false);
	if ( limit ){
		*con << " limit " << start << ',' << limit;
	}

	return con->load_stored<Song>();
}

void
PlayList::set_song_order( sqlite::id_t db_id, int order ){
	sqlite::connection *con = sqlite::db();
	*con << "update playlist_songs set present_order="
	     << order << " where playlist_songs.song_id = " << db_id << " and playlist_id = " << this->db_id();
	con->exec<sqlite::none>();
}


void
PlayList::insert( Spinny::Song::result_set songs, int position ){
	sqlite::connection *con = sqlite::db();

	*con <<  "update playlist_songs set present_order = present_order+" << songs.size() << " where present_order >= " 
	     << position << " and playlist_id = " << this->db_id();
	con->exec<sqlite::none>();

	for ( Spinny::Song::result_set::iterator song = songs.begin(); songs.end() != song; ++song){
		*con << "insert into playlist_songs ( song_id, playlist_id, present_order ) values ( "
		     << song->db_id() << ','
		     << this->db_id() << ','
		     << position++ << ')';
		con->exec<sqlite::none>();
	}
}


void
PlayList::insert( Song::ptr s, int position ) {
	sqlite::connection *con = sqlite::db();


	*con <<  "update playlist_songs set present_order = present_order+1 where present_order >= " 
	     << position << " and playlist_id = " << this->db_id();

	con->exec<sqlite::none>();

	*con << "insert into playlist_songs ( song_id, playlist_id, present_order ) values ( "
	     << s->db_id() << ','
	     << this->db_id() << ','
	     << position << ')';
	con->exec<sqlite::none>();
}

void
PlayList::insert( PlayList::ptr pl, int position ) {
	sqlite::connection *con = sqlite::db();
	
	*con <<  "update playlist_songs set present_order = present_order+" << pl->size() << " where present_order >= " 
	     << position << " and playlist_id = " << this->db_id();
	con->exec<sqlite::none>();

	*con << "select song_id from playlist_songs where playlist_id = "
	     << this->db_id() << " order by "
	     << sqlite::q(order_by_,false);

	std::list<sqlite::id_t> song_ids;
	sqlite::command cmd(con);
	for ( sqlite::command::reader_iterator it=cmd.reader_begin(); it != cmd.reader_end(); ++it ){
		song_ids.push_back( it->get<sqlite::id_t>(0) );
	}
	for ( std::list<sqlite::id_t>::iterator id = song_ids.begin(); id != song_ids.end(); ++id ){
		*con << "insert into playlist_songs ( song_id, playlist_id, present_order ) values ( "
		     << *id << ','
		     << this->db_id() << ','
		     << ++position << ')';
		con->exec<sqlite::none>();
	}
}

void
PlayList::insert( Spinny::MusicDir::ptr md, int position ){
	this->insert( md->songs(), position );
}

void
PlayList::insert( Spinny::Artist::ptr artist, int position ){
	this->insert( artist->songs(), position );
}

void
PlayList::insert( Spinny::Album::ptr album, int position ){
	this->insert( album->songs(), position );
}


bool
PlayList::save() const {
	return sqlite::db()->save<PlayList>(*this);
}


unsigned int
PlayList::size() const {
	sqlite::connection *con = sqlite::db();
	*con << "select count(*) from playlist_songs where playlist_id = " << this->db_id();
	return con->exec<unsigned int>();
}

void
PlayList::clear(){
	sqlite::connection *con = sqlite::db();
	*con << "delete from playlist_songs where playlist_id = " << this->db_id();
	con->exec<sqlite::none>();
}

void
PlayList::remove( sqlite::id_t song_id ){
	sqlite::connection *con = sqlite::db();

	*con <<  "update playlist_songs set present_order = present_order-1 where present_order >= " 
	     << "(select b.present_order from playlist_songs b where playlist_id = " << this->db_id() 
	     << " and b.song_id = " << song_id << ") and playlist_id = " << this->db_id();
	con->exec<sqlite::none>();

	
	*con <<	"delete from playlist_songs where playlist_id = " << this->db_id() 
	     << " and playlist_songs.song_id = " << song_id;

	con->exec<sqlite::none>();

}


} // namespace Spinny
