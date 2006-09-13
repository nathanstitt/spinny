/* @(#)album.hpp
 */

#ifndef _ALBUM_H
#define _ALBUM_H 1

#include "spinny.hpp"
#include "artist.hpp"
#include "song.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>


class Album : public sqlite::table {
	GRANT_NEEDED_FRIENDSHIP(Album);
	friend class Artist;

	std::string _name;

	// begin db methods
	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

	Album();
public:
	typedef ::sqlite::result_set<Album> result_set;
	typedef boost::shared_ptr<Album> ptr;
	bool save() const;

	// end db methods

	Album::result_set
	static all();

	static
	Album::ptr
	find_or_create( const std::string &name );

	void
	add_artist( Artist::ptr &artist );

	Song::result_set
	songs() const;

	Artist::result_set
	artists() const;

	std::string
	name() const;
};


#endif /* _ALBUM_H */

