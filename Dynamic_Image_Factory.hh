/*	Dynamic_Image_Factory

HiROC CVS ID: $Id: Dynamic_Image_Factory.hh,v 1.3 2012/09/17 04:52:22 castalia Exp $

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

#pragma once

#include	<QString>
#include	<QSize>


namespace UA::HiRISE
{
//	Forward reference.
class Dynamic_Image;
class Dynamic_QImage;
class JP2_Image;

/**	A <i>Dynamic_Image_Factory</i> provides a factory method for
    creating Dynamic_Image objects.

    @see	Dynamic_Image
    @author		Bradford Castalia, UA/HiROC
    @version	$Revision: 1.3 $
*/
class Dynamic_Image_Factory
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
    /**	Factory methods to construct a Dynamic_Image.

    */
    static Dynamic_Image* create(const QString& source_name,
                                 const QSize& size = QSize(), QString* message = NULL);


private:

    static Dynamic_QImage* create_Dynamic_QImage(const QString& source_name,
                                                 const QSize& size);

    static JP2_Image* create_JP2_Image(const QString& source_name,
                                       const QSize& size);

    /*==============================================================================
        Accessors
    */
public:

    static QString image_type()
    {
        return Type;
    }

    static QString error_message()
    {
        return Error_Message;
    }

    /*==============================================================================
        Utilities
    */
    static bool is_file(const QString& name);

    /*==============================================================================
        Data
    */
private:

    static QString
        Type,
        Error_Message;

};


}	//	namespace UA::HiRISE

