#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <string>
#include <map>
#include "boost/asio.hpp"
#include "header.hpp"
#include <sstream>
#include "boost/filesystem/path.hpp"
#include "cs/cs.h"

namespace ews {

	class connection;

	/// A reply to be sent to a client.
	class reply
	{
		reply& operator=(const reply& r);
		reply(const reply& );
		class stream :boost::noncopyable, public std::ostream {
		public:
			stream();
			class buffer : public std::streambuf {
			public:
				int overflow(int c);
				std::string buffer_;
			};

			buffer buf_;
			unsigned int size() const;
			const std::string& str() const;
			const std::string& str( const std::string &s );
			
		};
		const connection *conn_;
		
		boost::filesystem::path template_;
		CSPARSE *cs_parse_;
		HDF *hdf_;

		bool parse_template();
	public:
		const char *tmpl_results_;

		/// The content to be sent in the reply.
		stream content;

		bool set_template( const boost::filesystem::path &path );
		bool set_hdf_value( const std::string &hdf_pos, const std::string &value );
		bool set_hdf_value( const std::string &hdf_pos, int value );

		
		~reply();
		explicit reply( const connection *conn );
		/// The status of the reply.
		enum status_type
		{
			ok = 200,
			created = 201,
			accepted = 202,
			no_content = 204,
			multiple_choices = 300,
			moved_permanently = 301,
			moved_temporarily = 302,
			not_modified = 304,
			bad_request = 400,
			unauthorized = 401,
			forbidden = 403,
			not_found = 404,
			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503
		} status;


		/// The headers to be included in the reply.
		typedef std::map< std::string, std::string > headers_t;
		headers_t headers;

		// set to a stock reply
		void set_to( status_type stat );

		/// Convert the reply into a vector of buffers. The buffers do not own the
		/// underlying memory blocks, therefore the reply object must remain valid and
		/// not be changed until the write operation has completed.
		std::vector<asio::const_buffer> to_buffers();

		bool set_basic_headers( const std::string &file_ext );

		bool add_header( const std::string &name, const std::string& value );

	};


} // namespace ews

#endif // HTTP_REPLY_HPP
