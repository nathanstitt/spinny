/* @(#)playlist.hpp
 */

#ifndef _PLAYLIST_H
#define _PLAYLIST_H 1

#include "spinny/spinny.hpp"
#include "spinny/song.hpp"

#include <vector>



class PlayList : public sqlite::table {
public:
	typedef boost::shared_ptr<PlayList> ptr;
	typedef ::sqlite::result_set<PlayList> result_set;
private:
	GRANT_NEEDED_FRIENDSHIP(PlayList);

	PlayList();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

        int bitrate_;
	string name_;
	string description_;
	int present_order_;
public:
	static ptr
	load( sqlite::id_t db_id );

	static result_set
	all();

	static ptr
	create( int bitrate, const std::string &name, const std::string &desc );

	static void
	set_order( sqlite::id_t db_id, int order );

	std::string
	name() const;

	std::string
	set_name( const std::string& name );

	std::string
	description() const;

	std::string
	set_description( const std::string& desc );

	int
	bitrate() const;

	int
	set_bitrate( int br );

	int
	present_order() const;

	int
	set_present_order( int br );

	Song::result_set
	songs() const;

	void
	push_back( const Song &s );

	bool
	save() const;

	unsigned int
	size() const;

	void
	clear();

	void
	remove( const Song &s );
};

#endif /* _PLAYLIST_H */

