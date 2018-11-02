/*	Voice_Adapter

HiROC CVS ID: $Id: Mac_Voice_Adapter.hh,v 2.1 2013/04/08 19:29:10 guym Exp $

Copyright (C) 2009-2011  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/

#ifndef HiView_Mac_Voice_Adapter_hh
#define HiView_Mac_Voice_Adapter_hh

#include "Image_Viewer.hh"
#include "Statistics_Tool.hh"
#include "Data_Mapper_Tool.hh"
using namespace UA::HiRISE;

#include "Voice_Adapter.hh"
#include "SpeechHandler.hh"

class Mac_Voice_Adapter : Voice_Adapter
{
    
public:    
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const ID;
	
    Mac_Voice_Adapter(UA::HiRISE::Image_Viewer* viewer, UA::HiRISE::Statistics_Tools* stattool, UA::HiRISE::Data_Mapper_Tool* mapper);


    void toggle(bool on);
    
private:
    SpeechHandler* speechHandler;
    
};


#endif /* HiView_Mac_Voice_Adapter_hh */
