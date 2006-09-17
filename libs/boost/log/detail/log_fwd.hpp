// log_fwd.hpp

// Boost Logging Template library
//
// Author: John Torjo
//
// Copyright (C) 2004-2005 John Torjo (john@torjo.com)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.
 
// See http://www.boost.org for updates, documentation, and revision history.

/*
    Note: the library is thread-safe.
*/

#ifndef JT_BOOST_LOG_LOG_FWD_HPP
#define JT_BOOST_LOG_LOG_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/log/detail/defs.hpp>
#include <boost/config.hpp>

#include <boost/log/detail/ts.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#define BOOST_LOG_VC
#endif

#ifdef BOOST_LOG_VC
    #include <tchar.h>
#else
    #if defined(_UNICODE) || defined(UNICODE)
    #define _T(x)      L ## x
    #else
    #define _T(x)      x
    #endif
#endif

// sometimes only one of the two is defined...
// for example, for VC - OutputDebugWindow depends on UNICODE,
// while _T depends on _UNICODE
#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

#if defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif



namespace boost { namespace logging {

    /**
        Notes: 
        [1] There will be only one instance of the log manager class
        
        there are other functions you need to provide for your log_manager, 
        in case you choose to create one.

        See log_manager.hpp

        They are not provided here, since they make use of boost::function.
        Including boost::function is time-costly.
    
    */
    struct default_log_manager {

        // required typedefs
#ifndef BOOST_LOG_VC
        typedef char char_t;
        typedef std::string string;
        typedef std::ostringstream stream;
#else
        typedef TCHAR char_t;
        typedef std::basic_string<char_t> string;
        typedef std::basic_ostringstream<char_t> stream;
#endif

    };

    template<int i> struct log_manager {
        typedef default_log_manager type;
    };


}}


#endif
