/*	URL_Checker

HiROC CVS ID: $Id: URL_Checker.cc,v 1.8 2012/08/08 03:33:40 castalia Exp $

Copyright (C) 2011-2012  Arizona Board of Regents on behalf of the
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

#include	"URL_Checker.hh"

#include	<QNetworkAccessManager>
#include	<QNetworkReply>
#include	<QEventLoop>
#include	<QTimer>
#include	<QUrl>
#include	<QFileInfo>
#include	<QDir>
#include	<QMutex>
#include	<QMutexLocker>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_CHECK				(1 << 1)
#define	DEBUG_RUN				(1 << 2)

#define DEBUG_DEFAULT	DEBUG_ALL

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
	URL_Checker::ID =
		"UA::HiRISE::URL_Checker ($Revision: 1.8 $ $Date: 2012/08/08 03:33:40 $)";

/*==============================================================================
	Constructor
*/
URL_Checker::URL_Checker
	(
	QObject*	parent
	)
	:	QThread (parent),
		Network_Status (),
		Event_Loop (NULL),
		Timer (NULL)
{
setObjectName ("URL_Checker");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< URL_Checker" << endl;
#endif
}


URL_Checker::~URL_Checker ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~URL_Checker" << endl;
#endif
cancel ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
bool
	wait_completed =
#endif
wait (Default_Wait_Time);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    wait completed - " << boolalpha << wait_completed << endl
	 << "    Event_Loop @ " << (void*)Event_Loop << endl
	 << "    Timer @ " << (void*)Timer << endl;
#endif
if (Event_Loop)
	{
	delete Event_Loop;
	delete Timer;
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~URL_Checker" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
bool
URL_Checker::checking () const
{return isRunning () || request_status () == IN_PROGRESS;}

/*==============================================================================
	Run the thread
*/
bool
URL_Checker::check
	(
	const QUrl&	URL,
	bool		synchronous
	)
{
#if ((DEBUG_SECTION) & DEBUG_CHECK)
clog << ">>> URL_Checker::check: " << URL.toString () << endl
	 << "     URL scheme = " << URL.scheme () << endl
	 << "       URL path = " << URL.path () << endl
	 << "    synchronous = " << boolalpha << synchronous << endl;
#endif
int
	result = IN_PROGRESS;
if (! isRunning () &&
	! URL.isEmpty ())
	{
	Status_Lock->lock ();

	if (Request_Status == IN_PROGRESS)
		{
		#if ((DEBUG_SECTION) & DEBUG_FETCH)
		clog << "    Request_Status IN_PROGRESS!" << endl
			 << "<<< URL_Checker::check: false" << endl;
		#endif
		Status_Lock->unlock ();
		return false;
		}
	//	Reset the check state.
	Requested_URL = URL;
	Redirected_URL.clear ();
	Request_Status = result;
	HTTP_Status = NO_STATUS;
	HTTP_Status_Description.clear ();
	Status_Lock->unlock ();

	if (URL.scheme ().compare ("HTTP", Qt::CaseInsensitive) == 0 &&
		! URL.host ().isEmpty () &&
		! URL.path ().isEmpty ())
		{
		//	HTTP server check.
		#if ((DEBUG_SECTION) & DEBUG_CHECK)
		clog << "    URL_Checker::check: starting network request" << endl;
		#endif
		start ();

		if (synchronous)
			{
			#if ((DEBUG_SECTION) & DEBUG_CHECK)
			LOCKED_LOGGING ((
			clog << "    URL_Checker::check: waiting for thread to finish"
					<< endl));
			#endif
			wait ();	//	Wait for the thread to finish;
			result = Request_Status;
			synchronous = false;	//	No signal from here.
			}
		}
	else
	if ((URL.scheme ().compare ("FILE", Qt::CaseInsensitive) == 0 ||
		 URL.scheme ().isEmpty ()) &&
		! URL.path ().isEmpty ())
		{
		//	Local filesystem check.
		synchronous = true;
		#if ((DEBUG_SECTION) & DEBUG_CHECK)
		clog << "    filesystem check for " << URL.path () << endl;
		#endif
		QFileInfo
			file (QDir::toNativeSeparators (URL.path ()));
		if (file.exists ())
			{
			if (file.isReadable ())
				result = ACCESSIBLE_URL;
			else
				result = URL_ACCESS_DENIED;
			}
		else
			result = URL_NOT_FOUND;
		}
	else
		{
		synchronous = true;
		result = INVALID_URL;
		}

	if (synchronous)
		{
		//	Set the request status.
		request_status (result);

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & DEBUG_CHECK)
		LOCKED_LOGGING ((
		clog << "    URL_Checker::check: emit checked "
				<< boolalpha << (result == ACCESSIBLE_URL) << endl));
		#endif
		emit checked (result == ACCESSIBLE_URL);
		}
	}

#if ((DEBUG_SECTION) & DEBUG_CHECK)
LOCKED_LOGGING ((
clog << "    URL_Checker::check: result = " << result
		<< " - " << status_description (result) << endl
	 << "<<< URL_Checker::check: "
		<< boolalpha << (result == ACCESSIBLE_URL) << endl));
#endif
return result == ACCESSIBLE_URL;
}


void
URL_Checker::run ()
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << ">>> URL_Checker::run" << endl));
#endif
Status_Lock->lock ();
if (! Event_Loop)
	Event_Loop = new QEventLoop;
if (! Timer)
	{
	Timer = new QTimer;
	Timer->setSingleShot (true);
	connect
		(Timer,			SIGNAL (timeout ()),
		 Event_Loop,	SLOT (quit ()));
	}
if (! Network_Access_Manager)
	/*
		The QNetworkAccessManager must be constructed on the same thread
		where it is used with its QNetworkReply.
	*/
	Network_Access_Manager = new QNetworkAccessManager;
Network_Reply = Network_Access_Manager->head (QNetworkRequest (Requested_URL));
connect
	(Network_Reply,	SIGNAL (finished ()),
	 Event_Loop,	SLOT (quit ()));
Status_Lock->unlock ();

//	Wait in a local event loop on network reply for data or finished.
Timer->start (Wait_Time);
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "    starting event loop" << endl;
#endif
Event_Loop->exec ();

//	Event occured.
Status_Lock->lock ();
if (Timer->isActive ())
	{
	Timer->stop ();
	reset (*Network_Reply);	
	}
else
	{
	//	Timeout.
	Network_Reply->abort ();
	request_status (SYNCHRONOUS_TIMEOUT);
	}
delete Network_Reply;
Network_Reply = NULL;
int
	result = Request_Status;
Status_Lock->unlock ();

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & DEBUG_CHECK)
LOCKED_LOGGING ((
clog << "    URL_Checker::run: emit checked "
		<< boolalpha << (result == ACCESSIBLE_URL) << endl));
#endif
emit checked (result == ACCESSIBLE_URL);

#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "<<< URL_Checker::run" << endl));
#endif
}


}	//	namespace HiRISE
}	//	namespace UA
