/* @(#)song.hpp
 */

#ifndef _SPINNY_SONG_H
#define _SPINNY_SONG_H 1

#include "sqlite/sqlite.hpp"
#include "spinny/config.h"
#include "spinny/music_dir.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/shared_ptr.hpp"
#include <stdexcept>


namespace Spinny {


class MusicDir;
class Album;
class Artist;
class PlayList;

class Song : public sqlite::table {
	GRANT_NEEDED_FRIENDSHIP(Song);
	friend class PlayList;

	sqlite::id_t _dir_id;
	sqlite::id_t _artist_id;
	sqlite::id_t _album_id;
	std::string _file_name;
	std::string _title;
	int _track;
	int _length;
	int _bitrate;
	int _year;

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	virtual const description* m_table_description() const;

	Song();

public:
 	static const sqlite::table::description* table_description();


	typedef ::sqlite::result_set<Song> result_set;
	typedef boost::shared_ptr<Song> ptr;

	static bool		is_interesting( const boost::filesystem::path &path );
	static ptr		create_from_file( const MusicDir &md, const std::string name );
	static sqlite::id_t	count();
	static ptr		load( sqlite::id_t db_id );
	static result_set       find( const std::string &query, int first, int count );
	static unsigned int     num_matching( const std::string &query );

	bool save() const;

	boost::filesystem::path
  	path() const;

	boost::shared_ptr<MusicDir>
	directory() const;

	boost::shared_ptr<Album>
	album() const;

	boost::shared_ptr<Artist>
	artist() const;

	std::string
	title() const;

	std::string
	name() const;

	int
	track() const;

	int
	length() const;

	int
	bitrate() const;

	int
	year() const;

	std::string
	filesystem_name() const;

	struct file_error : public std::runtime_error {
		file_error( const char *msg );
	};


};

} // namespace Spinny

#endif /* _SPINNY_SONG_H */

