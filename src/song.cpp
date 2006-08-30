#include "song.hpp"

#include "boost/filesystem/operations.hpp"
#include "id3lib/tag.h"

class song_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "songs";
	};
	virtual int num_fields() const {
		return 7;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"dir_id",
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
			"int",
			"string",
			"string",
			"int",
			"int",
			"int",
			"int",
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
	str << _dir_id << "," << sqlite::q(_file_name) << "," << sqlite::q(_title)
	    << "," << _track << "," << _length << "," << _bitrate << "," << _year;
}

void
Song::table_update_values( std::ostream &str ) const {
	str << "dir_id=" << _dir_id 
	    << ",file_name=" << sqlite::q(_file_name) 
	    << ",title=" << sqlite::q(_title)
	    << ",track=" << _track 
	    << ",length" << _length 
	    << ",bitrate=" << _bitrate 
	    << ",year=" << _year;
}


void
Song::initialize_from_db( const sqlite::reader *reader ) {
	_dir_id = reader->get<int>(0);
	_file_name = reader->get<std::string>(1);
	_title	   = reader->get<std::string>(2);
	_track	   = reader->get<int>(3);
	_length	   = reader->get<int>(4);
	_bitrate   = reader->get<int>(5);
	_year	   = reader->get<int>(6);
	_dir       = MusicDir::load( _dir_id );
}


Song::Song() : sqlite::table(),
	       _dir_id(0), _file_name(""), _title(""), _track(0), _length(0), _bitrate( 0 ), _year( 0 ) {}


Song::file_error::file_error(const char *msg ) : std::runtime_error( msg ) { }

bool
Song::is_interesting( const boost::filesystem::path &path ){
	return boost::iends_with( path.string(), ".MP3" );
}



Song::ptr
Song::create_from_file( const MusicDir &md, const std::string name ){
	boost::filesystem::path path = md.path() / name;
	if ( ! boost::filesystem::exists( path ) || ! is_interesting( path ) ) {
		throw file_error("can't create as doesn't exist, or just plain don't care..." );
	}

	Song *song = new Song;
	song->_dir_id = md.db_id();
	song->_file_name=name;
	song->_dir = md;

	ID3_Tag tag( song->path().string().c_str() );
	ID3_Field *field;
	ID3_Frame *frame;

	frame = tag.Find( ID3FID_TITLE );
	if ( frame && ( field = frame->GetField(ID3FN_TEXT) ) ) {
		song->_title=field->GetRawText();
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

	return Song::ptr( song );
}



boost::filesystem::path
Song::path() const{
	return _dir.path() / _file_name;
}


bool
Song::save(){
	return Spinny::db()->save<Song>(*this);
}

std::string
Song::title() const {
	return _title;
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

