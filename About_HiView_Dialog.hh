/*	About_HiView_Dialog

HiROC CVS ID: $Id: About_HiView_Dialog.hh,v 1.2 2011/10/04 04:58:13 castalia Exp $

Copyright (C) 2011  Arizona Board of Regents on behalf of the
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

#ifndef About_HiView_Dialog_hh
#define About_HiView_Dialog_hh

#include	<QDialog>

//	Forward references.
class QIcon;
class QScrollArea;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class	Icon_Button;

/**	An <i>About_HiView_Dialog</i> provides a dialog box with information
	about the HiView application.

	@author		Andrew Stockton and Bradford Castalia, UA/HiROC
	@version	$Revision: 1.2 $
*/
class About_HiView_Dialog
:	public QDialog
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
//!	Constructs the About_HiView_Dialog.
About_HiView_Dialog ();

/*==============================================================================
	Slots
*/
private slots:

void expand_button_toggled (bool down);

/*==============================================================================
	Data
*/
private:

Icon_Button
	*Expand_Button;
QIcon
	*Expand_Down_Icon,
	*Expand_Up_Icon;
QScrollArea 
	*Expand_Panel;

};

}	//	namespace HiRISE
}	//	namespace UA
#endif
