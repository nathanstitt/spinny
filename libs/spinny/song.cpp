#include "song.hpp"

#include "boost/filesystem/operations.hpp"
#include "id3lib/tag.h"
#include "artist.hpp"
#include "album.hpp"

namespace Spinny {

class song_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "songs";
	};
	virtual int num_fields() const {
		return 9;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"dir_id",
			"artist_id",
			"album_id",
			"file_name",
			"title",
			"track",
			"length",
			"bitrate",
			"year",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"integer",
			"integer",
			"integer",
			"string",
			"string",
			"integer",
			"integer",
			"integer",
			"integer",
		};
		return field_types;
	};
};

static song_desc table_desc;


const sqlite::table::description*
Song::table_description(){
	return &table_desc;
}


const sqlite::table::description*
Song::m_table_description() const {
	return &table_desc;
}


void
Song::table_insert_values( std::ostream &str ) const {
	str << _dir_id << "," << _artist_id << "," << _album_id << "," << sqlite::q(_file_name) << "," << sqlite::q(_title)
	    << "," << _track << "," << _length << "," << _bitrate << "," << _year;
}

void
Song::table_update_values( std::ostream &str ) const {
	str << "dir_id=" << _dir_id 
	    << "artist_id=" << _artist_id
	    << "album_id=" << _album_id
	    << ",file_name=" << sqlite::q(_file_name) 
	    << ",title=" << sqlite::q(_title)
	    << ",track=" << _track 
	    << ",length" << _length 
	    << ",bitrate=" << _bitrate 
	    << ",year=" << _year;
}

sqlite::id_t
Song::count(){
	return sqlite::db()->count<Song>();
}

void
Song::initialize_from_db( const sqlite::reader *reader ) {
	_dir_id    = reader->get<sqlite::id_t>(0);
	_artist_id = reader->get<sqlite::id_t>(1);
	_album_id  = reader->get<sqlite::id_t>(2);
	_file_name = reader->get<std::string>(3);
	_title	   = reader->get<std::string>(4);
	_track	   = reader->get<int>(5);
	_length	   = reader->get<int>(6);
	_bitrate   = reader->get<int>(7);
	_year	   = reader->get<int>(8);
}


Song::Song() : sqlite::table(),
	       _dir_id(0),_artist_id(0), _file_name(""), _title(""), _track(0), _length(0), _bitrate( 0 ), _year( 0 ) {}



// END DB METHODS


Song::file_error::file_error(const char *msg ) : std::runtime_error( msg ) { }

bool
Song::is_interesting( const boost::filesystem::path &path ){
	return boost::iends_with( path.string(), ".MP3" );
}



Song::ptr
Song::create_from_file(  const MusicDir &md, const std::string name ){
	md.save_if_needed();

	boost::filesystem::path path = md.path() / name;

	if ( ! boost::filesystem::exists( path ) || ! is_interesting( path ) ) {
		BOOST_LOG(app) << "Can't load " << path.string() << " as it doesn't exist, or we can't handle that type of file." ;
		throw file_error("doesn't exist, or not equipped to handle");
	}

	Song::ptr song = Song::ptr( new Song );
	song->_dir_id = md.db_id();
	song->_file_name=name;

	ID3_Tag tag( path.string().c_str() );
	ID3_Field *field;
	ID3_Frame *frame;

	frame = tag.Find( ID3FID_TITLE );
	if ( frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
		song->_title=field->GetRawText();
	}
	Artist::ptr artist;
 	frame = tag.Find( ID3FID_LEADARTIST );
 	if ( frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
		Artist::ptr a = Artist::find_or_create( field->GetRawText() );
		artist.swap( a );
 		song->_artist_id=artist->db_id();
 	}

 	frame = tag.Find( ID3FID_ALBUM );
 	if ( artist && frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
		BOOST_LOGL( app,info ) << "Album: " << field->GetRawText();
		Album::ptr a = Album::find_or_create( artist, field->GetRawText() );
		a->add_artist( artist );
 		song->_album_id=a->db_id();
 	}


	try {
		frame = tag.Find( ID3FID_TRACKNUM );
		if ( frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
			song->_track=boost::lexical_cast<int>( field->GetRawText() );
		}
	} catch( boost::bad_lexical_cast & ){
		song->_track=0;
	}

	const Mp3_Headerinfo *header = tag.GetMp3HeaderInfo();
	if ( header ) {
		song->_length = header->time;
		song->_bitrate= header->bitrate;
		
	} else {
		song->_length = 0;
	}

	try {
		frame = tag.Find( ID3FID_YEAR );
		if ( frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
			song->_year=boost::lexical_cast<int>( field->GetRawText() );
		}
	} catch( boost::bad_lexical_cast & ){
		song->_year=0;
	}
	song->save();

	BOOST_LOGL( app,info ) << "Added song id: " << song->db_id() << " artist: " << song->_artist_id
			       << " album: " << song->_album_id << " : " << song->_title;

	return song;
}


Song::ptr
Song::load( sqlite::id_t db_id ){
	return sqlite::db()->load<Song>( db_id );
}

unsigned int
Song::num_matching( const std::string &query ){
	sqlite::connection *con = sqlite::db();
	*con << "select count(*) from songs where title like '%" << sqlite::q( query, false ) << "%'";
	return con->exec<unsigned int>();
}

Song::result_set
Song::find( const std::string &query, int first, int count ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	Song::table_description()->insert_fields( *con );
	*con << " from songs where title like '%" << sqlite::q( query, false ) << "%' order by lower(title) limit "
	     << first << ',' << count;
	cout << con->current_statement() << endl;
	return con->load_stored<Song>();
}


boost::filesystem::path
Song::path() const{
	MusicDir::ptr dir = this->directory();
	return dir->path() / _file_name;
}

MusicDir::ptr
Song::directory() const {
	return sqlite::db()->load<MusicDir>( _dir_id );
}


boost::shared_ptr<Album>
Song::album() const {
	Album::ptr al = sqlite::db()->load<Album>( _album_id );
	return al;
}

Artist::ptr
Song::artist() const {
	return sqlite::db()->load<Artist>( _artist_id );
}


bool
Song::save() const {
	return sqlite::db()->save<Song>(*this);
}

std::string
Song::title() const {
	return _title;
}


std::string
Song::name() const {
	return _title;
}

std::string
Song::filesystem_name() const {
	return _file_name;
}


int
Song::track() const {
	return _track;
}

int
Song::bitrate() const {
	return _bitrate;
}

int
Song::length() const {
	return _length;
}


int
Song::year() const {
	return _year;
}


} // namespace Spinny
