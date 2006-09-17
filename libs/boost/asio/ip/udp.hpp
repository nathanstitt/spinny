//
// udp.hpp
// ~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_UDP_HPP
#define ASIO_IP_UDP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "boost/asio/detail/push_options.hpp"

#include "boost/asio/basic_datagram_socket.hpp"
#include "boost/asio/basic_resolver.hpp"
#include "boost/asio/ip/basic_endpoint.hpp"
#include "boost/asio/ip/basic_resolver_iterator.hpp"
#include "boost/asio/ip/basic_resolver_query.hpp"
#include "boost/asio/detail/socket_types.hpp"

namespace asio {
namespace ip {

/// Encapsulates the flags needed for UDP.
/**
 * The asio::ip::udp class contains flags necessary for UDP sockets.
 *
 * @par Thread Safety:
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class udp
{
public:
  /// The type of a UDP endpoint.
  typedef basic_endpoint<udp> endpoint;

  /// The type of a resolver query.
  typedef basic_resolver_query<udp> resolver_query;

  /// The type of a resolver iterator.
  typedef basic_resolver_iterator<udp> resolver_iterator;

  /// Construct to represent the IPv4 UDP protocol.
  static udp v4()
  {
    return udp(PF_INET);
  }

  /// Construct to represent the IPv4 UDP protocol.
  static udp v6()
  {
    return udp(PF_INET6);
  }

  /// Obtain an identifier for the type of the protocol.
  int type() const
  {
    return SOCK_DGRAM;
  }

  /// Obtain an identifier for the protocol.
  int protocol() const
  {
    return IPPROTO_UDP;
  }

  /// Obtain an identifier for the protocol family.
  int family() const
  {
    return family_;
  }

  /// The IPv4 UDP socket type.
  typedef basic_datagram_socket<udp> socket;

  /// The UDP resolver type.
  typedef basic_resolver<udp> resolver;

private:
  // Construct with a specific family.
  explicit udp(int family)
    : family_(family)
  {
  }

  int family_;
};

} // namespace ip
} // namespace asio

#include "boost/asio/detail/pop_options.hpp"

#endif // ASIO_IP_UDP_HPP