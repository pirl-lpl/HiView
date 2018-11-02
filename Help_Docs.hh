/*	Help_Docs

HiROC CVS ID: $Id: Help_Docs.hh,v 1.5 2012/05/22 03:47:03 castalia Exp $

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

#ifndef HiView_Help_Docs_hh
#define HiView_Help_Docs_hh

#include	<QString>
//	Forward references.
class QUrl;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class URL_Checker;

class Help_Docs
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	The maximum number of URL redirections to follow.
static int
	Max_URL_Redirection;

/*==============================================================================
	Constructor
*/
/**	Construct a Help_Docs object.

	@param	docs_location	The {@link location(const QString&) location}
		of the help documentation files. If this is the empty string then
		no {@link help(const QString&) help document} can be opened until
		a valid location is provided.
*/
explicit Help_Docs (const QString& docs_location = QString ());

//!	Help_Docs destructor.
virtual ~Help_Docs ();

/*==============================================================================
	Accessors
*/
/**	Get the location of the help documentation files.

	@return	A QString containing the location of the documentation files.
*/
inline QString location () const
	{return Docs_Location;}

/**	Set the location of the documentation files.

	If the location is empty no {@link help(const QString&) help
	document} can be opened until a valid location is provided.

	If the location is an HTTP URL it is checked that it refers to an
	accessible source.

	If the location is a file pathname it is checked if it refers to a
	readable directory. A relative file pathname is relative to the
	application's runtime location.

	@param	docs_location	The location of the help documentation files.
	@return	true if the documenation location is valid; false otherwise.
		A empty location is valid.
*/
bool location (const QString& docs_location);

/**	Test if the help documentation {@link location(const
	QString&) location} is for local files.

	@return	true if the location refers to a local file directory
		pathname; false if the location is an HTTP URL. An empty
		location is considered to be for a local file.
*/
inline bool location_is_File () const
	{return Docs_Location_is_File;}

/*==============================================================================
	Manipulators
*/
/**	Open a document in the desktop services web browser.

	If the {@link location() location} of the help documents is empty
	nothing is done.

	The document filename is appended to the location and then used as
	the pathname to construct a URL. If the URL refers to an accessible
	file the URL is used to open the document in a
	web browser using QDesktopServices.

	@param	document	A QString specifying the help document to be
		displayed by QDesktopServices. The document name is a file
		pathname relative to the {@link location() location} of the help
		documents.
	@return	true if the help document was displayed; false otherwise.
*/
bool help (const QString& document);

/*==============================================================================
	Utility
*/
/**	Find a document in a list of possible locations.

	The entries in the locations list are checked for the document. The
	location with the first successful check is returned.

	Each locations list entry that is checked is appended with a pathname
	separator and the document pathname; i.e. the document pathname is
	relative to the location. A location may be an HTTP or "file" URL or
	a local filesystem pathname. A relative filesystem pathname location
	is relative to the application's runtime directory which is prepended
	to the location. The first fully qualified document location that
	successfully refers to an accessible file is returned. Note that if
	the document name is the empty string the first accessible locations
	entry is returned; and if the locations list is empty only the
	document pathname is checked. If the document is not accessbile in
	any location an empty location is returned.

	@param	document	A QString specifying a document file pathname.
		The pathname is expected to be relative to one of the locations.
		The emtpy string may be used to find the first accessible locations
		list entry.
	@param	locations	A QStringList of possible document locations. Each
		entry may be an HTTP or "file" URL or a local absolute or relative
		filesystem pathname.
	@return	A QString containing the first fully qualified location that
		refers to an accessible file; or the empty string if the no
		accessible document is found in all the locations. <b>N.B.</b>:
		The location returned will be normalized with a filesystem
		pathname location having native separators ('/' for Unix, '\' for
		MS/Windows) and an HTTP URL path having standard network
		separators ('/').
*/
static QString find (const QString& document, const QStringList& locations);

/**	Test if a source refers to an accessible file.

	<b>N.B.</b>: The source is {@link
	URL_Checker::normalized_URL(const QString&) normalized} before access
	is checked.

	@param	source	A file source URL or filesystem pathname.
	@return	true if the source refers to an accessible file; false
		otherwise.
	@see	is_accessible(const QUrl&);
*/
static bool is_accessible (const QString& source);

/**	Test if a URL refers to an accessible file.

	<b>N.B.</b>: The URL is NOT {@link
	URL_Checker::normalized_URL(const QString&) normalized} before access
	is checked.

	<b>>>> WARNING <<<</b> A filesystem pathname on MS/Windows that
	includes a drive specification - e.g. "D:" - will be interpreted as
	having a URL scheme specification that is the drive letter when a
	QUrl is constructed from such a pathname string. For this reason it
	is advisable that a {@link URL_Checker::normalized_URL(const QString&)
	normalized} QUrl be used here.

	URL redirection is followed up to {@link #Max_URL_Redirection} times.

	@param	URL	A QUrl.
	@return	true if the source refers to an accessible file; false otherwise.
	@see	is_accessible(const QString&);
*/
static bool is_accessible (const QUrl& URL);

/*==============================================================================
	Data
*/
private:

QString
	Docs_Location;
bool
	Docs_Location_is_File;

static URL_Checker
	*Checker;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
