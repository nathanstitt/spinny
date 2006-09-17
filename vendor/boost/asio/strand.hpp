//
// strand.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_STRAND_HPP
#define ASIO_STRAND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "boost/asio/detail/push_options.hpp"

#include "boost/asio/basic_strand.hpp"

namespace asio {

/// Typedef for the typical usage of strand.
typedef basic_strand<> strand;

} // namespace asio

#include "boost/asio/detail/pop_options.hpp"

#endif // ASIO_STRAND_HPP
