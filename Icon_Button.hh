/*	Icon_Button

HiROC CVS ID: $Id: Icon_Button.hh,v 1.2 2011/01/03 00:52:03 castalia Exp $

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

#ifndef HiView_Icon_Button_hh
#define HiView_Icon_Button_hh

#include	<QAbstractButton>


namespace UA
{
namespace HiRISE
{

class Icon_Button
:	public QAbstractButton
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
	Constructors
*/
explicit Icon_Button (const QIcon& icon, QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
virtual QSize sizeHint () const;

/*==============================================================================
	Event Handlers
*/
protected:

virtual void paintEvent (QPaintEvent* event);

};

}	//	namespace HiRISE
}	//	namespace UA
#endif
