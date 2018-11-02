/*	Navigator_Tool

HiROC CVS ID: $Id: Navigator_Tool.cc,v 1.77 2012/06/15 01:16:07 castalia Exp $

Copyright (C) 2009-2011  Arizona Board of Regents on behalf of the
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

#include	"Navigator_Tool.hh"

#include	"HiView_Config.hh"
#include	"Drawn_Line.hh"
#include	"Icon_Button.hh"
#include	"HiView_Utilities.hh"

#include	<QWidget>
#include	<QColor>
#include	<QSplitter>
#include	<QVBoxLayout>
#include	<QGridLayout>
#include	<QFrame>
#include	<QLabel>
#include	<QSpinBox>
#include	<QDoubleSpinBox>
#include	<QPushButton>
#include	<QComboBox>
#include	<QPalette>
#include	<QRubberBand>
#include	<QMouseEvent>
#include	<QPixmap>
#include	<QResizeEvent>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_INITIALIZE		(1 << 1)
#define DEBUG_MANIPULATORS		(1 << 2)
#define	DEBUG_HELPERS			(1 << 3)
#define DEBUG_SLOTS				(1 << 4)
#define DEBUG_SIGNALS			(1 << 5)
#define DEBUG_EVENTS			(1 << 6)
#define DEBUG_OVERVIEW			(1 << 7)
#define DEBUG_SPECIAL			(1 << 30)

#define DEBUG_DEFAULT			(DEBUG_ALL & ~DEBUG_EVENTS)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OVERVIEW
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
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
	Navigator_Tool::ID =
		"UA::HiRISE::Navigator_Tool ($Revision: 1.77 $ $Date: 2012/06/15 01:16:07 $)";


#ifndef NAVIGATOR_TOOL_IMAGE_MIN_WIDTH
#define NAVIGATOR_TOOL_IMAGE_MIN_WIDTH		250
#endif
#ifndef NAVIGATOR_TOOL_IMAGE_MIN_HEIGHT
#define NAVIGATOR_TOOL_IMAGE_MIN_HEIGHT		100
#endif
const QSize
	Navigator_Tool::IMAGE_MIN_SIZE
		(NAVIGATOR_TOOL_IMAGE_MIN_WIDTH, NAVIGATOR_TOOL_IMAGE_MIN_HEIGHT);

/*==============================================================================
	Application configuration parameters
*/
#define Panel_Frame_Style \
	HiView_Config::Panel_Frame_Style
#define Panel_Frame_Width \
	HiView_Config::Panel_Frame_Width
#define Label_Frame_Style \
	HiView_Config::Label_Frame_Style
#define Label_Frame_Width \
	HiView_Config::Label_Frame_Width
#define Label_Frame_Margin \
	HiView_Config::Label_Frame_Margin
#define Heading_Line_Weight \
	HiView_Config::Heading_Line_Weight

#define DISPLAY_BAND_NAMES \
	HiView_Config::DISPLAY_BAND_NAMES
#define DISPLAY_BAND_COLORS \
	HiView_Config::DISPLAY_BAND_COLORS

#define Shift_Region_Cursor \
	HiView_Config::Shift_Region_Cursor

#define Reset_Button_Icon \
	HiView_Config::Reset_Button_Icon

/*==============================================================================
	Defaults
*/
#ifndef NAVIGATOR_SHOW_ALL_REGION_ORIGINS
#define NAVIGATOR_SHOW_ALL_REGION_ORIGINS		false
#endif
bool
	Navigator_Tool::Default_Show_All_Region_Origins =
		NAVIGATOR_SHOW_ALL_REGION_ORIGINS;

#ifndef NAVIGATOR_SHOW_ALL_SCALINGS
#define NAVIGATOR_SHOW_ALL_SCALINGS				false
#endif
bool
	Navigator_Tool::Default_Show_All_Scalings =
		NAVIGATOR_SHOW_ALL_SCALINGS;

#ifndef NAVIGATOR_DEFAULT_SCALING_X_Y_DISTINCT
#define NAVIGATOR_DEFAULT_SCALING_X_Y_DISTINCT	false
#endif
bool
	Navigator_Tool::Default_Scaling_X_Y_Distinct =
		NAVIGATOR_DEFAULT_SCALING_X_Y_DISTINCT;

#ifndef NAVIGATOR_IMMEDIATE_MODE
#define NAVIGATOR_IMMEDIATE_MODE				true
#endif
bool
	Navigator_Tool::Default_Immediate_Mode	=
		NAVIGATOR_IMMEDIATE_MODE;

QErrorMessage
	*Navigator_Tool::Error_Message	= NULL;

/*==============================================================================
	Local constants
*/
enum Changes_Pending_Flags
	{
	REGION_ORIGIN	= (1 << 0),
	SCALING			= (1 << 1),
	BAND_MAPPING	= (1 << 2)
	};

enum Apply_When_Modes
	{
	DEFERRED_MODE	= 0,
	IMMEDIATE_MODE	= 1
	};

/*==============================================================================
	Constructors
*/
Navigator_Tool::Navigator_Tool
	(
	QWidget*	parent
	)
	:	QDockWidget (tr ("Navigator"), parent),
		Image_View (NULL),
		Band_Map_Reset_Button (NULL),
		Changes_Pending (0),
		Received_Knowledge (false),
		Region_Overlay (NULL),
		Region_Drag_Offset (-1, -1)
{
setObjectName ("Navigator_Tool");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << ">>> Navigator_Tool: " << object_pathname (this) << endl;
#endif
setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
QSplitter
	*split_panel = new QSplitter (Qt::Vertical, this);
split_panel->setObjectName (windowTitle () + " Splitter");
split_panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
split_panel->setChildrenCollapsible (true);

//	Enable keyboard focus on the tool.
setFocusPolicy (Qt::StrongFocus);

QWidget
	*image_widget,
	*info_widget;

//	Overview image.
image_widget = image_panel ();
image_widget->setObjectName (windowTitle () + " Image_Panel");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    image_panel sizeHint = " << image_widget->sizeHint () << endl;
#endif
split_panel->addWidget (image_widget);

//	Info panel.
info_widget = info_panel ();
info_widget->setObjectName (windowTitle () + " Info_Panel");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "     info_panel sizeHint = " << info_widget->sizeHint () << endl;
#endif
split_panel->addWidget (info_widget);

/*
QSize
	minimum_size (QDockWidget::sizeHint ());
minimum_size -= split_panel->sizeHint ();	//	Margins size.
minimum_size += info_widget->sizeHint ();
minimum_size.rheight () += IMAGE_MIN_SIZE.height ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    QDockWidget sizeHint = " << QDockWidget::sizeHint () << endl
	 << "    split_panel sizeHint = " << split_panel->sizeHint () << endl
	 << "          IMAGE_MIN_SIZE = " << IMAGE_MIN_SIZE << endl
	 << "            minimum_size = " << minimum_size << endl;
#endif
setMinimumSize (minimum_size);
*/

split_panel->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
image_widget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

QList<int>
	relative_sizes;
relative_sizes << 10000 << 10;
split_panel->setSizes (relative_sizes);

setWidget (split_panel);

//	Image loaded.
connect (Image_View,
	SIGNAL (image_loaded (bool)),
	SLOT (image_loaded (bool)));

//	Image pixel value.
connect (Image_View,
	SIGNAL (image_pixel_value
		(const Plastic_Image::Triplet&, const Plastic_Image::Triplet&)),
	SLOT (image_pixel_value
		(const Plastic_Image::Triplet&, const Plastic_Image::Triplet&)));

//	Image cursor location.
connect (Image_View,
	SIGNAL (image_cursor_moved (const QPoint&, const QPoint&)),
	SLOT (nav_image_cursor_moved (const QPoint&, const QPoint&)));

//	Overview image scrolling and scaling tracking.
connect (Image_View,
	SIGNAL (image_moved (const QPoint&, int)),
	SLOT (overview_image_moved (const QPoint&, int)));
connect (Image_View,
	SIGNAL (image_scaled (const QSizeF&, int)),
	SLOT (overview_image_scaled (const QSizeF&, int)));

//	Band mapping.
connect (this,
			SIGNAL (bands_mapped (const unsigned int*)),
		 Image_View,
			SLOT (map_bands (const unsigned int*)));

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "<<< Navigator_Tool" << endl;
#endif
}


Navigator_Tool::~Navigator_Tool ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Navigator_Tool: @ " << (void*)this << endl;
#endif
}

/*==============================================================================
	Accessors
*/
void
Navigator_Tool::image_name
	(
	const QString&	name
	)
{
if (name.isEmpty ())
	{
	Source_Name->clear ();
	Source_Name->setVisible (false);
	}
else
	{
	QString
		text (name);
	int
		index = text.lastIndexOf ('/');
	if (index >= 0)
		text.remove (0, ++index);
	text.insert (0, "<b>");	//	Bold text.
	Source_Name->setText (text);
	Source_Name->setVisible (true);
	}
}


QString
Navigator_Tool::image_name () const
{return Source_Name->text ().mid (3);}


void
Navigator_Tool::immediate_mode
	(
	bool	enabled
	)
{Apply_When->setCurrentIndex (enabled ? IMMEDIATE_MODE : DEFERRED_MODE);}


bool
Navigator_Tool::immediate_mode () const
{return Apply_When->currentIndex () == IMMEDIATE_MODE;}


QSize
Navigator_Tool::minimumSizeHint () const
{return minimumSize ();}

QSize
Navigator_Tool::sizeHint () const
{return minimumSize ();}

/*==============================================================================
	GUI elements
*/
QWidget*
Navigator_Tool::image_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << ">>> Navigator_Tool::image_panel" << endl;
#endif
Image_View = new Image_Viewer (this);
Image_View->setMinimumSize (IMAGE_MIN_SIZE);
//	Temporary size policy during setup.
Image_View->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

//	Region overlay.
Region_Overlay =
	new QRubberBand (QRubberBand::Rectangle, Image_View->image_display ());
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "<<< Navigator_Tool::image_panel" << endl;
#endif
return Image_View;
}


QWidget*
Navigator_Tool::info_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << ">>> Navigator_Tool::info_panel" << endl;
#endif
if (Reset_Button_Icon)
	{
	Band_Map_Reset_Button = new Icon_Button (*Reset_Button_Icon);
	Band_Map_Reset_Button->setToolTip (tr ("Reset to default band selections"));
	}

QFrame
	*panel = new QFrame;
QGridLayout
	*layout = new QGridLayout (panel);
panel->setFrameStyle (Panel_Frame_Style);
panel->setLineWidth (Panel_Frame_Width);
panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

QLabel
	*label;
int
	band,
	row	= -1,
	display_band_row,
	source_band_row,
	source_value_row,
	display_value_row,
	col,
	label_col		= 0,		//	Labels
	X_col			= 1,		//	X/Width/First values
	X_adjust_col	= 2,
	Y_col			= 3,		//	Y/Height/Second values
	Y_adjust_col	= 4,
	Z_col			= 5,		//	Third values
	Z_adjust_col	= 6,
	spacer_col		= 7,		//	Fill (quad left) space
/*
	HACK !!!
	The correct solution to determine the width of the numeric value
	fields to line up with the values in the edit fields of the spin
	boxes has not been found. This hack is based on trial-and-error to
	come up with values that seem to work; but they may not work on
	all platforms and with all styles.
*/
	value_width = QLabel ("999").sizeHint ().width () * 2;

layout->setColumnStretch (label_col, 1);
layout->setColumnStretch (X_col, 1);
layout->setColumnMinimumWidth (X_col, value_width);
layout->setColumnStretch (X_adjust_col, 1);
layout->setColumnMinimumWidth (X_adjust_col, 10);
layout->setColumnStretch (Y_col, 1);
layout->setColumnMinimumWidth (Y_col, value_width);
layout->setColumnStretch (Y_adjust_col, 1);
layout->setColumnMinimumWidth (Y_adjust_col, 10);
layout->setColumnStretch (Z_col, 1);
layout->setColumnMinimumWidth (Z_col, value_width);
layout->setColumnStretch (Z_adjust_col, 1);
layout->setColumnMinimumWidth (Z_adjust_col, 10);
layout->setColumnStretch (spacer_col, 100);
layout->setColumnMinimumWidth (spacer_col,
	(Band_Map_Reset_Button ? Band_Map_Reset_Button->iconSize ().width () : 1));

layout->setVerticalSpacing (1);

//------------------------------------------------------------------------------
//	Source Name
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, 0, 1, -1, Qt::AlignTop);
layout->setRowMinimumHeight (row, 10);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Image source name" << endl;
#endif
Source_Name = new QLabel;
Source_Name->setWordWrap (true);
Source_Name->setToolTip (tr ("Image source name"));
layout->addWidget (Source_Name,
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

//------------------------------------------------------------------------------
//	Bands
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, 0, 1, -1, Qt::AlignBottom);
layout->setRowMinimumHeight (row, 10);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Bands" << endl;
#endif
layout->addWidget (new QLabel (tr ("<b>Bands</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

display_band_row = ++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Band/Pixel labels -" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Display:")),
	display_band_row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Displayed image bands"));
source_band_row = ++row;
layout->addWidget (Source_Bands = new QLabel (tr ("Source:")),
	source_band_row, label_col, Qt::AlignRight | Qt::AlignVCenter);
Source_Bands->setToolTip (tr ("Source image bands"));

//------------------------------------------------------------------------------
//	Pixel Values
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (1),
	row, 0, 1, -1, Qt::AlignBottom);
layout->setRowMinimumHeight (row, 10);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Pixel Values" << endl;
#endif
layout->addWidget (new QLabel (tr ("<b>Pixel Values</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

display_value_row = ++row;
layout->addWidget (label = new QLabel (tr ("256 Display:")),
	display_value_row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Displayed image value at cursor location"));
source_value_row = ++row;
layout->addWidget (Source_Values = new QLabel (tr ("Source:")),
	source_value_row, label_col, Qt::AlignRight | Qt::AlignVCenter);
Source_Values->setToolTip (tr ("Source image value at cursor location"));

for (band = 0,
		col = X_col;
	 band < 3;
	 band++,
		col += 2)
	{
	//	Band names.
	label = new QLabel (tr (DISPLAY_BAND_NAMES[band]).prepend ("<b>"));
	label->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
	label->setFrameStyle (Label_Frame_Style);
	label->setLineWidth (Label_Frame_Width);
	label->setMargin (Label_Frame_Margin);
	label->setPalette (QPalette
		(QColor (DISPLAY_BAND_COLORS[band]).lighter ()));
	label->setAutoFillBackground (true);
	layout->addWidget (label,
		display_band_row, col, 1, 2, Qt::AlignVCenter);

	//	Source bands map.
	Image_Band[band] = new QSpinBox ();
	Image_Band[band]->setKeyboardTracking (false);
	Image_Band[band]->setAlignment (Qt::AlignRight);
	Image_Band[band]->setFrame (false);
	layout->addWidget (Image_Band[band],
		source_band_row, col, 1, 2, Qt::AlignVCenter);
	connect (Image_Band[band],
		SIGNAL (valueChanged (int)),
		SLOT (band_mapping_changed ()));

	//	Source values.
	Source_Value[band] = new QLabel;
	Source_Value[band]->setAlignment (Qt::AlignRight);
	layout->addWidget (Source_Value[band],
		source_value_row, col, Qt::AlignRight);

	//	Display values.
	Display_Value[band] = new QLabel;
	Display_Value[band]->setAlignment (Qt::AlignRight);
	layout->addWidget (Display_Value[band],
		display_value_row, col, Qt::AlignRight);
	}

//	Band map reset button
if (Band_Map_Reset_Button)
	{
	Band_Map_Reset_Button->setVisible (false);
	Band_Map_Reset_Button->setFocusPolicy (Qt::NoFocus);
	connect (Band_Map_Reset_Button,
		SIGNAL (clicked ()),
		SLOT (band_map_reset ()));
	layout->addWidget (Band_Map_Reset_Button,
		source_band_row, spacer_col, Qt::AlignLeft | Qt::AlignVCenter);
	}

//------------------------------------------------------------------------------
//	Geometry
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, label_col, 1, -1, Qt::AlignBottom);
layout->setRowMinimumHeight (row, 10);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Geometry" << endl;
#endif
layout->addWidget (new QLabel (tr ("<b>Geometry</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

//	Column names
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Column names -" << endl;
#endif
layout->addWidget (new QLabel (tr ("<b>X</b>")),
	row, X_col, Qt::AlignRight | Qt::AlignBottom);
layout->addWidget (new QLabel (tr ("<b>Y</b>")),
	row, Y_col, Qt::AlignRight | Qt::AlignBottom);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Column names underlines -" << endl;
#endif
layout->addWidget (new Drawn_Line,
	row, X_col);
layout->addWidget (new Drawn_Line,
	row, Y_col);

//	Source Size
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Source Size" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Source Size:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Source image size"));
layout->addWidget (Source_Size_X = new QLabel,
	row, X_col, Qt::AlignRight | Qt::AlignVCenter);
layout->addWidget (Source_Size_Y = new QLabel,
	row, Y_col, Qt::AlignRight | Qt::AlignVCenter);

//	Region Size
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Region Size" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Region Size:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Source image region size in the display viewport"));
layout->addWidget (Region_Size_X = new QLabel,
	row, X_col, Qt::AlignRight | Qt::AlignVCenter);
Region_Size_X->setAlignment (Qt::AlignRight);
layout->addWidget (Region_Size_Y = new QLabel,
	row, Y_col, Qt::AlignRight | Qt::AlignVCenter);
Region_Size_Y->setAlignment (Qt::AlignRight);

//	Region Origin
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << '-' << (row + 2) << ": Region Origin" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Region Origin:")),
	row, 0, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Source image location at the display viewport origin"));
Show_All_Region_Origins = Default_Show_All_Region_Origins;
for (band = 0;
	 band < 3;
	 band++,
		row++)
	{
	Region_Origin_X[band] = new QSpinBox;
	Region_Origin_X[band]->setKeyboardTracking (false);
	Region_Origin_X[band]->setAlignment (Qt::AlignRight);
	Region_Origin_X[band]->setAccelerated (true);
	Region_Origin_X[band]->setFrame (false);
	layout->addWidget (Region_Origin_X[band],
		row, X_col, 1, 2, Qt::AlignVCenter);
	connect (Region_Origin_X[band],
		SIGNAL (valueChanged (int)),
		SLOT (region_origin_changed ()));

	Region_Origin_Y[band] = new QSpinBox;
	Region_Origin_Y[band]->setKeyboardTracking (false);
	Region_Origin_Y[band]->setAlignment (Qt::AlignRight);
	Region_Origin_Y[band]->setAccelerated (true);
	Region_Origin_Y[band]->setFrame (false);
	layout->addWidget (Region_Origin_Y[band],
		row, Y_col, 1, 2, Qt::AlignVCenter);
	connect (Region_Origin_Y[band],
		SIGNAL (valueChanged (int)),
		SLOT (region_origin_changed ()));

	if (band)
		{
		Region_Origin_X[band]->setVisible (Show_All_Region_Origins);
		Region_Origin_Y[band]->setVisible (Show_All_Region_Origins);
		}
	}

//	Source Location
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Source Location" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Source Location:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Source image cursor location"));
layout->addWidget (Source_Location_X = new QLabel,
	row, X_col, Qt::AlignRight | Qt::AlignVCenter);
layout->addWidget (Source_Location_Y = new QLabel,
	row, Y_col, Qt::AlignRight | Qt::AlignVCenter);

//	Display Location
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Display Location" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Display Location:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Display viewport cursor location"));
layout->addWidget (Display_Location_X = new QLabel,
	row, X_col, Qt::AlignRight | Qt::AlignVCenter);
layout->addWidget (Display_Location_Y = new QLabel,
	row, Y_col, Qt::AlignRight | Qt::AlignVCenter);

//	Display Size
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Display Size" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Display Size:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Display viewport size"));
layout->addWidget (Display_Size_X = new QLabel,
	row, X_col, Qt::AlignRight | Qt::AlignVCenter);
layout->addWidget (Display_Size_Y = new QLabel,
	row, Y_col, Qt::AlignRight | Qt::AlignVCenter);

//	Spacing
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Gap" << endl;
#endif
layout->setRowMinimumHeight (row, 10);

//	Image Scale
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << '-' << (row + 2) << ": Image Scale" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Image Scale:")),
	row, 0, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Source image scaling factor"));
Show_All_Scalings = Default_Show_All_Scalings;
for (band = 0;
	 band < 3;
	 band++,
		row++)
	{
	Scaling_X[band] = new QDoubleSpinBox;
	Scaling_X[band]->setKeyboardTracking (false);
	Scaling_X[band]->setAlignment (Qt::AlignRight);
	Scaling_X[band]->setDecimals (3);
	Scaling_X[band]->setRange
		(Image_Viewer::min_scale (), Image_Viewer::max_scale ());
	Scaling_X[band]->setSingleStep (Image_Viewer::scaling_minor_increment ());
	Scaling_X[band]->setAccelerated (true);
	Scaling_X[band]->setFrame (false);
	layout->addWidget (Scaling_X[band],
		row, X_col, 1, 2, Qt::AlignVCenter);
	connect (Scaling_X[band],
		SIGNAL (valueChanged (double)),
		SLOT (image_scaling_changed ()));

	Scaling_Y[band] = new QDoubleSpinBox;
	Scaling_Y[band]->setKeyboardTracking (false);
	Scaling_Y[band]->setAlignment (Qt::AlignRight);
	Scaling_Y[band]->setDecimals (3);
	Scaling_Y[band]->setRange
		(Image_Viewer::min_scale (), Image_Viewer::max_scale ());
	Scaling_Y[band]->setSingleStep (Image_Viewer::scaling_minor_increment ());
	Scaling_Y[band]->setAccelerated (true);
	Scaling_Y[band]->setFrame (false);
	layout->addWidget (Scaling_Y[band],
		row, Y_col, 1, 2, Qt::AlignVCenter);
	connect (Scaling_Y[band],
		SIGNAL (valueChanged (double)),
		SLOT (image_scaling_changed ()));

	Scaling_X_Y_Distinct[band] = Default_Scaling_X_Y_Distinct;
	if (band)
		{
		Scaling_X[band]->setVisible (Show_All_Scalings);
		Scaling_Y[band]->setVisible
			(Show_All_Scalings && Scaling_X_Y_Distinct[band]);
		}
	else
		Scaling_Y[band]->setVisible (Scaling_X_Y_Distinct[band]);
	}

//------------------------------------------------------------------------------
//	Apply actions
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, label_col, 1, -1);
layout->setRowMinimumHeight (row, 10);

++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Apply actions" << endl;
#endif
QHBoxLayout
	*accept_actions_layout = new QHBoxLayout;

Apply = new QPushButton (tr ("Apply"));
Apply->setToolTip (tr ("Apply changed settings"));
Apply->setEnabled (false);
connect (Apply,
	SIGNAL (clicked (bool)),
	SLOT (apply ()));
accept_actions_layout->addWidget (Apply);

Apply_When = new QComboBox;
Apply_When->setToolTip (tr ("When to apply changed settings"));
Apply_When->setEditable (false);
Apply_When->addItem (tr ("Deferred"));
Apply_When->addItem (tr ("Immediate"));
Apply_When->setCurrentIndex
	(Default_Immediate_Mode ? IMMEDIATE_MODE : DEFERRED_MODE);
connect (Apply_When,
	SIGNAL (currentIndexChanged (int)),
	SLOT (apply_when_changed (int)));
accept_actions_layout->addWidget (Apply_When);

accept_actions_layout->addStretch (100);

layout->addLayout (accept_actions_layout,
	row, label_col, 1, -1);

//	Bottom space
++row;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    " << row << ": Spacer" << endl;
#endif
//layout->setRowMinimumHeight (row, 10);
layout->setRowStretch (row, 100);

#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "    sizeHint = " << panel->sizeHint () << endl
	 << "<<< Navigator_Tool::info_panel" << endl;
#endif
return panel;
}


void
Navigator_Tool::reset_info ()
{
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << ">>> Navigator_Tool::reset_info" << endl;
#endif
//	Prevent change signals during reset.
Received_Knowledge = true;

//	The Source_Name is set when the image is loaded.

int
	source_bands  = Image_View->image_bands (),
	source_values = 1 << Image_View->image_data_precision (),
	source_width  = Image_View->image_width (),
	source_height = Image_View->image_height ();

//	Source bands
Source_Bands->setVisible (source_bands);
Source_Bands->setText (QString::number (source_bands) + tr (" Source:"));

const unsigned int
	*band_map    = Image_View->image ()->source_band_map ();
Initial_Band_Map [0] = band_map[0];
Initial_Band_Map [1] = band_map[1];
Initial_Band_Map [2] = band_map[2];

//	Total source values
Source_Values->setText (QString::number (source_values) += tr (" Source:"));

//	Image Size
Source_Size_X->setNum (source_width);
Source_Size_Y->setNum (source_height);

//	Region Size is set by the displayed_image_region_resized slot.

for (int
		band = 0;
		band < 3;
	  ++band)
	{
	//	Region Origin
	if (band)
		{
		Region_Origin_X[band]->setVisible
			((band < source_bands) && Show_All_Region_Origins);
		Region_Origin_Y[band]->setVisible
			((band < source_bands) && Show_All_Region_Origins);
		}
	Region_Origin_X[band]->setRange (0, source_width - 1);
	Region_Origin_X[band]->setValue (0);
	Region_Origin_Y[band]->setRange (0, source_height - 1);
	Region_Origin_Y[band]->setValue (0);

	/*
		Scaling is set by a signal from the main Image_Viewer
		when the image is loaded.
	*/

	//	Image Band/Pixel Value
	Image_Band[band]->setVisible (source_bands);
	Image_Band[band]->setRange
		(HiView_Utilities::band_index_to_number (0),
		 HiView_Utilities::band_index_to_number (source_bands - 1));
	Image_Band[band]->setValue
		(HiView_Utilities::band_index_to_number (band_map[band]));
	}

Changes_Pending = 0;
Apply->setEnabled (false);
Received_Knowledge = false;
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "<<< Navigator_Tool::reset_info" << endl;
#endif
}


void
Navigator_Tool::refresh_band_numbers ()
{
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_SLOTS))
clog << ">>> Navigator_Tool::refresh_band_numbers" << endl;
#endif
int
	source_bands  = Image_View->image_bands ();
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_SLOTS))
clog << "    band range = " << HiView_Utilities::band_index_to_number (0)
	 	<< " - " << HiView_Utilities::band_index_to_number (source_bands - 1)
		<< endl;
#endif
const unsigned int
	*band_map = Image_View->image ()->source_band_map ();
bool
	blocked;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_SLOTS))
	clog << "    band " << band << "->" << band_map[band]
			<< " number "
			<< HiView_Utilities::band_index_to_number (band_map[band]) << endl;
	#endif
	blocked = Image_Band[band]->blockSignals (true);
	Image_Band[band]->setRange
		(HiView_Utilities::band_index_to_number (0),
		 HiView_Utilities::band_index_to_number (source_bands - 1));
	Image_Band[band]->setValue
		(HiView_Utilities::band_index_to_number (band_map[band]));
	Image_Band[band]->blockSignals (blocked);
	}
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_SLOTS))
clog << ">>> Navigator_Tool::refresh_band_numbers" << endl;
#endif
}


void
Navigator_Tool::show_all_region_origins
	(
	bool	enabled
	)
{
Show_All_Region_Origins = enabled;
int
	source_bands  = Image_View->image_bands ();
for (int
		band = 0;
		band < 3;
		band++)
	{
	if (band)
		{
		Region_Origin_X[band]->setVisible
			((band < source_bands) && Show_All_Region_Origins);
		Region_Origin_Y[band]->setVisible
			((band < source_bands) && Show_All_Region_Origins);
		}
	}
}


void
Navigator_Tool::show_all_scalings
	(
	bool	enabled
	)
{
Show_All_Scalings = enabled;
int
	source_bands  = Image_View->image_bands ();
for (int
		band = 0;
		band < 3;
		band++)
	{
	if (band)
		{
		Scaling_X[band]->setVisible
			((band < source_bands) && Show_All_Scalings);
		Scaling_Y[band]->setVisible
			((band < source_bands) &&
			Show_All_Scalings && Scaling_X_Y_Distinct[band]);
		}
	else
		Scaling_Y[band]->setVisible
			((band < source_bands) && Scaling_X_Y_Distinct[band]);
	}
}


void
Navigator_Tool::scaling_X_Y_distinct
	(
	bool	enabled,
	int		band 
	)
{
if (band < 3)
	{
	int
		source_bands  = Image_View->image_bands (),
		bands = 3;
	if (band < 0)
		band = 0;
	else
		bands = band + 1;
	while (band < bands)
		{
		Scaling_X_Y_Distinct[band] = enabled;
		Scaling_Y[band]->setVisible
			((band < source_bands) && Scaling_X_Y_Distinct[band]);
		++band;
		}
	}
}


void
Navigator_Tool::error_message
	(
	QErrorMessage*	dialog
	)
{Image_Viewer::error_message (Error_Message = dialog);}

/*==============================================================================
	Slots
*/
bool
Navigator_Tool::image
	(
	const Shared_Image&	source_image,
	const QString&		name
	)
{
#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_OVERVIEW))
clog << ">>> Navigator_Tool::image "
		<< object_pathname (this) << endl
	 << "    source_"  << *source_image << endl
	 << "    name = \"" << name << '"' << endl
	 << "    Image_Viewer size = " << Image_View->size () << endl
	 << "    Loading the image in the Image_Viewer ..." << endl;
#endif
QSize
	image_size;
if (! isVisible ())
	image_size = IMAGE_MIN_SIZE;
bool
	registered = Image_View->image (source_image, image_size);
if (registered)
	image_name (name);
#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_OVERVIEW))
clog << "<<< Navigator_Tool::image: " << boolalpha << registered << endl;
#endif
return registered;
}


void
Navigator_Tool::image_loaded
	(
	bool	successful
	)
{
#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_OVERVIEW))
clog << ">>> Navigator_Tool::image_loaded: " << boolalpha << successful << endl
	 << "    Source_Name = \"" << image_name () << '"' << endl;
#endif
if (! successful)
	image_name ("");
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "    Resetting the image info ..." << endl;
#endif
reset_info ();
#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_OVERVIEW))
clog << "<<< Navigator_Tool::image_loaded" << endl;
#endif
}


void
Navigator_Tool::image_pixel_value
	(
	const Plastic_Image::Triplet&	display_pixel,
	const Plastic_Image::Triplet&	image_pixel
	)
{
bool
	display_values = false;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (image_pixel.Datum[band] == Plastic_Image::UNDEFINED_PIXEL_VALUE)
		Source_Value[band]->clear ();
	else
		{
		Source_Value[band]
			->setNum (static_cast<int>(image_pixel.Datum[band]));
		display_values = true;
		}
	}
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (display_values &&
		display_pixel.Datum[band] != Plastic_Image::UNDEFINED_PIXEL_VALUE)
		Display_Value[band]
			->setNum (static_cast<int>(display_pixel.Datum[band]));
	else
		Display_Value[band]->clear ();
	}
}


void
Navigator_Tool::displayed_image_region_resized
	(
	const QSize&	region
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">>> Navigator_Tool::displayed_image_region_resized: " << region << endl
	 << "    current region size = " << Region_Size << endl
	 << "      Region_Size_X = " << Region_Size_X->text () << endl
	 << "      Region_Size_Y = " << Region_Size_Y->text () << endl;
#endif
int
	width  = region.width (),
	height = region.height ();
bool
	reset = false;
if (Region_Size.rwidth () != width)
	{
	Region_Size_X->setNum (width);
	Region_Size.rwidth () = width;
	reset = true;
	}
if (Region_Size.height () != height)
	{
	Region_Size_Y->setNum (height);
	Region_Size.rheight () = height;
	reset = true;
	}
if (reset)
	//	Reset the display region overlay rectangle.
	reset_region_overlay ();
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "<<< Navigator_Tool::displayed_image_region_resized" << endl;
#endif
}


void
Navigator_Tool::display_viewport_resized
	(
	const QSize&	viewport_size
	)
{
Display_Size_X->setNum (viewport_size.width ());
Display_Size_Y->setNum (viewport_size.height ());
}


void
Navigator_Tool::move_region
	(
	const QPoint&	origin,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">>> Navigator_Tool::move_region: "
		<< origin << ", " << band << 'b' << endl;
#endif
Received_Knowledge = true;
QPoint
	display_origin (origin);
int
	bands = 3;
if (band < 0)
	band = 0;
else
	bands = band + 1;
bool
	reset = false;
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "    Region_Origin values = "
	 	<< Region_Origin_X[band]->value () << "x, "
		<< Region_Origin_Y[band]->value () << 'y' << endl;
#endif

while (band < bands)
	{
	if (Region_Origin_X[band]->value () != display_origin.rx ())
		{
		Region_Origin_X[band]->setValue (display_origin.rx ());
		if (band == 0)
			reset = true;
		}
	if (Region_Origin_Y[band]->value () != display_origin.ry ())
		{
		Region_Origin_Y[band]->setValue (display_origin.ry ());
		if (band == 0)
			reset = true;
		}
	++band;
	}

#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "              reset = " << boolalpha << reset << endl
	 << "    Changes_Pending = " << Changes_Pending << endl;
#endif
if (reset ||
	Changes_Pending)
	//	Reset the display region overlay rectangle.
	reset_region_overlay ();

Received_Knowledge = false;

if (! (Changes_Pending &= ~REGION_ORIGIN))
	Apply->setEnabled (false);
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "<<< Navigator_Tool::move_region" << endl;
#endif
}


void
Navigator_Tool::scale_image
	(
	const QSizeF&	scaling,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">>> Navigator_Tool::scale_image: "
		<< scaling << ", " << band << 'b' << endl;
#endif
Received_Knowledge = true;
double
	scale_x = scaling.width (),
	scale_y = scaling.height ();
int
	bands = 3;
if (band < 0)
	band = 0;
else
	bands = band + 1;
bool
	reset = false;

while (band < bands)
	{
	if (Scaling_X[band]->value () != scale_x)
		{
		Scaling_X[band]->setValue (scale_x);
		if (band == 0)
			reset = true;
		}
	if (Scaling_Y[band]->value () != scale_y)
		{
		Scaling_Y[band]->setValue (scale_y);
		if (band == 0)
			reset = true;
		}
	++band;
	}

if (reset ||
	Changes_Pending)
	//	Reset the display region overlay rectangle.
	reset_region_overlay ();

Received_Knowledge = false;

if (! (Changes_Pending &= ~SCALING))
	Apply->setEnabled (false);
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "<<< Navigator_Tool::scale_image" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Private slots directly connected to GUI component signals.
*/
void
Navigator_Tool::region_origin_changed ()
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Navigator_Tool::region_origin_changed" << endl
	 << "    Received_Knowledge = " << boolalpha << Received_Knowledge << endl;
#endif
if (Received_Knowledge)
	return;

Changes_Pending |= REGION_ORIGIN;
Apply->setEnabled (true);
if (Apply_When->currentIndex () == IMMEDIATE_MODE)
	apply ();
}


void
Navigator_Tool::image_scaling_changed ()
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Navigator_Tool::image_scaling_changed" << endl
	 << "    Received_Knowledge = " << boolalpha << Received_Knowledge << endl;
#endif
if (Received_Knowledge)
	return;

Changes_Pending |= SCALING;
Apply->setEnabled (true);
if (Apply_When->currentIndex () == IMMEDIATE_MODE)
	apply ();
}


void
Navigator_Tool::band_mapping_changed ()
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Navigator_Tool::band_mapping_changed" << endl;
#endif
if (Band_Map_Reset_Button)
	Band_Map_Reset_Button->setVisible
		(Image_Band[0]->value ()
			!= HiView_Utilities::band_index_to_number (Initial_Band_Map[0]) ||
		 Image_Band[1]->value ()
		 	!= HiView_Utilities::band_index_to_number (Initial_Band_Map[1]) ||
		 Image_Band[2]->value ()
		 	!= HiView_Utilities::band_index_to_number (Initial_Band_Map[2]));

Changes_Pending |= BAND_MAPPING;
Apply->setEnabled (true);

if (! Received_Knowledge &&
	Apply_When->currentIndex () == IMMEDIATE_MODE)
	apply ();
}


void
Navigator_Tool::band_map_reset ()
{
if (Image_Band[0]->value ()
		!= HiView_Utilities::band_index_to_number (Initial_Band_Map[0]) ||
	Image_Band[1]->value ()
		!= HiView_Utilities::band_index_to_number (Initial_Band_Map[1]) ||
	Image_Band[2]->value ()
		!= HiView_Utilities::band_index_to_number (Initial_Band_Map[2]))
	{
	Received_Knowledge = true;
	Image_Band[0]->setValue
		(HiView_Utilities::band_index_to_number (Initial_Band_Map[0]));
	Image_Band[1]->setValue
		(HiView_Utilities::band_index_to_number (Initial_Band_Map[1]));
	Image_Band[2]->setValue
		(HiView_Utilities::band_index_to_number (Initial_Band_Map[2]));
	Received_Knowledge = false;
	if (Apply_When->currentIndex () == IMMEDIATE_MODE)
		apply ();
	}
}


void
Navigator_Tool::apply ()
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">>> Navigator_Tool::apply" << endl
	 << "    Changes_Pending = " << Changes_Pending << endl;
#endif
if (Changes_Pending)
	{
	int
		band;
	if (Changes_Pending & REGION_ORIGIN)
		{
		QPoint
			origin;
		if (Show_All_Region_Origins)
			{
			for (band = 0;
				 band < 3;
				 band++)
				{
				origin.rx () = Region_Origin_X[band]->value ();
				origin.ry () = Region_Origin_Y[band]->value ();
				//	>>> SIGNAL <<<
				#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
				clog << "^^^ Navigator_Tool::apply: emit region_moved" << endl
					 << "    origin = " << origin << endl
					 << "      band = " << band << endl;
				#endif
				emit region_moved (origin, band);
				}
			}
		else
			{
			origin.rx () = Region_Origin_X[0]->value ();
			origin.ry () = Region_Origin_Y[0]->value ();
			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
			clog << "^^^ Navigator_Tool::apply: emit region_moved" << endl
				 << "    origin = " << origin << endl
				 << "      band = -1" << endl;
			#endif
			emit region_moved (origin, -1);
			}
		}

	if (Changes_Pending & SCALING)
		{
		QSizeF
			scaling;
		QPoint
			center;
		if (Show_All_Scalings)
			{
			for (band = 0;
				 band < 3;
				 band++)
				{
				scaling.rwidth () = Scaling_X[band]->value ();
				if (Scaling_X_Y_Distinct[band])
					scaling.rheight () = Scaling_Y[band]->value ();
				else
					scaling.rheight () = scaling.rwidth ();

				//	Set the center point to the middle of the displayed region.
				center.rx () = Region_Origin_X[band]->value ()
					+ (Region_Size.rwidth () >> 1);
				center.ry () = Region_Origin_Y[band]->value ()
					+ (Region_Size.rheight () >> 1);

				//	>>> SIGNAL <<<
				#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
				clog << "^^^ Navigator_Tool::apply: emit image_scaled" << endl
					 << "    scaling = " << scaling << endl
					 << "     center = " << center << endl
					 << "       band = " << band << endl;
				#endif
				emit image_scaled (scaling, center, band);
				}
			}
		else
			{
			scaling.rwidth ()  = Scaling_X[0]->value ();
			if (Scaling_X_Y_Distinct[0])
				scaling.rheight () = Scaling_Y[0]->value ();
			else
				scaling.rheight () = scaling.rwidth ();

			//	Set the center point to the middle of the displayed region.
			center.rx () =
				Region_Origin_X[0]->value () + (Region_Size.rwidth () >> 1);
			center.ry () =
				Region_Origin_Y[0]->value () + (Region_Size.rheight () >> 1);

			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
			clog << "^^^ Navigator_Tool::apply: emit image_scaled" << endl
				 << "    scaling = " << scaling << endl
				 << "     center = " << center << endl
				 << "       band = -1" << endl;
			#endif
			emit image_scaled (scaling, center, -1);
			}
		}

	if (Changes_Pending & BAND_MAPPING)
		{
		unsigned int
			band_map[3];
		band_map[0] = HiView_Utilities::band_number_to_index
			(Image_Band[0]->value ());
		band_map[1] = HiView_Utilities::band_number_to_index
			(Image_Band[1]->value ());
		band_map[2] = HiView_Utilities::band_number_to_index
			(Image_Band[2]->value ());
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
		clog << "^^^ Navigator_Tool::apply: emit bands_mapped" << endl
			 << "    band_map = "
			 	<< band_map[0] << ", " << band_map[1] << ", " << band_map[2]
				<< endl;
		#endif
		emit bands_mapped (band_map);
		}

	Changes_Pending = 0;
	}
Apply->setEnabled (false);
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << "<<< Navigator_Tool::apply" << endl;
#endif
}


void
Navigator_Tool::apply_when_changed
	(
	int		mode
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Navigator_Tool::apply_when_changed: " << mode << endl;
#endif
if (mode == IMMEDIATE_MODE)
	apply ();
}


/*------------------------------------------------------------------------------
	Private slots for overview Image_Viewer signals.
*/
void
Navigator_Tool::overview_image_moved
	(
	//	Arguments unused.
	const QPoint&,
	int
	)
{reset_region_overlay ();}


void
Navigator_Tool::overview_image_scaled
	(
	//	Arguments unused.
	const QSizeF&,
	int
	)
{reset_region_overlay ();}


void
Navigator_Tool::reset_region_overlay ()
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> Navigator_Tool::reset_region_overlay" << endl;
#endif
QSizeF
	scaling (Image_View->image_scaling ());
QSize
	region_size (Region_Size);
region_size.rwidth ()  *= scaling.rwidth ();
region_size.rheight () *= scaling.rheight ();
QPointF
	region_origin (Region_Origin_X[0]->value (), Region_Origin_Y[0]->value ());
region_origin -= Image_View->displayed_image_origin ();
region_origin.rx () *= scaling.rwidth ();
region_origin.ry () *= scaling.rheight ();
Region_Overlay->move (round_down (region_origin));
Region_Overlay->resize (region_size);
if (! Region_Overlay->isVisible ())
	Region_Overlay->show ();
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "                  Region_Size = "
		<< Region_Size << endl
	 << "           Image_View scaling = "
	 	<< Image_View->image_scaling () << endl
	 << "         display image origin = "
	 	<< Region_Origin_X[0]->value () << "x, "
		<< Region_Origin_Y[0]->value () << 'y' << endl
	 << "    Image_View display origin = "
	 	<< Image_View->displayed_image_origin () << endl
	 << "        Region_Overlay origin = "
	 	<< round_down (region_origin) << endl
	 << "          Region_Overlay size = "
	 	<< region_size << endl
	 << "<<< Navigator_Tool::reset_region_overlay" << endl;
#endif
}


void
Navigator_Tool::image_cursor_moved
	(
	const QPoint&	display_position,
	const QPoint&	image_position
	)
{
//	Location reports.
if (display_position.x () < 0)
	{
	Display_Location_X->clear ();
	Display_Location_Y->clear ();
	}
else
	{
	Display_Location_X->setNum (display_position.x ());
	Display_Location_Y->setNum (display_position.y ());
	}

if (image_position.x () < 0)
	{
	Source_Location_X->clear ();
	Source_Location_Y->clear ();
	}
else
	{
	Source_Location_X->setNum (image_position.x ());
	Source_Location_Y->setNum (image_position.y ());
	}
}


void
Navigator_Tool::nav_image_cursor_moved
	(
	const QPoint&	display_position,
	const QPoint&	image_position
	)
{
image_cursor_moved (display_position, image_position);

QCursor
	*cursor = NULL;
if (Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE &&
	Region_Overlay->geometry ().contains (display_position))
	cursor = Shift_Region_Cursor;
Image_View->default_cursor (cursor);
}

/*==============================================================================
	Event Handlers
*/
void
Navigator_Tool::mousePressEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">>> Navigator_Tool::mousePressEvent:" << endl
	 << "    widget position = " << event->pos () << endl;
#endif
if (event->buttons () == Qt::LeftButton &&
	Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE)
	{
	QPoint
		display_position
			(Image_View->image_display ()->mapFromGlobal (event->globalPos ()));
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "     image display position = " << display_position << endl
		 << "    Region_Overlay geometry = "
		 	<< Region_Overlay->geometry () << endl;
	#endif
	if (Region_Overlay->geometry ().contains (display_position))
		{
		QPointF
			image_position
				(Image_View->image_display ()
					->map_display_to_image (display_position));
		#if ((DEBUG_SECTION) & DEBUG_EVENTS)
		clog << "             image position = " << image_position << endl
			 << "              region origin = "
			 	<< Region_Origin_X[0]->value () << "x, "
				<< Region_Origin_Y[0]->value () << 'y' << endl;
		#endif
		Region_Drag_Offset.rx () =
			static_cast<int>(image_position.rx ()
				- Region_Origin_X[0]->value ());
		Region_Drag_Offset.ry () =
			static_cast<int>(image_position.ry ()
				- Region_Origin_Y[0]->value ());

		if (Shift_Region_Cursor)
			Image_View->default_cursor (Shift_Region_Cursor);

		event->accept ();
		#if ((DEBUG_SECTION) & DEBUG_EVENTS)
		clog << "<<< Navigator_Tool::mousePressEvent" << endl;
		#endif
		return;
		}
	}
Region_Drag_Offset.rx () =
Region_Drag_Offset.ry () = -1;
event->ignore ();
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "<<< Navigator_Tool::mousePressEvent" << endl;
#endif
}


void
Navigator_Tool::mouseMoveEvent
	(
	QMouseEvent*	event
	)
{
if (Region_Drag_Offset.rx () >= 0 &&
	Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE)
	{
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << ">>> Navigator_Tool::mouseMoveEvent:" << endl
		 << "       widget position = " << event->pos () << endl
		 << "    Region_Drag_Offset = " << Region_Drag_Offset << endl;
	#endif
	QPoint
		position (round_down
			(Image_View->map_display_to_image
			(Image_View->image_display ()
				->mapFromGlobal (event->globalPos ()))));
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "      image position = " << position << endl;
	#endif
	position -= Region_Drag_Offset;

	//	>>>	SIGNAL <<<
	#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
	clog << "^^^ Navigator_Tool::mouseMoveEvent: emit region_moved" << endl
		 << "    origin = " << position << endl
		 << "      band = -1" << endl;
	#endif
	emit region_moved (position, -1);

	event->accept ();
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "<<< Navigator_Tool::mouseMoveEvent:" << endl;
	#endif
	}
else
	event->ignore ();
}


void
Navigator_Tool::mouseReleaseEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Navigator_Tool::mouseReleaseEvent" << endl;
#endif
Region_Drag_Offset.rx () =
Region_Drag_Offset.ry () = -1;

QCursor
	*cursor = NULL;
QPoint
	display_position
		(Image_View->image_display ()->mapFromGlobal (event->globalPos ()));
if (Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE &&
	Region_Overlay->geometry ().contains (display_position))
	cursor = Shift_Region_Cursor;
Image_View->default_cursor (cursor);

event->ignore ();
}


void
Navigator_Tool::mouseDoubleClickEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">>> Navigator_Tool::mouseDoubleClickEvent:" << endl
	 << "    widget position = " << event->pos () << endl;
#endif
if (event->buttons () == Qt::LeftButton &&
	Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE)
	{
	QPoint
		position
			(Image_View->image_display ()->mapFromGlobal (event->globalPos ()));
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "    image display position = " << position << endl
		 << "    image display region = "
		 	<< Image_View->image_display_region () << endl;
	#endif
	if (Image_View->image_display_region ().contains (position))
		{
		position = round_down
			(Image_View->image_display ()->map_display_to_image (position));
		#if ((DEBUG_SECTION) & DEBUG_EVENTS)
		clog << "      image display center = " << position << endl;
		#endif
		position.rx () -= (Region_Size.rwidth ()  >> 1);
		position.ry () -= (Region_Size.rheight () >> 1);
		#if ((DEBUG_SECTION) & DEBUG_EVENTS)
		clog << "      image display origin = " << position << endl;
		#endif

		if (Shift_Region_Cursor)
			Image_View->default_cursor (Shift_Region_Cursor);

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
		clog << "^^^ Navigator_Tool::mouseDoubleClickEvent: emit region_moved" << endl
			 << "    origin = " << position << endl
			 << "      band = -1" << endl;
		#endif
		emit region_moved (position, -1);

		Image_View->default_cursor ();

		event->accept ();
		#if ((DEBUG_SECTION) & DEBUG_EVENTS)
		clog << "<<< Navigator_Tool::mouseDoubleClickEvent" << endl;
		#endif
		return;
		}
	}
event->ignore ();
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "<<< Navigator_Tool::mouseDoubleClickEvent" << endl;
#endif
}


void
Navigator_Tool::resizeEvent
	(
	QResizeEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Navigator_Tool::resizeEvent: from "
		<< event->oldSize () << " to " << event->size () << endl;
#endif
Previous_Size = event->oldSize ();
QDockWidget::resizeEvent (event);
}


void
Navigator_Tool::contextMenuEvent
	(
	QContextMenuEvent* event
	)
{
//	>>> SIGNAL <<<
emit tool_context_menu_requested (this, event);
}


}	//	namespace HiRISE
}	//	namespace UA
