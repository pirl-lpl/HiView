/*	Image_Info_Panel

HiROC CVS ID: $Id: Image_Info_Panel.cc,v 1.26 2014/08/05 17:58:08 stephens Exp $

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

#include	"Image_Info_Panel.hh"

#include	"Plastic_Image.hh"
#include	"Projection.hh"
#include	"Coordinate.hh"
#include	"PDS_Metadata.hh"

#include	<string>
#include<QDateTime>
#include	<QFrame>
#include	<QHBoxLayout>
#include	<QVBoxLayout>
#include	<QLabel>
#include	<QComboBox>
#include	<QtCore/qmath.h>
#include	<QSettings>
#include	<QScriptEngine>
#include	<QStringList>

using std::string;

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SLOTS				(1 << 1)
#define DEBUG_IMAGE_DATA		(1 << 2)
#define DEBUG_WORLD_DATA		(1 << 3)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
using std::clog;
#include	<iomanip>
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION

#include	<iostream>
namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Image_Info_Panel::ID =
		"UA::HiRISE::Image_Info_Panel ($Revision: 1.26 $ $Date: 2014/08/05 17:58:08 $)";
const Coordinate XY_10(1,0);
const Coordinate XY_00(0,0);

/*==============================================================================
	Defaults
*/

#define	DISPLAY_DATA_LABEL			"<b>Display - </b>"
#define IMAGE_DATA_LABEL			"<b>Source - </b>"
#define	SCRIPT_DATA_LABEL			"<b>Script - </b>"

#define SECTION_SPACING				15
#define ITEM_SPACING				5

#ifndef DEFAULT_COORDINATE_FORMAT
#define DEFAULT_COORDINATE_FORMAT	1
#endif

#ifndef	DEFAULT_SHOW_SCRIPT
#define	DEFAULT_SHOW_SCRIPT		true
#endif

#ifndef MAX_SCRIPT_SHOWN
#define	MAX_SCRIPT_SHOWN			450
#endif

#ifndef MAX_OUTPUT_SHOWN
#define MAX_OUTPUT_SHOWN			300
#endif

const char
	*RESTORE_LONGITUDE_FORMAT_KEY				= "Restore_Longitude_Format";
const char
	*RESTORE_LATITUDE_FORMAT_KEY				= "Restore_Latitude_Format";
const char
	*RESTORE_LONGITUDE_DIRECTION_KEY			= "Restore_Longitude_Direction";
const char
	*SHOW_SCRIPT_KEY							= "Scripts_Show_Script";
const char
	*CURRENT_SCRIPT_KEY							= "Scripts_Current_Script";
	
/*==============================================================================
	Constructors
*/
Image_Info_Panel::Image_Info_Panel
	(
	QWidget*	parent
	)
	:	QFrame (parent),
		Image_Values (true),
		Use_Avg_Rgb (false),
		Image_Bands (3),
		Display_Data (tr (DISPLAY_DATA_LABEL)),
		Image_Data (tr (IMAGE_DATA_LABEL)),
		Projector (NULL),
		Statistics (NULL),
		Exception_List (QList<int>())
		
		
{
    
degree_precision = 6; // guess, VALGRIND
    Longitude_Location = Projection::INVALID_VALUE;
    Latitude_Location = Projection::INVALID_VALUE;

QSettings settings;
//	Get Coordinate Display Format Settings
Longitude_Units = settings.value (RESTORE_LONGITUDE_FORMAT_KEY, DEFAULT_COORDINATE_FORMAT).toInt();
Longitude_Direction = settings.value (RESTORE_LONGITUDE_DIRECTION_KEY, DEFAULT_COORDINATE_FORMAT).toInt();
Latitude_Units =  settings.value (RESTORE_LATITUDE_FORMAT_KEY, DEFAULT_COORDINATE_FORMAT).toInt();

Script = settings.value(CURRENT_SCRIPT_KEY, "").toString();
Show_Script = settings.value(SHOW_SCRIPT_KEY, DEFAULT_SHOW_SCRIPT).toBool();

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Image_Info_Panel" << endl;
QPalette
	colors = palette ();
colors.setBrush (QPalette::Window, Qt::red);
setPalette (colors);
setAutoFillBackground (true);
#endif

QVBoxLayout
	*layout = new QVBoxLayout (this);
layout->setAlignment (Qt::AlignTop); 
layout->setContentsMargins (0, 0, 0, 0);
layout->setSpacing (0);
layout->setSizeConstraint (QLayout::SetFixedSize);

layout->addWidget (image_data_panel ());
layout->addWidget (Script_Panel = create_script_engine ());
//Only show Script_Panel if there is a script, and the user wants the panel open.
Script_Panel->setVisible(Show_Script && (Script != ""));

setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Image_Info_Panel sizeHint = " << sizeHint () << endl
	 << "<<< Image_Info_Panel" << endl;
#endif
}


QWidget*
Image_Info_Panel::image_data_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Image_Info_Panel::image_data_panel" << endl;
QPalette
	colors = palette ();
colors.setColor (QPalette::Window, Qt::green);
#endif
QWidget
	*panel = new QWidget (this);
QHBoxLayout
	*layout = new QHBoxLayout (panel);
//	Tight layout.
layout->setSpacing (0);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
int
	left, top, right, bottom;
layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "    layout margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl;
#endif
layout->setContentsMargins (ITEM_SPACING, 0, ITEM_SPACING, 0);

QLabel
	*label;

//	Data Source.
Data_Source = new QLabel (Display_Data);
Data_Source->setAlignment (Qt::AlignRight | Qt::AlignVCenter);

//		Label fixed height.
Data_Source_Size = Data_Source->sizeHint ();
Data_Source->setFixedHeight (Data_Source_Size.height ());

//		Label fixed width is max of all possible.
Data_Source->setText (Image_Data);
if (Data_Source_Size.rwidth () < Data_Source->sizeHint ().width ())
	Data_Source_Size.rwidth () = Data_Source->sizeHint ().width ();

Data_Source->setFixedWidth (Data_Source_Size.width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Data_Source->setAutoFillBackground (true);
Data_Source->setPalette (colors);
clog << "        Data_Source_Size = " << Data_Source_Size << endl
	 << "    Data_Source sizeHint = " << Data_Source->sizeHint () << endl;
#endif

if (Image_Values)
	Data_Source->setText (Image_Data);
else
	Data_Source->setText (Display_Data);
layout->addWidget (Data_Source);

//	Location values.
label = new QLabel (tr ("<b>Location: </b>"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
layout->addWidget (label);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    Location sizeHint = " << label->sizeHint () << endl;
#endif

Location_X = new QLabel ("000000");
Location_X->setFixedHeight (Data_Source_Size.height ());
Location_X->setFixedWidth (Location_X->sizeHint ().width ());
Location_X->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
Location_X->clear ();
layout->addWidget (Location_X);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Location_X->setAutoFillBackground (true);
Location_X->setPalette (colors);
clog << "    Location_X sizeHint = " << Location_X->sizeHint () << endl;
#endif

Annotation_X = new QLabel ("<b>x</b>");
Annotation_X->setFixedHeight (Data_Source_Size.height ());
Annotation_X->setFixedWidth (Annotation_X->sizeHint ().width ());
layout->addWidget (Annotation_X);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Annotation_X->setAutoFillBackground (true);
Annotation_X->setPalette (colors);
clog << "    Annotation_X sizeHint = " << Annotation_X->sizeHint () << endl;
#endif

layout->addSpacing (ITEM_SPACING);

Location_Y = new QLabel ("000000");
Location_Y->setFixedHeight (Data_Source_Size.height ());
Location_Y->setFixedWidth (Location_Y->sizeHint ().width ());
Location_Y->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
Location_Y->clear ();
layout->addWidget (Location_Y);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Location_Y->setAutoFillBackground (true);
Location_Y->setPalette (colors);
clog << "    Location_Y sizeHint = " << Location_Y->sizeHint () << endl;
#endif

Annotation_Y = new QLabel ("<b>y</b>");
Annotation_Y->setFixedHeight (Data_Source_Size.height ());
Annotation_Y->setFixedWidth (Annotation_Y->sizeHint ().width ());
layout->addWidget (Annotation_Y);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Annotation_Y->setAutoFillBackground (true);
Annotation_Y->setPalette (colors);
clog << "    Annotation_Y sizeHint = " << Annotation_Y->sizeHint () << endl;
#endif

//layout->addSpacing (SECTION_SPACING);
layout->addWidget (World_Location = world_location_panel ());
World_Location->setVisible (false);
layout->addSpacing (SECTION_SPACING);

//	Pixel values.
label = new QLabel (tr ("<b>Value: </b>"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
layout->addWidget (label);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    Value sizeHint = " << label->sizeHint () << endl;
#endif

const char
	*band_label[] = {"<font color='red'><b>r</b></font>", "<font color='green'><b>g</b></font>", "<font color='blue'><b>b</b></font>"};
for (int
		band = 0;
		band < 3;
		band++)
	{
	if (band)
		layout->addSpacing (ITEM_SPACING);

	Pixel_Value[band] = new QLabel ("00000");
	Pixel_Value[band]->setFixedHeight (Data_Source_Size.height ());
	Pixel_Value[band]->setFixedWidth (Pixel_Value[band]->sizeHint ().width ());
	Pixel_Value[band]->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
	Pixel_Value[band]->clear ();
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	Pixel_Value[band]->setAutoFillBackground (true);
	Pixel_Value[band]->setPalette (colors);
	clog << "    Pixel_Value[" << band << "] sizeHint = "
		<< Pixel_Value[band]->sizeHint () << endl;
	#endif
	layout->addWidget (Pixel_Value[band]);

	Pixel_Label[band] = new QLabel (band_label[band]);
	Pixel_Label[band]->setFixedHeight (Data_Source_Size.height ());
	Pixel_Label[band]->setFixedWidth (Pixel_Label[band]->sizeHint ().width ());
	layout->addWidget (Pixel_Label[band]);
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	Pixel_Label[band]->setAutoFillBackground (true);
	Pixel_Label[band]->setPalette (colors);
	clog << "    Pixel_Label[" << band << "] sizeHint = "
		<< Pixel_Label[band]->sizeHint () << endl;
	#endif
	}

layout->addSpacing (SECTION_SPACING);

//	Scale.
label = new QLabel (tr ("<b>Scale: </b>"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
layout->addWidget (label);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    Scale sizeHint = " << label->sizeHint () << endl;
#endif

Scaling = new QLabel;
Scaling->setFixedHeight (Data_Source_Size.height ());
//	Variable width set by value formatting in image_scale.
Scaling->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
Scaling->clear ();
layout->addWidget (Scaling);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Scaling->setAutoFillBackground (true);
Scaling->setPalette (colors);
clog << "    Scaling sizeHint = " << Scaling->sizeHint () << endl;
#endif

panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "    sizeHint = " << panel->sizeHint () << endl
	 << "        size = " << panel->size () << endl
	 << "     margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl
	 << "<<< Image_Info_Panel::image_data_panel" << endl;
#endif
return panel;
}


QWidget*
Image_Info_Panel::world_location_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Image_Info_Panel::world_location_panel" << endl;
QPalette
	colors = palette ();
colors.setColor (QPalette::Window, Qt::yellow);
#endif
QWidget
	*panel = new QWidget (this);
	
QHBoxLayout
	*layout;
	
QLabel
	*label;

//	Projection name.
layout = new QHBoxLayout (panel);
layout->setSpacing (0);
layout->setContentsMargins (0, 0, 0, 0);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
int
	left, top, right, bottom;
layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "    projection name layout margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl;
#endif

//	Locations:
layout->setSpacing (ITEM_SPACING);
layout->setContentsMargins (ITEM_SPACING, 0, ITEM_SPACING, 0);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "    locations layout margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl;
#endif

//	Longitude.
Longitude = new QLabel ("-00h 00m 00.000s");
Longitude->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
int
	location_value_width = Longitude->sizeHint ().width ();
Longitude->setFixedHeight (Data_Source_Size.height ());
Longitude->setFixedWidth (location_value_width);
Longitude->clear ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Longitude->setAutoFillBackground (true);
Longitude->setPalette (colors);
clog << "    Longitude sizeHint = "
		<< Longitude->sizeHint () << endl;
#endif
layout->addWidget (Longitude);

label = new QLabel (tr ("<b>lon.</b>"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    lon. label sizeHint = "
		<< label->sizeHint () << endl;
#endif
layout->addWidget (label);

layout->addSpacing (ITEM_SPACING);

//	Latitude.
Latitude = new QLabel ();
Latitude->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
Latitude->setFixedHeight (Data_Source_Size.height ());
Latitude->setFixedWidth (location_value_width);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Latitude->setAutoFillBackground (true);
Latitude->setPalette (colors);
clog << "    Latitude sizeHint = "
		<< Latitude->sizeHint () << endl;
#endif
layout->addWidget (Latitude);

label = new QLabel (tr ("planetocentric"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    label sizeHint = "
		<< label->sizeHint () << endl;
#endif
layout->addWidget (label);

label = new QLabel (tr ("<b>lat.</b>"));
label->setFixedHeight (Data_Source_Size.height ());
label->setFixedWidth (label->sizeHint ().width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
label->setAutoFillBackground (true);
label->setPalette (colors);
clog << "    lat. label sizeHint = "
		<< label->sizeHint () << endl;
#endif
layout->addWidget (label);

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "     locations margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl;
//panel_layout->getContentsMargins (&left, &top, &right, &bottom);
clog << "        panel sizeHint = " << panel->sizeHint () << endl
	 << "            panel size = " << panel->size () << endl
	 << "         panel margins = "
	 	<< left << "l, " << top << "t, " << right << "r, " << bottom << 'b'
		<< endl;
clog << "<<< Image_Info_Panel::world_location_panel" << endl;
#endif
return panel;
}


Image_Info_Panel::~Image_Info_Panel ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Image_Info_Panel" << endl;
#endif
}

/*==============================================================================
	Manipulators
*/
void
Image_Info_Panel::image_bands
	(
	int		bands
	)
{
if (bands < 1)
	bands = 0;
else
if (bands > 3)
	bands = 3;

Image_Bands = bands;
if (Image_Values)
	{
	for (int
			band = 0;
			band < 3;
			band++)
		{
		Pixel_Value[band]->setVisible (band < Image_Bands);
		Pixel_Label[band]->setVisible (band < Image_Bands);
		}
	if (Image_Bands == 1)
		Pixel_Label[0]->setVisible (false);
	}
}

/*==============================================================================
	Slots
*/
/*------------------------------------------------------------------------------
	Image Data
*/
void
Image_Info_Panel::image_values
	(
	bool	enabled
	)
{
if (enabled != Image_Values)
	{
	Image_Values = enabled;
	int
		bands = Image_Values ? Image_Bands : 3;

	Data_Source->setText
		(Image_Values ? Image_Data : Display_Data);
	for (int
			band = 0;
			band < 3;
			band++)
		{
		Pixel_Value[band]->setVisible (band < bands);
		Pixel_Label[band]->setVisible (band < bands);
		}
	if (bands == 1)
		Pixel_Label[0]->setVisible (false);

	//	Reset all the values.
	QPoint
		display_location (Display_Location),
		image_location (Image_Location);
	Display_Location =
	Image_Location = QPoint (-1, -1);
	cursor_location (display_location, image_location);

	Plastic_Image::Triplet
		display_value (Display_Value),
		image_value (Image_Value);
	Display_Value.Datum[0] =
	Display_Value.Datum[1] =
	Display_Value.Datum[2] =
	Image_Value.Datum[0] =
	Image_Value.Datum[1] =
	Image_Value.Datum[2] = static_cast<Plastic_Image::Pixel_Datum>(-2);
	pixel_value (display_value, image_value);

	QSizeF
		scaling (Image_Scale);
	Image_Scale = QSizeF ();
	image_scale (scaling);
	}
}


void
Image_Info_Panel::cursor_location
	(
	const QPoint&	display_location,
	const QPoint&	image_location
	)
{
#if ((DEBUG_SECTION) & (DEBUG_IMAGE_DATA | DEBUG_WORLD_DATA))
clog << ">>> Image_Info_Panel::cursor_location:" << endl
	 << "    display_location = " << display_location << endl
	 << "      image_location = " << image_location << endl;
#endif
if (Image_Values)
	{
	if (image_location != Image_Location)
		{
		int
			x = image_location.x (),
			y = image_location.y ();
            
            if (x < 0 || y < 0) return;
		Coordinate
			coordinate (Projection::INVALID_VALUE, Projection::INVALID_VALUE);

		if (x < 0)
			{
			Location_X->clear ();
			if(Evaluate_X) 
				Script_Output->clear();
//			Annotation_X->clear ();
			}
		else
			{
			Location_X->setNum (x);
			if(Evaluate_X) {
				Global_Object.setProperty("x_px", x);
				evaluate_script();
			}
//			Annotation_X->setText ("<b>x</b>");
			coordinate.X = x;
			}
		if (y < 0)
			{
			Location_Y->clear ();
			if(Evaluate_Y)
				Script_Output->clear();
//			Annotation_Y->clear ();
			}
		else
			{
			Location_Y->setNum (y);
			if(Evaluate_Y) {
				Global_Object.setProperty("y_px", y);
				evaluate_script();
			}
//			Annotation_Y->setText ("<b>y</b>");

			coordinate.Y = y;
			}
        
		if (! Projector ||
			! World_Location->isVisible ())
			location (coordinate);
		else
			location (Projector->to_world (coordinate));
		}
	}
else
	{
	if (display_location != Display_Location)
		{
		Location_X->setNum (display_location.x ());
		Location_Y->setNum (display_location.y ());
		}
	}
Display_Location = display_location;
Image_Location   = image_location;
#if ((DEBUG_SECTION) & (DEBUG_IMAGE_DATA | DEBUG_WORLD_DATA))
clog << "<<< Image_Info_Panel::cursor_location" << endl;
#endif
}


void
Image_Info_Panel::pixel_value
	(
	const Plastic_Image::Triplet&	display_value,
	const Plastic_Image::Triplet&	image_value
	)
{
	if(!Use_Avg_Rgb && Statistics != NULL) {
		//r
		Plastic_Image::Pixel_Datum value = Image_Values ? image_value.Datum[0] : display_value.Datum[0];
		if (value != (Image_Values ? Image_Value.Datum[0] : Display_Value.Datum[0]))
		{
			if (value == Plastic_Image::UNDEFINED_PIXEL_VALUE) {
				Pixel_Value[0]->clear ();
				if(Evaluate_R)
					Script_Output->clear();
			}
			else {
				Pixel_Value[0]->setNum (static_cast<int>(value));
				if(Evaluate_R) {
					Global_Object.setProperty("red", static_cast<int>(value));
					evaluate_script();
				}
			}
		}
		//g
		value = Image_Values ? image_value.Datum[1] : display_value.Datum[1];
		if (value != (Image_Values ? Image_Value.Datum[1] : Display_Value.Datum[1]))
		{
			if (value == Plastic_Image::UNDEFINED_PIXEL_VALUE) {
				Pixel_Value[1]->clear ();
				if(Evaluate_G)
					Script_Output->clear();
			}
			else {
				Pixel_Value[1]->setNum (static_cast<int>(value));
				if(Evaluate_G) {
					Global_Object.setProperty("green", static_cast<int>(value));
					evaluate_script();
				}
			}
		}
		//b
		value = Image_Values ? image_value.Datum[2] : display_value.Datum[2];
		if (value != (Image_Values ? Image_Value.Datum[2] : Display_Value.Datum[2]))
		{
			if (value == Plastic_Image::UNDEFINED_PIXEL_VALUE) {
				Pixel_Value[2]->clear ();
				if(Evaluate_B)
					Script_Output->clear();
			}
			else {
				Pixel_Value[2]->setNum (static_cast<int>(value));
				if(Evaluate_B) {
					Global_Object.setProperty("blue", static_cast<int>(value));
					evaluate_script();
				}
			}
		}
	}

	Display_Value = display_value;
	Image_Value   = image_value;
}

void Image_Info_Panel::use_avg_pixel_value(bool use) {
	Use_Avg_Rgb = use;
}

void
Image_Info_Panel::image_scale
	(
	const QSizeF&	scaling,
	int				/* band argument is not used */
	)
{
if (Image_Scale != scaling)
	{
	Image_Scale = scaling;
	double
		scale_x = scaling.width (),
		scale_y = scaling.height ();
	if (! Image_Values)
		{
		scale_x = 1.0 / scale_x;
		scale_y = 1.0 / scale_y;
		}
	QString
		report;
	report =
		QString ("%1")
		.arg (scale_x, 6, 'f', 3);
	if (scale_y != scale_x)
		report +=
			QString (" / %1")
			.arg (scale_y, 6, 'f', 3);

	Scaling->setText (report);
	Global_Object.setProperty("display.scale", scale_x);
	if(Evaluate_Scale) {
		evaluate_script();
	}
	}
}

/*------------------------------------------------------------------------------
	World Data
*/
void
Image_Info_Panel::projection
	(
	Projection*	projector
	)
{
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << ">>> Image_Info_Panel::projection: @ " << (void*)projector << endl;
#endif
if (Projector != projector)
	Projector  = projector;

bool
	visible = false;
if (Projector && ! Projector->is_identity ()) {
	#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
	clog << "    canonical_projection_name = "
		<< Projector->canonical_projection_name () << endl;
	#endif
/*	QString
		name ("<b>");
	name += Projector->canonical_projection_name ();
	name += " Projection:</b>";
	Projection_Name->setText (name);*/
	visible = true;
	//set precision for the degree and radian values
	double result = qAbs(Projector->to_world(XY_10).X - Projector->to_world(XY_00).X);
	
	if(result < 0.000001) {
		degree_precision = 11;
	}
	else {
		int i;
		for(i = 0; result < 1; ++i) {
			result = result * 10;
		}
		degree_precision = i + 5;
	}
}
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << "    World_Location->isVisible = "
		<< World_Location->isVisible () << endl
	 << "    setVisible = " << visible << endl;
#endif
if (World_Location->isVisible () != visible)
	World_Location->setVisible (visible);
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << "<<< Image_Info_Panel::projection" << endl;
#endif
}


void
Image_Info_Panel::longitude_units
	(
	int		units
	)
{
if (Longitude_Units != units) {
	Longitude_Units = units;
}
else
if (! Projection::is_invalid (Longitude_Location))
	{
	//	Refresh the displayed value.
	double
		value = Longitude_Location;
	Longitude_Location = Projection::INVALID_VALUE;
	longitude (value);
	}
}


void
Image_Info_Panel::latitude_units
	(
	int		units
	)
{
if (Latitude_Units != units) {
	Latitude_Units = units; 
}
else
if (! Projection::is_invalid (Latitude_Location))
	{
	//	Refresh the displayed value.
	double
		value = Latitude_Location;
	Latitude_Location = Projection::INVALID_VALUE;
	latitude (value);
	}
}


void
Image_Info_Panel::longitude_direction
	(
	int		direction
	)
{
if (Longitude_Direction != direction)
	Longitude_Direction = direction;
else
if (! Projection::is_invalid (Longitude_Location))
	{
	//	Refresh the displayed value.
	double
		value = Longitude_Location;
	Longitude_Location = Projection::INVALID_VALUE;
	longitude (value);
	}
}


void
Image_Info_Panel::location
	(
	const Coordinate&	coordinate
	)
{
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << ">>> Image_Info_Panel::location: " << coordinate << endl;
#endif
longitude (coordinate.X);
latitude (coordinate.Y);
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << "<<< Image_Info_Panel::location" << endl;
#endif
}


void
Image_Info_Panel::longitude
	(
	double	value
	)
{
if (Longitude_Location != value)
	{
	Longitude_Location = value;
	if (Projection::is_invalid (value))
		Longitude->clear ();
	else
		{
		if (Longitude_Direction == WEST)
			value = 360 - value;
		Longitude->setText (location_representation
			(value, Longitude_Units));
		}
	}
}


void
Image_Info_Panel::latitude
	(
	double	value
	)
{
if (Latitude_Location != value)
	{
	Latitude_Location = value;
	if (Projection::is_invalid (value))
		Latitude->clear ();
	else
		Latitude->setText (location_representation
			(value, Latitude_Units));
	}
}


QString
Image_Info_Panel::location_representation
	(
	double	value,
	int		units
	) const
{
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << ">>> Image_Info_Panel::location_representation: " << value
		<< " units " << units << endl;
#endif
QString
	representation;
switch (units)
	{
	case DEGREES:
		representation = QString ("%1")
			.arg (value, 16, 'f', degree_precision);
		break;
	case HMS:
		representation = Projection::hours_minutes_seconds (value);
		break;
	case RADIANS:
		value = Projection::to_radians (value);
		representation = QString ("%1")
			.arg (value, 16, 'f', degree_precision + 2);
	}
#if ((DEBUG_SECTION) & DEBUG_WORLD_DATA)
clog << "<<< Image_Info_Panel::location_representation: "
		<< representation << endl;
#endif
return representation;
}

/***************************************************************
 * Qt Script Code
 ***************************************************************/
void Image_Info_Panel::update_statistics(Stats *stats) {
	Statistics = stats;
	Use_Avg_Rgb = Use_Avg_Rgb;
}

void Image_Info_Panel::update_region_stats() {
	Pixel_Value[0]->setText (QString::number(Statistics->mean_value(0),'f',1));
	Pixel_Value[1]->setText (QString::number(Statistics->mean_value(1),'f',1));
	Pixel_Value[2]->setText (QString::number(Statistics->mean_value(2),'f',1));
	if(Evaluate_R || Evaluate_G || Evaluate_B)
		evaluate_script();
}
 
void Image_Info_Panel::add_exception(int exception) {
	if(Exception_List.indexOf(exception) == -1) {
		Exception_List.push_back(exception);
	}
}

void Image_Info_Panel::clear_exceptions() {
	Exception_List.clear();
}
 

void Image_Info_Panel::unset_properties() {
	initialize_script_values();
	QStringList::const_iterator end = Properties_List.end();
	for(QStringList::const_iterator i = Properties_List.begin(); i != end; ++i) {
		//to unset properties from last image, make them undefined
		Global_Object.setProperty(*i, Engine->undefinedValue());
	}
}

void Image_Info_Panel::array_to_string(idaeim::PVL::Array &array, QScriptValue &engine_array) {
	idaeim::PVL::Array::Depth_Iterator end = array.end_depth();
	int index = 0;
	for(idaeim::PVL::Array::Depth_Iterator i = array.begin_depth(); i != end; ++i) {
		if(i->is_Array())
			array_to_string(static_cast<idaeim::PVL::Array &>(*i), engine_array);
		else {
			engine_array.setProperty(index, QString::fromStdString(static_cast<std::string>(*i).c_str ()));
			++index;
		}
	}
}

//recursively iterate through metadata to get properties for engine.
void Image_Info_Panel::get_properties(idaeim::PVL::Aggregate &metadata) {	
	idaeim::PVL::Aggregate::Depth_Iterator end = metadata.end_depth();
	
	//iterate through the metadata
	for(idaeim::PVL::Aggregate::Depth_Iterator parameters = metadata.begin_depth(); parameters != end; ++parameters) {
		//if is_Aggregate() the parameter contains metadata, so make recursive call to this function
		if(parameters->is_Aggregate()) {
			get_properties(static_cast<idaeim::PVL::Aggregate &>(*parameters));
		}
		else {
			QString name = QString::fromStdString(parameters->name()).remove(':');
			idaeim::PVL::Value &value = parameters->value();
			//names containing '^' typically tell the location of a file, so they are unnecissary
			if(name.contains('^'))
				continue;
			//if array representation is used, convert to QScriptValue array
			else if(value.is_Array()) {
				QScriptValue engine_array = Engine->newArray();
				array_to_string(static_cast<idaeim::PVL::Array &>(value), engine_array);
				Properties_List.push_back(name);
				Global_Object.setProperty(name, engine_array);
			}
			//otherwise push name onto property list, and set property in engine
			else {
				Properties_List.push_back(name);

             if (value.is_Real())
             {
                 Global_Object.setProperty(name, static_cast<idaeim::PVL::Value::Real_type>(value));
             }
             else if (value.is_Integer())
             {
                /*
                 * Note, no method signature corresponding to
                 * Value::Integer_type (long long) or Value::Unsigned_Integer_type (unsigned long long)
                 */
                 if (value.is_signed())
                 {
                     Global_Object.setProperty(name, static_cast<int>(value));
                 }
                 else
                 {
                     Global_Object.setProperty(name, static_cast<uint>(value));
                 }
             }
             else if (value.is_Date_Time())
             {
                 // TODO check this
                 QString tstr = QString::fromStdString(static_cast<idaeim::PVL::Value::String_type>(value).c_str ());

                 Global_Object.setProperty(name, 1.0 * QDateTime::fromString(tstr, "yyyyMMdd'T'HHmmss").toMSecsSinceEpoch());
             }
             // TODO: check is_Identifier or is_Symbol ??
             else
             {
				Global_Object.setProperty(name, QString::fromStdString(static_cast<std::string>(value).c_str ()));
			 }
			}
		}
	}
}

//if there is a decent way to check if a property is being used, make sure to insert it here.
void Image_Info_Panel::set_property(const char * name, unsigned long long data) {
	Global_Object.setProperty(name, qsreal(data));
	if(Script.contains(name))
		evaluate_script();
}
void Image_Info_Panel::set_property(const char * name, unsigned int data){
	Global_Object.setProperty(name, data);
	if(Script.contains(name))
		evaluate_script();
}
void Image_Info_Panel::set_property_qsreal(const char * name, qsreal data){
	Global_Object.setProperty(name, data);
	if(Script.contains(name))
		evaluate_script();
}
void Image_Info_Panel::set_property_f(const char * name, double data){
	Global_Object.setProperty(name, data);
	if(Script.contains(name))
		evaluate_script();
}


void Image_Info_Panel::evaluate_script() {
	if((Evaluate_R || Evaluate_G || Evaluate_B) && Use_Avg_Rgb && Statistics != NULL) {
		QVector<Plastic_Image::Histogram*> &histograms = Statistics->histograms();
		int lower_limit = Statistics->lower_limit();
		int upper_limit = histograms[0]->size () - Statistics->upper_limit() - 1;
		int count = 0;
		qsreal result = 0;
		
		for(int i = lower_limit; i <= upper_limit; ++i) {
			QScriptValue val;
			bool skip = false;
			int sum = 0;
			for(int j = 0; j < Exception_List.size(); ++j) {
				if(i == Exception_List.at(i)) {
					skip = true;
					break;
				}
			}
			if(!skip) {
				if(Evaluate_R) {
					Global_Object.setProperty("red", i);
					sum = histograms[0]->at(i);
				}
				if(Evaluate_G) {
					if(histograms[1] != NULL) {
						Global_Object.setProperty("green", i);
						sum += histograms[1]->at(i);
					}
					else {
						Global_Object.setProperty("green", 0);
					}
				}
				if(Evaluate_B) {
					if(histograms[2] != NULL) {
						Global_Object.setProperty("blue", i);
						sum += histograms[2]->at(i);
					}
					else {
						Global_Object.setProperty("blue", 0);
					}
				}
				
				val = Engine->evaluate(Script);
				if(!val.isNumber()) {
					return;
				}
				count += sum;
				result += val.toNumber() * sum;
			}
		}
		result /= count;
		Script_Output->setNum(result);
	}
	else {
		Script_Output->setText(Engine->evaluate(Script).toString());
	}
}
 
void Image_Info_Panel::set_metadata(idaeim::PVL::Aggregate *metadata) {
	//unset properties set by last metadata
	unset_properties();
	//clear properties list
	Properties_List.clear();
	Exception_List.clear();
	//get new properties
	if(metadata != NULL) {
		get_properties(*metadata);
		
		idaeim::PVL::Parameter * param = metadata->find("CORE_NULL",false,0,idaeim::PVL::Parameter::ASSIGNMENT);
    	if(param != NULL) {
   			idaeim::PVL::Value &value = param->value();
  			if(value.is_Integer()) {
   				add_exception(static_cast<int>(value));
    		}
    	}
    	param = metadata->find("CORE_LOW_REPR_SATURATION",false,0,idaeim::PVL::Parameter::ASSIGNMENT);
    	if(param != NULL) {
   			idaeim::PVL::Value &value = param->value();
   			if(value.is_Integer()) {
   				add_exception(static_cast<int>(value));
   			}
   		}
    	param = metadata->find("CORE_LOW_INSTR_SATURATION",false,0,idaeim::PVL::Parameter::ASSIGNMENT);
    	if(param != NULL) {
    		idaeim::PVL::Value &value = param->value();
    		if(value.is_Integer()) {
    			add_exception(static_cast<int>(value));
    		}
    	}
    	param = metadata->find("CORE_HIGH_REPR_SATURATION",false,0,idaeim::PVL::Parameter::ASSIGNMENT);
    	if(param != NULL) {
    		idaeim::PVL::Value &value = param->value();
    		if(value.is_Integer()) {
    			add_exception(static_cast<int>(value));
    		}
    	}
    	param = metadata->find("CORE_HIGH_INSTR_SATURATION",false,0,idaeim::PVL::Parameter::ASSIGNMENT);
    	if(param != NULL) {
    		idaeim::PVL::Value &value = param->value();
    		if(value.is_Integer()) {
    			add_exception(static_cast<int>(value));
    		}
    	}
	}
	emit variables_updated(Properties_List);
	preparse_script(Script);
	Script_Value->setText(Script);
	evaluate_script();
}

void Image_Info_Panel::initialize_script_values() {
	// All script values always available to the user need to be initialized here 
	Global_Object.setProperty("x_px", 0);
 	Global_Object.setProperty("y_px", 0);
 	Global_Object.setProperty("red", 0);
 	Global_Object.setProperty("green", 0);
 	Global_Object.setProperty("blue", 0);
 	Global_Object.setProperty("scale", 0);
 	Global_Object.setProperty("region_area_px", 0);
 	Global_Object.setProperty("region_area_m", 0);
 	Global_Object.setProperty("region_width_px", 0);
 	Global_Object.setProperty("region_width_m", 0);
 	Global_Object.setProperty("region_height_px", 0);
 	Global_Object.setProperty("region_height_m", 0);
 	Global_Object.setProperty("distance_length_px", 0);
 	Global_Object.setProperty("distance_length_m", 0);
}

QWidget* Image_Info_Panel::create_script_engine() {
	//create the QScriptEngine
 	Engine = new QScriptEngine(this);
 	Global_Object = Engine->globalObject();
 	//set properties, which will act like variales in the script
 	initialize_script_values();

 	check_names();
	
 	//Layout
 	QWidget *panel = new QWidget(this);
 	QHBoxLayout *layout = new QHBoxLayout(panel);
 	layout->setSpacing (0);
 	layout->setContentsMargins(ITEM_SPACING, 0, ITEM_SPACING, 0);
 	
 	QLabel *label = new QLabel(SCRIPT_DATA_LABEL);
 	label->setFixedHeight(label->sizeHint().height());
 	label->setFixedWidth(Data_Source_Size.width ());
 	label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
 	layout->addWidget(label);
 	
 	Script_Value = new QLabel(Script);
 	Script_Value->setFixedHeight(Data_Source_Size.height ());
 	Script_Value->setMaximumWidth(MAX_SCRIPT_SHOWN);
 	Script_Value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
 	layout->addWidget(Script_Value);
 	
 	label = new QLabel(" = ");
 	label->setFixedHeight(Data_Source_Size.height ());
 	label->setFixedWidth(label->sizeHint().width ());
 	label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
 	layout->addWidget(label);
 	
 	Script_Output = new QLabel("Invalid Script");
 	Script_Output->setFixedHeight(Script_Output->sizeHint().height());
 	Script_Output->setMaximumWidth(MAX_OUTPUT_SHOWN);
 	Script_Output->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
 	layout->addWidget(Script_Output);
 	Script_Output->clear();
 	panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
 	return panel;
 }

//Makes metadata properties case insensitive
void Image_Info_Panel::preparse_script(QString &script) {
	QStringList::const_iterator end = Properties_List.end();
	for(QStringList::const_iterator i = Properties_List.begin(); i != end; ++i) {
		script.replace(*i, *i, Qt::CaseInsensitive);
	}
}

void Image_Info_Panel::check_names() {
	QList<QString> list = QList<QString>() <<"x_px"<<"y_px"<<"red"<<"green"<<"blue"<<"scale";
	QList<bool> *inList = parse_variable_names(&list);
	Evaluate_X = inList->at(0);
	Evaluate_Y = inList->at(1);
	Evaluate_R = inList->at(2);
	Evaluate_G = inList->at(3);
	Evaluate_B = inList->at(4);
	Evaluate_Scale = inList->at(5);
	std::clog << "x_px : " << Evaluate_X << std::endl;
	std::clog << "y_px : " << Evaluate_Y << std::endl;
	std::clog << "red : " << Evaluate_R << std::endl;
	std::clog << "green : " << Evaluate_G << std::endl;
	std::clog << "blue : " << Evaluate_B << std::endl;
	std::clog << "scale : " << Evaluate_Scale << std::endl;
}

QList<bool> *Image_Info_Panel::parse_variable_names(QList<QString> *list) {
	//It is not the variable if:
	// 1)the name is contained within another variable
	//		-Variable names can contain letters, digits, underscores, and dollar signs.
	// 2)If the a field with the same name is being used in a custom object
	//		-Will use '.' to access the field and ':' to declare a value in an initializer list
	//      -Dont include ':' in reject list incase someone uses ? operator
	// 3)If a function with the same name is being used
	//		-Will have '(' after the name
	// 4)If the name is used in a comment or a string
	
	QString script(Script);
	int loc = 0;
	while(loc < script.length()) {
		int min_type = 0;
		int min = script.indexOf('\"',loc);
		min = (min == -1) ? script.length() : min;
		int single_quote = script.indexOf('\'',loc);
		if(single_quote != -1 && single_quote < min) {
			min = single_quote;
			min_type = 1;
		}
		int line_comment = script.indexOf("//",loc);
		if(line_comment != -1 && line_comment < min) {
			min = line_comment;
			min_type = 2;
		}
		int multiline_comment = script.indexOf("/*",loc);
		if(multiline_comment != -1 && multiline_comment < min) {
			min = multiline_comment;
			min_type = 3;
		}
		if(min != script.length()) {
			int j;
			switch(min_type) {
				case 0:
					j = script.indexOf('\"',min+1);
					if(j != -1) {
						script.remove(min,j-min+1);
					}
					else {
						script.remove(min,script.length()-min);
					}
					break;
				case 1:
					j = script.indexOf('\'',min+1);
					if(min != -1) {
						script.remove(min,j-min+1);
					}
					else {
						script.remove(min,script.length()-min);
					}
					break;
				case 2:
					j = script.indexOf('\n',min+2);
					if(j != -1) {
						//leave in the new line
						script.remove(min,j-min);
					}
					else {
						script.remove(min,script.length()-min);
					}
					break;
				case 3:
					j = script.indexOf("*/",min+2);
					if(j != -1) {
						script.remove(min,j-min+2);
					}
					else {
						script.remove(min,script.length()-min);
					}
					break;
			}
		}
		loc = min;
	}
	
	//check for exceptions
	QList<bool> *inList = new QList<bool>();
	QString reject_chars_before_var = "_$.";
	QString reject_chars_after_var = "_$.(";

	for(int i = 0; i < list->size(); ++i) {
		int j = 0;
		int len = list->at(i).length();
		while((j = script.indexOf(list->at(i),j)) != -1) {
			if((j == 0 || !(script[j-1].isLetterOrNumber() || reject_chars_before_var.contains(script[j-1]))) && 
				((j + len) >= script.length() || !(script[j+len].isLetterOrNumber() || reject_chars_after_var.contains(script[j+len])))) {
				break;
			}
			++j;
		}
		
		inList->push_back(j > -1);
	}
	return inList;
}

void Image_Info_Panel::script_changed(const QString& script) {
	if(!Engine->toStringHandle(script).isValid()) {
		Script_Output->setText("Invalid Script");
	}
	else {
		Script = script;
		preparse_script(Script);
		Script_Value->setText(Script);
		check_names();
		evaluate_script();
	}
	//check if Script was changed to ""
	Script_Panel->setVisible(Show_Script && (Script != ""));
}

void Image_Info_Panel::show_script_changed(bool show_script) {
	Show_Script = show_script;
	Script_Panel->setVisible(Show_Script && (Script != ""));
}

}	//	namespace HiRISE
}	//	namespace UA
