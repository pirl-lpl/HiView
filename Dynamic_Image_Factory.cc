/*	Dynamic_Image_Factory

HiROC CVS ID: $Id: Dynamic_Image_Factory.cc,v 1.6 2012/12/14 23:00:41 guym Exp $

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

#include	"Dynamic_Image_Factory.hh"

#include	"Dynamic_Image.hh"
#include	"Dynamic_QImage.hh"
#include	"JP2_Image.hh"
#include	"HiView_Utilities.hh"

//	UA::HiRISE::JP2_Reader.
#include	"JP2.hh"
using UA::HiRISE::JP2;
#include	"JP2_Reader.hh"
using UA::HiRISE::JP2_Reader;
#include	"JP2_Exception.hh"
using UA::HiRISE::JP2_Exception;

#include	<QString>
#include	<QSize>
#include	<QFileInfo>
#include	<QObject>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

    DEBUG_SECTION report selection options.
    Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<iostream>
using std::clog;
using std::endl;
#endif

#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
    Constants
*/
const char* const
Dynamic_Image_Factory::ID =
"UA::HiRISE::Dynamic_Image_Factory ($Revision: 1.6 $ $Date: 2012/12/14 23:00:41 $)";

static QString
Type,
Error_Message;

/*==============================================================================
    Creators
*/
Dynamic_Image*
Dynamic_Image_Factory::create
(
const QString& source_name,
const QSize& size,
QString* message
)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    clog << ">>> Dynamic_Image_Factory::create:" << endl
        << "    source_name = " << source_name << endl
        << "    size = " << size << endl;
#endif
    Dynamic_Image
        * image = NULL;
    Type.clear();
    Error_Message.clear();
    if (!source_name.isEmpty())
    {
        if (JP2_Image::is_JP2_file(source_name) ||
            HiView_Utilities::is_URL(source_name))
        {
            Type = "JP2";
            image = create_JP2_Image(source_name, size);
        }
        else
        {
            Type = "QImage";
            image = create_Dynamic_QImage(source_name, size);
        }

        if (image)
            //	Apply the source name.
            image->source_name(source_name);
        else
            Error_Message
            .prepend(ID)
            .prepend('\n')
            .prepend(QObject::tr("Failed to load a "))
            .prepend(Type)
            .prepend(QObject::tr(" image from -\n"))
            .prepend(source_name)
            .prepend("\n\n");
    }
    else
        Error_Message
        .prepend(ID)
        .prepend('\n')
        .prepend(QObject::tr("No image source specified."));

    if (!image &&
        message)
        *message += Error_Message;

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    if (image)
        clog << "    plastic_" << *image << endl;
    else
        clog << "   unable to obtain source -" << endl
        << Error_Message << endl;
    clog << "<<< Dynamic_Image_Factory::create: @ " << (void*)image << endl;
#endif
    return image;
}

/*==============================================================================
    Helpers
*/
Dynamic_QImage*
Dynamic_Image_Factory::create_Dynamic_QImage
(
const QString& source_name,
const QSize& size
)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    clog << ">>> Dynamic_Image_Factory::create_Dynamic_QImage" << endl;
#endif
    Dynamic_QImage
        * image = NULL;
    if (is_file(source_name))
    {
        QImage
            * source_image = new QImage(source_name);
        if (!source_image->isNull())
        {
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
            clog << "    loaded QImage @ " << (void*)source_image << endl;
#endif
            image = new Dynamic_QImage(source_image, size);
        }
        else
        {
            delete source_image;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
            clog << "   unable to load the source" << endl;
#endif
            Error_Message +=
                QObject::tr("An image could not be obtained from the file.");
        }
    }
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    clog << "<<< Dynamic_Image_Factory::create_Dynamic_QImage" << endl;
#endif
    return image;
}


JP2_Image*
Dynamic_Image_Factory::create_JP2_Image
(
const QString& source_name,
const QSize& size
)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    clog << ">>> Dynamic_Image_Factory::create_JP2_Image" << endl;
#endif
    JP2_Reader
        * reader = NULL;
    JP2_Image
        * image = NULL;
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
    clog << "    constructing a JP2::reader on \""
        << source_name.toStdString() << '"' << endl;
#endif
    try { reader = JP2::reader(source_name.toStdString()); }
    catch (JP2_Exception& except)
    {
        Error_Message += except.what();
    }
    catch (std::exception& except)
    {
        Error_Message += except.what();
    }
    catch (...)
    {
        Error_Message += QObject::tr("Unknown exception!");
    }
    if (reader)
    {
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
        clog << "    JP2_Reader @ " << (void*)reader << endl
            << "    constructing a JP2_Image" << endl;
#endif
        //	N.B.: Ownership of the JP2_Reader is transferred.
        try { image = new JP2_Image(reader, size); }
        catch (JP2_Exception& except)
        {
            Error_Message += except.what();
        }
        catch (std::exception& except)
        {
            Error_Message += except.what();
        }
        catch (...)
        {
            Error_Message += QObject::tr("Unknown exception!");
        }
        if (!image)
            delete reader;
    }
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
    clog << "<<< Dynamic_Image_Factory::create_JP2_Image" << endl;
#endif
    return image;
}

/*==============================================================================
    Utilities
*/
bool
Dynamic_Image_Factory::is_file
(
const QString& pathname
)
{
    bool
        OK = false;
    QFileInfo
        file(pathname);
    if (!file.exists())
        Error_Message += "The file could not be found.";
    else
        if (file.isDir())
            Error_Message += "The file is a directory.";
        else
            if (!file.isReadable())
                Error_Message += "The file is not readable.";
            else
                if (file.size() == 0)
                    Error_Message += "The file is empty.";
                else
                    OK = true;
    return OK;
}

/*==============================================================================
    Data
*/
QString
Dynamic_Image_Factory::Type,
Dynamic_Image_Factory::Error_Message;


}	//	namespace UA::HiRISE
