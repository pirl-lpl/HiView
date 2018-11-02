/*	About_HiView_Dialog

HiROC CVS ID: $Id: About_HiView_Dialog.cc,v 1.4 2012/03/09 02:13:54 castalia Exp $

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

#include	"About_HiView_Dialog.hh"

#include	"Icon_Button.hh"

#include	<QApplication>
#include	<QVBoxLayout>
#include	<QHBoxLayout>
#include	<QPainter>
#include	<QLabel>
#include	<QPixmap>
#include	<QIcon>
#include	<QPushButton>
#include	<QAction>
#include	<QScrollArea>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SLOTS				(1 << 2)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	About_HiView_Dialog::ID =
		"UA::HiRISE::About_HiView_Dialog ($Revision: 1.4 $ $Date: 2012/03/09 02:13:54 $)";


#ifndef ABOUT_HIVIEW_DIALOG_MARGIN
#define ABOUT_HIVIEW_DIALOG_MARGIN				10
#endif
#ifndef HIVIEW_LOGO_MARGIN
#define HIVIEW_LOGO_MARGIN						10
#endif
#ifndef ABOUT_HIVIEW_DIALOG_TEXT_Y_OFFSET
#define ABOUT_HIVIEW_DIALOG_TEXT_Y_OFFSET		-10
#endif
#ifndef ABOUT_HIVIEW_DIALOG_CREDITS_X_OFFSET
#define ABOUT_HIVIEW_DIALOG_CREDITS_X_OFFSET	-10
#endif

/*==============================================================================
	Constructor
*/
About_HiView_Dialog::About_HiView_Dialog ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> About_HiView_Dialog" << endl;
#endif
setWindowTitle (tr ("About") + ' ' + qApp->applicationName ());

//	Allow non-modal (show instead of exec) operation.
setModal (false);

QVBoxLayout
	*dialog_layout = new QVBoxLayout (this);
dialog_layout->setAlignment (Qt::AlignTop); 
dialog_layout->setContentsMargins (0, 0, 0, 0);
dialog_layout->setSpacing (0);
dialog_layout->setSizeConstraint (QLayout::SetFixedSize);

//	Background image.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Background image" << endl;
#endif
QPixmap
	background (":/Images/About_Background.png");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "      size " << background.size () << endl;
#endif
QPainter
	painter (&background);
QLabel
	*content;

//	Application logo.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Application logo" << endl;
#endif
QPixmap
	logo (":/Images/HiView_Icon.png");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "      size " << logo.size () << endl
	 << "    draw HiView logo onto the background image" << endl
	 << "      at 0x, 0y" << endl;
#endif
painter.drawPixmap (0, 0, logo);

//	Content text.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Content text" << endl
	 << "      size "
		 << (background.width ()
		 	- (logo.width () + ABOUT_HIVIEW_DIALOG_MARGIN)) << "w, "
		 << (background.height ()
		 	- (ABOUT_HIVIEW_DIALOG_MARGIN * 2)) << 'h' << endl
	 << "      default font family = " <<
	 	font ().defaultFamily () << endl;
#endif
content = new QLabel
	(tr (
		#include "About_HiView.html"
		)
		.arg (qApp->applicationVersion ()));
content->setStyleSheet ("background:transparent;");
content->setFixedSize
	(background.width () - (logo.width () + ABOUT_HIVIEW_DIALOG_MARGIN),
	 background.height () - (ABOUT_HIVIEW_DIALOG_MARGIN * 2));
content->setContentsMargins (0, 0, 0, 0);
content->setWordWrap (true);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    render content text label onto the background image" << endl
	 << "      at " << logo.width () << "x, "
		<< (ABOUT_HIVIEW_DIALOG_MARGIN + ABOUT_HIVIEW_DIALOG_TEXT_Y_OFFSET)
		<< 'y' << endl;
#endif
content->render (&painter, QPoint
	(logo.width (),
	 ABOUT_HIVIEW_DIALOG_MARGIN + ABOUT_HIVIEW_DIALOG_TEXT_Y_OFFSET));
delete content;

//	Primary panel label image. 
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    add content label with background image to layout" << endl;
#endif
content = new QLabel;
content->setPixmap (background);
dialog_layout->addWidget (content);


//	Buttons panel.
QHBoxLayout
	*horizontal_layout = new QHBoxLayout;
horizontal_layout->setContentsMargins (10, 5, 5, 5);

//	Close button.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Close button" << endl;
#endif
QPushButton
	*close_button = new QPushButton (tr ("Close"));
connect (close_button,
	SIGNAL (clicked ()),
	SLOT (close ()));
horizontal_layout->addWidget (close_button);

//	Close action.
QAction
	*action = new QAction (tr ("Close Window"), this);
action->setShortcut (tr ("Ctrl+W"));
connect (action,
	SIGNAL (triggered ()),
	SLOT (close ()));
addAction (action);

horizontal_layout->addStretch (100);

//	Expand button.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Expand button" << endl;
#endif
Expand_Down_Icon = new QIcon (":/Images/Expand_Down_Icon.png");
Expand_Up_Icon   = new QIcon (":/Images/Expand_Up_Icon.png");
Expand_Button = new Icon_Button (*Expand_Down_Icon);
Expand_Button->setCheckable (true);
Expand_Button->setChecked (false);
Expand_Button->setToolTip (tr ("Open credits panel"));
connect (Expand_Button,
	SIGNAL (toggled (bool)),
	SLOT (expand_button_toggled (bool)));
horizontal_layout->addWidget (Expand_Button);
dialog_layout->addLayout (horizontal_layout);


//	Credits panel.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Credits panel" << endl
	 << "      label column width = "
	 	<< (logo.width ()
			- HIVIEW_LOGO_MARGIN + ABOUT_HIVIEW_DIALOG_CREDITS_X_OFFSET) << endl
	 << "      cell spacing = " << HIVIEW_LOGO_MARGIN << endl;
#endif
content = new QLabel
	(tr (
		#include "Credits.html"
		)
		.arg (logo.width ()
			- HIVIEW_LOGO_MARGIN + ABOUT_HIVIEW_DIALOG_CREDITS_X_OFFSET)
		.arg (HIVIEW_LOGO_MARGIN));
content->setStyleSheet ("background:transparent;");
content->setContentsMargins (0, 0, 0, 0);
//	Enable links to be opened in the user's web browser.
content->setOpenExternalLinks (true);

Expand_Panel = new QScrollArea;
Expand_Panel->setMaximumHeight (background.height ());
Expand_Panel->viewport ()->setPalette (QPalette (QColor (255, 247, 228)));
Expand_Panel->viewport ()->setAutoFillBackground (true);
Expand_Panel->setWidget (content);
Expand_Panel->setVisible (Expand_Button->isChecked ());
dialog_layout->addWidget (Expand_Panel);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< About_HiView_Dialog" << endl;
#endif
}


void
About_HiView_Dialog::expand_button_toggled
	(
	bool	down
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">>> About_HiView_Dialog::expand_button_toggled: "
		<< boolalpha << down << endl;
#endif
if (down)
	{
	Expand_Button->setIcon (*Expand_Up_Icon);
	Expand_Button->setToolTip (tr ("Close credits panel"));
	resize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	}
else
	{
	Expand_Button->setIcon (*Expand_Down_Icon);
	Expand_Button->setToolTip (tr ("Open credits panel"));
	}
Expand_Panel->setVisible (down);
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "<<< About_HiView_Dialog::expand_button_toggled" << endl;
#endif
}


}	//	namespace HiRISE
}	//	namespace UA
