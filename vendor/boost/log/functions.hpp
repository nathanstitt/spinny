// functions.hpp

// Boost Logging Template library
//
// Author: John Torjo
//
// Copyright (C) 2004-2005 John Torjo (john@torjo.com)
// Copyright (C) 2004-2005 Caleb Epstein (caleb.epstein@gmail.com)
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
    Helper Logging functions
*/

#ifndef JT_BOOST_FUNCTIONS_HPP
#define JT_BOOST_FUNCTIONS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/log/log_impl.hpp>
#include <time.h>

// many thanks Pavel Vozenilek!
#include <boost/detail/workaround.hpp>
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#  pragma warn -8026 // some functions are not expanded inline
#endif

namespace boost { namespace logging {

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Log functions

// ... Windows only 
#if defined(BOOST_LOG_WIN32)
    BOOST_LOG_DECL void write_to_dbg_wnd(const logging_types::string &, const logging_types::string &msg);
#endif






struct BOOST_LOG_DECL write_to_file {
    write_to_file(const std::string & file_name, bool overwrite = true) : file_name(file_name), overwrite_file(new bool(overwrite) ) {}

    // ... so that it can be used as a Log Function
    void operator()(const logging_types::string&, const logging_types::string & msg) { write(msg); }
private:
    void write(const logging_types::string& msg);
private:
    std::string file_name;
    // note: each logger has its own copy of a write_to_file object. This needs to be shared,
    //       so that different copies don't overwrite each-other
    boost::shared_ptr<bool> overwrite_file;
};





/* 
    Represents an array of appenders. It forwards each received message to all of its
    underlying appenders
*/
struct BOOST_LOG_DECL appender_array {

    appender_array& add( appender_func func) {
        m_funcs.push_back(func);
        return *this;
    }

    void operator()( const logging_types::string & log_name, const logging_types::string & msg);

private:
    std::vector<appender_func> m_funcs;
};





    
    
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Modifier functions


// writes the prefix
BOOST_LOG_DECL void prepend_prefix(const logging_types::string &prefix, logging_types::string & msg);
// appends an enter if not already found
BOOST_LOG_DECL void append_enter(const logging_types::string &, logging_types::string & msg);


// prefixes the message with the time. You pass the format logging_types::string at construction.
//
// The format can contain escape sequences:
// $dd - day, 2 digits
// $MM - month, 2 digits
// $yy - year, 2 digits
// $yyyy - year, 4 digits
// $hh - hour, 2 digits
// $mm - minute, 2 digits
// $ss - second, 2 digits
//
// Example: 'Today is $dd/$MM/$yyyy'
struct BOOST_LOG_DECL prepend_time {
    prepend_time(const logging_types::string & format);

    void operator()(const logging_types::string &, logging_types::string & msg);

private:
    // the indexes of each escape sequence within the format logging_types::string
    int m_day, m_month, m_yy, m_yyyy, m_hour, m_min, m_sec;
    logging_types::string m_format;
    logging_types::char_t m_buffer[256];
};



// prefixes the message with the time. You pass the format logging_types::string at
// construction.
 //
// The format will be expanded using std::strftime.
//
// Example: 'Today is %c'
struct BOOST_LOG_DECL prepend_time_strf {
    prepend_time_strf(const logging_types::string & format, bool localtime = true);

    void operator()(const logging_types::string &, logging_types::string & msg);

private:
    logging_types::string m_format;
    bool m_localtime;
    logging_types::char_t m_buffer[256];
    time_t m_t;
    tm m_tm;
};



BOOST_LOG_DECL void prepend_thread_id(const logging_types::string &, logging_types::string & msg);

BOOST_LOG_DECL void write_to_cout(const logging_types::string &, const logging_types::string &msg) ;


}}


// many thanks Pavel Vozenilek!
#include <boost/detail/workaround.hpp>
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#  pragma warn .8026 // some functions are not expanded inline
#endif

#endif

