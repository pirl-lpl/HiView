/*	Icon_Button

HiROC CVS ID: $Id: Icon_Button.cc,v 1.4 2012/03/09 02:13:57 castalia Exp $

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

#include	"Icon_Button.hh"

#include	<QPainter>
#include	<QPaintEvent>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF			0
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_EVENTS		(1 << 3)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<string>
using std::string;
#include	<iostream>
using std::clog;
using std::boolalpha;
using std::hex;
using std::dec;

#endif	//	DEBUG_SECTION

namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Icon_Button::ID =
		"UA::HiRISE::Icon_Button ($Revision: 1.4 $ $Date: 2012/03/09 02:13:57 $)";

/*==============================================================================
	Constructors
*/
Icon_Button::Icon_Button
	(
	const QIcon&	icon,
	QWidget*		parent
	)
	:	QAbstractButton (parent)
{setIcon (icon);}

/*==============================================================================
	Accessors
*/
QSize
Icon_Button::sizeHint () const
{return iconSize ();}

/*==============================================================================
	Event Handlers
*/
void
Icon_Button::paintEvent
	(
	QPaintEvent*	event
	)
{
QPainter
	painter (this);
icon ().paint (&painter, event->rect (), Qt::AlignCenter,
		isEnabled () ? QIcon::Normal : QIcon::Disabled,
		isDown () ? QIcon::Off : QIcon::On);
}


}	//	namespace HiRISE
}	//	namespace UA
