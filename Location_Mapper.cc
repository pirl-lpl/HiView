/*	Location_Mapper

HiROC CVS ID: $Id: Location_Mapper.cc,v 1.6 2012/09/27 22:09:16 castalia Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
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

#include	"Location_Mapper.hh"

#include	"Coordinate.hh"
#include	"Projection.hh"
#include	"PDS_Metadata.hh"
#include	"HiView_Utilities.hh"

#include	"PVL.hh"
using idaeim::PVL::Aggregate;

#include	<QString>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_HELPERS			(1 << 2)
#define	DEBUG_TRANSFORM			(1 << 3)
#define	DEBUG_PROJECTION		(1 << 4)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::setw;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Location_Mapper::ID =
		"UA::HiRISE::Location_Mapper ($Revision: 1.6 $ $Date: 2012/09/27 22:09:16 $)";

/*==============================================================================
	Constructors
*/
Location_Mapper::Location_Mapper
	(
	idaeim::PVL::Aggregate*	parameters
	)
	:	Projector (NULL)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Location_Mapper" << endl;
#endif
try {this->parameters (parameters);}
catch (...) {reset ();}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Location_Mapper" << endl;
#endif
}


Location_Mapper::Location_Mapper
	(
	const Location_Mapper&	location_mapper
	)
	:	Projector (NULL)
{*this = location_mapper;}


Location_Mapper&
Location_Mapper::operator=
	(
	const Location_Mapper&	location_mapper
	)
{
if (this != &location_mapper)
	{
	delete Projector;
	if (location_mapper.Projector)
		Projector = location_mapper.Projector->clone ();
	else
		Projector = NULL;
	}
return *this;
}


Location_Mapper::~Location_Mapper ()
{delete Projector;}

/*==============================================================================
	Accessors
*/
QString
Location_Mapper::projection_name () const
{
if (Projector)
	return Projector->projection_name ();
return QString ();
}


Coordinate
Location_Mapper::project_to_world
	(
	const Coordinate&	image_coordinate
	) const
{
if (Projector)
	return Projector->to_world (image_coordinate);
return Coordinate ();
}


Coordinate
Location_Mapper::project_to_image
	(
	const Coordinate&	world_coordinate
	) const
{
if (Projector)
	return Projector->to_image (world_coordinate);
return Coordinate ();
}

/*==============================================================================
	Manipulators
*/
Location_Mapper&
Location_Mapper::parameters
	(
	const idaeim::PVL::Aggregate*	parameters
	)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Location_Mapper::parameters" << endl;
#endif
delete Projector;
Projector = Projection::create (parameters);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Location_Mapper::parameters" << endl;
#endif
return *this;
}


void
Location_Mapper::reset ()
{
if (Projector)
	Projector->reset ();
}


}	//	namespace HiRISE
}	//	namespace UA
