/*	HiView_Utilities

HiROC CVS ID: $Id: HiView_Utilities.hh,v 1.39 2012/11/21 18:54:05 guym Exp $

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

#ifndef HiView_Utilities_hh
#define HiView_Utilities_hh

#ifndef DOXYGEN_PROCESSING

#include	<QPoint>
#include	<QPointF>
#include	<QSize>
#include	<QSizeF>
#include	<QRect>
#include	<QColor>
#include	<QString>
#include	<QMutex>
#include	<QObject>
#include <QUrl>

class QStringList;
class QFont;

#include	<ostream>
#include	<iomanip>
#include	<cmath>
#include	<string>


namespace UA
{
namespace HiRISE
{

/*==============================================================================
	Global functions
*/

//	Thread safe DEBUG_SECTION log (clog) locking.
#if defined (DEBUG_SECTION)
extern QMutex Log_Lock;
#define LOCK_LOG	Log_Lock.lock ()
#define UNLOCK_LOG	Log_Lock.unlock ()
#define	LOCKED_LOGGING(expression)	\
	LOCK_LOG; \
	expression; \
	UNLOCK_LOG
#endif

/*------------------------------------------------------------------------------
	Output operators
*/
//!	QPoint output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QPoint& point)
	{return stream << point.x () << "x, " << point.y () << 'y';}

//!	QPointF output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QPointF& point)
	{return stream << point.x () << "x, " << point.y () << 'y';}

//!	QSize output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QSize& size)
	{return stream << size.width () << "w, " << size.height () << 'h';}

//!	QSizeF output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QSizeF& size)
	{return stream << size.width () << "w, " << size.height () << 'h';}

//!	QRect output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QRect& rectangle)
	{return 
		stream << rectangle.x () << "x, " << rectangle.y () << "y, "
			   << rectangle.width () << "w, " << rectangle.height () << 'h';}

//!	QRectF output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QRectF& rectangle)
	{return 
		stream << rectangle.x () << "x, " << rectangle.y () << "y, "
			   << rectangle.width () << "w, " << rectangle.height () << 'h';}

//!	QColor output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QColor& color)
	{return
		stream << std::hex << std::uppercase << std::setfill ('0')
			   << '#' << std::setw (8) << color.rgba ()
			   << std::dec << std::nouppercase << std::setfill (' ');}

//!	QString output operator.
inline std::ostream& operator<< (std::ostream& stream,
	const QString& qstring)
	{return stream << qPrintable (qstring);}

/*------------------------------------------------------------------------------
	Rounders

	Functions to convert objects with floating point data to the
	equivalent object with integer data.

	N.B.: These functions either round towards zero - round_down - or
	away from zero - round_up. For example, -1.2 will round_down to -1 and
	round_up to -2.

	The Qt qRound function, used by the toXxx methods of the
	corresponding classes, rounds to the "nearest integer" which rounds
	down when the real value has a fractional part < .5, and rounds up
	otherwise. This would split a pixel stradling a tile boundary such
	that it occurred twice: once on each side of the boundary. This is
	definately not what is intended. Instead, any partial pixel location
	(in floating point units) should be bound to a single pixel location
	(in integer units), which logically would be the location without any
	fractional part; i.e. "integer truncation" should be used. This would
	create a problem at the zero boundary - the pixels including location
	0.5 and -0.5 would both be at location 0 - but the image space being
	used here is unsigned integer pixel units. Any negative pixel
	locations are defined as outside the image.

	Note: One would expect integer truncation to be sufficient, however a
	floating point value may be an "epsilon" amount away from an integer
	value so that truncation alone produces the neighboring integer
	instead. To compensate for this effect the epsilon amouont (here set
	to 0.0000001) is added to the floating point value before truncation.
*/
#ifndef TO_INTEGER_EPSILON
#define TO_INTEGER_EPSILON	0.0000001
#endif

inline int round_down (double value)
	{return static_cast<int>(value +
		((value > 0.0) ? TO_INTEGER_EPSILON : -TO_INTEGER_EPSILON));}

inline int round_up (double value)
	{return static_cast<int>(value > 0.0 ? ceil (value) : floor (value));}

inline QPoint round_down (const QPointF& point)
	{return QPoint (round_down (point.x ()), round_down (point.y ()));}

inline QPoint round_up (const QPointF& point)
	{return QPoint (round_up (point.x ()), round_up (point.y ()));}

inline QSize round_down (const QSizeF& size)
	{return QSize (round_down (size.width ()), round_down (size.height ()));}

inline QSize round_up (const QSizeF& size)
	{return QSize (round_up (size.width ()), round_up (size.height ()));}

inline QRect round_down (const QRectF& rect)
	{return QRect (round_down (rect.topLeft ()), round_down (rect.size ()));}

inline QRect round_up (const QRectF& rect)
	{return QRect (round_up (rect.topLeft ()), round_up (rect.size ()));}

double round_to (double value, int decimal_places);

/*------------------------------------------------------------------------------
	C-String utilities
*/
/**	Convert a string to its uppercase form.

	<b>N.B.</b>: Subclass implementations may throw
	@param	a_string	A std::string to be converted.
	@return	A copy of the string in uppercase form.
*/
std::string uppercase (const std::string& a_string);

/**	Remove all occurances of a character from a string.

	@param	a_string	A std::string to be filtered.
	@param	character	The character to be removed.
	@return	A copy of the string with all occurances of the
		character removed.
*/
std::string remove (const std::string& a_string, const char character);

/**	Replace all occurances of a character in a string with a new character.

	@param	a_string	A std::string to be filtered.
	@param	old_character	The character to be replaced.
	@param	new_character	The new character to replace the old character.
	@return	A copy of the string with all occurances of the
		old character replaced with the new character.
*/
std::string replace (const std::string& a_string,
	const char old_character, const char new_character);

/**	Compare two C-strings.

	If both string pointers are NULL the comparison is true. Otherwise,
	if either string pointer is NULL the comparison is false.

	@param	this_string	A C-string.
	@param	that_string	A C-string.
	@param	case_sensitive	If true a case sensitive comparison is done;
		if false the case of the strings is ignored.
	@return	true if the strings match; false otherwise.
*/
bool compare (const char* this_string, const char* that_string,
	bool case_sensitive = false);

/*------------------------------------------------------------------------------
	Miscellaneous
*/

/**	Generate the representation of a value with magnitude annotation.

	The representation has the form:

	V.vvM

	where V.vv is the scaled value and m is the magnitude annotation.

	The value is scaled down by binary magnitude (1024) increments until
	it is less than 1024. This is the scaled value. It is written with
	up to two digits of decimal precision.

	The magnitude annotation is determined by the number of scaling
	increments applied to the value selected from "KMGTPEZ" where one
	scaling increment selects 'K', two selects 'M', etc. If no scaling
	increments are applied (the value is less than 1024) then no
	magnitude annotation will be appened to the value representation.

	@param	value	An unsigned long long integer value to be represented.
	@return	A QString with the scaled value representation and a possible
		magnitude annotation suffix.
*/
QString magnitude_of (unsigned long long value);

/**	Get the number of digits that would be used for the decimal
	representation of a value.

	@param	value	An unsigned long long integer value.
	@return	The number of digits that would be used for the decimal
		representation of the value.
*/
int decimal_digits (unsigned long long value);

/**	Get the number of digits that would be used for the hexadecimal
	representation of a value.

	@param	value	An unsigned long long integer value.
	@return	The number of digits that would be used for the hexadecimal
		representation of the value.
*/
int hex_digits (unsigned long long value);

/**	Add new-lines to a pathname as needed so it will fit within
	a specified display length.

	The pathname is split at pathname separators where line wrapping
	is applied.

	@param	pathnam	A QString containing a pathname.
	@param	wrap_length	The display length within which to wrap the
		pathname.
	@param	font	The QFont that will be used to display the
		pathname.
	@return	A QString with the possibly wrapped pathname that will
		display within the wrap length.
*/
QString wrapped_pathname (const QString& pathname, int wrap_length,
	const QFont& font);

/**	Get the fully qualified name for a QObject.

	A fully qualified QObject name is the QObject name prepended
	by the names of all its parent QObjects separated by the
	period ('.') character.

	@param	object	A pointer to a QObject.
	@return	A QString containing the fully qualified pathname for the
		object.
*/
inline QString object_pathname (const QObject* object)
{
QString
	pathname (object->objectName ());
while ((object = object->parent ()))
	pathname.prepend ('.').prepend (object->objectName ());
return pathname;
}

/*==============================================================================
	HiView_Utilities
*/
//!	HiView utility functions.
class HiView_Utilities
{
public:

//!	Class identification name with source code version and date.
static const char* const
	ID;

//!	JP2 metadata PDS label file URL parameter name.
static const char* const
	PDS_LABEL_URL_PARAMETER;

//!	Filename extension for a PDS label file associated with the JP2 file.
static const char* const
	PDS_LABEL_FILENAME_EXT;

/**	Test if a string provides a valid URL representation.

	A valid URL is of the form:

	<i>protocol</i><b>://<i>hostname</i>[<b>:</b><i>port</i>]<b>/</b><i>source</i>

	where the <i>protocol</i> is either "jpip" or "http" (case insensitive).

	The <i>hostname</i>, <i>port</i> and <i>source</i> are not checked
	to have valid values, however both the <i>hostname</i> and
	<i>source</i> fields must be present and the <i>port</i> field if
	present must have a numeric value.

	@param	URL	A URL string.
	@return	true if the URL appears to be valid; false otherwise.
	@see	UA::HiRISE::is_valid_URL(const std::string&, std::string*, int)
*/
static bool is_URL (const QString& URL);

/**	Tests if a string provides a valid JPIP URL representation.

	The string must be a {@link is_URL(const QString&) valid URL} and
	begin with the "jpip" protocol specification.

	@param	URL	A URL string.
	@return	true if the URL appears to be a valid JPIP URL; false otherwise.
	@see	UA::HiRISE::is_JPIP_URL(const std::string&)
*/
static bool is_JPIP_URL (const QString& URL);


//!	Band numbering is indexed (from 0), rather than counted (from 1) flag.
static bool
	Band_Numbering_Indexed;

inline static int band_index_to_number (int band)
	{return Band_Numbering_Indexed ? band : (band + 1);}
inline static int band_number_to_index (int band)
	{return Band_Numbering_Indexed ? band : (band - 1);}


static QString image_reader_formats_file_filters ();
static QStringList image_reader_formats ();
static QString image_writer_formats_file_filters ();
static QStringList image_writer_formats ();
static QString file_filters_from (QStringList& list);
static QString file_filter_for (const QString& format);

/**	Generate a URL for a potential source of PDS metadata from a model
	URL.

	If the model URL is empty or identical to a URL constructed from the
	image {@link source_name() source name} then the source name URL will
	be modified by replacing the filename extension with the {@link
	#PDS_LABEL_FILENAME_EXT}, or adding this extension if the source name
	does not have an extension.

	If the model URL does not specify a protocol scheme, or this is the
	"file" protocol, then the pathname of this URL is used to modify the
	pathname of the source name URL. An absolute pathname will replace
	the pathname of the source name URL; a relative pathname will replace
	the filename segment of the source name URL.

	If the model URL specifies a non-empty protocol scheme other than
	"file", the model URL replaces the entire source name URL.

	If the resulting URL specifies the "jpip" protocol scheme this is
	replaced with "http".

	@param	Source_Name	A QString which is the original path or URL to the image.
	@param	model_URL	A QUrl providing a model for the URL to be
		generated.
	@return	A QUrl for a potential source of PDS metadata associated with
		the source image.
*/
static QUrl PDS_metadata_URL (const QString& Source_Name, const QUrl& model_URL);


private:

static QString
	Image_Reader_Formats_File_Filters,
	Image_Writer_Formats_File_Filters;
static QStringList
	*Image_Reader_Formats,
	*Image_Writer_Formats;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
#endif
