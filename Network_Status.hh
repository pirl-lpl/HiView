/*	Network_Status

HiROC CVS ID: $Id: Network_Status.hh,v 1.3 2012/08/22 21:36:10 castalia Exp $

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

#ifndef HiView_Network_Status_hh
#define HiView_Network_Status_hh

#include	<QUrl>
#include	<QString>
class QMutex;
class QNetworkAccessManager;
class QNetworkReply;

#include	<limits.h>	//	For ULONG_MAX


namespace UA
{
namespace HiRISE
{
/**	The <i>Network_Status</i> class provides simple network utilities
	for use by other classes.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.3 $
*/
class Network_Status
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Local status code: No status has been set.
static const int
	NO_STATUS;

//! Local status code: The specified URL is invalid.
static const int
	INVALID_URL;

//!	Local status code: A network request is in progress.
static const int
	IN_PROGRESS;

//! Local status code: Timeout of the opertion.
static const int
	SYNCHRONOUS_TIMEOUT;

/**	The network status code for an accessible URL.

	Equivalent to QNetworkReply::NoError
*/
static const int
	ACCESSIBLE_URL;

/**	The network status code when a URL is not found.

	Equivalent to QNetworkReply::ContentNotFoundError
*/
static const int
	URL_NOT_FOUND;

/**	The network status code for a URL to which access is denied.

	Equivalent to QNetworkReply::ContentAccessDenied
*/
static const int
	URL_ACCESS_DENIED;

/*==============================================================================
	Static Data
*/
protected:

//!	The default network request (@link wait_time() wait_time}.
static unsigned long
	Default_Wait_Time;

//!	The application executable runtime directory location.
static QString
	Application_Location;

/*==============================================================================
	Constructor
*/
public:

//!	Constructs the Network_Status with default status values.
Network_Status ();

//!	Destructor.
~Network_Status ();

/*==============================================================================
	Accessors
*/
/**	Reset the network request status values to their unset state.

	If the {@link request_status() request_status} value is {@link
	#IN_PROGRESS} nothing is done.

	The {@link request_status() request status} and {@link HTTP_status()
	HTTP status} values are set to {@link #NO_STATUS}. The {@link
	HTTP_status_description() HTTP status description}, {@link
	requested_URL() requested URL} and {@link redirected_URL() redirected
	URL} are cleared.

	@return	true if the network request status values were reset to their
		unset state; false if a network request is in progress and the
		values are unchanged.
*/
bool reset ();

/**	Reset the network request status values based on a QNetworkReply state.

	If the {@link request_status() request_status} value is {@link
	#IN_PROGRESS} nothing is done.

	The {@link request_status() request status} is set to the network
	reply error code. The {@link HTTP_status() HTTP status} is set to the
	network reply QNetworkRequest::HttpStatusCodeAttribute value, or
	{@link #NO_STATUS} if the attribute is not present. The {@link
	HTTP_status_description() HTTP status description} is set to the
	network reply QNetworkRequest::HttpReasonPhraseAttribute value, or
	cleared if the attribute is not present. The {@link redirected_URL()
	redirected URL} is set to the network reply
	QNetworkRequest::RedirectionTargetAttribute value, or cleared if the
	attribute is not present.  The {@link requested_URL() requested URL}
	is unchanged (it can be {@link requested_URL(const QUrl&) set} by
	subclasses).

	@param	network_reply	A reference to a QNetworkReply.
	@return	true if the network request status values were reset; false
		if a network request is in progress and the values are unchanged.
*/
bool reset (const QNetworkReply& network_reply);

/**	Get the network request status.

	@return	A QNetworkReply::NetworkError value. In addition, the value
		may be:

		<dl>
		<dt>{@link #NO_STATUS}
		<dd>If no request has been submitted since the last {@link
			reset() reset} to the default status values.
		<dt>{@link #INVALID_URL}
		<dd>If the URL submitted for the network request was found to be
			invalid.
		<dt>{@link #IN_PROGRESS}
		<dd>If a network request is currently in progress.
		<dt>{@link #SYNCHRONOUS_TIMEOUT}
		<dd>If a network request did not complete in within the {@link
			wait_time(unsignedlong) wait time}.
		</dl>
*/
int request_status () const;

/**	Set the network request status.

	This method is typically only used by subclasses that control the
	network request process.

	@param	status	A network request status code.
*/
void request_status (int status);

/**	Get the HTTP status code for the last network request.

	@return	The HTTP status code for the last network request URL. This
		will be {@link #NO_STATUS} if the network status values have been
		{@link reset() reset} or no HTTP status was available for the
		last request.
	@see reset(const QNetworkReply&)
*/
int HTTP_status () const;

/**	Get the HTTP status description provided by the server for the last
	network request.

	@return	A QString containing the HTTP status description provided by
		the last network request. This will be an
		empty string if the network status values have been
		{@link reset() reset} or no HTTP status description was available
		for last request.
	@see reset(const QNetworkReply&)
*/
QString HTTP_status_description () const;

/**	Get the requested URL.

	@return	The URL used by the last network request. This will be empty
		if the network status values have been {@link reset() reset} or
		the value was forcibly cleared.
	@see requested_URL (const QUrl&)
*/
QUrl requested_URL () const;

/**	Set the requested URL.

	This method is typically only used by subclasses that control the
	network request process.

	@param	URL	A QUrl reference that will be used to set the
		{@link requested_URL() requested URL}.
*/
void requested_URL (const QUrl& URL);

/**	Get the URL to which the last network request was redirected by the
	server.

	<b>N.B.</b>: A network request does not follow URL redirection; this
	is the responsibility of the application. If the netowrk request was
	not redirected then the redirected URL is empty; otherwise the
	network request can be repeated using the provided URL to follow the
	redirection.

	@return	A QUrl containing the URL to which the last network request
		was redirected. This will be empty if no redirection was done by
		the server.
	@see reset(const QNetworkReply&)
*/
QUrl redirected_URL () const;

/**	Set the default amount of time to wait for a network request to
	complete.

	New objects will use this time; existing objects will not be affected.

	@param	msecs	The time to wait, in milliseconds. A zero or negative
		value will be set to the maximum wait time, i.e. effectively no
		timeout.
	@see wait_time(unsigned long)
*/
inline static void default_wait_time (unsigned long msecs)
{Default_Wait_Time = (msecs ? msecs : ULONG_MAX);}

/**	Get the default amount of time to wait for a network request to
	complete.

	@return	The time to wait, in milliseconds.
	@see default_wait_time (unsigned long)
*/
inline static unsigned long default_wait_time ()
{return Default_Wait_Time;}

/**	Set the amount of time to wait for a network request to complete.

	The wait time is used for network data delivery and for synchronous
	network request timeout.

	@param	msecs	The time to wait, in milliseconds. A zero or negative
		value will be set to the maximum wait time, i.e. effectively no
		timeout.
	@see wait_time()
	@see default_wait_time (unsigned long)
*/
void wait_time (unsigned long msecs);

/**	Get the amount of time to wait for a network request to complete.

	@return	The time to wait, in milliseconds.
	@see	wait_time(unsigned long)
*/
unsigned long wait_time () const;

/**	Cancel a network request in progress.

	It is safe to cancel when no request is in progress.
*/
void cancel ();


protected:

/**	Unconditionally reset the network request status values.

	<b.N.B.</b>: Access to the status values is not locked; it
	is assumed that the subclass will lock the Status_Lock.

	@see reset()
*/
void reset_state ();

/**	Unconditionally reset the network request status values from
	a QNetworkReply.

	<b.N.B.</b>: Access to the status values is not locked; it
	is assumed that the subclass will lock the Status_Lock.

	@see reset(const QNetworkReply&)
*/
void reset_state (const QNetworkReply& network_reply);

/*==============================================================================
	Utilities
*/
public:

/**	Get a normalized, host independent, form of a URL.

	If the URL does not specify a protocol it is coerced to be a "file"
	protocol. A source with a "file" protocol has the path segment
	coverted to use the host's native filesystem separators. On Unix
	systems the separater is the forward slash ('/'); on MS/Windows
	systems the separater is the back slash ('\').

	For any other URL protocol the path segment of the URL is converted
	to use the network standard forward slash ('/') separators.

	@param	URL	The QUrl to be normalized.
	@return	The normalized URL.
	@see	normalized_URL(const QString&)
*/
static QUrl normalized_URL (const QUrl& URL);

/**	Get a normalized, host independent, URL for a file source.

	If the source does not specify an HTTP, FTP, or JPIP URL protocol it
	is taken to be a pathname for the local filesystem. A relative
	pathname is prepended with the application's runtime directory
	location. The pathname is coverted to use the host's native
	filesystem separators. On Unix systems the separater is the forward
	slash ('/'); on MS/Windows systems the separater is the back slash
	('\'). A URL with the "file" protocol and pathname path segment is
	returned. 

	For any other protocol specified by the source the pathname portion
	of the URL is converted to use the network standard forward slash
	('/') separators.

	<b>>>> WARNING <<<</b> A filesystem pathname on MS/Windows that
	includes a drive specification - e.g. "D:" - will be interpreted as
	having a URL scheme specification that is the drive letter when a
	QUrl is constructed from such a pathname string. That is why only
	HTTP, FTP, or JPIP protocols are recognized here.

	@param	source	The file source to be converted to a normalized URL.
	@return	The normalized URL.
	@see	normalized_URL(const QUrl&)
*/
static QUrl normalized_URL (const QString& source);

/**	Get a brief description of a {@link request_status() request status}
	code.

	<b>N.B.</b>: The errorString method of the QIODevice base class of
	QNetworkReply may provide an additional description.

	@param	result	A network request status code.
	@return	A QString containing a brief, one line, description of the
		code.
*/
static QString status_description (int code);

/**	Get the pathname for the directory where the application executable
	is located.

	@return	A QString containing the application location directory
		pathname.
*/
static QString application_location ();

/*==============================================================================
	Data
*/
protected:

QUrl
	Requested_URL,
	Redirected_URL;
int
	Request_Status,
	HTTP_Status;
QString
	HTTP_Status_Description;

unsigned long
	Wait_Time;

//!	Thread-safe status values lock. N.B.: A recursive mutex is used.
QMutex
	*Status_Lock;

QNetworkAccessManager
	*Network_Access_Manager;
QNetworkReply
	*Network_Reply;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
