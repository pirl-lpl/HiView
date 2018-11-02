/*	Image_Renderer_Thread

HiROC CVS ID: $Id: Image_Renderer_Thread.cc,v 1.17 2012/03/09 02:13:58 castalia Exp $

Copyright (C) 2010-2011  Arizona Board of Regents on behalf of the
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

#include	"Image_Renderer_Thread.hh"


#if defined (DEBUG_SECTION)
/*******************************************************************************
	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SIGNALS			(1 << 3)
#define DEBUG_RENDER			(1 << 4)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
using std::clog;
#include	<iomanip>
using std::endl;
using std::boolalpha;
using std::flush;

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Image_Renderer_Thread::ID =
		"UA::HiRISE::Image_Renderer_Thread ($Revision: 1.17 $ $Date: 2012/03/09 02:13:58 $)";

/*==============================================================================
	Constructors
*/
Image_Renderer_Thread::Image_Renderer_Thread
	(
	QObject	*parent
	)
	:	Image_Renderer (parent),
		QThread (parent)
{
Image_Renderer::setObjectName ("Image_Renderer_Thread");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread @ " << (void*)this
		<< ": " << object_pathname ((Image_Renderer*)this) << endl
	 << "    start the thread ..." << endl));
#endif
//	Start the the thread running which will start the rendering loop.
start ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "<<< Image_Renderer_Thread @ " << (void*)this << endl));
#endif
}


Image_Renderer_Thread::~Image_Renderer_Thread()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << ">>> ~Image_Renderer_Thread @ " << (void*)this << endl));
#endif
/*	>>> WARNING <<< This Image_Renderer_Thread is destroyed BEFORE the
	base Image_Renderer.
*/
if (! Finish)
	finish (WAIT_UNTIL_DONE | FORCE_CANCEL);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "<<< ~Image_Renderer_Thread @ " << (void*)this << endl));
#endif
}

/*==============================================================================
	Run the rendering thread
*/
void
Image_Renderer_Thread::run ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread::run " << thread_ID << endl));
#endif
Runnable  = true;
Suspended = false;
Finish    = false;

//	Start the rendering loop. This will not return until finish is called.
render ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "<<< Image_Renderer_Thread::run: exiting thread " << thread_ID << endl));
#endif
}

/*==============================================================================
	Rendering
*/
void
Image_Renderer_Thread::run_rendering ()
{}


bool
Image_Renderer_Thread::is_ready ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (static_cast<Image_Renderer*>(this)));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread::is_ready " << thread_ID << endl
	 << "    in " << pathname << endl
	 << ">>> Image_Renderer_Thread::is_ready "
		<< ": lock Ready_Lock" << endl));
#endif
Ready_Lock.lock ();
if (Suspended)
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Image_Renderer_Thread::is_ready " << thread_ID
			<< ": emit status " << NOT_RENDERING << " - "
			<< status_description (NOT_RENDERING) << endl
		 << "    in " << pathname << endl));
	#endif
	//	>>> SIGNAL <<<
	emit status (NOT_RENDERING);

	//	Wait for new rendering operations to become available.
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "xxx Image_Renderer_Thread::is_ready " << thread_ID
			<< ": wait for Ready_Event" << endl
		 << "    in " << pathname << endl));
	#endif
	Ready_Event.wait (&Ready_Lock);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "+++ Image_Renderer_Thread::is_ready " << thread_ID
			<< ": reset Ready_Event" << endl
		 << "    in " << pathname << endl));
	#endif
	Suspended = false;
	Ready_Event.reset ();
	}
bool
	continue_rendering = ! Finish;
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer_Thread::is_ready " << thread_ID
		<< ": unlock Ready_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Ready_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer_Thread::is_ready " << thread_ID
		<< ": " << boolalpha << continue_rendering << endl));
#endif
return continue_rendering;
}


void
Image_Renderer_Thread::start_rendering ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS | DEBUG_STATUS))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (static_cast<Image_Renderer*>(this)));
#endif
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread::start_rendering " << thread_ID << endl
	 << "    in " << pathname << endl));
#endif
if (isRunning ())
	{
	//	The rendering loop is running. 
	Ready_Lock.lock ();
	Runnable = true;
	Suspended = false;
	Ready_Event.set ();
	Ready_Lock.unlock ();
	}
else
	//	Start the thread running (it is not already running).
	start ();

#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer_Thread::start_rendering " << thread_ID << endl));
#endif
}


bool
Image_Renderer_Thread::suspend_rendering
	(
	bool	wait
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (static_cast<Image_Renderer*>(this)));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread::suspend_rendering " << thread_ID
		<< ": wait = " << boolalpha << wait << endl
	 << "    in " << pathname << endl));
#endif
bool
	ready_locked = Ready_Lock.tryLock (),
	done = Image_Renderer::suspend_rendering (wait);
Ready_Event.reset ();
if (ready_locked)
	Ready_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer_Thread::suspend_rendering " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer_Thread::finish
	(
	int		cancel_options
	)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (static_cast<Image_Renderer*>(this))),
	description (cancel_options_descriptions (cancel_options));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer_Thread::finish " << thread_ID
		<< ": " << cancel_options << " - " << description << endl));
#endif
//	Finish any rendering operations; this will set Finish true.
Image_Renderer::finish (cancel_options);

if (isRunning ())
	{
	if (suspended ())
		//	Release the thread if suspended so it will run to completion.
		start_rendering ();

	if ((cancel_options & WAIT_UNTIL_DONE) &&
		isRunning ())
		{
		//	Wait for the thread to finish.
		#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer_Thread::finish " << thread_ID
				<< ": wait up to " << Wait_Seconds
				<< " seconds for rendering thread to finish" << endl));
		#endif
		wait (Wait_Seconds * 1000);
		}
	}
bool
	finished = isFinished ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << "<<< Image_Renderer_Thread::finish " << thread_ID
		<< ": " << boolalpha << finished << endl));
#endif
return finished;
}



}	//	namespace HiRISE
}	//	namespace UA

