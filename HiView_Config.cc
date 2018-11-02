/*	HiView_Config

HiROC CVS ID: $Id: HiView_Config.cc,v 1.15 2012/03/09 02:13:55 castalia Exp $

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

#include	"HiView_Config.hh"

//	Qt
#include	<QFrame>
#include	<QCursor>
#include	<QIcon>
#include	<QPixmap>
#include	<QBitmap>
#include	<QColor>

//	Qwt
#include	<qwt_picker.h>
#include <qwt_picker_machine.h>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::hex;
using std::dec;
using std::boolalpha;

#include	"HiView_Utilities.hh"

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	HiView_Config::CONFIG_ID =
		"UA::HiRISE::HiView_Config ($Revision: 1.15 $ $Date: 2012/03/09 02:13:55 $)";

/*------------------------------------------------------------------------------
	Data constraints.
*/
#ifndef MAX_DATA_PRECISION
#define MAX_DATA_PRECISION			16
#endif
const int
	HiView_Config::MAX_SOURCE_DATA_PRECISION
		= MAX_DATA_PRECISION,
	HiView_Config::MAX_DISPLAY_VALUE
		= 255;

const int
	HiView_Config::PERCENT_DECIMAL_PLACES
		= 2;

/*------------------------------------------------------------------------------
	Style
*/
#ifndef PANEL_FRAME_STYLE
#define PANEL_FRAME_STYLE			QFrame::StyledPanel | QFrame::Plain
#endif
int
	HiView_Config::Panel_Frame_Style =
		PANEL_FRAME_STYLE;

#ifndef PANEL_FRAME_WIDTH
#define PANEL_FRAME_WIDTH			1
#endif
int
	HiView_Config::Panel_Frame_Width =
		PANEL_FRAME_WIDTH;

#ifndef LABEL_FRAME_STYLE
#define LABEL_FRAME_STYLE			QFrame::Box | QFrame::Sunken
#endif
int
	HiView_Config::Label_Frame_Style =
		LABEL_FRAME_STYLE;

#ifndef LABEL_FRAME_WIDTH
#define LABEL_FRAME_WIDTH			1
#endif
int
	HiView_Config::Label_Frame_Width =
		LABEL_FRAME_WIDTH;

#ifndef LABEL_FRAME_MARGIN
#define LABEL_FRAME_MARGIN			2
#endif
int
	HiView_Config::Label_Frame_Margin =
		LABEL_FRAME_MARGIN;

#ifndef HEADLNG_LINE_WEIGHT
#define HEADLNG_LINE_WEIGHT			3
#endif
int
	HiView_Config::Heading_Line_Weight =
		HEADLNG_LINE_WEIGHT;

/*------------------------------------------------------------------------------
	Cursors
*/
#ifndef RETICULE_CURSOR
#define RETICULE_CURSOR \
	":/Images/reticule_cursor.png"
#endif
#ifndef RETICULE_CURSOR_MASK
#define RETICULE_CURSOR_MASK \
	":/Images/reticule_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Reticule_Cursor					= NULL;

#ifndef SHIFT_CURSOR
#define SHIFT_CURSOR \
	":/Images/shift_cursor.png"
#endif
#ifndef SHIFT_REGION_CURSOR
#define SHIFT_REGION_CURSOR \
	":/Images/shift_region_cursor.png"
#endif
#ifndef SHIFT_CURSOR_MASK
#define SHIFT_CURSOR_MASK \
	":/Images/shift_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Shift_Cursor					= NULL,
	*HiView_Config::Shift_Region_Cursor				= NULL;

#ifndef SHIFT_VERTICAL_CURSOR
#define SHIFT_VERTICAL_CURSOR \
	":/Images/shift_vertical_cursor.png"
#endif
#ifndef SHIFT_REGION_VERTICAL_CURSOR
#define SHIFT_REGION_VERTICAL_CURSOR \
	":/Images/shift_region_vertical_cursor.png"
#endif
#ifndef SHIFT_VERTICAL_CURSOR_MASK
#define SHIFT_VERTICAL_CURSOR_MASK \
	":/Images/shift_vertical_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Shift_Vertical_Cursor			= NULL,
	*HiView_Config::Shift_Region_Vertical_Cursor	= NULL;

#ifndef SHIFT_HORIZONTAL_CURSOR
#define SHIFT_HORIZONTAL_CURSOR \
	":/Images/shift_horizontal_cursor.png"
#endif
#ifndef SHIFT_REGION_HORIZONTAL_CURSOR
#define SHIFT_REGION_HORIZONTAL_CURSOR \
	":/Images/shift_region_horizontal_cursor.png"
#endif
#ifndef SHIFT_HORIZONTAL_CURSOR_MASK
#define SHIFT_HORIZONTAL_CURSOR_MASK \
	":/Images/shift_horizontal_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Shift_Horizontal_Cursor			= NULL,
	*HiView_Config::Shift_Region_Horizontal_Cursor	= NULL;

#ifndef SHIFT_REGION_FDIAG_CURSOR
#define SHIFT_REGION_FDIAG_CURSOR \
	":/Images/shift_region_FDiag_cursor.png"
#endif
#ifndef SHIFT_REGION_FDIAG_CURSOR_MASK
#define SHIFT_REGION_FDIAG_CURSOR_MASK \
	":/Images/shift_region_FDiag_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Shift_Region_FDiag_Cursor		= NULL;

#ifndef SHIFT_REGION_BDIAG_CURSOR
#define SHIFT_REGION_BDIAG_CURSOR \
	":/Images/shift_region_BDiag_cursor.png"
#endif
#ifndef SHIFT_REGION_BDIAG_CURSOR_MASK
#define SHIFT_REGION_BDIAG_CURSOR_MASK \
	":/Images/shift_region_BDiag_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Shift_Region_BDiag_Cursor		= NULL;

#ifndef SCALE_CURSOR
#define SCALE_CURSOR \
	":/Images/scale_cursor.png"
#endif
#ifndef SCALE_CURSOR_MASK
#define SCALE_CURSOR_MASK \
	":/Images/scale_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Scale_Cursor					= NULL;

#ifndef CROSSHAIR_CURSOR
#define CROSSHAIR_CURSOR \
	":/Images/crosshair_cursor.png"
#endif
#ifndef CROSSHAIR_CURSOR_MASK
#define CROSSHAIR_CURSOR_MASK \
	":/Images/crosshair_cursor_mask.png"
#endif
QCursor
	*HiView_Config::Crosshair_Cursor				= NULL;

#ifndef MOVE_HORIZONTAL_CURSOR
#define MOVE_HORIZONTAL_CURSOR \
	Qt::SizeHorCursor
#endif
QCursor
	*HiView_Config::Move_Horizontal_Cursor			= NULL;

//	Bounds selection cursors, by SELECTED color (zero is NULL).
#ifndef GREATER_THAN_CURSOR_RED
#define GREATER_THAN_CURSOR_RED \
	":/Images/greater_than_cursor_red.png"
#endif
#ifndef GREATER_THAN_CURSOR_GREEN
#define GREATER_THAN_CURSOR_GREEN \
	":/Images/greater_than_cursor_green.png"
#endif
#ifndef GREATER_THAN_CURSOR_YELLOW
#define GREATER_THAN_CURSOR_YELLOW \
	":/Images/greater_than_cursor_yellow.png"
#endif
#ifndef GREATER_THAN_CURSOR_BLUE
#define GREATER_THAN_CURSOR_BLUE \
	":/Images/greater_than_cursor_blue.png"
#endif
#ifndef GREATER_THAN_CURSOR_MAGENTA
#define GREATER_THAN_CURSOR_MAGENTA \
	":/Images/greater_than_cursor_magenta.png"
#endif
#ifndef GREATER_THAN_CURSOR_CYAN
#define GREATER_THAN_CURSOR_CYAN \
	":/Images/greater_than_cursor_cyan.png"
#endif
#ifndef GREATER_THAN_CURSOR_WHITE
#define GREATER_THAN_CURSOR_WHITE \
	":/Images/greater_than_cursor_white.png"
#endif
QCursor
	*HiView_Config::Greater_Than_Cursors[8]
		= {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#ifndef GREATER_THAN_CURSOR_MASK
#define GREATER_THAN_CURSOR_MASK \
	":/Images/greater_than_cursor_mask.png"
#endif

#ifndef LESS_THAN_CURSOR_RED
#define LESS_THAN_CURSOR_RED \
	":/Images/less_than_cursor_red.png"
#endif
#ifndef LESS_THAN_CURSOR_GREEN
#define LESS_THAN_CURSOR_GREEN \
	":/Images/less_than_cursor_green.png"
#endif
#ifndef LESS_THAN_CURSOR_YELLOW
#define LESS_THAN_CURSOR_YELLOW \
	":/Images/less_than_cursor_yellow.png"
#endif
#ifndef LESS_THAN_CURSOR_BLUE
#define LESS_THAN_CURSOR_BLUE \
	":/Images/less_than_cursor_blue.png"
#endif
#ifndef LESS_THAN_CURSOR_MAGENTA
#define LESS_THAN_CURSOR_MAGENTA \
	":/Images/less_than_cursor_magenta.png"
#endif
#ifndef LESS_THAN_CURSOR_CYAN
#define LESS_THAN_CURSOR_CYAN \
	":/Images/less_than_cursor_cyan.png"
#endif
#ifndef LESS_THAN_CURSOR_WHITE
#define LESS_THAN_CURSOR_WHITE \
	":/Images/less_than_cursor_white.png"
#endif
QCursor
	*HiView_Config::Less_Than_Cursors[8]
		= {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#ifndef LESS_THAN_CURSOR_MASK
#define LESS_THAN_CURSOR_MASK \
	":/Images/less_than_cursor_mask.png"
#endif

/*------------------------------------------------------------------------------
	Icons
*/
#ifndef APPLY_BUTTON_ICON
#define APPLY_BUTTON_ICON \
	":/Images/OK.png"
#endif
QIcon
	*HiView_Config::Apply_Button_Icon;

#ifndef DEFAULTS_BUTTON_ICON
#define DEFAULTS_BUTTON_ICON \
	":/Images/defaults_button.png"
#endif
QIcon
	*HiView_Config::Defaults_Button_Icon;

#ifndef RESET_BUTTON_ICON
#define RESET_BUTTON_ICON \
	":/Images/reset_button.png"
#endif
QIcon
	*HiView_Config::Reset_Button_Icon;

/*------------------------------------------------------------------------------
	Display image band names and colors.
*/
const char*
	HiView_Config::DISPLAY_BAND_NAMES[] = {"Red", "Green", "Blue"};
	
const QRgb
	HiView_Config::DISPLAY_BAND_COLORS[] =
		{
		0xFFFF0000,
		0xFF00FF00,
		0xFF0000FF
		};

/*------------------------------------------------------------------------------
	Data graphs.
*/
const int
	HiView_Config::TRACKER_TRACK_MODE =
		QwtPickerMachine::PointSelection |
		QwtPickerMachine::RectSelection,
	HiView_Config::TRACKER_DRAG_MODE =
		QwtPickerMachine::PointSelection |
		QwtPickerMachine::RectSelection,
	HiView_Config::TRACKER_BOUND_MODE =
		QwtPickerMachine::PointSelection |
		QwtPickerMachine::RectSelection;

#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)				"" #__VA_ARGS__ ""
#define AS_STRING(...)					STRINGIFIED(__VA_ARGS__)
#endif

#ifndef DEFAULT_GRAPH_CANVAS_COLOR
#define DEFAULT_GRAPH_CANVAS_COLOR		#E0D2BA
#endif
#define _DEFAULT_GRAPH_CANVAS_COLOR_	AS_STRING(DEFAULT_GRAPH_CANVAS_COLOR)
const QRgb
	HiView_Config::Default_Graph_Canvas_Color =
		QColor (_DEFAULT_GRAPH_CANVAS_COLOR_).rgba ();

/*==============================================================================
	Initialization
*/
void
HiView_Config::initialize ()
{
if (! Reticule_Cursor)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << ">>> HiView_Config" << endl
		 << "    initialization" << endl;
	#endif

	//	Cursors:

	QPixmap
		*cursor_image;
	QBitmap
		*cursor_mask;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Reticule_Cursor from " << RETICULE_CURSOR << endl
		 << "                     and " << RETICULE_CURSOR_MASK << endl;
	#endif
	Reticule_Cursor = new QCursor
		(QBitmap (RETICULE_CURSOR),
		 QBitmap (RETICULE_CURSOR_MASK));

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Cursor from " << SHIFT_CURSOR << endl
		 << "                  and " << SHIFT_CURSOR_MASK << endl;
	#endif
	cursor_mask = new QBitmap (SHIFT_CURSOR_MASK);
	Shift_Cursor = new QCursor
		(QBitmap (SHIFT_CURSOR),
		 *cursor_mask);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Region_Cursor from " << SHIFT_REGION_CURSOR << endl;
	#endif
	cursor_image = new QPixmap (SHIFT_REGION_CURSOR);
	cursor_image->setMask (*cursor_mask);
	Shift_Region_Cursor = new QCursor (*cursor_image);
	delete cursor_image;
	delete cursor_mask;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Vertical_Cursor from "
			<< SHIFT_VERTICAL_CURSOR << endl
		 << "                           and "
		 	<< SHIFT_VERTICAL_CURSOR_MASK << endl;
	#endif
	cursor_mask = new QBitmap (SHIFT_VERTICAL_CURSOR_MASK);
	Shift_Vertical_Cursor = new QCursor
		(QBitmap (SHIFT_VERTICAL_CURSOR), *cursor_mask);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Region_Vertical_Cursor from "
			<< SHIFT_REGION_VERTICAL_CURSOR << endl;
	#endif
	cursor_image = new QPixmap (SHIFT_REGION_VERTICAL_CURSOR);
	cursor_image->setMask (*cursor_mask);
	Shift_Region_Vertical_Cursor = new QCursor (*cursor_image);
	delete cursor_image;
	delete cursor_mask;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Horizontal_Cursor from "
			<< SHIFT_HORIZONTAL_CURSOR << endl
		 << "                             and "
		 	<< SHIFT_HORIZONTAL_CURSOR_MASK << endl;
	#endif
	cursor_mask = new QBitmap (SHIFT_HORIZONTAL_CURSOR_MASK);
	Shift_Horizontal_Cursor = new QCursor
		(QBitmap (SHIFT_HORIZONTAL_CURSOR), *cursor_mask);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Region_Horizontal_Cursor from "
			<< SHIFT_REGION_HORIZONTAL_CURSOR << endl;
	#endif
	cursor_image = new QPixmap (SHIFT_REGION_HORIZONTAL_CURSOR);
	cursor_image->setMask (*cursor_mask);
	Shift_Region_Horizontal_Cursor = new QCursor (*cursor_image);
	delete cursor_image;
	delete cursor_mask;	

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Scale_Cursor from " << SCALE_CURSOR << endl
		 << "                 and " << SCALE_CURSOR_MASK << endl;
	#endif
	Scale_Cursor =
		new QCursor (QBitmap (SCALE_CURSOR),
			QBitmap (SCALE_CURSOR_MASK));

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Region_FDiag_Cursor from "
			<< SHIFT_REGION_FDIAG_CURSOR << endl
		 << "                               and "
		 	<< SHIFT_REGION_FDIAG_CURSOR_MASK << endl;
	#endif
	cursor_image = new QPixmap (SHIFT_REGION_FDIAG_CURSOR);
	cursor_image->setMask (QBitmap (SHIFT_REGION_FDIAG_CURSOR_MASK));
	Shift_Region_FDiag_Cursor = new QCursor (*cursor_image);
	delete cursor_image;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Shift_Region_BDiag_Cursor from "
			<< SHIFT_REGION_BDIAG_CURSOR << endl
		 << "                               and "
		 	<< SHIFT_REGION_BDIAG_CURSOR_MASK << endl;
	#endif
	cursor_image = new QPixmap (SHIFT_REGION_BDIAG_CURSOR);
	cursor_image->setMask (QBitmap (SHIFT_REGION_BDIAG_CURSOR_MASK));
	Shift_Region_BDiag_Cursor = new QCursor (*cursor_image);
	delete cursor_image;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Crosshair_Cursor from " << CROSSHAIR_CURSOR << endl
		 << "                      and " << CROSSHAIR_CURSOR_MASK << endl;
	#endif
	Crosshair_Cursor = new QCursor
		(QBitmap (CROSSHAIR_CURSOR),
		 QBitmap (CROSSHAIR_CURSOR_MASK));

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Move_Horizontal_Cursor from "
			<< MOVE_HORIZONTAL_CURSOR << endl;
	#endif
	Move_Horizontal_Cursor = new QCursor (MOVE_HORIZONTAL_CURSOR);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_RED << "] from "
			<< GREATER_THAN_CURSOR_RED << endl
		 << "                             and "
		 	<< GREATER_THAN_CURSOR_MASK << endl;
	#endif
	cursor_mask = new QBitmap (GREATER_THAN_CURSOR_MASK);
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_RED);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_RED] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_GREEN << "] from "
			<< GREATER_THAN_CURSOR_GREEN << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_GREEN);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_GREEN] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_YELLOW << "] from "
			<< GREATER_THAN_CURSOR_YELLOW << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_YELLOW);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_YELLOW] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_BLUE << "] from "
			<< GREATER_THAN_CURSOR_BLUE << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_BLUE);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_BLUE] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_MAGENTA << "] from "
			<< GREATER_THAN_CURSOR_MAGENTA << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_MAGENTA);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_MAGENTA] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_CYAN << "] from "
			<< GREATER_THAN_CURSOR_CYAN << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_CYAN);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_CYAN] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Greater_Than_Cursors[" << SELECTED_WHITE << "] from "
			<< GREATER_THAN_CURSOR_WHITE << endl;
	#endif
	cursor_image = new QPixmap (GREATER_THAN_CURSOR_WHITE);
	cursor_image->setMask (*cursor_mask);
	Greater_Than_Cursors[SELECTED_WHITE] = new QCursor (*cursor_image);
	delete cursor_image;
	delete cursor_mask;

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_RED << "] from "
			<< LESS_THAN_CURSOR_RED << endl
		 << "                         and "
		 	<< LESS_THAN_CURSOR_MASK << endl;
	#endif
	cursor_mask = new QBitmap (LESS_THAN_CURSOR_MASK);
	cursor_image = new QPixmap (LESS_THAN_CURSOR_RED);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_RED] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_GREEN << "] from "
			<< LESS_THAN_CURSOR_GREEN << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_GREEN);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_GREEN] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_YELLOW << "] from "
			<< LESS_THAN_CURSOR_YELLOW << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_YELLOW);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_YELLOW] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_BLUE << "] from "
			<< LESS_THAN_CURSOR_BLUE << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_BLUE);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_BLUE] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_MAGENTA << "] from "
			<< LESS_THAN_CURSOR_MAGENTA << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_MAGENTA);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_MAGENTA] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_CYAN << "] from "
			<< LESS_THAN_CURSOR_CYAN << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_CYAN);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_CYAN] = new QCursor (*cursor_image);
	delete cursor_image;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Less_Than_Cursors[" << SELECTED_WHITE << "] from "
			<< LESS_THAN_CURSOR_WHITE << endl;
	#endif
	cursor_image = new QPixmap (LESS_THAN_CURSOR_WHITE);
	cursor_image->setMask (*cursor_mask);
	Less_Than_Cursors[SELECTED_WHITE] = new QCursor (*cursor_image);
	delete cursor_image;
	delete cursor_mask;

	//	Icons:

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Apply_Button_Icon from " << APPLY_BUTTON_ICON << endl;
	#endif
	Apply_Button_Icon = new QIcon (APPLY_BUTTON_ICON);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Defaults_Button_Icon from " << DEFAULTS_BUTTON_ICON << endl;
	#endif
	Defaults_Button_Icon = new QIcon (DEFAULTS_BUTTON_ICON);

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Reset_Button_Icon from " << RESET_BUTTON_ICON << endl;
	#endif
	Reset_Button_Icon = new QIcon (RESET_BUTTON_ICON);
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Reset_Button_Icon = " << (void*)Reset_Button_Icon << endl;
	#endif

	//	Colors:

	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    _DEFAULT_GRAPH_CANVAS_COLOR_ = "
			<< _DEFAULT_GRAPH_CANVAS_COLOR_ << endl
		 << "     Default_Graph_Canvas_Color  = "
		 	<< hex << Default_Graph_Canvas_Color << dec << endl
		 << "<<< HiView_Config" << endl;
	#endif
	}
}


HiView_Config::HiView_Config ()
{initialize ();}

}	//	namespace HiRISE
}	//	namespace UA
