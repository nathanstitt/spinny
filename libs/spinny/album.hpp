/* @(#)album.hpp
 */

#ifndef _SPINNY_ALBUM_H
#define _SPINNY_ALBUM_H 1


#include "spinny/artist.hpp"
#include "spinny/song.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace Spinny {


class Album : public sqlite::table {
	GRANT_NEEDED_FRIENDSHIP(Album);
	friend class Artist;

	std::string _name;
	sqlite::id_t num_songs_;
	bool counts_loaded_;

	// begin db methods
	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	virtual const description* m_table_description() const;

	Album();
public:
 	static const sqlite::table::description* table_description();

	typedef ::sqlite::result_set<Album> result_set;
	typedef boost::shared_ptr<Album> ptr;
	bool save() const;

	// end db methods

	Album::ptr
	static load( sqlite::id_t db_id );

	Album::result_set
	static all();

	static
	Album::ptr
	find_or_create( const std::string &name );

	static
	result_set
	name_starts_with( const std::string &name );

	static
	result_set
	with_artist_id( sqlite::id_t db_id );

	typedef std::list<std::pair<char, unsigned int> > starting_char_t;
	static
	starting_char_t
	starting_chars();

	static
	sqlite::id_t
	count();

	sqlite::id_t
	num_songs();

	Song::result_set
	songs() const;

	Artist::result_set
	artists() const;

	std::string
	name() const;
};

} // namespace Spinny

#endif /* _SPINNY_ALBUM_H */

