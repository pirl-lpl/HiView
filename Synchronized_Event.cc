/*	Synchronized_Event

HiROC CVS ID: $Id: Synchronized_Event.cc,v 1.2 2011/01/03 00:52:04 castalia Exp $

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

#include	"Synchronized_Event.hh"


namespace UA
{
namespace HiRISE
{
Synchronized_Event::Synchronized_Event
	(
	bool	auto_reset
	)
	:	Auto_Reset (auto_reset),
		Event_is_Set (false)
{}


Synchronized_Event::~Synchronized_Event ()
{Wait_Condition.wakeAll ();}


void
Synchronized_Event::set ()
{
if (! Event_is_Set)
	{
	Event_is_Set = true;
	if (Auto_Reset)
		Wait_Condition.wakeOne ();
	else
		Wait_Condition.wakeAll ();
	}
}


bool
Synchronized_Event::wait
	(
	QMutex*			mutex,
	unsigned long	timeout
	)
{
bool
	condition = Event_is_Set;
if (! condition)
	{
	if (timeout <= 0)
		timeout = ULONG_MAX;
	condition = Wait_Condition.wait (mutex, timeout);
	if (Auto_Reset)
		Event_is_Set = false;
	}
return condition;
}


}	//	namespace HiRISE
}	//	namespace UA
