/*	Plastic_Image_Factory

HiROC CVS ID: $Id: Plastic_Image_Factory.hh,v 1.3 2012/09/17 04:52:22 castalia Exp $

Copyright (C) 2011  Arizona Board of Regents on behalf of the
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

#ifndef HiView_Plastic_Image_Factory_hh
#define HiView_Plastic_Image_Factory_hh

#include	<QString>
#include	<QSize>


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class Plastic_Image;
class Plastic_QImage;
class JP2_Image;

/**	A <i>Plastic_Image_Factory</i> provides a factory method for
	creating Plastic_Image objects.

	@see	Plastic_Image
	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.3 $
*/
class Plastic_Image_Factory
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Creators
*/
/**	Factory methods to construct a Plastic_Image.

*/
static Plastic_Image* create (const QString& source_name,
	const QSize& size = QSize (), QString* message = NULL);


private:

static Plastic_QImage* create_Plastic_QImage (const QString& source_name,
	const QSize& size);

static JP2_Image* create_JP2_Image (const QString& source_name,
	const QSize& size);

/*==============================================================================
	Accessors
*/
public:

static QString image_type ()
	{return Type;}

static QString error_message ()
	{return Error_Message;}

/*==============================================================================
	Utilities
*/
static bool is_file (const QString& name);

/*==============================================================================
	Data
*/
private:

static QString
	Type,
	Error_Message;

};	//	Class Plastic_Image_Factory


}	//	namespace HiRISE
}	//	namespace UA
#endif
