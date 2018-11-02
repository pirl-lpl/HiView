/*	Image_Renderer_Thread

HiROC CVS ID: $Id: Image_Renderer_Thread.hh,v 1.11 2012/06/15 01:16:06 castalia Exp $

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

#ifndef HiView_Image_Renderer_Thread_hh
#define HiView_Image_Renderer_Thread_hh

#include	"Image_Renderer.hh"
#include	"Synchronized_Event.hh"

#include	<QThread>


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class JP2_Image;


/**	An <i>Image_Renderer_Thread</i> provides asychronous image rendering.
*/
class Image_Renderer_Thread
:	public Image_Renderer,
	public QThread
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Constructors
*/
public:

explicit Image_Renderer_Thread (QObject *parent = NULL);

virtual ~Image_Renderer_Thread ();

/*==============================================================================
	Run
*/
/**	Finish running the rendering loop and exit the thread.

	The rendering loop is flagged to finish running. The rendering loop
	is {@link reset(int) reset} which canels any rendering in progress
	and clears the rendering queue using the specified cancel options -
	the FORCE_CANCEL option is always applied - and any tiles in the
	delete queue are {@link delete_tiles() deleted}.

	If the rendering thread is not running, nothing more is done.

	If the rendering loop is {@link suspended() suspended} it is
	{@link start_rendering() started} again so the thread can proceed to
	completion. If the cancel options have the
	{@link #WAIT_UNTIL_DONE} flag set the calling thread will wait up to
	{@link #Wait_Seconds} seconds for the execution of the thread to finish.

	@param	cancel_options	One or more cancel option flags to be applied
		when rendering is {@link reset(int) reset}. Also, if the {@link
		#WAIT_UNTIL_DONE} flag is set the calling thread will not block
		until the rendering thread has finished or the {@link
		#MAXIMUM_WAIT_SECONDS} seconds timeout has occurred.
	@return	true	if the thread has finished running; false if the
		thread had not exited by the time the method returned.
*/
virtual bool finish (int cancel_options = WAIT_UNTIL_DONE);

/**	Start the rendering loop.

	If the rendering thread is not running it is started.

	The rendering loop is set to be {@link runnable() runnable} and not
	{@link suspended() suspended}, and the ready condition event is set.

	If the rendering loop has been {@link suspend_rendering(bool)
	suspended} or {@link stop_rendering(bool) stopped}, and  is waiting
	(execution is blocked) for the ready condition event, execution will
	be unblocked. If the {@link finish(int) finish} flag was set the
	rendering loop stops which causes the thread to stop running.

	If the rendering loop is currently running the next check to see if
	it should {@link is_ready() continue} will not block. 

	@see	render()
	@see	stop_rendering(bool)
*/
virtual void start_rendering ();

/**	Suspend the rendering loop.

	The rendering loop is set to be {@link suspended() suspended}. Any
	rendering in progress will continue to completion after which the
	rendering loop will stop; any tiles that are still in the {@link
	queue(Plastic_Image*, const QPoint&, bool) rendering queue} will
	remain queued. If the rendering loop is {@link runnable( runnable}
	the next time an image is {@link add_tile(Image_Tile*) added} to the
	queue rendering will automatically begin again.

	The ready condition event is reset so the next time the event is
	checked the thread will wait (execution will be blocked) until the
	ready condition event is set, unless rendering was {@link
	start_rendering() restarted} before the event is checked.

	@param	wait	true if the thread should try to wait until any
		redering in progress has completed before returning; false
		otherwise.
	@return	true if rendering is suspended when the method returns;
		false if wait was false and rendering was not suspended when
		this method was called, or if wait was true and waiting for
		rendering to complete timed out.
	@see	stop_rendering(bool)
*/
virtual bool suspend_rendering (bool wait = false);


protected:

/**	Run the thread.

	This method is called by the base QThread class when its start method
	is invoked. It initializes the operating state and starts the
	rendering loop. It will not return until the rendering loop is {@link
	finish(int) finished}.

	<b>N.B.</b>: This method should not be used directly by the
	application; use the {@link start_rendering()} or start method
	instead.
*/
virtual void run ();

/**	Run the rendering loop.

	The rendering thread is usually always running, though it may be
	blocked awaiting the ready condition event. So the implementation of
	this method does nothing.

	If the rendering thread is not running (isFinished()) use the {@link
	start_rendering()} or start method to start the thread and the
	rendering loop again.
*/
virtual void run_rendering ();

/**	Test if the rendering loop is ready to continue.

	<b>N.B.</b>: The Ready_Lock is locked during this method.

	This method is called in each cycle of the rendering loop. If the
	rendering loop has been {@link suspend_rendering(bool) suspended} the
	{@link status(int) status} signal is emitted with the {@link
	#Image_Tile::FINISHED} condition, and the thread will wait (block
	execution) until the ready condition event has been set before
	clearing the suspended flag and returning. If the ready event was
	already set then the thread will not wait.

	@return	false if the thread has been flagged to {@link finish(int)
		finish} running the rendering loop and thread execution is to
		stop; true otherwise.
*/
virtual bool is_ready ();

/*==============================================================================
	Data
*/
private:

//!	The thread's ready to run condition.
Synchronized_Event
	Ready_Event;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
