/* @(#)artist.hpp
 */

#ifndef _ARTIST_H
#define _ARTIST_H 1

#include "spinny.hpp"
#include "song.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

class MusicDir;

class Artist : public sqlite::table {
	GRANT_NEEDED_FRIENDSHIP(Artist);

	std::string _name;

	// begin db methods
	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

	Artist();
public:
	typedef ::sqlite::result_set<Artist> result_set;
	typedef boost::shared_ptr<Artist> ptr;
	bool save();

	// end db methods

	Song::result_set
	songs() const;

// 	static Song::ptr
// 	load_or_create( const std::string &name );


};


#endif /* _ARTIST_H */

