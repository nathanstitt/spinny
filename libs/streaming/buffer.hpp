#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Streaming {


// A reference-counted non-modifiable buffer class.
	class buffer
	{
	public:
		// Construct from a std::string.
		explicit buffer( const std::string& data );

		// Implement the Const_Buffers concept.
		typedef asio::const_buffer value_type;
		typedef const asio::const_buffer* const_iterator;
		const asio::const_buffer* begin() const;
		const asio::const_buffer* end() const;

		void append( const char *data, unsigned int len );
		boost::shared_ptr<std::vector<char> > data_;
		asio::const_buffer buffer_;
	private:
	};

}
