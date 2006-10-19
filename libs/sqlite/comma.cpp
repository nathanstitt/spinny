/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: commas.cc 2 2002-08-24 19:02:47Z nas $ 
 * Copyright (C) 2001 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#include <string>
#include "sqlite/comma.hpp"

using namespace sqlite;

commas::commas(){
	times_called_=0;
}

char
commas::get(){
	if ( times_called_++ )
		return ',';
	else 
		return ' ';

}

int
commas::times_called() const {
	return times_called_;
}

void
commas::reset(){
	times_called_=0;
}

std::ostream &
operator << ( std::ostream& out_stream, sqlite::commas &comma ){
	out_stream << comma.get();
        return out_stream;
}
