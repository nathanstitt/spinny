//
// select_reactor_fwd.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SELECT_REACTOR_FWD_HPP
#define ASIO_DETAIL_SELECT_REACTOR_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "boost/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <bool Own_Thread>
class select_reactor;

} // namespace detail
} // namespace asio

#include "boost/asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SELECT_REACTOR_FWD_HPP
