/*	HiView_Utilities

HiROC CVS ID: $Id: HiView_Utilities.cc,v 1.21 2014/05/27 17:13:51 guym Exp $

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

#include	"HiView_Utilities.hh"

#include	<QImageReader>
#include	<QImageWriter>
#include	<QStringList>
#include	<QFont>
#include	<QFontMetrics>
#include	<QDir>
#include	<QUrl>
#include	<QMutex>
#include <QUrl>

#include	<string>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF			0
#define DEBUG_ALL			-1
#define DEBUG_IMAGE_FORMATS	(1 << 1)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#else

#include	<iostream>
using std::clog;
#include	<iomanip>
using std::endl;

#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	HiView_Utilities
*/
const char* const
	HiView_Utilities::ID =
		"UA::HiRISE::HiView_Utilities ($Revision: 1.21 $ $Date: 2014/05/27 17:13:51 $)";

const char* const
	HiView_Utilities::PDS_LABEL_URL_PARAMETER	= "uinf/url /URL";

#ifndef PDS_LABEL_FILENAME_EXTENSION
#define PDS_LABEL_FILENAME_EXTENSION	"LBL"
#endif
const char* const
	HiView_Utilities::PDS_LABEL_FILENAME_EXT	= PDS_LABEL_FILENAME_EXTENSION;


bool
HiView_Utilities::is_URL
	(
	const QString&	URL
	)
{
bool
	OK = false;
QUrl
	Url (URL);
if (Url.isValid ())
	{
	QString
		protocol (Url.scheme ().toUpper ());
	if (! protocol.isEmpty () &&
		 (protocol == "HTTP" || protocol == "HTTPS" ||
		  protocol == "JPIP") &&
		! Url.host ().isEmpty () &&
		! Url.path ().isEmpty ())
		OK = true;
	}
return OK;
}


bool
HiView_Utilities::is_JPIP_URL
	(
	const QString&	URL
	)
{
return
	is_URL (URL) &&
	QUrl (URL).scheme ().compare ("JPIP", Qt::CaseInsensitive) == 0;
}


#ifndef DEFAULT_BAND_NUMBERING_INDEXED
#define DEFAULT_BAND_NUMBERING_INDEXED	true
#endif
bool
	HiView_Utilities::Band_Numbering_Indexed =
		DEFAULT_BAND_NUMBERING_INDEXED;


QString
	HiView_Utilities::Image_Reader_Formats_File_Filters,
	HiView_Utilities::Image_Writer_Formats_File_Filters;
QStringList
	*HiView_Utilities::Image_Reader_Formats = NULL,
	*HiView_Utilities::Image_Writer_Formats = NULL;

QUrl
HiView_Utilities::PDS_metadata_URL
	(
	const QString& Source_Name,
	const QUrl&	model_URL
	)
{
#if ((DEBUG_SECTION) & DEBUG_METADATA)
LOCKED_LOGGING ((
clog << ">>> HiView_Utilities::PDS_metadata_URL: "
		<< model_URL.toString () << endl));
#endif
QUrl
	URL (Source_Name);
#if ((DEBUG_SECTION) & DEBUG_METADATA)
LOCKED_LOGGING ((
clog << "    source URL: " << URL.toString () << endl));
#endif
if (model_URL.isEmpty () ||
	model_URL == URL)
	{
	QString
		pathname (QDir::fromNativeSeparators (URL.path ()));
	int
		index = pathname.lastIndexOf ('.');
	if (index < 0)
		//	No existing extension; add on.
		pathname.append ('.');
	++index;
	if (index < pathname.length ())
		//	Remove existing extension.
		pathname.truncate (index);
	pathname.append (HiView_Utilities::PDS_LABEL_FILENAME_EXT);
	URL.setPath (pathname);
	}
else
	{
	QString
		name (model_URL.scheme ());
	if (name.isEmpty () ||
		name.compare ("file", Qt::CaseInsensitive) == 0)
		{
		//	Model file pathname.
		name = QDir::fromNativeSeparators (model_URL.path ());
		if (name.endsWith ('/'))
			name.truncate (name.length () - 1);
		#if ((DEBUG_SECTION) & DEBUG_METADATA)
		LOCKED_LOGGING ((
		clog << "     model pathname: " << name << endl));
		#endif
		if (name.startsWith ('/'))
			//	Replace the source pathname with the model's absolute pathname.
			URL.setPath (name);
		else
			{
			//	Modify the source pathname with the model's relative pathname.
			QString
				pathname (QDir::fromNativeSeparators (URL.path ()));
			if (pathname.endsWith ('/'))
				pathname.truncate (pathname.length () - 1);
			#if ((DEBUG_SECTION) & DEBUG_METADATA)
			LOCKED_LOGGING ((
			clog << "    source pathname: " << name << endl));
			#endif
			//	Remove the filename segment, leaving the segment separator.
			pathname.truncate (pathname.lastIndexOf ('/') + 1);
			#if ((DEBUG_SECTION) & DEBUG_METADATA)
			LOCKED_LOGGING ((
			clog << "     source dirname: " << name << endl));
			#endif
			pathname.append (name);
			URL.setPath (pathname);
			}
		}
	else
		//	Use the non-file model URL as-is.
		URL = model_URL;
	}
if (URL.scheme ().compare ("jpip", Qt::CaseInsensitive) == 0)
	{
	URL.setScheme ("http");
	URL.setPort (-1);
	}
#if ((DEBUG_SECTION) & DEBUG_METADATA)
LOCKED_LOGGING ((
clog << "<<< HiView_Utilities::PDS_metadata_URL: " << URL.toString () << endl));
#endif
return URL;
}


namespace
{
void
image_formats
	(
	const QList<QByteArray>&	formats,
	QStringList*				list,
	QString&					filters
	)
{
#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
clog << ">>> HiView_Utilities image_formats" << endl;
#endif
QString
	name;
int
	index = 0,
	entry = 0;
while (index < formats.size ())
	{
	name = formats.at (index++);
	//	Check for a case insensitive duplicate.
	entry = list->size ();
	while (--entry >= 0)
		if (name.compare (list->at (entry), Qt::CaseInsensitive) == 0)
			break;
	if (entry < 0)
		//	New entry; lowercase.
		list->append (name.toLower ());
	}

list->sort ();

filters = HiView_Utilities::file_filters_from (*list);
#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
clog << "<<< HiView_Utilities image_formats" << endl;
#endif
}
}	//	Local namespace


QString
HiView_Utilities::file_filters_from
	(
	QStringList&	list
	)
{
#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
clog << ">>> HiView_Utilities::file_filters_from" << endl;
#endif
QString
	filters;
int
	index;
/*
	Special case: format name alias removal.
	N.B.: List entries are presumed to be lowercase.
*/
if ((index = list.indexOf ("jpg")) >= 0)
	{
	if ((list.indexOf ("jpeg")) >= 0)
		list.removeAt (index);
	else
		list.replace (index, QString ("jpeg"));
	}
if ((index = list.indexOf ("tif")) >= 0)
	{
	if ((list.indexOf ("tiff")) >= 0)
		list.removeAt (index);
	else
		list.replace (index, QString ("tiff"));
	}

#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
clog << "    list -" << endl;
#endif
for (index = 0;
	 index < list.size ();
	 index++)
	{
	if (! filters.isEmpty ())
		filters += ";; ";
	filters += file_filter_for (list.at (index));
	#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
	clog << "    " << index << ": " << list.at (index) << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_IMAGE_FORMATS)
clog << "<<< HiView_Utilities::file_filters_from: " << filters << endl;
#endif
return filters;
}


QString
HiView_Utilities::file_filter_for
	(
	const QString&	format
	)
{
QString
	filter;
if (! format.isEmpty ())
	{
	QString
		upper (format.toUpper ());
	if (upper == "JPG")
		upper = "JPEG";
	if (upper == "TIF")
		upper = "TIFF";
	QString
		lower (upper.toLower ());
	filter = upper + " (*." + lower + " *." + upper;
	if (upper == "JPEG")
		filter += " *.jpg *.JPG";
	if (upper == "TIFF")
		filter += " *.tif *.TIF";
	filter += ')';
	}
return filter;
}


QString
HiView_Utilities::image_reader_formats_file_filters ()
{
if (! Image_Reader_Formats)
	image_reader_formats ();
return Image_Reader_Formats_File_Filters;
}


QStringList
HiView_Utilities::image_reader_formats ()
{
if (! Image_Reader_Formats)
	{
	Image_Reader_Formats = new QStringList;
	//	JP2 is to be included regardless of available plugins.
	Image_Reader_Formats->append ("jp2");
	image_formats
		(QImageReader::supportedImageFormats (),
		 Image_Reader_Formats,
		 Image_Reader_Formats_File_Filters);
	}
return *Image_Reader_Formats;
}


QString
HiView_Utilities::image_writer_formats_file_filters ()
{
if (! Image_Writer_Formats)
	image_writer_formats ();
return Image_Writer_Formats_File_Filters;
}


QStringList
HiView_Utilities::image_writer_formats ()
{
if (! Image_Writer_Formats)
	image_formats
		(QImageWriter::supportedImageFormats (),
		 Image_Writer_Formats = new QStringList,
		 Image_Writer_Formats_File_Filters);
return *Image_Writer_Formats;
}

/*==============================================================================
	Global functions
*/
QMutex
	Log_Lock;


double
round_to
	(
	double	value,
	int		decimal_places
	)
{
if (decimal_places == 0)
	{
	value += 0.5;
	value = static_cast<int>(value);
	}
else
	{
	double
		places = 1;
	if (decimal_places > 0)
		while (decimal_places--)
			places *= 10;
	else
		while (decimal_places++)
			places /= 10;
	value *= places;
	value += 0.5;
	value = (int)value / places;
	}
return value;
}


QString
magnitude_of
	(
	unsigned long long	value
	)
{
static const char* const
	MAGNITUDE = " KMGTPEZ";
double
	amount = value;
const char*
	mag;
for (mag = MAGNITUDE;
	*(mag + 1) &&
		amount >= 1024.0;
	++mag,
		amount /= 1024.0) ;

QString
	representation (QString::number (amount, 'g', 2));
if (*mag != ' ')
	representation += *mag;
return representation;
}


int
decimal_digits
	(
	unsigned long long	value
	)
{
int
	digits = 1;
while ((value /= 10))
	++digits;
return digits;
}


int
hex_digits
	(
	unsigned long long	value
	)
{
int
	digits = 1;
while ((value >>= 4))
	++digits;
return digits;
}


#ifndef DEFAULT_PATHNAME_WRAP_LENGTH
#define DEFAULT_PATHNAME_WRAP_LENGTH	300
#endif

QString
wrapped_pathname
	(
	const QString&	pathname,
	int				wrap_length,
	const QFont&	font
	)
{
QString
	path (pathname);
if (wrap_length < 50)
	wrap_length = DEFAULT_PATHNAME_WRAP_LENGTH;
QFontMetrics
	font_metrics (font);
int
	length = font_metrics.width (path);
if (length > wrap_length)
	{
	QChar
		separator (QDir::separator ());
	QStringList
		segments (pathname.split (separator));
	QString
		section;
	path.clear ();
	for (int
			index = 0;
			index < segments.count ();
		  ++index)
		{
		if (index)
			section += separator;
		section += segments.at (index);
		length = font_metrics.width (section);
		if (length > wrap_length)
			{
			section += '\n';
			path += section;
			section.clear ();
			}
		}
	if (path.at (path.length () - 1) == '\n')
		path.remove (path.length () - 1, 1);
	if (! section.isEmpty ())
		path += section;
	}
return path;
}


std::string
uppercase
	(
	const std::string&	a_string
	)
{
std::string
	new_string (a_string);
for (std::string::iterator
		character = new_string.begin (),
		end = new_string.end ();
		character < end;
	  ++character)
	*character = (char)toupper (*character);
return new_string;
}


std::string
remove
	(
	const std::string&	a_string,
	const char			character
	)
{
std::string
	new_string (a_string);
size_t
	index = 0;
while ((index = new_string.find (character, index)) != std::string::npos)
	new_string.erase (index, 1);
return new_string;
}


std::string
replace
	(
	const std::string&	a_string,
	const char			old_character,
	const char			new_character
	)
{
std::string
	new_string (a_string);
for (std::string::iterator
		character = new_string.begin (),
		end = new_string.end ();
		character < end;
	  ++character)
	if (*character == old_character)
		*character =  new_character;
return new_string;
}


bool
compare
	(
	const char*	this_string,
	const char*	that_string,
	bool		case_sensitive
	)
{
if (this_string &&
	that_string)
	{
	const char
		*this_ = this_string,
		*that_ = that_string;
	while (*this_ &&
		   *that_)
		if ((case_sensitive ?
				(*this_++ != *that_++) :
				(toupper (*this_++) != toupper (*that_++))))
			return false;
	if (*this_ == *that_)
		//	EOS of both strings.
		return true;
	//	EOS of only one string.
	return false;
	}
else
if (this_string ||
	that_string)
	//	Only one string is NULL.
	return false;
//	Both strings are NULL.
return true;
}


}
}
