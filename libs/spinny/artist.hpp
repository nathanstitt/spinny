/* @(#)artist.hpp
 */

#ifndef _ARTIST_H
#define _ARTIST_H 1

#include "spinny.hpp"
#include "song.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

class Album;

class Artist : public sqlite::table {
	GRANT_NEEDED_FRIENDSHIP(Artist);

	std::string name_;
	sqlite::id_t num_songs_;
	sqlite::id_t num_albums_;
	bool counts_loaded_;


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
	bool save() const;

	// end db methods

	Artist::ptr
	static load( sqlite::id_t db_id );

	Artist::result_set
	static all();

	static
	Artist::ptr
	find_or_create( const std::string &name );

	static
	result_set
	name_starts_with( const std::string &name );

	typedef std::list<std::pair<char, unsigned int> > starting_char_t;
	static
	starting_char_t
	starting_chars();

	static
	sqlite::id_t count();

	static
	result_set
	with_album( const Album *alb );

	sqlite::result_set<Album>
	albums() const;

	Song::result_set
	songs() const;

	std::string
	name() const;
	
	sqlite::id_t
	num_albums();

	sqlite::id_t
	num_songs();
};


#endif /* _ARTIST_H */

