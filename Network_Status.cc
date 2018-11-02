/*	Network_Status

HiROC CVS ID: $Id: Network_Status.cc,v 1.3 2013/06/13 16:27:32 guym Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/

#include	"Network_Status.hh"

#include	<QApplication>
#include	<QMutex>
#include	<QMutexLocker>
#include	<QNetworkAccessManager>
#include	<QNetworkReply>
#include	<QUrl>
#include	<QFileInfo>
#include	<QDir>
#include	<QObject>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define	DEBUG_ACCESSORS			(1 << 1)
#define	DEBUG_UTILITIES			(1 << 2)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#else
#include	"HiView_Utilities.hh"
#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*=*****************************************************************************
	Network_Status
*/
/*==============================================================================
	Constants
*/
const char* const
	Network_Status::ID =
		"UA::HiRISE::Network_Status ($Revision: 1.3 $ $Date: 2013/06/13 16:27:32 $)";


const int
	//	Local status codes:
	Network_Status::NO_STATUS			= -1,
	Network_Status::INVALID_URL			= -2,
	Network_Status::IN_PROGRESS			= -3,
	Network_Status::SYNCHRONOUS_TIMEOUT	= -4,

	//	Aliases:
	Network_Status::ACCESSIBLE_URL
		= QNetworkReply::NoError,
	Network_Status::URL_NOT_FOUND
		= QNetworkReply::ContentNotFoundError,
	Network_Status::URL_ACCESS_DENIED
		= QNetworkReply::ContentAccessDenied;

/*==============================================================================
	Static Data
*/
#ifndef DEFAULT_NETWORK_WAIT_TIME
#define DEFAULT_NETWORK_WAIT_TIME		30000
#endif
unsigned long
	Network_Status::Default_Wait_Time	= DEFAULT_NETWORK_WAIT_TIME;

QString
	Network_Status::Application_Location;

/*==============================================================================
	Constructor
*/
Network_Status::Network_Status ()
	:
	Requested_URL (),
	Redirected_URL (),
	Request_Status (NO_STATUS),
	HTTP_Status (NO_STATUS),
	HTTP_Status_Description (),
	Wait_Time (Default_Wait_Time),
	Status_Lock (new QMutex (QMutex::Recursive)),
	//	N.B.: The Network_Access_Manager must be constructed on the thread.
	Network_Access_Manager (NULL),
	Network_Reply (NULL)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< Network_Status" << endl;
#endif
}


Network_Status::~Network_Status ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~Network_Status" << endl;
#endif
if (Network_Reply)
	delete Network_Reply;
if (Network_Access_Manager)
	delete Network_Access_Manager;
delete Status_Lock;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~Network_Status" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
bool
Network_Status::reset ()
{
QMutexLocker
	lock (Status_Lock);
if (Request_Status != IN_PROGRESS)
	reset_state ();
return Request_Status != IN_PROGRESS;
}


void
Network_Status::reset_state ()
{
Request_Status = NO_STATUS;
HTTP_Status = NO_STATUS;
Requested_URL.clear ();
Redirected_URL.clear ();
HTTP_Status_Description.clear ();
}


bool
Network_Status::reset
	(
	const QNetworkReply&	network_reply
	)
{
QMutexLocker
	lock (Status_Lock);
if (Request_Status != IN_PROGRESS)
	reset_state (network_reply);
return Request_Status != IN_PROGRESS;
}


void
Network_Status::reset_state
	(
	const QNetworkReply&	network_reply
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">>> Network_Status::reset: QNetworkReply @ "
		<< (void*)(&network_reply) << endl
	 << "    Requested_URL = " << Requested_URL.toString () << endl
	 << "        reply URL = " << network_reply.url ().toString ()));
#endif
//	Request status.
Request_Status = network_reply.error ();
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "    Request_Status = " << Request_Status << " - "
		<< status_description (Request_Status) << endl));
#endif

//	Attributes.
QVariant
	value;
value = network_reply.attribute (QNetworkRequest::HttpStatusCodeAttribute);
if (value.isValid ())
	HTTP_Status = value.toInt ();
else
	HTTP_Status = NO_STATUS;
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "    HTTP_Status = " << HTTP_Status << endl));
#endif

value = network_reply.attribute (QNetworkRequest::HttpReasonPhraseAttribute);
if (value.isValid ())
	{
	HTTP_Status_Description = value.toByteArray ();
	}
else
	HTTP_Status_Description.clear ();
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "    HTTP_Status_Description = " << HTTP_Status_Description << endl));
#endif

value = network_reply.attribute (QNetworkRequest::RedirectionTargetAttribute);
if (value.isValid ())
	{
	Redirected_URL = Requested_URL.resolved (value.toUrl ());
	}
else
	Redirected_URL.clear ();
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "    Redirected_URL = " << Redirected_URL.toString () << endl));
#endif
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "<<< Network_Status::reset" << endl));
#endif
}


int
Network_Status::request_status () const
{
QMutexLocker
	lock (Status_Lock);
return Request_Status;
}


void
Network_Status::request_status
	(
	int		status
	)
{
QMutexLocker
	lock (Status_Lock);
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">-< Network_Status::request_status: " << status << endl));
#endif
Request_Status = status;
}


int
Network_Status::HTTP_status () const
{
QMutexLocker
	lock (Status_Lock);
return HTTP_Status;
}


QString
Network_Status::HTTP_status_description () const
{
QMutexLocker
	lock (Status_Lock);
return HTTP_Status_Description;
}


QUrl
Network_Status::requested_URL () const
{
QMutexLocker
	lock (Status_Lock);
return Requested_URL;
}


void
Network_Status::requested_URL
	(
	const QUrl&	URL
	)
{
QMutexLocker
	lock (Status_Lock);
Requested_URL = URL;
}


QUrl
Network_Status::redirected_URL () const
{
QMutexLocker
	lock (Status_Lock);
return Redirected_URL;
}


unsigned long
Network_Status::wait_time () const
{
QMutexLocker
	lock (Status_Lock);
return Wait_Time;
}


void
Network_Status::wait_time
	(
	unsigned long	msecs
	)
{
QMutexLocker
	lock (Status_Lock);
if (msecs == 0)
	msecs = ULONG_MAX;
Wait_Time = msecs;
}


void
Network_Status::cancel ()
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">>> Network_Status::cancel" << endl));
#endif
QMutexLocker
	lock (Status_Lock);
if (Network_Reply)
	{
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	LOCKED_LOGGING ((
	clog << "    Network_Reply abort" << endl));
	#endif
	Network_Reply->abort ();
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "<<< Network_Status::cancel" << endl));
#endif
}

/*==============================================================================
	Utility
*/
QUrl
Network_Status::normalized_URL
	(
	const QUrl&	URL
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">>> Network_Status::normalized_URL: " << URL.toString () << endl));
#endif
QUrl
	normalized (URL);
if (URL.scheme ().isEmpty () ||
	URL.scheme ().compare ("file", Qt::CaseInsensitive) == 0)
	{
	normalized.setScheme ("file");
	QString
		path (URL.path ());
	if (QFileInfo (path).isRelative ())
		normalized.setPath (Application_Location + QDir::separator ()
				+ QDir::toNativeSeparators (path));
	else
		normalized.setPath (QDir::toNativeSeparators (path));
	}
else
	normalized.setPath (QDir::fromNativeSeparators (URL.path ()));
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "<<< Network_Status::normalized_URL: "
		<< normalized.toString () << endl));
#endif
return normalized;
}


QUrl
Network_Status::normalized_URL
	(
	const QString&	source
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">>> Network_Status::normalized_URL: " << source << endl));
#endif
QUrl
	normalized (source);
QString
	protocol (normalized.scheme ());
if (protocol.compare ("HTTP", Qt::CaseInsensitive) == 0 ||
	protocol.compare ("JPIP", Qt::CaseInsensitive) == 0 ||
	protocol.compare ("FTP",  Qt::CaseInsensitive) == 0)
	normalized.setPath (QDir::fromNativeSeparators (normalized.path ()));
else
	{
	normalized.setScheme ("file");
	if (QFileInfo (source).isRelative ())
		normalized.setPath (Application_Location + QDir::separator ()
				+ QDir::toNativeSeparators (source));
	else
		normalized.setPath (QDir::toNativeSeparators (source));
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "<<< Network_Status::normalized_URL: "
		<< normalized.toString () << endl));
#endif
return normalized;
}


QString
Network_Status::status_description
	(
	int		code
	)
{
QString
	description;
switch (code)
	{
	//	Local (negative) status code values.
	case NO_STATUS:
		description =
			QObject::tr ("No status"); break;
	case INVALID_URL:
		description = 
			QObject::tr ("Invalid URL"); break;
	case IN_PROGRESS:
		description = 
			QObject::tr ("Operation in progress"); break;
	case SYNCHRONOUS_TIMEOUT:
		description = 
			QObject::tr ("Synchronous wait timeout"); break;

	case QNetworkReply::NoError:	//	ACCESSIBLE_URL:
		description = 
			QObject::tr ("Accessible URL"); break;
	case QNetworkReply::ConnectionRefusedError:
		description = 
			QObject::tr ("Connection refused"); break;
	case QNetworkReply::RemoteHostClosedError:
		description = 
			QObject::tr ("Server closed the connection prematurely"); break;
	case QNetworkReply::HostNotFoundError:
		description = 
			QObject::tr ("Hostname not found"); break;
	case QNetworkReply::TimeoutError:
		description = 
			QObject::tr ("Server connection timeout"); break;
	case QNetworkReply::OperationCanceledError:
		description = 
			QObject::tr ("Operation canceled"); break;
	case QNetworkReply::SslHandshakeFailedError:
		description = 
			QObject::tr ("SSL/TLS handshake failed"); break;
	#if QT_VERSION >= 0x40700
	case QNetworkReply::TemporaryNetworkFailureError:
		description = 
			QObject::tr ("Broken connection for access point roaming"); break;
	#endif
	case QNetworkReply::ProxyConnectionRefusedError:
		description = 
			QObject::tr ("Proxy server connection refused"); break;
	case QNetworkReply::ProxyConnectionClosedError:
		description = 
			QObject::tr ("Proxy server connection closed prematurely"); break;
	case QNetworkReply::ProxyNotFoundError:
		description = 
			QObject::tr ("Proxy server hostname not found"); break;
	case QNetworkReply::ProxyTimeoutError:
		description = 
			QObject::tr ("Proxy server connection timeout"); break;
	case QNetworkReply::ProxyAuthenticationRequiredError:
		description = 
			QObject::tr ("Proxy server authentication failed"); break;
	case QNetworkReply::ContentAccessDenied:	//	URL_ACCESS_DENIED
		description = 
			QObject::tr ("Content access denied"); break;
	case QNetworkReply::ContentOperationNotPermittedError:
		description = 
			QObject::tr ("Operation not permitted"); break;
	case QNetworkReply::ContentNotFoundError:	//	URL_NOT_FOUND
		description = 
			QObject::tr ("Content not found"); break;
	case QNetworkReply::AuthenticationRequiredError:
		description = 
			QObject::tr ("Server authentication failed"); break;
	case QNetworkReply::ContentReSendError:
		description = 
			QObject::tr ("Request resent failed"); break;
	case QNetworkReply::ProtocolUnknownError:
		description = 
			QObject::tr ("Unknown protocol"); break;
	case QNetworkReply::ProtocolInvalidOperationError:
		description = 
			QObject::tr ("Invalid operation for protocol"); break;
	case QNetworkReply::UnknownNetworkError:
		description = 
			QObject::tr ("Unknown network error"); break;
	case QNetworkReply::UnknownProxyError:
		description = 
			QObject::tr ("Unknown proxy error"); break;
	case QNetworkReply::UnknownContentError:
		description = 
			QObject::tr ("Unknown content error"); break;
	case QNetworkReply::ProtocolFailure:
		description = 
			QObject::tr ("Protocol failure"); break;
	default:
		description = 
			QObject::tr ("Unknown status code (%1)").arg (code); break;
	}
return description;
}


QString
Network_Status::application_location ()
{
if (Application_Location.isEmpty ())
	Application_Location =
		QDir::toNativeSeparators (qApp->applicationDirPath ());
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">-< Network_Status::application_location: "
		<< Application_Location << endl;
#endif
return Application_Location;
}


}	//	namespace HiRISE
}	//	namespace UA
