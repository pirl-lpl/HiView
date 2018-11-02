/*	Synchronized_Event

HiROC CVS ID: $Id: Synchronized_Event.hh,v 1.6 2012/07/28 02:43:00 castalia Exp $

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

#ifndef HiView_Synchronized_Event_hh
#define HiView_Synchronized_Event_hh

#include	<QWaitCondition>
#include	<QMutex>


namespace UA
{
namespace HiRISE
{
/**	A <i>Synchronized_Event<i> provides a thread block and release
	mechanism for a conditional synchronization point controlled by some
	event.

	A Synchronized_Event is in either the set or reset state. When the
	event(s) with which the Synchronized_Event is associated occur the
	Synchronized_Event is expected to be set by the event handler while
	an event QMutex is locked. At a point in a thread's execution where
	synchronization on the occurance of the event is to occur the thread
	locks the event QMutex and then {@link wait(QMutex*, unsigned long)
	waits} on the Synchronized_Event which will block the thread unless
	or until the Synchronized_Event becomes set, or an optional timeout
	occurs. When the wait completes the Synchronized_Event is reset (the
	Synchronized_Event may be configured to do this automatically). Note
	that the event may occur before or after waiting on the
	Synchronized_Event; the calling thread will only be blocked if the
	event has not yet occurred.

	@see	QWaitCondition

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.6 $
*/
class Synchronized_Event
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
/**	Constructs a Synchronized_Event object.

	The Synchronized_Event is initially in the {@link reset() reset} state.

	@param	auto_reset	 If true, the object state is automatically reset
		whenever a {@link wait(QMutex*, unsigned long) wait} completes. If
		false, when the object state is {@link set() set} it remains in
		the set state until it is @ link reset() reset}.
*/
Synchronized_Event (bool auto_reset = false);

/**	Destroys the Synchronized_Event object.

	<b>N.B.</b>: Any threads waiting on the condition will be woken.
*/
~Synchronized_Event ();

/**	Tests if this Synchronized_Event object does auto-reset.

	@return	true if auto-reset is enabled; false otherwise.
*/
inline bool auto_reset () const
	{return Auto_Reset;}

/*	Sets the state of this Synchronized_Event.

	If the Synchronized_Event {@link is_set() is already in the set state}
	nothing is done.

	A {@link wait(QMutex*, unsigned long) waiting} thread will be woken
	up and return from its wait call. The mutex provided to the wait call
	will have been returned to the locked state. If the
	Synchronized_Event was constructed with the auto-reset attribute
	disabled, then all threads waiting on this Synchronized_Event will be
	woken. However, if auto-reset was enabled, then only one thread
	waiting on this Synchronized_Event will be woken (which one depends
	on the operating system's scheduling policies).

	As long as the object is in the set state any future wait call will
	also return immediately. The set state remains in effect until {@link
	reset() reset}. However, if the object was constructed with the
	auto-reset attribute enabled, then the set state is automatically
	reset whenever a wait call completes.
*/
void set ();

/**	Reset the state of this Synchronized_Event.

	@see	set()
*/
inline void reset ()
	{Event_is_Set = false;}

/**	Test if the state of this Synchronized_Event is set.

	@return	true if the event condition is {@link set() set}; false if
		it is {@link reset() reset}.
*/
inline bool is_set () const
	{return Event_is_Set;}

/**	Wait until this Synchronized_Event state is set or the timeout
	expires.

	If this Synchronized_Event {@link is_set() is already in the set
	state}, or the specified mutex is not not in a locked state or is a
	recursive mutex, the method returns true immediately.

	The specified mutex is first unlocked. Then the calling thread is
	blocked until the Synchronized_Event state is {@link set() set}.
	The mutex is returned to its locked state before the method returns.

	If a positive (greater than zero) timeout value is specified the
	calling thread will be blocked for a maximum of timeout milliseconds.
	If and only if a timeout occurs the return value will be false.

	@param	mutex	A pointer to a QMutex. If null the method returns
		false immediately. <b>N.B.</b>: The mutex is expected to be in
		the locked state.
	@param	timeout	The maximum amount of time, in milliseconds, to wait
		for the Synchronized_Event to be set. If not non-zero, positive
		the calling thread will wait forever for the event to be set.
	@return	true if this Synchronized_Event is, or becomes, set before
		the timeout expires. false if the mutex is null, a recursive
		mutex, or the timeout expires.
*/
bool wait (QMutex* mutex, unsigned long timeout = 0);

/*==============================================================================
	Data
*/
private:

bool
	Auto_Reset,		//	Auto-reset mode.
	Event_is_Set;	//	The state of the event variable

QWaitCondition
	Wait_Condition;
};

}	//	namespace HiRISE
}	//	namespace UA
#endif
