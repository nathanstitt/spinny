#include "streaming/buffer.hpp"

//using asio::ip::tcp;
using namespace Streaming;

buffer::buffer(const std::string& data)
    : data_(new std::vector<char>(data.begin(), data.end())),
      buffer_(asio::buffer(*data_))
{ }

const asio::const_buffer* 
buffer::begin() const { 
	return &buffer_; 
}

const asio::const_buffer*
buffer::end() const { 
	return &buffer_ + 1; 
}

void
buffer::append( const char *data, unsigned int len ){
	//data_->insert( data, len );
}
