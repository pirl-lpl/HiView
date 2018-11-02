/*	Help_Docs

HiROC CVS ID: $Id: Help_Docs.cc,v 1.10 2013/09/24 19:05:42 guym Exp $

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

#include	"Help_Docs.hh"

#include	"URL_Checker.hh"

#include	<QUrl>
#include	<QDir>
#include	<QDesktopServices>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_MANIPULATORS		(1 << 2)
#define	DEBUG_UTILITY			(1 << 3)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Help_Docs::ID =
		"UA::HiRISE::Help_Docs ($Revision: 1.10 $ $Date: 2013/09/24 19:05:42 $)";

/*==============================================================================
	Statics
*/
#ifndef MAX_URL_REDIRECTION
#define MAX_URL_REDIRECTION	5
#endif
int
	Help_Docs::Max_URL_Redirection = MAX_URL_REDIRECTION;

URL_Checker
	*Help_Docs::Checker;

/*==============================================================================
	Constructor
*/
Help_Docs::Help_Docs
	(
	const QString&	docs_location
	)
	:	Docs_Location_is_File (true)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Help_Docs::Help_Docs: \"" << docs_location << '"' << endl;
#endif
Checker = new URL_Checker ();
location (docs_location);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	 clog << "<<< Help_Docs::Help_Docs" << endl;
#endif
}


Help_Docs::~Help_Docs ()
{delete Checker;}

/*==============================================================================
	Accessors
*/
bool
Help_Docs::location
	(
	const QString&	docs_location
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">>> Help_Docs::location: \"" << docs_location << '"' << endl;
#endif
bool
	OK = false;
if (docs_location == Docs_Location)
	OK = true;
else
if (docs_location.isEmpty ())
	{
	Docs_Location_is_File = true;
	Docs_Location.clear ();
	OK = true;
	}
else
	{
	QUrl
		URL (Checker->normalized_URL (docs_location));
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	clog << "    URL = " << URL.toString () << endl;
	#endif
	if ((OK = is_accessible (URL)))
		{
		Docs_Location_is_File =
			(URL.scheme ().compare ("FILE", Qt::CaseInsensitive) == 0);
		if (Docs_Location_is_File)
			Docs_Location = URL.path ();
		else
			Docs_Location = URL.toString ();
		#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
		clog << "    Docs_Location = " << Docs_Location << endl
			 << "    location is a "
				<< (Docs_Location_is_File ? "file" : "URL") << endl;
		#endif
		}
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "<<< Help_Docs::location: " << boolalpha << OK << endl;
#endif
return OK;
}

/*==============================================================================
	Manipulators
*/
bool
Help_Docs::help
	(
	const QString&	document
	)
{
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << ">>> Help_Docs::help: \"" << document << '"' << endl
	 << "    Docs_Location = \"" << Docs_Location << '"' << endl;
#endif
bool
	OK = false;
if (! Docs_Location.isEmpty ())
	{
	QString
		location (Docs_Location);
	location += QDir::separator ();
	location += document;

	QUrl
		URL (Checker->normalized_URL (location));
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	clog << "    checking URL " << URL.toString () << endl;
	#endif
	if (is_accessible (URL))
		{
		#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
		clog << "    QDesktopServices::openUrl" << endl;
		#endif
		OK = QDesktopServices::openUrl (URL);
		}
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	else
		clog << "    URL is not accessible" << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "<<< Help_Docs::help: " << boolalpha << OK << endl;
#endif
return OK;
}

/*==============================================================================
	Utility
*/
QString
Help_Docs::find
	(
	const QString& document, const QStringList& locations
	)
{
#if ((DEBUG_SECTION) & DEBUG_UTILITY)
clog << ">>> Help_Docs::find: \"" << document << '"' << endl;
#endif
QString
	location;
QUrl
	URL;
#if ((DEBUG_SECTION) & DEBUG_UTILITY)
clog << "    searching locations -" << endl;
#endif
int
	index = -1,
	amount = locations.size ();
if (amount == 0)
	amount = 1;	//	Allow for no list, just document.
while (++index < amount)
	{
	if (! locations.isEmpty ())
		location = locations.at (index);
	if (! document.isEmpty ())
		{
		if (! location.isEmpty ())
			location += QDir::separator ();
		location += document;
		}
	#if ((DEBUG_SECTION) & DEBUG_UTILITY)
	clog << "      " << index << ": " << location << endl;
	#endif
	if (location.isEmpty ())
		continue;

	URL = Checker->normalized_URL (location);
	#if ((DEBUG_SECTION) & DEBUG_UTILITY)
	clog << "      checking " << URL.toString () << endl;
	#endif
	if (is_accessible (URL))
		{
		if (URL.scheme ().compare ("FILE", Qt::CaseInsensitive) == 0)
			location = URL.path ();
		else
			location = URL.toString ();
		if (location.size () != document.size ())
			//	Remove the document from the location.
			location.truncate (location.size () - document.size () - 1);
		break;
		}
	}
if (index == amount)
	//	Document not found.
	location.clear ();

#if ((DEBUG_SECTION) & DEBUG_UTILITY)
clog << "<<< Help_Docs::find: " << location << endl;
#endif
return location;
}


bool
Help_Docs::is_accessible
	(
	const QString&	location
	)
{
/*	MS/Windows gotcha:

	A filesystem pathname on MS/Windows that includes a drive
	specification - e.g. "D:" - will be interpreted as having a URL
	scheme specification that is the drive letter when a QUrl is
	constructed from such a pathname string. The
	URL_Checker::normalized_URL method works around this problem by only
	recognizing HTTP, FTP, or JPIP protocols; anything else is assumed to
	be a filesystem pathname.
*/
return is_accessible (Checker->normalized_URL (location));
}


bool
Help_Docs::is_accessible
	(
	const QUrl&	URL
	)
{
bool
	accessible = false;
QUrl
	url (URL);
int
	count = Max_URL_Redirection;
while (count-- &&
		! url.isEmpty ())
	{
	accessible = Checker->check (url);
	url = Checker->redirected_URL ();
	}
if (count < 0)
	//	Too many redirects.
	accessible = false;
return accessible;
}


}	//	namespace HiRISE
}	//	namespace UA
