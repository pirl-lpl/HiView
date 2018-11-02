/*	HiView_Config

HiROC CVS ID: $Id: HiView_Config.hh,v 1.9 2011/08/18 00:40:35 castalia Exp $

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

#ifndef HiView_Config_hh
#define HiView_Config_hh

//	Qt forward references.
class QCursor;
class QIcon;


namespace UA
{
namespace HiRISE
{
/**	The <i>HiView_Conf</i> provides the HiView compile-time default
	configuration values and objects shared by various components.

	HiView_Config is expected to be specified as a base classs of
	those components sharing in the use of the application-wide
	values and objects.
*/
class HiView_Config
{
public:
/*==============================================================================
	Constants
*/
static const char* const
	CONFIG_ID;

/*------------------------------------------------------------------------------
	Data constraints.
*/
static const int
	MAX_SOURCE_DATA_PRECISION,
	MAX_DISPLAY_VALUE;

static const int
	PERCENT_DECIMAL_PLACES;

/*------------------------------------------------------------------------------
	Style.
*/
static int
	Panel_Frame_Style,
	Panel_Frame_Width,
	Label_Frame_Style,
	Label_Frame_Width,
	Label_Frame_Margin,
	Heading_Line_Weight;

/*------------------------------------------------------------------------------
	Cursors.
*/
static QCursor
	*Reticule_Cursor,
	*Shift_Cursor,
	*Shift_Region_Cursor,
	*Shift_Vertical_Cursor,
	*Shift_Region_Vertical_Cursor,
	*Shift_Horizontal_Cursor,
	*Shift_Region_Horizontal_Cursor,
	*Shift_Region_FDiag_Cursor,			//	"Forward"  TL-BR
	*Shift_Region_BDiag_Cursor,			//	"Backward" TR-BL
	*Scale_Cursor,
	*Crosshair_Cursor,
	*Move_Horizontal_Cursor,
	//	Bounds selection cursors, by SELECTED color (zero is NULL).
	*Greater_Than_Cursors[8],
	*Less_Than_Cursors[8];

/*------------------------------------------------------------------------------
	Icons.
*/
static QIcon
	*Apply_Button_Icon,
	*Defaults_Button_Icon,
	*Reset_Button_Icon;

/*------------------------------------------------------------------------------
	Display image band names and colors.
*/
static const char*
	DISPLAY_BAND_NAMES[3];
	
static const unsigned int
	DISPLAY_BAND_COLORS[3];

#define BLACK_COLOR	0xFF000000
#define WHITE_COLOR	0xFFFFFFFF

/*------------------------------------------------------------------------------
	Data graphs.
*/
enum Band_Selection
	{
	SELECTED_NONE		= 0,
	SELECTED_RED		= (1 << 0),
	SELECTED_GREEN		= (1 << 1),
	SELECTED_YELLOW		= SELECTED_RED |
						  SELECTED_GREEN,
	SELECTED_BLUE		= (1 << 2),
	SELECTED_MAGENTA	= SELECTED_RED |
						  SELECTED_BLUE,
	SELECTED_CYAN		= SELECTED_GREEN |
						  SELECTED_BLUE,
	SELECTED_WHITE		= SELECTED_RED |
						  SELECTED_GREEN |
						  SELECTED_BLUE
	};
#define SELECTED_ALL \
	HiView_Config::SELECTED_WHITE

static const int
	TRACKER_TRACK_MODE,
	TRACKER_DRAG_MODE,
	TRACKER_BOUND_MODE;

static const unsigned int
	Default_Graph_Canvas_Color;

/*==============================================================================
	Constructor
*/

static void initialize ();

private:
//	Nothing to construct.
HiView_Config ();

};

}	//	namespace HiRISE
}	//	namespace UA
#endif
