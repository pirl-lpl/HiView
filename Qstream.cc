/*	Qistream

HiROC CVS ID: $Id: Qstream.cc,v 1.6 2013/06/13 17:42:24 guym Exp $

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

#include	"Qstream.hh"

#include	<QIODevice>
#include	<QNetworkReply>
#include	<QEventLoop>
#include	<QTimer>

#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::invalid_argument;
using std::runtime_error;


#if defined (DEBUG_SECTION)
/*******************************************************************************
	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_STREAMBUF			(1 << 1)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<iostream>
using std::clog;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Qstreambuf::ID =
		"UA::HiRISE::Qstreambuf ($Revision: 1.6 $ $Date: 2013/06/13 17:42:24 $)";


#ifndef QSTREAM_BUFFER_SIZE
#define QSTREAM_BUFFER_SIZE		1024
#endif
#if QSTREAM_BUFFER_SIZE <= 0
#error	Invalid QSTREAM_BUFFER_SIZE.
#endif
const int
	Qstreambuf::BUFFER_SIZE		= QSTREAM_BUFFER_SIZE;

#ifndef QSTREAM_PUTBACK_SIZE
#define QSTREAM_PUTBACK_SIZE	4
#endif
#if QSTREAM_PUTBACK_SIZE < 0
#error	Invalid QSTREAM_PUTBACK_SIZE.
#endif
#if QSTREAM_PUTBACK_SIZE >= (QSTREAM_BUFFER_SIZE / 2)
#error	The QSTREAM_PUTBACK_SIZE is not less than half the QSTREAM_BUFFER_SIZE.
#endif
const int
	Qstreambuf::PUTBACK_SIZE	= QSTREAM_PUTBACK_SIZE;

enum
	{
	WAIT_TIMEOUT	= -1,
	NO_DATA			= 0,
	HAS_DATA		= 1
	};

/*==============================================================================
	Static data
*/
#ifndef QSTREAM_DEFAULT_WAIT_TIME
#define QSTREAM_DEFAULT_WAIT_TIME		10000
#endif
int
	Qstreambuf::Default_Wait_Time		= QSTREAM_DEFAULT_WAIT_TIME;

/*==============================================================================
	Constructors
*/
Qstreambuf::Qstreambuf
	(
	QIODevice*	qiodevice
	)
	:	QObject (),
		std::streambuf (),
		QIO_Device (qiodevice),
		Network_Reply (NULL),
		Buffer (NULL),
		Event_Loop (NULL),
		Timer (NULL),
		Wait_Time (Default_Wait_Time),
		Timeout (false)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Qstreambuf @ " << (void*)this << endl;
#endif
setg (0, 0, 0);
if (QIO_Device)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    QIO_Device openMode = " << QIO_Device->openMode () << endl;
	#endif
	if (! QIO_Device->isReadable ())
		{
		QIO_Device = NULL;
		ostringstream
			message;
		message
			<< ID << endl
			<< "The QIODevice is not readable.";
		throw runtime_error (message.str ());
		}

	Buffer = new std::streambuf::char_type[QSTREAM_BUFFER_SIZE];
	setg
		(Buffer + PUTBACK_SIZE,
		 Buffer + PUTBACK_SIZE,
		 Buffer + PUTBACK_SIZE);
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    " << QSTREAM_BUFFER_SIZE << " character Buffer @ "
			<< (void*)Buffer
			<< " with " << PUTBACK_SIZE << " character put-back section"
			<< endl
		 << "    eback @ " << (void*)eback () << endl
		 << "     gptr @ " << (void*)gptr () << endl
		 << "    egptr @ " << (void*)egptr () << endl;
	#endif

	//	Test for the case where the QIODevice is a QNetworkReply.
	Network_Reply = dynamic_cast<QNetworkReply*>(QIO_Device);
	if (Network_Reply)
		{
         if (Network_Reply->isFinished())
         {
            delete[] Buffer;
            return;
         }
         else if (Network_Reply->error() != QNetworkReply::NoError)
         {
            delete[] Buffer;
            return;
         }


		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    QIODevice is a QNetworkReply" << endl
			 << "    new QEventLoop" << endl;
		#endif
		Event_Loop = new QEventLoop;
		connect
			(Network_Reply,	SIGNAL (finished ()),
			 Event_Loop,	SLOT (quit ()));
		connect
			(Network_Reply,	SIGNAL (readyRead ()),
			 Event_Loop,	SLOT (quit ()));

		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    new QTimer" << endl;
		#endif
		Timer = new QTimer;
		Timer->setSingleShot (true);
		connect
			(Timer,			SIGNAL (timeout ()),
			 Event_Loop,	SLOT (quit ()));
		}
	}
else
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't construct a Qstreambuf on a NULL QIODevice.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Qstreambuf" << endl;
#endif
}


Qstreambuf::~Qstreambuf ()
{
if (Buffer)
	delete[] Buffer;
if (Network_Reply)
	{
	delete Event_Loop;
	delete Timer;
	}
}

/*==============================================================================
	streambuf implementation
*/
std::streamsize
Qstreambuf::xsgetn
	(
	std::streambuf::char_type*	data,
	std::streamsize				amount
	)
{
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << ">>> Qstreambuf:xsgetn: data @ " << (void*)data
		<< " amount = " << amount << endl;
#endif
std::streamsize
	got (0),
	available;
while (amount)
	{
	if (refill_buffer () != HAS_DATA)
		break;
	available = egptr () - gptr ();
	if (available > amount)
		available = amount;
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    copying " << available << " bytes from " << (void*)gptr ()
			<< " to " << (void*)data << endl;
	#endif
	traits_type::copy (data, gptr (), available);
	gbump ((int)available);
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    bumped gptr @ " << (void*)gptr () << endl;
	#endif
	data += available;
	got += available;
	amount -= available;
	}
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "<<< Qstreambuf:xsgetn: " << got << endl;
#endif
return got;
}


int
Qstreambuf::refill_buffer ()
{
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << ">>> Qstreambuf:refill_buffer" << endl
	 << "    eback @ " << (void*)eback () << endl
	 << "     gptr @ " << (void*)gptr () << endl
	 << "    egptr @ " << (void*)egptr () << endl;
#endif
int
	status (NO_DATA);
if (gptr () < egptr ())
	{
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    buffer still has data" << endl;
	#endif
	status = HAS_DATA;
	}
else
if (QIO_Device)
	{
	int
		putback (gptr () - eback ());
	if (putback > PUTBACK_SIZE)
		putback = PUTBACK_SIZE;
	if (putback)
		{
		//	Move last read characters to the put-back section.
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    moving " << putback << " put-back characters from "
				<< (void*)(gptr () - putback) << " to "
				<< (void*)(Buffer + (PUTBACK_SIZE - putback)) << endl;
		#endif
		traits_type::move
			(Buffer + (PUTBACK_SIZE - putback), gptr () - putback,
				putback);

		//	Reset the buffer pointers.
		setg
			(Buffer + (PUTBACK_SIZE - putback),
			 Buffer + PUTBACK_SIZE,
			 Buffer + PUTBACK_SIZE);
		}

	//	Refill the buffer.
	std::streamsize
		available = QIO_Device->bytesAvailable ();
	if (! available &&
		Network_Reply &&
		Wait_Time)
		{
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    starting " << Wait_Time << " ms timer" << endl;
		#endif
		Timeout = false;
		//	Wait in a local event loop on network reply for data or finished.
		Timer->start (Wait_Time);
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    starting event loop" << endl;
		#endif
		Event_Loop->exec ();
		//	Event occured.
		if (Timer->isActive ())
			{
			Timer->stop ();
			//	Did more data arrive?
			available = QIO_Device->bytesAvailable ();
			}
		else
			{
			status = WAIT_TIMEOUT;
			Timeout = true;
			}
		}
	if (available)
		{
		status = HAS_DATA;
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    bytes available = " << available << endl
			 << "    refilling buffer @ "
				<< (void*)(Buffer + PUTBACK_SIZE) << " with "
				<< (QSTREAM_BUFFER_SIZE - PUTBACK_SIZE)
				<< " characters" << endl;
		#endif
		available = QIO_Device->read (Buffer + PUTBACK_SIZE,
			QSTREAM_BUFFER_SIZE - PUTBACK_SIZE);

		//	Reset the buffer pointers.
		setg
			(Buffer + (PUTBACK_SIZE - putback),
			 Buffer + PUTBACK_SIZE,
			 Buffer + PUTBACK_SIZE + available);
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    got " << available << " characters" << endl
			 << "    eback @ " << (void*)eback () << endl
			 << "     gptr @ " << (void*)gptr () << endl
			 << "    egptr @ " << (void*)egptr () << endl;
		#endif
		}
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	else
		clog << "    no data available" << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "<<< Qstreambuf:refill_buffer: " << status << endl;
#endif
return status;
}
 

std::streambuf::pos_type
Qstreambuf::seekoff
	(
	std::streambuf::off_type	offset,	//	signed integer type.
	std::ios_base::seekdir		direction,
	std::ios_base::openmode		mode
	)
{
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << ">>> Qstreambuf:seekoff: " << offset << " from "
		<< ((direction == std::ios_base::cur) ? "current position" :
		   ((direction == std::ios_base::beg) ? "beginning of stream" :
		   ((direction == std::ios_base::end) ? "end of stream" : "???")))
		<< endl
	 << "    eback @ " << (void*)eback () << endl
	 << "     gptr @ " << (void*)gptr () << endl
	 << "    egptr @ " << (void*)egptr () << endl;
#endif
/*	N.B. a pos_type value is not necessarily an integer type
	but it is required to conversion to/from int and off_type
	as well as the addition and substraction of off_type values.
*/
std::streambuf::pos_type
	position (-1);
if (QIO_Device)
	{
	switch (direction)
		{
		case std::ios_base::cur:
			position = QIO_Device->pos () - (egptr () - gptr ()) + offset;
			if (! offset)
				//	Return current position (tellg implementation).
				goto Done;
			break;
		case std::ios_base::beg:
			position = offset;
			break;
		case std::ios_base::end:
			if (! QIO_Device->isSequential ())
				{
				#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
				clog << "    stream size = " << QIO_Device->size () << endl;
				#endif
				position = QIO_Device->size () + offset;
				}
			#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
			else
				clog << "    QIO_Device is sequential" << endl;
			#endif
			break;
		default:
			break;
		}
	if (position != std::streambuf::pos_type (-1))
		position = seekpos (position, mode);
	}
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
else
	clog << "    QIO_Device is NULL" << endl;
#endif
Done:
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "    eback @ " << (void*)eback () << endl
	 << "     gptr @ " << (void*)gptr () << endl
	 << "    egptr @ " << (void*)egptr () << endl
	 << "<<< Qstreambuf:seekoff: " << position << endl;
#endif
return position;
}


std::streambuf::pos_type
Qstreambuf::seekpos
	(
	std::streambuf::pos_type	position,
	std::ios_base::openmode		/* mode */
	)
{
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << ">>> Qstreambuf:seekpos: " << position << endl
	 << "    eback @ " << (void*)eback () << endl
	 << "     gptr @ " << (void*)gptr () << endl
	 << "    egptr @ " << (void*)egptr () << endl;
#endif
if (QIO_Device)
	{
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    position of QIO_Device = " << QIO_Device->pos () << endl;
	#endif
	if (position < QIO_Device->pos () &&
		position >= (QIO_Device->pos () - (egptr () - eback ())))
		{
		#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
		clog << "    new position is within buffer" << endl;
		#endif
		setg (eback (), egptr () - (QIO_Device->pos () - position), egptr ());
		}
	else
	if (QIO_Device->seek (position))
		{
		position = QIO_Device->pos ();
		//	Reset buffer pointers to empty buffer.
		setg
			(Buffer + PUTBACK_SIZE,
			 Buffer + PUTBACK_SIZE,
			 Buffer + PUTBACK_SIZE);
		}
	else
		position = std::streambuf::pos_type (-1);
	}
else
	{
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    QIO_Device is NULL" << endl;
	#endif
	position = std::streambuf::pos_type (-1);
	}
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "    eback @ " << (void*)eback () << endl
	 << "     gptr @ " << (void*)gptr () << endl
	 << "    egptr @ " << (void*)egptr () << endl
	 << "<<< Qstreambuf:seekpos: " << position << endl;
#endif
return position;
}


std::streambuf::int_type
Qstreambuf::underflow ()
{
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << ">>> Qstreambuf:underflow" << endl;
#endif
std::streambuf::int_type
	result;
if (gptr () < egptr () ||
	refill_buffer () == HAS_DATA)
	{
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    next datum" << endl;
	#endif
	result = traits_type::to_int_type (*gptr ());
	}
else
	{
	#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
	clog << "    EOF" << endl;
	#endif
    result = traits_type::eof ();
	}
#if ((DEBUG_SECTION) & DEBUG_STREAMBUF)
clog << "<<< Qstreambuf:underflow: " << result << endl;
#endif
return result;
}


}	//	namespace HiRISE
}	//	namespace UA
