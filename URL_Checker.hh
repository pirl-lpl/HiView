/*	URL_Checker

HiROC CVS ID: $Id: URL_Checker.hh,v 1.4 2012/08/07 05:04:49 castalia Exp $

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

#ifndef HiView_URL_Checker_hh
#define HiView_URL_Checker_hh

#include	"Network_Status.hh"

#include	<QThread>
#include	<QUrl>
class QEventLoop;
class QTimer;


namespace UA
{
namespace HiRISE
{
/**	A <i>URL_Checker</i> checks if a URL refers to an accessible source.

	The URL_Checker is a fully thread safe QThread subclass, and a
	subclass of Network_Status. When checking an HTTP URL a thread is run
	that uses a QNetworkAccessManager to check the URL for
	acccessibility. The URL check may be done asynchronously or
	synchrously with a maximum wait time for it to complete, and a
	request in progress may be canceled. A local file URL may also be
	checked; this is always done synchronously. A signal is emitted
	whenever a check has been completed.

	The results of a URL check include a QNetworkReply::NetworkError
	value with special condition extensions, the server HTTP status code
	and status description for HTTP URL checks, and the URL for a server
	redirection.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.4 $
*/
class URL_Checker
:	public QThread,
	public Network_Status
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
/**	Constructs a URL_Checker.

	@param	parent	A QObject that owns the constructed object and will
		destroy it when the parent is destroyed.
*/
explicit URL_Checker (QObject* parent = NULL);

//!	Destroys the URL_Checker.
virtual ~URL_Checker ();

/*==============================================================================
	Accessors
*/
/**	Test if a {@link check(const QUrl&, bool) URL check} is currently in
	progress.

	@return	true if a check is in progress; false otherwise.
*/
bool checking () const; 

/*==============================================================================
	Thread run
*/
/**	Check a URL for an accessible source.

	If a URL check is {@link checking() in progress}, or the specified
	URL is empty, nothing is done and false is returned immediately.

	The check state is reset to unset values when a check is started.

	The URL is checked to see if it is accessible. Only a URL with an
	HTTP or "file" protocol will be accepted for checking. A URL without
	a protocol is taken to be a file URL. The URL must have a pathname.
	HTTP URLs must have a hostname; a hostname in a file URLs is ignored.
	Any other URLs are rejected and the {@link request_status() request
	status} value will be set to {@link #INVALID_URL}.

	For an HTTP URL a network request will be made for the source access
	information. The source content is not acquired; instead of
	requesting the source data only the network headers are requested
	from the server connection. If any error occurs while attempting to
	obtain the network headers for the source - including a content not
	found error - then the source is not accessible.

	<b>N.B.</b>: A URL source will appear to be accessible when the
	server returns a {@link redirected_URL() redirected URL} where the
	ultimate source may or may not be accesible; it is the responsibility
	of the application to decide if a redirected URL is to be checked.

	An HTTP URL check may be synchronous or asynchronous. A synchronous
	check will wait - the thread of the caller will be blocked - until
	the check completes. The wait for the network request to complete
	will timeout if the {@link wait_time() wait time} is exceeded, in
	which case the network request will be {@link cancel() canceled} and
	the {@link request_status() request status} will be set to {@link
	#SYNCHRONOUS_TIMEOUT}. Thread blocking can be avoided - this can be
	important, for example, when the check is being done from the thread
	running the main GUI event loop - by specifying an asynchronous check
	with a connection to the {@link checked(bool) checked} signal to
	obtain the results of the check.

	If the URL specifies the file protocol, or does not specify any
	protocol, then the local filesystem is checked for the path portion
	of the URL. The pathname must exist and be a readable file for it to
	be accessible.

	The {@link checked(bool) checked} signal is always emitted when
	the check completes regardless of whether the check is synchronous
	or asynchronous, or if a timeout occurs.

	<b>>>> WARNING <<<</b> A filesystem pathname on MS/Windows that
	includes a drive specification - e.g. "D:" - will be interpreted as
	having a URL scheme specification that is the drive letter when a
	QUrl is constructed from such a pathname string. For this reason it
	is advisable that a {@link normalized_URL(const QString&) normalized}
	QUrl be used here.

	@param	URL	A QUrl that is to be checked.
	@param	synchronous	This argument is ignored unless the URL specifies
		an HTTP protocol. If synchronous is true the method will wait for
		the network request to complete or timeout before returning;
		otherwise the method will return without waiting for the network
		request to complete.
	@return	true if the check completed successfully and the source was
		found to be accessible; false otherwise. <b>N.B.</b>: For an
		asynchronous HTTP URL check the return value is always false;
		connect to the {@link checked(bool) checked} signal to get the
		results of the check, or test if the {@link request_status()
		request status} is {@link #ACCESSIBLE_URL} when {@link
		checking() checking} becomes false.
*/
bool check (const QUrl& URL, bool synchronous = true);


protected:

/**	Begin running the network request thread.

	<b>N.B.</b>: Running the thread should be done by calling the start
	method. The thread will finish when the network fetch has finished.
*/
virtual void run ();

/*==============================================================================
	Qt signals
*/
public:

signals:

/**	Signals the result of a {@link check(const QUrl&, bool) URL check}.

	The signal is emitted whenever a URL check completes.

	@param	exists	true if the checked URL is accessible; false otherwise.
	@see request_status()
*/
void checked (bool exists);

/*==============================================================================
	Data
*/
private:

QEventLoop
	*Event_Loop;
QTimer
	*Timer;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
