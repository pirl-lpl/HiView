/*	HiView_Application

HiROC CVS ID: $Id: HiView_Application.hh,v 1.5 2014/05/22 19:45:23 guym Exp $

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

#ifndef HiView_Application_hh
#define HiView_Application_hh

#include	<QApplication>

//	Meta-command filename suffix.
#ifndef JPIP_PASSTHRU_LINK_SUFFIX
#define JPIP_PASSTHRU_LINK_EXT	".jpl"
#else
#define JPIP_PASSTHRU_LINK_EXT	AS_STRING(JPIP_PASSTHRU_LINK_SUFFIX)
#endif

//	Meta-command maximum command line size.
#ifndef MAX_JPIP_PASSTHRU_LINK_SIZE
#define MAX_JPIP_PASSTHRU_LINK_SIZE		4095
#endif


namespace UA
{
namespace HiRISE
{

/**	The <i>HiView_Application</i> extends the QApplication with HiView
	specific capabilities.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.5 $
*/
class HiView_Application
:	public QApplication
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Constructor
*/
HiView_Application (int& argc, char** argv);

/*==============================================================================
	Utilities
*/
/** Determines if an input source pathname is a Jpip Pass-thru Link.

    @param A QString representing a file pathname
    @return true if the input looks like a Jpip Pass-thru Link.
*/
static bool
is_jpip_passthru_link
	(
	const QString& input
	);

/** Parses an input source pathname to retrieve the Jpip Pass-thru Link.

    @param A QString representing a file pathname
    @return The Jpip Pass-thru Link, or empty string if the parse fails.
*/
static QString
parse_jpip_passthru_link
	(
	const QString& input
	);
	
/*==============================================================================
	Qt events:
*/
/**	Overrides the application's QObject::event method to watch for
	QEvent::FileOpen events.

	When a QEvent::FileOpen is detected its file pathname content is
	{@link file_open_request(constQString&) signalled}.

	@param	A QEvent delivered to the application.
	@return	true if a QEvent::FileOpen event was detected and signalled, or
		whatever the base QApplication::event method returns.
*/
#ifdef DEBUG_SECTION
bool notify(QObject* receiver, QEvent* event) ;
#endif

bool event (QEvent* event);


/*==============================================================================
	Qt signals:
*/
signals:

/**	Signals the occurance of a QEvent::FileOpen {@link event(QEvent*) event}.

	@param	The file pathname associated with the QFileOpenEvent.
	@see	event(QEvent*)
*/
void file_open_request (const QString& pathname);


public:

QString
	Requested_Pathname;

};

class NullEventFilter : public QObject 
{
    Q_OBJECT
    
protected:
inline bool eventFilter(QObject*, QEvent*)
{
    return true;
}
};

}	//	namespace HiRISE
}	//	namespace UA
#endif
