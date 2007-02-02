/* @(#)stream.hpp
 */

#ifndef _HANDLERS_STREAM_H
#define _HANDLERS_STREAM_H 1


#include "handlers/shared.hpp"
#include "streaming/lame.hpp"

namespace handlers {

	class Stream
		: public ews::request_handler {
	public:
		Stream();
		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
		
		//void send_more( Lame *ls, asio::ip::tcp::socket *socket, const asio::error& e, std::size_t bytes_transferred ) const;
	};

} // handlers



#endif /* _HANDLERS_STREAM_H */

