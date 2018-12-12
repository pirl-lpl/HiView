/*	Data_Mapper_Tool

HiROC CVS ID: $Id: Data_Mapper_Tool.cc,v 1.74 2013/09/09 22:11:55 stephens Exp $

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

#include	"Data_Mapper_Tool.hh"

#include	"HiView_Config.hh"
#include	"Drawn_Line.hh"
#include	"Icon_Button.hh"
#include	"Rotated_Label.hh"
#include	"Graph_Tracker.hh"
#include	"HiView_Utilities.hh"

//	Qt
#include	<QWidget>
#include	<QCursor>
#include	<QBitmap>
#include	<QHBoxLayout>
#include	<QGridLayout>
#include	<QFrame>
#include	<QLabel>
#include	<QPushButton>
#include	<QMenu>
#include	<QAction>
#include	<QSpinBox>
#include	<QDoubleSpinBox>
#include	<QColor>
#include	<QPen>
#include	<QPolygon>
#include	<QFileDialog>
#include	<QFile>
#include	<QTextStream>
#include	<QString>
#include	<QStringList>
#include	<QRegExp>
#include	<QImageWriter>
#include	<QErrorMessage>
#include	<QResizeEvent>
#include <QBrush>

//	Qwt
#include	<qwt_slider.h>
#include	<qwt_scale_draw.h>
#include	<qwt_plot.h>
#include	<qwt_plot_grid.h>
#include	<qwt_plot_curve.h>
#include	<qwt_symbol.h>
#include	<qwt_plot_marker.h>
#include	<qwt_plot_zoomer.h>
#include	<qwt_plot_panner.h>
#include <qwt_series_store.h>

#include	<iostream>
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::dec;
using std::hex;
using std::setw;
using std::setfill;
using std::endl;
#include	<stdexcept>
using std::invalid_argument;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define	DEBUG_MANIPULATORS		(1 << 2)
#define DEBUG_SLOTS				(1 << 3)
#define DEBUG_SIGNALS			(1 << 4)
#define DEBUG_CONTROL_PANEL		(1 << 5)
#define DEBUG_DATA_MAPS			(1 << 6)
#define DEBUG_BOUNDS			(1 << 7)
#define DEBUG_PLOT_PANEL		(1 << 9)
#define DEBUG_GRAPH_POSITION	(1 << 10)
#define DEBUG_DRAGGING			(1 << 11)
#define DEBUG_FILE				(1 << 12)
#define DEBUG_LAYOUT			(1 << 13)
#define DEBUG_SPECIAL			(1 << 30)

#define DEBUG_DEFAULT			(DEBUG_CONSTRUCTORS & \
								 DEBUG_CONTROL_PANEL)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
using std::dec;
using std::hex;
using std::setw;
using std::setfill;
#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Data_Mapper_Tool::ID =
		"UA::HiRISE::Data_Mapper_Tool ($Revision: 1.74 $ $Date: 2013/09/09 22:11:55 $)";


#ifndef DATA_MAPPER_TOOL_GRAPH_MIN_WIDTH
#define DATA_MAPPER_TOOL_GRAPH_MIN_WIDTH	350
#endif
#ifndef DATA_MAPPER_TOOL_GRAPH_MIN_HEIGHT
#define DATA_MAPPER_TOOL_GRAPH_MIN_HEIGHT	200
#endif
const QSize
	Data_Mapper_Tool::GRAPH_MIN_SIZE
		(DATA_MAPPER_TOOL_GRAPH_MIN_WIDTH, DATA_MAPPER_TOOL_GRAPH_MIN_HEIGHT);

#ifndef RED_BAND_SELECTION_ICON
#define RED_BAND_SELECTION_ICON		":/Images/Blank_Bead_Red-32.png"
#endif
#ifndef GREEN_BAND_SELECTION_ICON
#define GREEN_BAND_SELECTION_ICON	":/Images/Blank_Bead_Green-32.png"
#endif
#ifndef BLUE_BAND_SELECTION_ICON
#define BLUE_BAND_SELECTION_ICON	":/Images/Blank_Bead_Blue-32.png"
#endif
#ifndef ALL_BANDS_SELECTION_ICON
#define ALL_BANDS_SELECTION_ICON	":/Images/Blank_Bead_White-32.png"
#endif
#ifndef BAND_NOT_SELECTED_ICON
#define BAND_NOT_SELECTED_ICON		":/Images/Blank_Bead_Grey-32.png"
#endif
#ifndef SATURATION_ICON
#define SATURATION_ICON				":/Images/BW-32.png"
#endif
#ifndef CONTRAST_STRETCH_ICON
#define CONTRAST_STRETCH_ICON		":/Images/Contrast_Stretch.png"
#endif
#ifndef APPLY_ICON
#define APPLY_ICON					":/Images/redo_button.png"
#endif
#ifndef DEFAULTS_ICON
#define DEFAULTS_ICON				":/Images/defaults_button.png"
#endif

/*==============================================================================
	Application configuration parameters
*/
#define MAX_DISPLAY_VALUE \
	HiView_Config::MAX_DISPLAY_VALUE
#define MAX_SOURCE_DATA_PRECISION \
	HiView_Config::MAX_SOURCE_DATA_PRECISION
#define PERCENT_DECIMAL_PLACES \
	HiView_Config::PERCENT_DECIMAL_PLACES

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

#define Crosshair_Cursor \
	HiView_Config::Crosshair_Cursor
#define Move_Horizontal_Cursor \
	HiView_Config::Move_Horizontal_Cursor
#define Greater_Than_Cursors \
	HiView_Config::Greater_Than_Cursors
#define Less_Than_Cursors \
	HiView_Config::Less_Than_Cursors

#define Default_Graph_Canvas_Color \
	HiView_Config::Default_Graph_Canvas_Color

#define DISPLAY_BAND_NAMES \
	HiView_Config::DISPLAY_BAND_NAMES
#define DISPLAY_BAND_COLORS \
	HiView_Config::DISPLAY_BAND_COLORS

#define SELECTED_NONE \
	HiView_Config::SELECTED_NONE
#define SELECTED_RED \
	HiView_Config::SELECTED_RED
#define SELECTED_GREEN \
	HiView_Config::SELECTED_GREEN
#define SELECTED_BLUE \
	HiView_Config::SELECTED_BLUE

#define TRACKER_BOUND_MODE \
	HiView_Config::TRACKER_BOUND_MODE
#define TRACKER_TRACK_MODE \
	HiView_Config::TRACKER_TRACK_MODE
#define TRACKER_DRAG_MODE \
	HiView_Config::TRACKER_DRAG_MODE

/*==============================================================================
	Class data members
*/
QErrorMessage
	*Data_Mapper_Tool::Error_Message	= NULL;

/*==============================================================================
	Local constants
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
const char*
	BAND_SELECTION_ICONS[] =
		{
		RED_BAND_SELECTION_ICON,
		GREEN_BAND_SELECTION_ICON,
		BLUE_BAND_SELECTION_ICON,
		ALL_BANDS_SELECTION_ICON
		};

//	Plot items are painted in order of increasing depth; highest on top.
const double
	PLOT_DEPTH[] = {3, 2, 1};
#define MARKER_DEPTH	10

enum Operating_Modes
	{
	TRACKING_MODE	= 0,
	SATURATION_MODE	= (1 << 0),
	DRAGGING_MODE	= (1 << 1)
	};
}
#endif

/*==============================================================================
	Defaults
*/
#ifndef DEFAULT_DATA_MAPPER_GRAPH_VISIBLE
#define DEFAULT_DATA_MAPPER_GRAPH_VISIBLE	true
#endif

#ifndef DEFAULT_SELECTION_DISTANCE
#define DEFAULT_SELECTION_DISTANCE		5
#endif
int
	Data_Mapper_Tool::Default_Selection_Distance =
		DEFAULT_SELECTION_DISTANCE;

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

#ifndef DEFAULT_GRAPH_DATA_FORMAT
#define DEFAULT_GRAPH_DATA_FORMAT		CSV
#endif
#define _DEFAULT_GRAPH_DATA_FORMAT_		AS_STRING(DEFAULT_GRAPH_DATA_FORMAT)
QString
	Data_Mapper_Tool::Default_Graph_Data_Format =
		_DEFAULT_GRAPH_DATA_FORMAT_;

/*==============================================================================
	Constructor
*/
Data_Mapper_Tool::Data_Mapper_Tool
	(
	Data_Map**	maps,
	QWidget*	parent
	)
	:	QDockWidget (tr ("Data Mapper"), parent),
		Data_Maps (NULL),
		Max_Source_Value (0),
		Selected_Bands (SELECTED_ALL),
		Selection_Distance (Default_Selection_Distance),
		Signal_Changes (true),
		Upper_Limit (0),
		Lower_Limit (0),
		Operating_Mode (TRACKING_MODE),
		Graph (NULL),
		Load_File_Dialog (NULL),
		Save_File_Dialog (NULL),
		Graph_Data_Format (Default_Graph_Data_Format),
		Visible_Graph (true)
{
setObjectName ("Data_Mapper_Tool");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << ">>> Data_Mapper_Tool: " << object_pathname (this) << endl;
#endif
//setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

//	Enable keyboard focus on the tool.
setFocusPolicy (Qt::StrongFocus);

Functions[0] =
Functions[1] =
Functions[2] = NULL;
Nodes[0] =
Nodes[1] =
Nodes[2] = NULL;

//	Initialize the data maps.
reset_data_maps (maps);

QWidget
	*panel = new QWidget (this),
	*sub_panel;
panel->setObjectName (windowTitle () + " Panel");
panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
QHBoxLayout
	*layout = new QHBoxLayout (panel);
layout->setSpacing (5);

//	Graph.
sub_panel = graph_panel ();
sub_panel->setObjectName (windowTitle () + " Graph_Panel");
layout->addWidget (sub_panel);
layout->setStretchFactor (sub_panel, 100);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "      graph_panel sizeHint = " << sub_panel->sizeHint () << endl;
#endif

//	Controls.
sub_panel = control_panel ();
sub_panel->setObjectName (windowTitle () + " Control_Panel");
layout->addWidget (sub_panel);
layout->setStretchFactor (sub_panel, 0);
setWidget (panel);

//	Dynamic size control.
Minimum_Size = QDockWidget::sizeHint ();
//	Incremental width to subtract/add when graph area is hidden/visible.
Graph_Width_Increment = GRAPH_MIN_SIZE.width () + 5;
visible_graph (DEFAULT_DATA_MAPPER_GRAPH_VISIBLE);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "            GRAPH_MIN_SIZE = " << GRAPH_MIN_SIZE << endl
	 << "     Graph_Width_Increment = " << Graph_Width_Increment << endl
	 << "    control_panel sizeHint = " << sub_panel->sizeHint () << endl
	 << "            panel sizeHint = " << panel->sizeHint () << endl
	 << "         tool Minimum_Size = " << Minimum_Size << endl
	 << "<<< Data_Mapper_Tool" << endl;
#endif
}


Data_Mapper_Tool::~Data_Mapper_Tool ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Data_Mapper_Tool: @ " << (void*)this << endl;
#endif
//	N.B.: The Data_Maps are not locally owned.
}

/*==============================================================================
	Graphed Data
*/
#if ((DEBUG_SECTION) != DEBUG_OFF)
void
print_nodes
	(
	Function_Nodes**	nodes
	)
{
int
	band = -1;
while (++band < 3)
	{
	clog << "      band " << band << " -" << endl;
	nodes[band]->print (clog);
	}
}
#endif

/*------------------------------------------------------------------------------
	Function

	The Function class wraps a Data_Map in a QwtData interface.
*/
class Function
:	public QwtSeriesStore<QPointF>
{
public:

Function
	(
	Data_Mapper_Tool::Data_Map*	data_map
	)
	:	QwtSeriesStore (),
		Map (data_map)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< Data_Mapper_Tool::Function @ " << (void*)this
		<< ": Data_Map @ " << (void*)Map << endl;
#endif
}

/*..............................................................................
	Accessors
*/
Data_Mapper_Tool::Data_Map*
data_map ()
{return Map;}


void
data_map
	(
	Data_Mapper_Tool::Data_Map*	data_map
	)
{Map = data_map;}


int
max_x () const
{return Map->size () - 1;}

inline int max_y () const
{return MAX_DISPLAY_VALUE;}

/*..............................................................................
	QwtData implementation.
*/
//	N.B.: No copying; just return self.
virtual QwtSeriesStore* copy () const
{return const_cast<Function*>(this);}


virtual size_t size () const
{return Map->size ();}


double x (size_t index) const
{return index;}

double y (size_t index) const
{
if (index < (size_t)Map->size ())
	return Map->at ((int)index);
return -1;
}

virtual QPointF sample(size_t index) const
{
   return QPointF(x(index), y(index));
}

virtual void dataChanged() {}

virtual QRectF boundingRect () const
{
return QRectF
	(
	0,			//	Left.
	max_y (),	//	Top.
	max_x (),	//	Width.
	max_y ()	//	Height.
	);
}

/*..............................................................................
	Data.
*/
private:

Data_Mapper_Tool::Data_Map
	*Map;

};	//	Function class.

/*==============================================================================
	Accessors
*/
void
Data_Mapper_Tool::data_maps
	(
	Data_Map**	maps
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << ">>> Data_Mapper_Tool::data_maps: @ " << (void*)maps << endl;
#endif
if (maps == Data_Maps)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "    same maps" << endl
		 << "<<< Data_Mapper_Tool::data_maps" << endl;
	#endif
	return;
	}

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "    old Nodes -" << endl;
print_nodes (Nodes);
clog << "    Upper_Bound_Value[0] = "
		<< Upper_Bound_Value[0]->value () << endl
	 << "    Upper_Bound_Value[1] = "
		<< Upper_Bound_Value[1]->value () << endl
	 << "    Upper_Bound_Value[2] = "
		<< Upper_Bound_Value[2]->value () << endl
	 << "    Lower_Bound_Value[0] = "
		<< Lower_Bound_Value[0]->value () << endl
	 << "    Lower_Bound_Value[1] = "
		<< Lower_Bound_Value[1]->value () << endl
	 << "    Lower_Bound_Value[2] = "
		<< Lower_Bound_Value[2]->value () << endl;
#endif
//	Check for default mapping in the current maps.
bool
	changed = false;
QPolygon
	*nodes;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	nodes = Nodes[band]->nodes ();
	if (nodes->size () != 2 ||
		nodes->size () != maps[band]->size () ||
		nodes->at (0).x () != 0 ||
		nodes->at (0).y () != 0 ||
		nodes->at (1).x () != max_x () ||
		nodes->at (1).y () != max_y ())
		{
		changed = true;
		break;
		}
	}

//	Apply the new data maps.
reset_data_maps (maps);

bool
	blocked;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "    reset Functions/Nodes[" << band << "] data_map = "
			<< (void*)(Data_Maps[band]) << endl;
	#endif
	Functions[band]->data_map (Data_Maps[band]);
	//	The Data_Map will be reset to a linear interpolation.
	Nodes[band]->data_map (Data_Maps[band]);
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "    Nodes[" << band << "] max = "
			<< Nodes[band]->nodes ()->at (1) << endl;
	#endif

	if (changed)
		{
		/*	Reset the bounds values range.

			Note: The widget is blocked from signaling while the change is
			made to prevent a value changed signal leading to a Graph
			replot when all the Functions and Nodes have not yet been
			updated.
		*/
		blocked = Upper_Bound_Value[band]->blockSignals (true);
		Upper_Bound_Value[band]->setRange (1, max_x ());
		Upper_Bound_Value[band]->blockSignals (blocked);

		blocked = Lower_Bound_Value[band]->blockSignals (true);
		Lower_Bound_Value[band]->setRange (0, max_x () - 1);
		Lower_Bound_Value[band]->blockSignals (blocked);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "    new Nodes -" << endl;
print_nodes (Nodes);
#endif
if (changed)
	{
	//	Reset the bounds sliders range.
	blocked = Upper_Bound_Value_Slider->blockSignals (true);
	Upper_Bound_Value_Slider->setScale (0, max_x ());//, 1, 1);
	Upper_Bound_Value_Slider->blockSignals (blocked);

	blocked = Lower_Bound_Value_Slider->blockSignals (true);
	Lower_Bound_Value_Slider->setScale (0, max_x () - 1);//, 1, 1);
	Lower_Bound_Value_Slider->blockSignals (blocked);

	//	Reset the source data scale.
	Graph->setAutoReplot (false);
	Graph->setAxisScale (QwtPlot::xBottom, 0, Max_Source_Value);
	Graph->setAutoReplot (true);

	//	Remap the data.
	remap_data (REMAP_VALUES, true);
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "<<< Data_Mapper_Tool::data_maps" << endl;
#endif
}


void
Data_Mapper_Tool::reset_data_maps
	(
	Data_Map**	maps
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << ">>> Data_Mapper_Tool::reset_data_maps: @ " << (void*)maps << endl;
#endif
if (! maps ||
	! maps[0] ||
	! maps[0]->size () ||
	! maps[1] ||
	! maps[1]->size () ||
	! maps[2] ||
	! maps[2]->size ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't use empty data maps.";
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "!!! " << message.str () << endl
		 << "<<< Data_Mapper_Tool::reset_data_maps" << endl;
	#endif
	throw invalid_argument (message.str ());
	}

int
	map_size = maps[0]->size ();
if (map_size > (1 << MAX_SOURCE_DATA_PRECISION))
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid data maps have " << map_size << " entries,"
			" but the maximum is " << (1 << MAX_SOURCE_DATA_PRECISION)
			<< " (" << MAX_SOURCE_DATA_PRECISION << "-bit precision).";
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "!!! " << message.str () << endl
		 << "<<< Data_Mapper_Tool::reset_data_maps" << endl;
	#endif
	throw invalid_argument (message.str ());
	}
if (map_size != maps[1]->size () ||
	map_size != maps[2]->size ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid data maps have different sizes: "
			<< map_size << ", " << maps[1]->size () << ", and "
			<< maps[2]->size () << " entries.";
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
	clog << "!!! " << message.str () << endl
		 << "<<< Data_Mapper_Tool::reset_data_maps" << endl;
	#endif
	throw invalid_argument (message.str ());
	}

Data_Maps = maps;
Max_Source_Value = map_size - 1;
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "<<< Data_Mapper_Tool::reset_data_maps" << endl;
#endif
}


void
Data_Mapper_Tool::remap_data
	(
	int		bounds_selection,
	bool	unconditional
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << ">>> Data_Mapper_Tool::remap_data: "
		<< bounds_selection << ", " << boolalpha << unconditional << endl
	 << "    pre-remap Nodes:" << endl;
print_nodes (Nodes);
#endif
bool
	upper_changed = unconditional,
	lower_changed = unconditional;
Signal_Changes = false;
int
	band = 3;
if (bounds_selection == REMAP_VALUES)
	{
	QVector<int>
		upper_values (3, -1),
		lower_values (3, -1);
	while (band--)
		{
		if (Selected_Bands & (1 << band))
			{
			upper_values[band] = qMax (0, max_x () - Upper_Limit);
			upper_changed |= upper_bound_value (upper_values[band], band, true);
			lower_values[band] = qMin (max_x (), Lower_Limit);
			lower_changed |= lower_bound_value (lower_values[band], band, true);
			}
		}

	//	Graph update.
	Graph->replot ();

	if (upper_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::remap_data: "
				"emit upper_bound_values_changed "
				<< upper_values[0] << ", "
				<< upper_values[1] << ", "
				<< upper_values[2] << endl;
		#endif
		emit upper_bound_values_changed (upper_values);
		}
	if (lower_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::remap_data: "
				"emit lower_bound_values_changed "
				<< lower_values[0] << ", "
				<< lower_values[1] << ", "
				<< lower_values[2] << endl;
		#endif
		emit lower_bound_values_changed (lower_values);
		}
	}
else	//	REMAP_PERCENTS
	{
	QVector<double>
		upper_percents (3, -1.0),
		lower_percents (3, -1.0);
	while (band--)
		{
		if (Selected_Bands & (1 << band))
			{
			upper_percents[band] =
				Upper_Bound_Percent_Setting[band]->value ();
			upper_changed |=
				upper_bound_percent (upper_percents[band], band, true);
			lower_percents[band] =
				Lower_Bound_Percent_Setting[band]->value ();
			lower_changed |=
				lower_bound_percent (lower_percents[band], band, true);
			}
		}

	//	Graph update.
	Graph->replot ();

	if (upper_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::remap_data: "
				"emit upper_bound_percents_changed "
				<< upper_percents[0] << ", "
				<< upper_percents[1] << ", "
				<< upper_percents[2] << endl;
		#endif
		emit upper_bound_percents_changed (upper_percents);
		}
	if (lower_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::remap_data: "
				"emit lower_bound_percents_changed "
				<< lower_percents[0] << ", "
				<< lower_percents[1] << ", "
				<< lower_percents[2] << endl;
		#endif
		emit lower_bound_percents_changed (lower_percents);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "    post-remap Nodes:" << endl;
print_nodes (Nodes);
#endif

Signal_Changes = true;

if (upper_changed ||
	lower_changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::remap_data: emit data_maps_changed" << endl;
	#endif
	emit data_maps_changed (Data_Maps);
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "<<< Data_Mapper_Tool::remap_data" << endl;
#endif
}


int
Data_Mapper_Tool::max_y () const
{return MAX_DISPLAY_VALUE;}


int
Data_Mapper_Tool::default_selection_distance
	(
	int		distance
	)
{
int
	previous_distance = Default_Selection_Distance;
if (distance < 0)
	distance = 0;
Default_Selection_Distance = distance;
return previous_distance;
}


void
Data_Mapper_Tool::selection_distance
	(
	int		distance
	)
{
if (distance < 0)
	distance = 0;
Selection_Distance= distance;
}


void
Data_Mapper_Tool::band_map
	(
	const unsigned int*	bands
	)
{
for (int
		band = 0;
		band < 3;
		band++)
	Band_Mapping_Labels[band]->setText
		(QString::number
			(HiView_Utilities::band_index_to_number
				(Band_Map[band] = bands[band]))
			.prepend (tr ("<b>Band "))
			.append (" - &")
			.append (tr (DISPLAY_BAND_NAMES[band])));
}


void
Data_Mapper_Tool::refresh_band_numbers ()
{band_map (Band_Map);}


void
Data_Mapper_Tool::canvas_color
	(
	QRgb	color
	)
{
Graph->setCanvasBackground (QBrush(color));
Graph->replot ();
}


QList<QAction*>
Data_Mapper_Tool::band_selection_actions () const
{
QList<QAction*>
	actions;
actions
	<< Band_Selection_Actions[0]
	<< Band_Selection_Actions[1]
	<< Band_Selection_Actions[2]
	<< Band_Selection_Actions[3];	//	All bands.
return actions;
}


QList<QAction*>
Data_Mapper_Tool::presets_actions () const
{
QList<QAction*>
	actions;
actions
	<< Limited_Linear_Action
	<< Bounded_Linear_Action
	<< Unbounded_Linear_Action;
return actions;
}


QList<QAction*>
Data_Mapper_Tool::file_actions () const
{
QList<QAction*>
	actions;
actions
	<< Load_CSV_File_Action
	<< Load_Last_CSV_File_Action
	<< Save_File_Action
	<< Save_Last_CSV_File_Action;
return actions;
}

/*==============================================================================
	GUI elements
*/
QSize
Data_Mapper_Tool::minimumSizeHint () const
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">-< Data_Mapper_Tool::minimumSizeHint: " << Minimum_Size << endl;
#endif
return Minimum_Size;
}


QSize
Data_Mapper_Tool::sizeHint () const
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">-< Data_Mapper_Tool::sizeHint: " << Minimum_Size << endl;
#endif
return Minimum_Size;
}

/*==============================================================================
	Event Handlers
*/
void
Data_Mapper_Tool::resizeEvent
	(
	QResizeEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Data_Mapper_Tool::resizeEvent: from "
		<< event->oldSize () << " to " << event->size () << endl;
#endif
Previous_Size = event->oldSize ();
QDockWidget::resizeEvent (event);
}


void
Data_Mapper_Tool::contextMenuEvent
	(
	QContextMenuEvent* event
	)
{
//	>>> SIGNAL <<<
emit tool_context_menu_requested (this, event);
}

/*------------------------------------------------------------------------------
	Control Panel
*/
QWidget*
Data_Mapper_Tool::control_panel ()
{
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::control_panel" << endl;
#endif
QFrame
	*panel = new QFrame;
panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
panel->setFrameStyle (Panel_Frame_Style);
panel->setLineWidth (Panel_Frame_Width);

QVBoxLayout
	*panel_layout = new QVBoxLayout (panel);
panel_layout->setSpacing (0);

Drawn_Line
	*line;
QLabel
	*label;
QPushButton
	*button;
QHBoxLayout
	*horizontal_layout;
QGridLayout
	*grid_layout;

//	Band Selections:

//		Heading.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Drawn_Line" << endl;
#endif
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
panel_layout->addWidget (line, 1, Qt::AlignBottom);

#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Band Selections" << endl;
#endif
panel_layout->addWidget (label = new QLabel (tr ("<b>Band Selections</b>")),
	1, Qt::AlignLeft | Qt::AlignVCenter);
Band_Selection_Icon_Size = label->sizeHint ();
Band_Selection_Icon_Size.rwidth () =
	Band_Selection_Icon_Size.rheight () +=
		((Label_Frame_Margin + Label_Frame_Width) * 2);

//		Spacing.
panel_layout->addSpacing (5);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    space" << endl;
#endif

//		Band Selections.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    band selections" << endl;
#endif
int
	band;
grid_layout = new QGridLayout;
grid_layout->setSpacing (5);
grid_layout->setColumnStretch (0, 1);
grid_layout->setColumnStretch (1, 1);
grid_layout->setColumnStretch (2, 100);

char
	band_character[2] = {' ', 0};

for (band = 0;
	 band < 3;
	 band++)
	{
	Band_Selection_Actions[band] = new QAction
		(QString ("&") + tr (DISPLAY_BAND_NAMES[band]) + tr (" Band"), this);
	Band_Selection_Icons[band] = new QIcon (BAND_SELECTION_ICONS[band]);
	#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
	clog << "    Band_Selection_Icons[" << band << "] @ "
			<< (void*)Band_Selection_Icons[band] << endl;
	#endif
	Band_Selection_Actions[band]->setIcon (*Band_Selection_Icons[band]);
	Band_Selection_Actions[band]->setCheckable (true);
	Band_Selection_Actions[band]->setChecked (true);
	Band_Selection_Actions[band]->setToolTip
		(tr ("Disable mapping modification of the ") +
		 tr (DISPLAY_BAND_NAMES[band]) + tr (" band"));
	*band_character = *DISPLAY_BAND_NAMES[band];
	Band_Selection_Actions[band]->setShortcut
		(QString ("Ctrl+Shift+") + tr (band_character));
	#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
	clog << "      band_character " << band_character << ", shortcut = "
			<< Band_Selection_Actions[band]->shortcut ().toString () << endl;
	#endif
	connect (Band_Selection_Actions[band],
		SIGNAL (toggled (bool)),
		SLOT (select_band ()));

	Band_Selection_Buttons[band]
		= new Icon_Button (Band_Selection_Actions[band]->icon ());
	Band_Selection_Buttons[band]->setIconSize (Band_Selection_Icon_Size);
	Band_Selection_Buttons[band]->setCheckable (true);
	Band_Selection_Buttons[band]->setChecked (true);
	Band_Selection_Buttons[band]->setToolTip
		(Band_Selection_Actions[band]->toolTip ());
	connect
		(Band_Selection_Buttons[band], SIGNAL (toggled (bool)),
		 Band_Selection_Actions[band], SLOT (setChecked (bool)));
	grid_layout->addWidget (Band_Selection_Buttons[band],
		band, 0, Qt::AlignCenter);

	Band_Mapping_Labels[band] = new QLabel;
	Band_Mapping_Labels[band]->setBuddy (Band_Selection_Buttons[band]);
	Band_Mapping_Labels[band]->setToolTip
		(tr ("Source to display mapping"));
	Band_Mapping_Labels[band]->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
	Band_Mapping_Labels[band]->setFrameStyle (Label_Frame_Style);
	Band_Mapping_Labels[band]->setLineWidth (Label_Frame_Width);
	Band_Mapping_Labels[band]->setMargin (Label_Frame_Margin);
	Band_Mapping_Labels[band]->setPalette (QPalette
		(QColor (DISPLAY_BAND_COLORS[band]).lighter ()));
	Band_Mapping_Labels[band]->setAutoFillBackground (true);
	grid_layout->addWidget (Band_Mapping_Labels[band],
		band, 1, Qt::AlignVCenter);
	}
//	Extra select all bands action.
Band_Selection_Actions[3] = new QAction (tr ("&All Bands"), this);
Band_Selection_Icons[3] = new QIcon (BAND_SELECTION_ICONS[3]);
Band_Selection_Actions[3]->setIcon (*Band_Selection_Icons[3]);
Band_Selection_Actions[3]->setEnabled (false);
Band_Selection_Actions[3]->setToolTip
	(tr ("Enable mapping modification of all bands"));
Band_Selection_Actions[3]->setShortcut (tr ("Ctrl+Shift+A"));
connect (Band_Selection_Actions[3],
	SIGNAL (triggered ()),
	SLOT (select_band ()));

//		Initialize the Band_Mapping_Labels.
Band_Map[0] = 0;
Band_Map[1] = 1;
Band_Map[2] = 2;
band_map (Band_Map);

panel_layout->addLayout (grid_layout, 1);

Not_Selected_Icon = new QIcon (BAND_NOT_SELECTED_ICON);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Not_Selected_Icon @ " << (void*)Not_Selected_Icon << endl;
#endif

//		Spacing.
panel_layout->addSpacing (10);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    space" << endl;
#endif

//	Mappings:

//		Heading.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Drawn_Line" << endl;
#endif
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
panel_layout->addWidget (line, 1, Qt::AlignBottom);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Default Mapping" << endl;
#endif
panel_layout->addWidget (new QLabel (tr ("<b>Mapping</b>")),
	1, Qt::AlignLeft | Qt::AlignVCenter);

horizontal_layout = new QHBoxLayout;
horizontal_layout->setSpacing (5);
QMenu
	*menu = new QMenu;

//		Presets popup menu.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Presets menu" << endl;
#endif
button = new QPushButton (tr ("&Presets"));
menu = new QMenu;
//			Limited Linear mapping.
Limited_Linear_Action = new QAction (tr ("&Limited Linear"), this);
Limited_Linear_Action->setToolTip
	(tr ("Linear mapping between the valid data limits"));
Limited_Linear_Action->setShortcut (tr ("Ctrl+Alt+L"));
connect (Limited_Linear_Action,
	SIGNAL (triggered ()),
	SLOT (limited_linear_mapping ()));
menu->addAction (Limited_Linear_Action);
//			Bounded Linear mapping.
Bounded_Linear_Action = new QAction (tr ("&Bounded Linear"), this);
Bounded_Linear_Action->setToolTip
	(tr ("Linear mapping between the saturation bounds"));
Bounded_Linear_Action->setShortcut (tr ("Ctrl+Alt+B"));
connect (Bounded_Linear_Action,
	SIGNAL (triggered ()),
	SLOT (bounded_linear_mapping ()));
menu->addAction (Bounded_Linear_Action);
//			Full Linear mapping.
Unbounded_Linear_Action = new QAction (tr ("&Unbounded Linear"), this);
Unbounded_Linear_Action->setToolTip (tr ("Linear mapping of all data values"));
Unbounded_Linear_Action->setShortcut (tr ("Ctrl+Alt+U"));
connect (Unbounded_Linear_Action,
	SIGNAL (triggered ()),
	SLOT (linear_mapping ()));
menu->addAction (Unbounded_Linear_Action);
button->setMenu (menu);
horizontal_layout->addWidget (button,
	1, Qt::AlignLeft | Qt::AlignVCenter);

//		File popup menu.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    File menu" << endl;
#endif
File_Menu_Button = new QPushButton (tr ("F&ile"));
menu = new QMenu;
//			Load graph data.
Load_CSV_File_Action = new QAction (tr ("L&oad..."), this);
Load_CSV_File_Action->setToolTip
	(tr ("Load mappings of enabled bands from a file"));
Load_CSV_File_Action->setShortcut (tr ("Ctrl+Alt+O"));
connect (Load_CSV_File_Action,
	SIGNAL (triggered ()),
	SLOT (load ()));
menu->addAction (Load_CSV_File_Action);
//			Load graph data from last file.
Load_Last_CSV_File_Action = new QAction (tr ("Load &from Last"), this);
Load_Last_CSV_File_Action->setShortcut (tr ("Ctrl+Alt+Shift+O"));
Load_Last_CSV_File_Action->setEnabled (false);	//	Starts out disabled.
connect (Load_Last_CSV_File_Action,
	SIGNAL (triggered ()),
	SLOT (last_file_action ()));
menu->addAction (Load_Last_CSV_File_Action);
//			Save graph data or image.
Save_File_Action = new QAction (tr ("&Save..."), this);
Save_File_Action->setToolTip (tr ("Save graph mappings or image to a file"));
Save_File_Action->setShortcut (tr ("Ctrl+Alt+S"));
connect (Save_File_Action,
	SIGNAL (triggered ()),
	SLOT (save ()));
menu->addAction (Save_File_Action);
//			Save graph data to last file.
Save_Last_CSV_File_Action = new QAction (tr ("Save &to Last"), this);
Save_Last_CSV_File_Action->setShortcut (tr ("Ctrl+Alt+Shift+S"));
Save_Last_CSV_File_Action->setEnabled (false);	//	Starts out disabled.
connect (Save_Last_CSV_File_Action,
	SIGNAL (triggered ()),
	SLOT (last_file_action ()));
menu->addAction (Save_Last_CSV_File_Action);

File_Menu_Button->setMenu (menu);
horizontal_layout->addWidget (File_Menu_Button,
	1, Qt::AlignLeft | Qt::AlignVCenter);

horizontal_layout->addStretch (100);
panel_layout->addLayout (horizontal_layout, 1);

//		Spacing.
panel_layout->addSpacing (10);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    space" << endl;
#endif

//	Saturation Bounds:

//		Heading.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Drawn_Line" << endl;
#endif
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
panel_layout->addWidget (line, 1, Qt::AlignBottom);

#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Saturation Bounds" << endl;
#endif
horizontal_layout = new QHBoxLayout;
horizontal_layout->setSpacing (5);

//		Icon button.
Bounds_Icon = new QIcon (SATURATION_ICON);
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Bounds_Icon @ " << (void*)Bounds_Icon << endl;
#endif
Bounds_Button = new Icon_Button (*Not_Selected_Icon);
Bounds_Button->setToolTip (tr ("Saturation bounds graph manipulation enable"));
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Bounds_Button @ " << (void*)Bounds_Button << endl;
#endif
Bounds_Button->setShortcut (tr ("Ctrl+Shift+T"));
Bounds_Button->setIconSize (Band_Selection_Icon_Size);
Bounds_Button->setCheckable (true);
Bounds_Button->setChecked (false);
connect (Bounds_Button,
	SIGNAL (toggled (bool)),
	SLOT (saturation_controls (bool)));
horizontal_layout->addWidget (Bounds_Button,
	1, Qt::AlignLeft | Qt::AlignVCenter);
label = new QLabel (tr ("<b>Sa&turation Bounds</b>"));
horizontal_layout->addWidget (label,
	1, Qt::AlignLeft | Qt::AlignVCenter);
label->setBuddy (Bounds_Button);

//		Default Contrast Stretch.
Upper_Default_Contrast_Stretch << 0.0 << 0.0 << 0.0;
Lower_Default_Contrast_Stretch << 0.0 << 0.0 << 0.0;

Default_Contrast_Stretch_Action = new QAction
	(tr ("Default &Constrast Stretch"), this);
Default_Contrast_Stretch_Action->setToolTip
	(tr ("Apply default contrast stretch percentages"));
Default_Contrast_Stretch_Action->setShortcut (tr ("Ctrl+Alt+C"));
Default_Contrast_Stretch_Action->setIcon (QIcon (CONTRAST_STRETCH_ICON));
connect (Default_Contrast_Stretch_Action,
	SIGNAL (triggered ()),
	SLOT (default_contrast_stretch ()));

Icon_Button
	*icon_button;
icon_button = new Icon_Button (Default_Contrast_Stretch_Action->icon ());
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    default contrast stretch button @ " << (void*)icon_button << endl;
#endif
icon_button->setIconSize (Band_Selection_Icon_Size);
icon_button->setToolTip (Default_Contrast_Stretch_Action->toolTip ());
connect
	(icon_button, SIGNAL (clicked ()),
	 Default_Contrast_Stretch_Action, SLOT (trigger ()));
horizontal_layout->addWidget (icon_button,
	1, Qt::AlignLeft | Qt::AlignVCenter);


Restore_Original_Contrast_Stretch_Action = new QAction
	(tr ("Restore &Original Contrast Stretch"), this);
Restore_Original_Contrast_Stretch_Action->setToolTip
	(tr ("Restore the original contrast stretch percentages"));
Restore_Original_Contrast_Stretch_Action->setShortcut (tr ("Ctrl+Alt+R"));
Restore_Original_Contrast_Stretch_Action->setIcon (QIcon (DEFAULTS_ICON));
connect (Restore_Original_Contrast_Stretch_Action,
	SIGNAL (triggered ()),
	SLOT (restore_original_contrast_stretch()));
icon_button = new Icon_Button (Restore_Original_Contrast_Stretch_Action->icon());
icon_button->setIconSize (Band_Selection_Icon_Size);
icon_button->setToolTip (Restore_Original_Contrast_Stretch_Action->toolTip ());
connect (icon_button, SIGNAL (clicked()),
	Restore_Original_Contrast_Stretch_Action, SLOT (trigger()));
horizontal_layout->addWidget (icon_button,
	1, Qt::AlignLeft | Qt::AlignVCenter);

horizontal_layout->addStretch (100);
panel_layout->addLayout (horizontal_layout, 1);
//		Bounds.
QWidget
	*saturation_controls = new QWidget;
int
	row = -1,
	headings_top_row		= 0,
	headings_bottom_row		= headings_top_row + 1,
	headings_underline_row	= headings_bottom_row + 1,
	first_upper_row			= headings_underline_row + 1,
	divider_row				= first_upper_row + 3,
	first_lower_row			= divider_row + 1,
	labels_col				= 0,
	labels_line_col			= labels_col + 1,
	values_col				= labels_line_col + 1,
	values_slider_col		= values_col + 1,
	divider_col				= values_slider_col + 1,
	percents_slider_col		= divider_col + 1,
	percents_setting_col	= percents_slider_col + 1,
	percents_actual_col		= percents_setting_col + 1,
	padding_col				= percents_actual_col + 1;

#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    Upper_Bound" << endl;
#endif
grid_layout = new QGridLayout (saturation_controls);
grid_layout->setColumnStretch (labels_col, 1);
grid_layout->setColumnStretch (labels_line_col, 1);
grid_layout->setColumnStretch (values_col, 1);
grid_layout->setColumnStretch (values_slider_col, 1);
grid_layout->setColumnStretch (divider_col, 1);
grid_layout->setColumnStretch (percents_slider_col, 1);
grid_layout->setColumnStretch (percents_setting_col, 1);
grid_layout->setColumnStretch (percents_actual_col, 1);
grid_layout->setColumnStretch (padding_col, 100);
grid_layout->setHorizontalSpacing (5);
grid_layout->setVerticalSpacing (2);

//		Column headings.
label = new QLabel (tr ("Source"));
label->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
grid_layout->addWidget (label,
	headings_top_row, values_col, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
label = new QLabel (tr ("Values"));
label->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
grid_layout->addWidget (label,
	headings_bottom_row, values_col, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

label = new QLabel (tr ("Histogram Percents"));
label->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
grid_layout->addWidget (label,
	headings_top_row, percents_slider_col, 1, 3,
		Qt::AlignLeft | Qt::AlignVCenter);

//		Settings with redo button.
horizontal_layout = new QHBoxLayout;
label = new QLabel (tr ("Settings"));
label->setAlignment (Qt::AlignLeft | Qt::AlignVCenter);
horizontal_layout->addWidget (label, 1, Qt::AlignLeft);
QSize
	heading_size (label->sizeHint ());
heading_size.rwidth () = heading_size.rheight ();

//		Apply Percents.
Apply_Percents_Action = new QAction (tr ("Apply &Percents Settings"), this);
Apply_Percents_Action->setToolTip
	(tr ("Apply Percents Settings to determine Source Values"));
Apply_Percents_Action->setShortcut (tr ("Ctrl+Alt+P"));
Apply_Percents_Action->setIcon (QIcon (APPLY_ICON));
connect (Apply_Percents_Action,
	SIGNAL (triggered ()),
	SLOT (apply_bound_percents ()));

icon_button = new Icon_Button (Apply_Percents_Action->icon ());
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    apply percents button @ " << (void*)icon_button << endl;
#endif
icon_button->setIconSize (heading_size);
icon_button->setToolTip  (Apply_Percents_Action->toolTip ());
connect
	(icon_button, SIGNAL (clicked ()),
	 Apply_Percents_Action, SLOT (trigger ()));
horizontal_layout->addWidget (icon_button, 1, Qt::AlignRight);

//		Actual percents to settings action.
Actual_to_Settings_Action = new QAction
	(tr ("Percents Settings from Actual"), this);
Actual_to_Settings_Action->setToolTip
	(tr ("Percents Settings set from Actual Percents values"));
Actual_to_Settings_Action->setShortcut (tr ("Ctrl+Alt+Shift+P"));
connect (Actual_to_Settings_Action,
	SIGNAL (triggered ()),
	SLOT (bound ()));

grid_layout->addLayout (horizontal_layout,
	headings_bottom_row, percents_slider_col, 1, 2,
		Qt::AlignLeft | Qt::AlignVCenter);

//		Actual percents column.
label = new QLabel (tr ("Actual"));
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label,
	headings_bottom_row, percents_actual_col,
		Qt::AlignRight | Qt::AlignVCenter);
//		Actual percent column minimum width.
label = new QLabel ("100.00");
grid_layout->setColumnMinimumWidth (percents_actual_col,
	label->sizeHint ().width () + 5);
delete label;

//		Underline.
grid_layout->addWidget (new Drawn_Line,
	headings_underline_row, labels_line_col, 1, padding_col - labels_line_col);

double
	percent_increment = 1.0;
for (int
		decimal_places = PERCENT_DECIMAL_PLACES;
		decimal_places;
	  --decimal_places)
	percent_increment /= 10;

for (band = 0;
	 band < 3;
	 band++)
	{
	//	Upper bound.
	row = first_upper_row + band;

	line = new Drawn_Line (Heading_Line_Weight);
	line->brush (QBrush (QColor (DISPLAY_BAND_COLORS[band])));
	line->orientation (Qt::Vertical);
	line->alignment (Qt::AlignVCenter);
	grid_layout->addWidget (line,
		row, labels_line_col);

	Upper_Bound_Value[band] = new QSpinBox;
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Upper_Bound_Value[" << band << "] @ "
			<< (void*)Upper_Bound_Value[band] << endl;
	#endif
	Upper_Bound_Value[band]->setRange (1, max_x ());
	Upper_Bound_Value[band]->setValue (Nodes[band]->upper_bound ());
	Upper_Bound_Value[band]->setSingleStep (1);
	Upper_Bound_Value[band]->setAccelerated (true);
	Upper_Bound_Value[band]->setKeyboardTracking (false);
	Upper_Bound_Value[band]->setAlignment (Qt::AlignRight);
	Upper_Bound_Value[band]->setFrame (false);
	connect (Upper_Bound_Value[band],
		SIGNAL (valueChanged (int)),
		SLOT (bound ()));
	grid_layout->addWidget (Upper_Bound_Value[band],
		row, values_col, Qt::AlignRight | Qt::AlignTop);

	Upper_Bound_Percent_Setting[band] = new QDoubleSpinBox;
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Upper_Bound_Percent_Setting[" << band << "] @ "
			<< (void*)Upper_Bound_Percent_Setting[band] << endl;
	#endif
	Upper_Bound_Percent_Setting[band]->setRange (0.0, 100.0);
	Upper_Bound_Percent_Setting[band]->setValue (0.0);
	Upper_Bound_Percent_Setting[band]->setDecimals (PERCENT_DECIMAL_PLACES);
	Upper_Bound_Percent_Setting[band]->setSingleStep (percent_increment);
	Upper_Bound_Percent_Setting[band]->setAccelerated (true);
	Upper_Bound_Percent_Setting[band]->setKeyboardTracking (false);
	Upper_Bound_Percent_Setting[band]->setAlignment (Qt::AlignRight);
	Upper_Bound_Percent_Setting[band]->setFrame (false);
	connect (Upper_Bound_Percent_Setting[band],
		SIGNAL (valueChanged (double)),
		SLOT (bound ()));
	grid_layout->addWidget (Upper_Bound_Percent_Setting[band],
		row, percents_setting_col, Qt::AlignLeft | Qt::AlignTop);

	Upper_Bound_Percent_Actual[band] = new QLabel ("  0.00");
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Upper_Bound_Percent_Actual[" << band << "] @ "
			<< (void*)Upper_Bound_Percent_Actual[band] << endl;
	#endif
	Upper_Bound_Percent_Actual[band]->setAlignment
		(Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Upper_Bound_Percent_Actual[band],
		row, percents_actual_col, Qt::AlignRight | Qt::AlignVCenter);

	grid_layout->setRowStretch (row, 1);

	//	Lower bound.
	row = first_lower_row + band;

	line = new Drawn_Line (Heading_Line_Weight);
	line->brush (QBrush (QColor (DISPLAY_BAND_COLORS[band])));
	line->orientation (Qt::Vertical);
	line->alignment (Qt::AlignVCenter);
	grid_layout->addWidget (line,
		row, labels_line_col);

	Lower_Bound_Value[band] = new QSpinBox;
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Lower_Bound_Value[" << band << "] @ "
			<< (void*)Lower_Bound_Value[band] << endl;
	#endif
	Lower_Bound_Value[band]->setRange (0, max_x () - 1);
	Lower_Bound_Value[band]->setValue (Nodes[band]->lower_bound ());
	Lower_Bound_Value[band]->setSingleStep (1);
	Lower_Bound_Value[band]->setAccelerated (true);
	Lower_Bound_Value[band]->setKeyboardTracking (false);
	Lower_Bound_Value[band]->setAlignment (Qt::AlignRight);
	Lower_Bound_Value[band]->setFrame (false);
	connect (Lower_Bound_Value[band],
		SIGNAL (valueChanged (int)),
		SLOT (bound ()));
	grid_layout->addWidget (Lower_Bound_Value[band],
		row, values_col, Qt::AlignRight | Qt::AlignBottom);

	Lower_Bound_Percent_Setting[band] = new QDoubleSpinBox;
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Lower_Bound_Percent_Setting[" << band << "] @ "
			<< (void*)Lower_Bound_Percent_Setting[band] << endl;
	#endif
	Lower_Bound_Percent_Setting[band]->setRange (0.0, 100.0);
	Lower_Bound_Percent_Setting[band]->setValue (0.0);
	Lower_Bound_Percent_Setting[band]->setDecimals (PERCENT_DECIMAL_PLACES);
	Lower_Bound_Percent_Setting[band]->setSingleStep (percent_increment);
	Lower_Bound_Percent_Setting[band]->setAccelerated (true);
	Lower_Bound_Percent_Setting[band]->setKeyboardTracking (false);
	Lower_Bound_Percent_Setting[band]->setAlignment (Qt::AlignRight);
	Lower_Bound_Percent_Setting[band]->setFrame (false);
	connect (Lower_Bound_Percent_Setting[band],
		SIGNAL (valueChanged (double)),
		SLOT (bound ()));
	grid_layout->addWidget (Lower_Bound_Percent_Setting[band],
		row, percents_setting_col, Qt::AlignLeft | Qt::AlignBottom);

	Lower_Bound_Percent_Actual[band] = new QLabel ("  0.00");
	#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    Lower_Bound_Percent_Actual[" << band << "] @ "
			<< (void*)Lower_Bound_Percent_Actual[band] << endl;
	#endif
	Lower_Bound_Percent_Actual[band]->setAlignment
		(Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Lower_Bound_Percent_Actual[band],
		row, percents_actual_col, Qt::AlignRight | Qt::AlignVCenter);

	grid_layout->setRowStretch (row, 1);
	}

//	Focus tab ordering.
setTabOrder (Upper_Bound_Value[0], Upper_Bound_Value[1]);
setTabOrder (Upper_Bound_Value[1], Upper_Bound_Value[2]);
setTabOrder (Upper_Bound_Value[2], Lower_Bound_Value[0]);
setTabOrder (Lower_Bound_Value[0], Lower_Bound_Value[1]);
setTabOrder (Lower_Bound_Value[1], Lower_Bound_Value[2]);
setTabOrder (Lower_Bound_Value[2], Upper_Bound_Percent_Setting[0]);
setTabOrder (Upper_Bound_Percent_Setting[0], Upper_Bound_Percent_Setting[1]);
setTabOrder (Upper_Bound_Percent_Setting[1], Upper_Bound_Percent_Setting[2]);
setTabOrder (Upper_Bound_Percent_Setting[2], Lower_Bound_Percent_Setting[0]);
setTabOrder (Lower_Bound_Percent_Setting[0], Lower_Bound_Percent_Setting[1]);
setTabOrder (Lower_Bound_Percent_Setting[1], Lower_Bound_Percent_Setting[2]);
/*
	The focus tab order is held in a single, global list
	so it is not possible to loop back to the first widget in a group.
*/

//		Section labels.
Rotated_Label
	*rotated_label;
rotated_label = new Rotated_Label (tr ("Upper"));
rotated_label->setToolTip (tr ("Saturation upper bounds"));
rotated_label->text_rotation (Rotated_Label::COUNTER_CLOCKWISE);
grid_layout->addWidget (rotated_label,
	first_upper_row, labels_col, 3, 1, Qt::AlignHCenter | Qt::AlignTop);

rotated_label = new Rotated_Label (tr ("Lower"));
rotated_label->setToolTip (tr ("Saturation lower bounds"));
rotated_label->text_rotation (Rotated_Label::COUNTER_CLOCKWISE);
grid_layout->addWidget (rotated_label,
	first_lower_row, labels_col, 3, 1, Qt::AlignHCenter | Qt::AlignBottom);

//		Upper bounds value slider.
int
	slider_height = Upper_Bound_Value[0]->sizeHint ().height () * 3;
Upper_Bound_Value_Slider = new QwtSlider (Qt::Vertical, this);
	// QwtSlider::NoScale, QwtSlider::BgBoth);
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    Upper_Bound_Value_Slider @ "
		<< (void*)Upper_Bound_Value_Slider << endl;
#endif
Upper_Bound_Value_Slider->setToolTip
	(tr ("Change all upper bounds values"));
Upper_Bound_Value_Slider->setFixedHeight (slider_height);
Upper_Bound_Value_Slider->setScale (0, max_x ());//, 1, 1);
Upper_Bound_Value_Slider->setValue (max_x ());
Upper_Bound_Value_Slider->setHandleSize (QSize(15,7));
Upper_Bound_Value_Slider->setTracking (true);
grid_layout->addWidget (Upper_Bound_Value_Slider,
	first_upper_row, values_slider_col, 3, 1);
connect (Upper_Bound_Value_Slider,
	SIGNAL (valueChanged (double)),
	SLOT (bound ()));

//		Values-Percents dividing line.
line = new Drawn_Line (2);
line->orientation (Qt::Vertical);
line->alignment (Qt::AlignVCenter);
grid_layout->addWidget (line,
	0, divider_col, -1, 1, Qt::AlignHCenter);

//		Upper bounds percent slider.
Upper_Bound_Percent_Slider = new QwtSlider (Qt::Vertical, this);
	//, QwtSlider::NoScale, QwtSlider::BgBoth);
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    Upper_Bound_Percent_Slider @ "
		<< (void*)Upper_Bound_Percent_Slider << endl;
#endif
Upper_Bound_Percent_Slider->setToolTip
	(tr ("Change all upper bounds percentages"));
Upper_Bound_Percent_Slider->setFixedHeight (slider_height);
//	Reversed range.
Upper_Bound_Percent_Slider->setScale(100, 0);
//Upper_Bound_Percent_Slider->setScaleStepSize(0.01);
Upper_Bound_Percent_Slider->setSingleSteps(1);
Upper_Bound_Percent_Slider->setPageSteps(10);
Upper_Bound_Percent_Slider->setValue (0);
Upper_Bound_Percent_Slider->setHandleSize (QSize(15,7));
Upper_Bound_Percent_Slider->setTracking (true);
grid_layout->addWidget (Upper_Bound_Percent_Slider,
	first_upper_row, percents_slider_col, 3, 1);
connect (Upper_Bound_Percent_Slider,
	SIGNAL (valueChanged (double)),
	SLOT (bound ()));

//		Lower-Upper bounds dividing line.
line = new Drawn_Line (1);
line->alignment (Qt::AlignVCenter);
grid_layout->addWidget (line,
	divider_row, 0, 1, padding_col, Qt::AlignVCenter);
grid_layout->setRowStretch (divider_row, 1);

//		Lower bounds value slider.
Lower_Bound_Value_Slider = new QwtSlider (Qt::Vertical, this);
	//, QwtSlider::NoScale, QwtSlider::BgBoth);
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    Lower_Bound_Value_Slider @ "
		<< (void*)Lower_Bound_Value_Slider << endl;
#endif
Lower_Bound_Value_Slider->setToolTip
	(tr ("Change all lower bounds values"));
Lower_Bound_Value_Slider->setFixedHeight (slider_height);
Lower_Bound_Value_Slider->setScale (0, max_x () - 1);//, 1, 1);
Lower_Bound_Value_Slider->setValue (0);
Lower_Bound_Value_Slider->setHandleSize (QSize(15,7));
Lower_Bound_Value_Slider->setTracking (true);
grid_layout->addWidget (Lower_Bound_Value_Slider,
	first_lower_row, values_slider_col, 4, 1);
connect (Lower_Bound_Value_Slider,
	SIGNAL (valueChanged (double)),
	SLOT (bound ()));

//		Lower bounds percent slider.
Lower_Bound_Percent_Slider = new QwtSlider (Qt::Vertical, this);
	// QwtSlider::NoScale, QwtSlider::BgBoth);
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    Lower_Bound_Percent_Slider @ "
		<< (void*)Lower_Bound_Percent_Slider << endl;
#endif
Lower_Bound_Percent_Slider->setToolTip
	(tr ("Change all lower bounds percentages"));
Lower_Bound_Percent_Slider->setFixedHeight (slider_height);
Lower_Bound_Percent_Slider->setScale (0, 100);
//Upper_Bound_Percent_Slider->setScaleStepSize(0.01);
Lower_Bound_Percent_Slider->setSingleSteps(1);
Lower_Bound_Percent_Slider->setPageSteps(10);
Lower_Bound_Percent_Slider->setValue (0);
Lower_Bound_Percent_Slider->setHandleSize (QSize(15,7));
Lower_Bound_Percent_Slider->setTracking (true);
grid_layout->addWidget (Lower_Bound_Percent_Slider,
	first_lower_row, percents_slider_col, 4, 1);
connect (Lower_Bound_Percent_Slider,
	SIGNAL (valueChanged (double)),
	SLOT (bound ()));

panel_layout->addWidget (saturation_controls, 1);

//	Bottom space.
#if ((DEBUG_SECTION) & DEBUG_CONTROL_PANEL)
clog << "    space" << endl;
#endif
panel_layout->addSpacing (10);
panel_layout->addStretch (100);

panel->setMinimumSize (panel_layout->sizeHint ());
#if ((DEBUG_SECTION) & (DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    control_panel minimum size = "
		<< panel_layout->sizeHint () << endl
	 << "<<< Data_Mapper_Tool::control_panel" << endl;
#endif
return panel;
}

/*..............................................................................
	Band Selection.
*/
void
Data_Mapper_Tool::select_band
	(
	bool	enabled,
	int		band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
clog << ">>> Data_Mapper_Tool::select_band: "
		<< boolalpha << enabled << " - band " << band << endl;
#endif
if (band < 0)
	{
	//	All bands selection.
	Band_Selection_Actions[3]->setEnabled (! enabled);

	//	Recursively select each band individually.
	for (band = 0;
		 band < 3;
		 band++)
		select_band (enabled, band);
	}
else
if (band < 3)
	{
	if (enabled == Band_Selection_Actions[band]->isChecked ())
		{
		//	Apply the appropriate icon.
		if (enabled)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
			clog << "    set Band_Selection_Icons[" << band << "] @ "
					<< (void*)Band_Selection_Icons[band] << endl;
			#endif
			Selected_Bands |= (1 << band);
			Band_Selection_Actions[band]->setIcon (*Band_Selection_Icons[band]);
			Band_Selection_Actions[band]->setToolTip
				(tr ("Disable mapping modification of the ") +
		 		 tr (DISPLAY_BAND_NAMES[band]) + tr (" band"));

			if (Selected_Bands == SELECTED_ALL)
				//	Disable select all bands.
				Band_Selection_Actions[3]->setEnabled (false);
			}
		else
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
			clog << "    set Not_Selected_Icon @ "
					<< (void*)Not_Selected_Icon << endl;
			#endif
			Selected_Bands &= ~(1 << band);
			Band_Selection_Actions[band]->setIcon (*Not_Selected_Icon);
			Band_Selection_Actions[band]->setToolTip
				(tr ("Enable mapping modification of the ") +
		 		 tr (DISPLAY_BAND_NAMES[band]) + tr (" band"));

			//	Enable select all bands.
			Band_Selection_Actions[3]->setEnabled (true);
			}
		Band_Selection_Buttons[band]->setIcon
			(Band_Selection_Actions[band]->icon ());
		Band_Selection_Buttons[band]->setToolTip
			(Band_Selection_Actions[band]->toolTip ());

		//	Enable/Disable the corresponding saturation bounds values.
		Upper_Bound_Value[band]->setEnabled (enabled);
		Lower_Bound_Value[band]->setEnabled (enabled);
		Upper_Bound_Percent_Setting[band]->setEnabled (enabled);
		Lower_Bound_Percent_Setting[band]->setEnabled (enabled);

		Upper_Bound_Value_Slider->setEnabled (Selected_Bands);
		Lower_Bound_Value_Slider->setEnabled (Selected_Bands);
		Upper_Bound_Percent_Slider->setEnabled (Selected_Bands);
		Lower_Bound_Percent_Slider->setEnabled (Selected_Bands);

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::select_band: emit: "
				"selected_bands_changed " << Selected_Bands << endl;
		#endif
		emit selected_bands_changed (Selected_Bands);
		}
	else
		//	Toggle the action which will recursively select the band.
		Band_Selection_Actions[band]->setChecked (enabled);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
clog << "<<< Data_Mapper_Tool::select_band" <<endl;
#endif
}


void
Data_Mapper_Tool::selected_bands
	(
	int		selection
	)
{
selection &= SELECTED_ALL;
if (selection != Selected_Bands)
	for (int
			band = 0;
		 	band < 3;
		  ++band)
		if ((selection & band) != (Selected_Bands & band))
			select_band ((selection & band), band);
}


void
Data_Mapper_Tool::select_band ()
{
//	N.B.: The Band_Selection_Actions are expecte to be connected to this slot.
QObject
	*source = sender ();
if (source)
	{
	if (source == Band_Selection_Actions[3])
		//	Select all bands.
		select_band (true, -1);
	else
		{
		int
			band = 3;
		while (band-- &&
				source != Band_Selection_Actions[band]) ;
		if (band >= 0)
			select_band (Band_Selection_Actions[band]->isChecked (), band);
		}
	}
}

/*..............................................................................
	Default Mapping.
*/
bool
Data_Mapper_Tool::linear_mapping
	(
	Mapping_Extent	extent
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::linear_mapping: " << extent << endl
	 << "    Selected_Bands = " << Selected_Bands << endl;
print_data_maps (clog);
bool
	nodes_changed;
#endif
bool
	changed = false;
if (Selected_Bands)
	{
	bool
		upper_changed = false,
		lower_changed = false;
    QVector<int>
        upper_values (3, -1),
        lower_values (3, -1);
	int
		band = 3;
	while (band--)
		{
		if (Selected_Bands & (1 << band))
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
			nodes_changed = Nodes[band]->reset ();
			clog << "    Nodes[" << band << "] reset = "
					<< boolalpha << nodes_changed << endl;
			changed |= nodes_changed;
			#else
			changed |= Nodes[band]->reset ();
			#endif
			switch (extent)
				{
				case UNBOUNDED_MAPPING:
					upper_values[band] = max_x ();
					lower_values[band] = 0;
					break;
				case BOUNDED_MAPPING:
					upper_values[band] = Upper_Bound_Value[band]->value ();
					lower_values[band] = Lower_Bound_Value[band]->value ();
					break;
				case LIMITED_MAPPING:
					upper_values[band] = max_x () - Upper_Limit;
					lower_values[band] = Lower_Limit;
					break;
				}
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
			clog << "    upper_values[" << band << "] = "
					<< upper_values[band] << endl;
			#endif
			if (upper_bound_value (upper_values[band], band, true))
				upper_changed = changed = true;
			else
				upper_values[band] = -1;
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
			clog << "      changed = "
					<< ((upper_values[band] != -1) ? "true" : "false") << endl;
			clog << "    lower_values[" << band << "] = "
					<< lower_values[band] << endl;
			#endif
			if (lower_bound_value (lower_values[band], band, true))
				lower_changed = changed = true;
			else
				lower_values[band] = -1;
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
			clog << "      changed = "
					<< ((lower_values[band] != -1) ? "true" : "false") << endl;
			#endif
			}
		}
	Signal_Changes = true;

	if (changed)
		Graph->replot ();

	if (upper_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_DATA_MAPS | \
						DEBUG_BOUNDS | \
						DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::linear_mapping: "
				"emit upper_bound_values_changed "
				<< upper_values[0] << ", "
				<< upper_values[1] << ", "
				<< upper_values[2] << endl;
		#endif
		emit upper_bound_values_changed (upper_values);
		}
	if (lower_changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_DATA_MAPS | \
						DEBUG_BOUNDS | \
						DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::linear_mapping: "
				"emit lower_bound_values_changed "
				<< lower_values[0] << ", "
				<< lower_values[1] << ", "
				<< lower_values[2] << endl;
		#endif
		emit lower_bound_values_changed (lower_values);
		}

	if (changed)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::linear_mapping: "
				"emit data_maps_changed" << endl;
		#endif
		emit data_maps_changed (Data_Maps);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPS | DEBUG_BOUNDS))
print_data_maps (clog);
clog << "<<< Data_Mapper_Tool::linear_mapping: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


bool
Data_Mapper_Tool::limited_linear_mapping ()
{return linear_mapping (LIMITED_MAPPING);}


bool
Data_Mapper_Tool::bounded_linear_mapping ()
{return linear_mapping (BOUNDED_MAPPING);}

/*..............................................................................
	Saturation Bounds
*/
void
Data_Mapper_Tool::saturation_controls
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
clog << ">>> Data_Mapper_Tool::saturation_controls: "
		<< boolalpha << enabled << endl;
#endif
if (enabled == Bounds_Button->isChecked ())
	{
	//	Apply the appropriate icon.
	if (enabled)
		{
		Operating_Mode |= SATURATION_MODE;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
		clog << "    set Bounds_Icon @ "
				<< (void*)Bounds_Icon << endl;
		#endif
		Bounds_Button->setIcon (*Bounds_Icon);
		Bounds_Button->setToolTip
			(tr ("Saturation bounds graph manipulation disable"));
		//Tracker->setSelectionFlags (TRACKER_BOUND_MODE);
		Tracker->setRubberBand (QwtPicker::NoRubberBand);
		Tracker->set_cursor (*Move_Horizontal_Cursor);

		Selection_Marker->setVisible (false);
		}
	else
		{
		Operating_Mode &= ~SATURATION_MODE;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
		clog << "    set Not_Selected_Icon @ "
				<< (void*)Not_Selected_Icon << endl;
		#endif
		Bounds_Button->setIcon (*Not_Selected_Icon);
		Bounds_Button->setToolTip
			(tr ("Saturation bounds graph manipulation enable"));
		//Tracker->setSelectionFlags (TRACKER_TRACK_MODE);
		Tracker->setRubberBand (QwtPicker::CrossRubberBand);
		Tracker->set_cursor (*Crosshair_Cursor);

		Bound_Marker->setVisible (false);
		}

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::saturation_controls: "
			"emit saturation_mode " << boolalpha << enabled << endl;
	#endif
	emit saturation_mode (enabled);
	}
else
	//	Toggle the button which will recursively call this method.
	Bounds_Button->setChecked (enabled);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL))
clog << "<<< Data_Mapper_Tool::saturation_controls" <<endl;
#endif
}


void
Data_Mapper_Tool::bound ()
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::bound" << endl;
#endif
QObject
	*source = sender ();
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    sender @ " << (void*)source << endl;
#endif
if (source)
	{
	int
		band = 3;

	if (source == Upper_Bound_Value_Slider)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Upper_Bound_Value_Slider value = "
				<< (int)Upper_Bound_Value_Slider->value () << endl;
		#endif
		QVector<int>
			values (3, (int)Upper_Bound_Value_Slider->value ());
		while (band--)
			if (! (Selected_Bands & (1 << band)))
				values[band] = -1;
		upper_bound_values (values);
		}
	else
	if (source == Lower_Bound_Value_Slider)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Lower_Bound_Value_Slider value = "
				<< (int)Lower_Bound_Value_Slider->value () << endl;
		#endif
		QVector<int>
			values (3, (int)Lower_Bound_Value_Slider->value ());
		while (band--)
			if (! (Selected_Bands & (1 << band)))
				values[band] = -1;
		lower_bound_values (values);
		}
	else
	if (source == Upper_Bound_Percent_Slider)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_CONTROL_PANEL | \
						DEBUG_BOUNDS))
		clog << "    Upper_Bound_Percent_Slider = "
				<< Upper_Bound_Percent_Slider->value () << endl;
		#endif
		QVector<double>
			percents (3, Upper_Bound_Percent_Slider->value ());
		while (band--)
			if (! (Selected_Bands & (1 << band)))
				percents[band] = -1.0;
		upper_bound_percents (percents);
		}
	else
	if (source == Lower_Bound_Percent_Slider)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_CONTROL_PANEL | \
						DEBUG_BOUNDS))
		clog << "    Lower_Bound_Percent_Slider = "
				<< Lower_Bound_Percent_Slider->value () << endl;
		#endif
		QVector<double>
			percents (3, Lower_Bound_Percent_Slider->value ());
		while (band--)
			if (! (Selected_Bands & (1 << band)))
				percents[band] = -1.0;
		lower_bound_percents (percents);
		}
	else
	if (source == Actual_to_Settings_Action)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_CONTROL_PANEL | \
						DEBUG_BOUNDS))
		clog << "    Actual_to_Settings_Action" << endl;
		#endif
		bool
			enabled;
		while (band--)
			{
			if ((Selected_Bands & (1 << band)))
				{
				enabled = Upper_Bound_Percent_Setting[band]->blockSignals (true);
				Upper_Bound_Percent_Setting[band]->setValue
					(Upper_Bound_Percent_Actual[band]->text ().toDouble ());
				Upper_Bound_Percent_Setting[band]->blockSignals (enabled);

				enabled = Lower_Bound_Percent_Setting[band]->blockSignals (true);
				Lower_Bound_Percent_Setting[band]->setValue
					(Lower_Bound_Percent_Actual[band]->text ().toDouble ());
				Lower_Bound_Percent_Setting[band]->blockSignals (enabled);
				}
			}
		}
	else
		{
		while (band--)
			{
			if (source == Upper_Bound_Value[band])
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
								DEBUG_CONTROL_PANEL | \
								DEBUG_BOUNDS))
				clog << "    Upper_Bound_Value[" << band << "] = "
						<< Upper_Bound_Value[band]->value () << endl;
				#endif
				upper_bound_value (Upper_Bound_Value[band]->value (), band, true);
				break;
				}
			if (source == Lower_Bound_Value[band])
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
								DEBUG_CONTROL_PANEL | \
								DEBUG_BOUNDS))
				clog << "    Lower_Bound_Value[" << band << "] = "
						<< Lower_Bound_Value[band]->value () << endl;
				#endif
				lower_bound_value (Lower_Bound_Value[band]->value (), band, true);
				break;
				}
			if (source == Upper_Bound_Percent_Setting[band])
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
								DEBUG_CONTROL_PANEL | \
								DEBUG_BOUNDS))
				clog << "    Upper_Bound_Percent_Setting[" << band << "] = "
						<< Upper_Bound_Percent_Setting[band]->value () << endl;
				#endif
				upper_bound_percent
					(Upper_Bound_Percent_Setting[band]->value (), band, true);
				break;
				}
			if (source == Lower_Bound_Percent_Setting[band])
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
								DEBUG_CONTROL_PANEL | \
								DEBUG_BOUNDS))
				clog << "    Lower_Bound_Percent_Setting[" << band << "] = "
						<< Lower_Bound_Percent_Setting[band]->value () << endl;
				#endif
				lower_bound_percent
					(Lower_Bound_Percent_Setting[band]->value (), band, true);
				break;
				}
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::bound" << endl;
#endif
return;
}


void
Data_Mapper_Tool::apply_bound_values ()
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << ">>> Data_Mapper_Tool::apply_bound_values:" << endl
	 << "    pre-apply Nodes:" << endl;
print_nodes (Nodes);
#endif

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "<<< Data_Mapper_Tool::apply_bound_values" << endl;
#endif
}

/*..............................................................................
	Bounds Percents
*/
double
Data_Mapper_Tool::upper_bound_percent
	(
	int		band
	) const
{
double
	percent;
if (band >= 0 &&
	band < 3)
	percent = Upper_Bound_Percent_Setting[band]->value ();
else
	percent = -1.0;
return percent;
}


void
Data_Mapper_Tool::actual_upper_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool:: actual_upper_bound_percents" << endl;
#endif
double
	percent;
QString
	format ("%1");
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	percent = percents[band];
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    percents[" << band << "] = " << percent;
	#endif
	if (percent < 0.0)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << endl;
		#endif
		continue;
		}
	if (percent > 100.0)
		percent = 100.0;
	else
		percent = round_to (percent, PERCENT_DECIMAL_PLACES);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << " -> " << percent << endl;
	#endif
	Upper_Bound_Percent_Actual[band]->setText
		(format.arg (percent, 6, 'f', 2));
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool:: actual_upper_bound_percents" << endl;
#endif
}


bool
Data_Mapper_Tool::upper_bound_percent
	(
	double	percent,
	int		band,
	bool	force_change
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::upper_bound_percent: "
		<< percent << ", band " << band << endl;
#endif
bool
	changed = force_change,
	actually_changed = false;
double
	previous_percent = upper_bound_percent (band);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    previous_percent = " << previous_percent << endl;
#endif
if (previous_percent >= 0.0)
	{
	if (percent < 0.0)
		percent = 0.0;
	else
	if (percent > 100.0)
		percent = 100.0;
	else
		percent = round_to (percent, PERCENT_DECIMAL_PLACES);

	bool
		enabled;
	if (previous_percent != percent)
		{
		actually_changed = true;
		changed = true;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Upper_Bound_Percent_Setting set to " << percent << endl;
		#endif
		enabled = Upper_Bound_Percent_Setting[band]->blockSignals (true);
		Upper_Bound_Percent_Setting[band]->setValue (percent);
		Upper_Bound_Percent_Setting[band]->blockSignals (enabled);
		}

	if (Upper_Bound_Percent_Slider->value () != percent)
		{
		//	Check for slider synchronization.
		enabled = true;
		for (int
				selected_band = 0;
				selected_band < 3;
			  ++selected_band)
			if ((Selected_Bands & (1 << selected_band)) &&
				! (enabled =
					(Upper_Bound_Percent_Setting[selected_band]->value ()
					== percent)))
				break;
		if (enabled)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
			clog << "    Upper_Bound_Percent_Slider set to " << percent << endl;
			#endif
			enabled = Upper_Bound_Percent_Slider->blockSignals (true);
			Upper_Bound_Percent_Slider->setValue (percent);
			Upper_Bound_Percent_Slider->blockSignals (enabled);
			}
		}

	if (changed &&
		Signal_Changes)
		{
		QVector<double>
			percents (3, -1.0);
		percents[band] = percent;
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_CONTROL_PANEL | \
						DEBUG_BOUNDS | \
						DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::upper_bound_percent: "
				"emit upper_bound_percents_changed "
					<< percents[0] << ", "
					<< percents[1] << ", "
					<< percents[2] << endl;
		#endif
		emit upper_bound_percents_changed (percents);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::upper_bound_percent: "
		<< boolalpha << actually_changed << endl;
#endif
return actually_changed;
}


bool
Data_Mapper_Tool::upper_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::upper_bound_percents: "
		<< percents[0] << ", " << percents[1] << ", " << percents[2] << endl;
#endif
bool
	changed = false,
	signal_changes = Signal_Changes;
Signal_Changes = false;

QVector<double>
	changed_percents (percents);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (percents[band] >= 0.0)
		{
		if (upper_bound_percent (percents[band], band))
			changed = true;
		else
			changed_percents[band] = -1;
		}
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    upper_bound_percent[" << band << "] = "
			<< upper_bound_percent (band) << endl;
	#endif
	}

Signal_Changes = signal_changes;
if (changed &&
	Signal_Changes)
	{
	Graph->replot ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::upper_bound_percents: "
			"emit upper_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit upper_bound_percents_changed (changed_percents);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::upper_bound_percents: "
			"emit data_maps_changed" << endl;
	#endif
	emit data_maps_changed (Data_Maps);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::upper_bound_percents: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
Data_Mapper_Tool::upper_default_contrast_stretch
	(
	double	percent,
	int		band
	)
{
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid upper default contrast stretch band: " << band;
	throw invalid_argument (message.str ());
	}
if (percent < 0.0)
	percent = 0.0;
else
if (percent > 100.0)
	percent = 100.0;
Upper_Default_Contrast_Stretch[band] = percent;
}


double
Data_Mapper_Tool::lower_bound_percent
	(
	int		band
	) const
{
double
	percent;
if (band >= 0 &&
	band < 3)
	percent = Lower_Bound_Percent_Setting[band]->value ();
else
	percent = -1.0;
return percent;
}


void
Data_Mapper_Tool::actual_lower_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool:: actual_lower_bound_percents" << endl;
#endif
double
	percent;
QString
	format ("%1");
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	percent = percents[band];
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << "    percents[" << band << "] = " << percent;
	#endif
	if (percent < 0.0)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << endl;
		#endif
		continue;
		}
	if (percent > 100.0)
		percent = 100.0;
	else
		percent = round_to (percent, PERCENT_DECIMAL_PLACES);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
	clog << " -> " << percent << endl;
	#endif
	Lower_Bound_Percent_Actual[band]->setText
		(format.arg (percent, 6, 'f', 2));
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool:: actual_lower_bound_percents" << endl;
#endif
}


bool
Data_Mapper_Tool::lower_bound_percent
	(
	double	percent,
	int		band,
	bool	force_change
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::lower_bound_percent: "
		<< percent << ", band " << band << endl
	 << "    force_change = " << boolalpha << force_change << endl;
#endif
bool
	changed = force_change,
	actually_changed = false;
double
	previous_percent = lower_bound_percent (band);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    previous_percent = " << previous_percent << endl;
#endif
if (previous_percent >= 0.0)
	{
	if (percent < 0.0)
		percent = 0.0;
	else
	if (percent > 100.0)
		percent = 100.0;
	else
		percent = round_to (percent, PERCENT_DECIMAL_PLACES);

	bool
		enabled;
	if (previous_percent != percent)
		{
		actually_changed = true;
		changed = true;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Lower_Bound_Percent_Setting set to " << percent << endl;
		#endif
		enabled = Lower_Bound_Percent_Setting[band]->blockSignals (true);
		Lower_Bound_Percent_Setting[band]->setValue (percent);
		Lower_Bound_Percent_Setting[band]->blockSignals (enabled);
		}

	if (Lower_Bound_Percent_Slider->value () != percent)
		{
		//	Check for slider synchronization.
		enabled = true;
		for (int
				selected_band = 0;
				selected_band < 3;
			  ++selected_band)
			if ((Selected_Bands & (1 << selected_band)) &&
				! (enabled =
					(Lower_Bound_Percent_Setting[selected_band]->value ()
					== percent)))
				break;
		if (enabled)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
			clog << "    Lower_Bound_Percent_Slider set to " << percent << endl;
			#endif
			enabled = Lower_Bound_Percent_Slider->blockSignals (true);
			Lower_Bound_Percent_Slider->setValue (percent);
			Lower_Bound_Percent_Slider->blockSignals (enabled);
			}
		}

	if (changed &&
		Signal_Changes)
		{
		QVector<double>
			percents (3, -1.0);
		percents[band] = percent;
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_CONTROL_PANEL | \
						DEBUG_BOUNDS | \
						DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::lower_bound_percent: "
				"emit lower_bound_percents_changed "
					<< percents[0] << ", "
					<< percents[1] << ", "
					<< percents[2] << endl;
		#endif
		emit lower_bound_percents_changed (percents);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::lower_bound_percent: "
		<< boolalpha << actually_changed << endl;
#endif
return actually_changed;
}


bool
Data_Mapper_Tool::lower_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::lower_bound_percents: "
		<< percents[0] << ", " << percents[1] << ", " << percents[2] << endl;
#endif
bool
	changed = false,
	signal_changes = Signal_Changes;
Signal_Changes = false;

QVector<double>
	changed_percents (percents);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (percents[band] >= 0.0)
		{
		if (lower_bound_percent (percents[band], band))
			changed = true;
		else
			changed_percents[band] = -1;
		}
	}

Signal_Changes = signal_changes;
if (changed &&
	Signal_Changes)
	{
	Graph->replot ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::lower_bound_percents: "
			"emit lower_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit lower_bound_percents_changed (changed_percents);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::lower_bound_percents: "
			"emit data_maps_changed" << endl;
	#endif
	emit data_maps_changed (Data_Maps);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::lower_bound_percents: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
Data_Mapper_Tool::lower_default_contrast_stretch
	(
	double	percent,
	int		band
	)
{
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid lower default contrast stretch band: " << band;
	throw invalid_argument (message.str ());
	}
if (percent < 0.0)
	percent = 0.0;
else
if (percent > 100.0)
	percent = 100.0;
Lower_Default_Contrast_Stretch[band] = percent;
}


void
Data_Mapper_Tool::bound_percents
	(
	const QVector<double>&	upper,
	const QVector<double>&	lower
	)
{
bool
	signal_changes = Signal_Changes;
Signal_Changes = false;
upper_bound_percents (upper);
lower_bound_percents (lower);
Signal_Changes = signal_changes;
}


void
Data_Mapper_Tool::apply_bound_percents ()
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << ">>> Data_Mapper_Tool::apply_bound_percents:" << endl
	 << "    pre-apply Nodes:" << endl;
print_nodes (Nodes);
#endif
Signal_Changes = false;
QVector<double>
	upper_percents (3, -1.0),
	lower_percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (Selected_Bands & (1 << band))
		{
		upper_percents[band] =
			Upper_Bound_Percent_Setting[band]->value ();
		#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
		clog << "    Upper_Bound_Percent_Setting[" << band << "] = "
				<< upper_percents[band] << endl;
		#endif
		upper_bound_percent (upper_percents[band], band);

		lower_percents[band] =
			Lower_Bound_Percent_Setting[band]->value ();
		#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
		clog << "    Lower_Bound_Percent_Setting[" << band << "] = "
				<< lower_percents[band] << endl;
		#endif
		lower_bound_percent (lower_percents[band], band);
		}
	}
Signal_Changes = true;

//	Graph update.
//Graph->replot ();

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
clog << "^^^ Data_Mapper_Tool::apply_bound_percents: "
		"emit upper_bound_percents_changed "
		<< upper_percents[0] << ", "
		<< upper_percents[1] << ", "
		<< upper_percents[2] << endl;
#endif
emit upper_bound_percents_changed (upper_percents);

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
clog << "^^^ Data_Mapper_Tool::apply_bound_percents: "
		"emit lower_bound_percents_changed "
		<< lower_percents[0] << ", "
		<< lower_percents[1] << ", "
		<< lower_percents[2] << endl;
#endif
emit lower_bound_percents_changed (lower_percents);

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPS | DEBUG_SIGNALS))
clog << "^^^ Data_Mapper_Tool::apply_bound_percents: "
		"emit data_maps_changed" << endl;
#endif
emit data_maps_changed (Data_Maps);

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPS)
clog << "    post-apply Nodes:" << endl;
print_nodes (Nodes);
clog << "<<< Data_Mapper_Tool::apply_bound_percents" << endl;
#endif
}


void
Data_Mapper_Tool::default_contrast_stretch ()
{
bound_percents
	(Upper_Default_Contrast_Stretch,
	 Lower_Default_Contrast_Stretch);
apply_bound_percents ();
}

void
Data_Mapper_Tool::restore_original_contrast_stretch ()
{
upper_bound_value(max_x (),0);
upper_bound_value(max_x (),1);
upper_bound_value(max_x (),2);
lower_bound_value(0,0);
lower_bound_value(0,1);
lower_bound_value(0,2);

/*

Doing it through the percentages caused the Actual percentages to return to their original values of 0,
however the source values did not.

QVector<double> Original_Upper_Contrast_Stretch;
Original_Upper_Contrast_Stretch << 0.0 << 0.0 << 0.0;
QVector<double> Original_Lower_Contrast_Stretch;
Original_Lower_Contrast_Stretch << 0.0 << 0.0 << 0.0;

bound_percents
	(Original_Upper_Contrast_Stretch,
	 Original_Lower_Contrast_Stretch);
apply_bound_percents();*/
}

/*..............................................................................
	Bounds Values
*/
int
Data_Mapper_Tool::upper_bound_value
	(
	int		band
	) const
{
int
	value;
if (band >= 0 &&
	band < 3)
	value = Upper_Bound_Value[band]->value ();
else
	value = -1;
return value;
}


bool
Data_Mapper_Tool::upper_bound_value
	(
	int		value,
	int		band,
	bool	force_change
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::upper_bound_value: "
		<< value << ", band " << band
		<< ", force_change = " << boolalpha << force_change << endl;
#endif
bool
	changed = force_change,
	actually_changed = false;
int
	previous_value = upper_bound_value (band);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    previous_value = " << previous_value << endl;
#endif
if (previous_value >= 0)
	{
	if (value < 1)
		value = 1;
	else
	if (value > max_x ())
		value = max_x ();

	bool
		enabled;
	if (previous_value != value)
		{
		actually_changed = true;
		changed = true;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Upper_Bound_Value set to " << value << endl;
		#endif
		enabled = Upper_Bound_Value[band]->blockSignals (true);
		Upper_Bound_Value[band]->setValue (value);
		Upper_Bound_Value[band]->blockSignals (enabled);
		}

	if ((int)Upper_Bound_Value_Slider->value () != value)
		{
		//	Check for slider synchronization.
		enabled = true;
		for (int
				selected_band = 0;
				selected_band < 3;
			  ++selected_band)
			if ((Selected_Bands & (1 << selected_band)) &&
				! (enabled =
					(Upper_Bound_Value[selected_band]->value ()
					== value)))
				break;
		if (enabled)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
			clog << "    Upper_Bound_Value_Slider set to " << value << endl;
			#endif
			enabled = Upper_Bound_Value_Slider->blockSignals (true);
			Upper_Bound_Value_Slider->setValue (value);
			Upper_Bound_Value_Slider->blockSignals (enabled);
			}
		}

	//	Prevent range endpoints overlap.
	if (lower_bound_value (band) >= value)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    lower bound overlap moved from "
				<< lower_bound_value (band) << " to "
				<< (value - 1) << endl;
		#endif
		lower_bound_value (value - 1, band);
		}

	if (changed)
		{
		//	Reposition the corresponding node.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    reposition Nodes[" << band << "] upper_bound to "
				<< value << endl;
		#endif
		Nodes[band]->upper_bound (value);

		if (Signal_Changes)	//	Signaling data maps changes not deferred.
			{
			Graph->replot ();

			QVector<int>
				values (3, -1);
			values[band] = value;
			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
							DEBUG_CONTROL_PANEL | \
							DEBUG_BOUNDS | \
							DEBUG_SIGNALS))
			clog << "^^^ Data_Mapper_Tool::upper_bound_value: "
					"emit upper_bound_values_changed "
					<< values[0] << ", "
					<< values[1] << ", "
					<< values[2] << endl;
			#endif
			emit upper_bound_values_changed (values);

			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
							DEBUG_CONTROL_PANEL | \
							DEBUG_BOUNDS | \
							DEBUG_SIGNALS))
			clog << "^^^ Data_Mapper_Tool::upper_bound_value: "
					"emit data_maps_changed" << endl;
			#endif
			emit data_maps_changed (Data_Maps);
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::upper_bound_value: "
		<< boolalpha << actually_changed << endl;
#endif
return actually_changed;
}


bool
Data_Mapper_Tool::upper_bound_values
	(
	const QVector<int>&	values
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::upper_bound_values: "
		<< values[0] << ", " << values[1] << ", " << values[2] << endl;
#endif
bool
	changed = false,
	signal_changes = Signal_Changes;
Signal_Changes = false;

QVector<int>
	changed_values (values);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (values[band] >= 0)
		{
		if (upper_bound_value (values[band], band))
			changed = true;
		else
			changed_values[band] = -1;
		}
	}

Signal_Changes = signal_changes;
if (changed &&
	Signal_Changes)
	{
	Graph->replot ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::upper_bound_values: "
			"emit upper_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit upper_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::upper_bound_values: "
			"emit data_maps_changed" << endl;
	#endif
	emit data_maps_changed (Data_Maps);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::upper_bound_values: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


int
Data_Mapper_Tool::lower_bound_value
	(
	int		band
	) const
{
int
	value;
if (band >= 0 &&
	band < 3)
	value = Lower_Bound_Value[band]->value ();
else
	value = -1;
return value;
}


bool
Data_Mapper_Tool::lower_bound_value
	(
	int		value,
	int		band,
	bool	force_change
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::lower_bound_value: "
		<< value << ", band " << band << endl;
#endif
bool
	changed = force_change,
	actually_changed = false;
int
	previous_value = lower_bound_value (band);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "    previous_value = " << previous_value << endl;
#endif
if (previous_value >= 0)
	{
	if (value < 0)
		value = 0;
	else
	if (value >= max_x ())
		value  = max_x () - 1;

	bool
		enabled;
	if (previous_value != value)
		{
		actually_changed = true;
		changed = true;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    Lower_Bound_Value set to " << value << endl;
		#endif
		enabled = Lower_Bound_Value[band]->blockSignals (true);
		Lower_Bound_Value[band]->setValue (value);
		Lower_Bound_Value[band]->blockSignals (enabled);
		}

	if ((int)Lower_Bound_Value_Slider->value () != value)
		{
		//	Check for slider synchronization.
		enabled = true;
		for (int
				selected_band = 0;
				selected_band < 3;
			  ++selected_band)
			if ((Selected_Bands & (1 << selected_band)) &&
				! (enabled =
					(Lower_Bound_Value[selected_band]->value ()
					== value)))
				break;
		if (enabled)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
			clog << "    Lower_Bound_Value_Slider set to " << value << endl;
			#endif
			enabled = Lower_Bound_Value_Slider->blockSignals (true);
			Lower_Bound_Value_Slider->setValue (value);
			Lower_Bound_Value_Slider->blockSignals (enabled);
			}
		}

	//	Prevent range endpoints overlap.
	if (upper_bound_value (band) <= value)
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    upper bound overlap moved from "
				<< upper_bound_value (band) << " to "
				<< (value + 1) << endl;
		#endif
		upper_bound_value (value + 1, band);
		}

	if (changed)
		{
		//	Reposition the corresponding node.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
		clog << "    reposition Nodes[" << band << "] lower_bound to "
				<< value << endl;
		#endif
		Nodes[band]->lower_bound (value);

		if (Signal_Changes)	//	Signaling data maps changes not deferred.
			{
			Graph->replot ();

			QVector<int>
				values (3, -1);
			values[band] = value;
			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
							DEBUG_CONTROL_PANEL | \
							DEBUG_BOUNDS | \
							DEBUG_SIGNALS))
			clog << "^^^ Data_Mapper_Tool::lower_bound_value: "
					"emit lower_bound_values_changed "
					<< values[0] << ", "
					<< values[1] << ", "
					<< values[2] << endl;
			#endif
			emit lower_bound_values_changed (values);

			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
							DEBUG_CONTROL_PANEL | \
							DEBUG_BOUNDS | \
							DEBUG_SIGNALS))
			clog << "^^^ Data_Mapper_Tool::lower_bound_value: "
					"emit data_maps_changed" << endl;
			#endif
			emit data_maps_changed (Data_Maps);
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::lower_bound_value: "
		<< boolalpha << actually_changed << endl;
#endif
return actually_changed;
}


bool
Data_Mapper_Tool::lower_bound_values
	(
	const QVector<int>&	values
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::lower_bound_values: "
		<< values[0] << ", " << values[1] << ", " << values[2] << endl;
#endif
bool
	changed = false,
	signal_changes = Signal_Changes;
Signal_Changes = false;

QVector<int>
	changed_values (values);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (values[band] >= 0)
		{
		if (lower_bound_value (values[band], band))
			changed = true;
		else
			changed_values[band] = -1;
		}
	}

Signal_Changes = signal_changes;
if (changed &&
	Signal_Changes)
	{
	Graph->replot ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::lower_bound_values: "
			"emit lower_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit lower_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_CONTROL_PANEL | \
					DEBUG_BOUNDS | \
					DEBUG_SIGNALS))
	clog << "^^^ Data_Mapper_Tool::lower_bound_values: "
			"emit data_maps_changed" << endl;
	#endif
	emit data_maps_changed (Data_Maps);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_CONTROL_PANEL | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::lower_bound_values: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
Data_Mapper_Tool::upper_limit
	(
	int		limit
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::upper_limit: " << limit << endl;
#endif
Upper_Limit = limit;

limit = max_x () - limit;	//	Convert to image value.
if (limit < 0)
	limit = 0;
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << "    max_x - limit = " << limit << endl;
#endif

bool
	change = false;
QVector<int>
	values (3, -1);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
	clog << "    band " << band << " upper_bound_value = "
			<< upper_bound_value (band) << endl;
	#endif
	if (upper_bound_value (band) > limit)
		{
		//	Move the saturation bound to the limit.
		change = true;
		values[band] = limit;
		}
	}
if (change)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
	clog << "    changed upper_bound_values = "
			<< values[0] << ", " << values[1] << ", " << values[2] << endl;
	#endif
	upper_bound_values (values);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::upper_limit" << endl;
#endif
}


void
Data_Mapper_Tool::lower_limit
	(
	int		limit
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << ">>> Data_Mapper_Tool::lower_limit: " << limit << endl;
#endif
Lower_Limit = limit;

bool
	change = false;
QVector<int>
	values (3, -1);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
	clog << "    band " << band << " lower_bound_value = "
			<< lower_bound_value (band) << endl;
	#endif
	if (lower_bound_value (band) < limit)
		{
		//	Move the saturation bound to the limit.
		change = true;
		values[band] = limit;
		}
	}
if (change)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
	clog << "    changed lower_bound_values = "
			<< values[0] << ", " << values[1] << ", " << values[2] << endl;
	#endif
	lower_bound_values (values);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << "<<< Data_Mapper_Tool::lower_limit" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Graphing
*/
/*
class Plot_Zoomer
:	public QwtPlotZoomer
{
public:

Plot_Zoomer
	(
	int				xAxis,
	int				yAxis,
	QwtPlotCanvas	*canvas
	)
	:	QwtPlotZoomer (xAxis, yAxis, canvas)
{
setSelectionFlags (QwtPicker::DragSelection | QwtPicker::CornerToCorner);
setTrackerMode (QwtPicker::AlwaysOff);
setRubberBand (QwtPicker::NoRubberBand);

// Ctrl+RightButton: zoom out to full size
setMousePattern (QwtEventPattern::MouseSelect2,
	Qt::RightButton, Qt::ControlModifier);
// RightButton: zoom out by 1
setMousePattern (QwtEventPattern::MouseSelect3,
    Qt::RightButton);
}
};
*/


QWidget*
Data_Mapper_Tool::graph_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << ">>> Data_Mapper_Tool::graph_panel " << object_pathname (this) << endl;
#endif
//	The plot.
Graph = new QwtPlot;
Graph->setAutoReplot (false);
Graph->setAutoDelete (false);
Graph->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
Graph->setMinimumSize (GRAPH_MIN_SIZE);
Graph->setCanvasBackground (QBrush(Default_Graph_Canvas_Color));
//Graph->setMargin (5);

//	Disable the context menu event on the graph.
Graph->setContextMenuPolicy (Qt::NoContextMenu);

//	Grid lines for the plot.
QwtPlotGrid
	*grid = new QwtPlotGrid;
grid->enableXMin (true);
grid->enableYMin (true);
grid->setMajorPen (QPen (Qt::black, 0, Qt::DotLine));
grid->setMinorPen (QPen (Qt::gray,  0, Qt::DotLine));
grid->attach (Graph);

//	Graph axes.
QFont
	axis_title_font (font ());
axis_title_font.setBold (true);
QwtText
	text;
text.setFont (axis_title_font);

#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << "    Source (x) axis: 0 - " << MAX_DISPLAY_VALUE << endl;
#endif
Graph->setAxisScale (QwtPlot::xBottom, 0, MAX_DISPLAY_VALUE);
Graph->setAxisFont  (QwtPlot::xBottom, font ());
text.setText (tr ("Source Values"));
Graph->setAxisTitle (QwtPlot::xBottom, text);

#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << "    Display (y) axis: 0 - " << MAX_DISPLAY_VALUE << endl;
#endif
Graph->setAxisScale (QwtPlot::yLeft, 0, MAX_DISPLAY_VALUE);
Graph->setAxisFont  (QwtPlot::yLeft, font ());
text.setText (tr ("Display Values"));
Graph->setAxisTitle (QwtPlot::yLeft, text);

//	Source-to-Display Plots.
for (int
		band = 0;
		band < 3;
		band++)
	{
	//	Function data map.
	Functions[band] = new Function (Data_Maps[band]);
	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	clog << "    Functions[" << band << "] @ "
			<< (void*)Functions[band] << endl;
	#endif

	Function_Plots[band] = new QwtPlotCurve ();
	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	clog << "    Function_Plots[" << band << "] @ " << endl
			<< (void*)Function_Plots[band] << endl;
	#endif
	Function_Plots[band]->setXAxis (QwtPlot::xBottom);
	Function_Plots[band]->setYAxis(QwtPlot::yLeft);
	Function_Plots[band]->setZ (PLOT_DEPTH[band] - .5);
	Function_Plots[band]->setStyle (QwtPlotCurve::Steps);
	Function_Plots[band]->setRenderHint (QwtPlotItem::RenderAntialiased);
	Function_Plots[band]->setPen (QPen (DISPLAY_BAND_COLORS[band]));
	Function_Plots[band]->setData (Functions[band]->data());
	Function_Plots[band]->attach (Graph);

	//	Function Nodes.
	Nodes[band] = new Function_Nodes (Data_Maps[band]);
	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	clog << "    Nodes[" << band << "] @ "
			<< (void*)Nodes[band] << endl;
	#endif

	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	Nodes[band]->add_node (max_x (), band * 100);
	#endif

	Node_Plots[band] = new QwtPlotCurve ();
	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	clog << "    Node_Plots[" << band << "] @ "
			<< (void*)Node_Plots[band] << endl;
	#endif
	Node_Plots[band]->setXAxis (QwtPlot::xBottom);
	Node_Plots[band]->setYAxis(QwtPlot::yLeft);
	Node_Plots[band]->setZ (PLOT_DEPTH[band]);
	Node_Plots[band]->setStyle (QwtPlotCurve::NoCurve);
	Node_Plots[band]->setSymbol (new QwtSymbol (QwtSymbol::Diamond,
		QBrush (DISPLAY_BAND_COLORS[band]),
		QPen (DISPLAY_BAND_COLORS[band]), QSize (7, 7)));
	Node_Plots[band]->setRenderHint (QwtPlotItem::RenderAntialiased);
	Node_Plots[band]->setPen (QPen (DISPLAY_BAND_COLORS[band]));
	Node_Plots[band]->setData (Nodes[band]->data());
	Node_Plots[band]->attach (Graph);

	#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
	//	For testing purposes.
	Nodes[band]->move_node (QPoint (), QPoint (band * 25, 0));
	Nodes[band]->move_node
		(QPoint (max_x (), max_y ()),
		 QPoint (max_x () - (band * 25), max_y ()));
	#endif
	}

//	Node selection symbol/marker.
Selection_Symbol =
	new QwtSymbol (QwtSymbol::Diamond,
		QBrush (Qt::white), QPen (Qt::white), QSize (9, 9));
Selection_Symbol->setBrush (QColor
	(DISPLAY_BAND_COLORS[0] |
	 DISPLAY_BAND_COLORS[1] |
	 DISPLAY_BAND_COLORS[2]));
Selection_Marker = new QwtPlotMarker;
Selection_Marker->setSymbol (Selection_Symbol);
Selection_Marker->setZ (MARKER_DEPTH);
Selection_Marker->hide ();
Selection_Marker->attach (Graph);

//	Bounds selection marker.
Bound_Marker = new QwtPlotMarker;
Bound_Marker->setLineStyle (QwtPlotMarker::VLine);
Bound_Marker->setLinePen (QPen (Qt::black));
Bound_Marker->setZ (MARKER_DEPTH);
Bound_Marker->hide ();
Bound_Marker->attach (Graph);

//	Plotting add-ons:

//	Cursor tracking.
Tracker = new Graph_Tracker ((QwtPlotCanvas*)Graph->canvas ());
Tracker->setRubberBandPen (QColor (Qt::yellow));
Tracker->setRubberBand (QwtPicker::CrossRubberBand);
Tracker->setTrackerPen (QColor (Qt::black));
Tracker->setEnabled (true);

//	Custom crosshair cursor.
Tracker->set_cursor (*Crosshair_Cursor);

connect (Tracker,
	SIGNAL (position (const QPoint&)),
	SLOT   (graph_position (const QPoint&)));
connect (Tracker,
	SIGNAL (appended (const QPointF&)),
	SLOT   (mouse_down (const QPointF&)));
connect (Tracker,
	SIGNAL (moved (const QPointF&)),
	SLOT   (mouse_drag (const QPointF&)));
connect (Tracker,
	SIGNAL (selected (const QPointF&)),
	SLOT   (mouse_up (const QPointF&)));
connect (Tracker,
	SIGNAL (leave_widget ()),
	SLOT (leave_graph ()));

/*
//	Panning.
Panner = new QwtPlotPanner (Graph->canvas ());
Panner->setMouseButton (Qt::MidButton);

//	Zooming.
Zoomer = new Plot_Zoomer (QwtPlot::xBottom, QwtPlot::yLeft, Graph->canvas());
Zoomer->setRubberBand (QwtPicker::RectRubberBand);
Zoomer->setRubberBandPen (QColor (Qt::yellow));
Zoomer->setTrackerMode (QwtPicker::ActiveOnly);
Zoomer->setTrackerPen (QColor (Qt::black));

enableZoomMode (false);
*/

Graph->setAutoReplot (true);

#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << "<<< Data_Mapper_Tool::image_panel" << endl;
#endif
return Graph;
}


bool
Data_Mapper_Tool::visible_graph
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">>> Data_Mapper_Tool::visible_graph: " << boolalpha << enabled << endl
	 << "    Graph isVisible = " << Graph->isVisible () << endl
	 << "      Visible_Graph = " << Visible_Graph << endl;
#endif
bool
	visible = Visible_Graph;
Graph->setVisible (Visible_Graph = enabled);
if (visible != Visible_Graph)
	{
	if (Visible_Graph)
		Minimum_Size.rwidth () += Graph_Width_Increment;
	else
		Minimum_Size.rwidth () -= Graph_Width_Increment;
	#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
	clog << "    Minimum_Size = " << Minimum_Size << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << "    Graph isVisible = " << Graph->isVisible () << endl
	 << "      Visible_Graph = " << Visible_Graph << endl
	 << "<<< Data_Mapper_Tool::visible_graph: " << visible << endl;
#endif
return visible;
}


void
Data_Mapper_Tool::refresh_graph ()
{
#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << ">>> Data_Mapper_Tool::refresh_graph" << endl;
#endif
if (Graph)
	Graph->replot ();
#if ((DEBUG_SECTION) & DEBUG_PLOT_PANEL)
clog << "<<< Data_Mapper_Tool::refresh_graph" << endl;
#endif
}


/*
void
Data_Mapper_Tool::enableZoomMode
	(
	bool	on
	)
{
Panner->setEnabled (on);

Zoomer->setEnabled (on);
Zoomer->zoom (0);

Tracker->setEnabled (! on);
}
*/

/*------------------------------------------------------------------------------
	Graph Tracker
*/
void
Data_Mapper_Tool::graph_position
	(
	const QPoint&	position
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
clog << ">>> Data_Mapper_Tool::graph_position: " << position << endl
	 << "    Operating_Mode = " << Operating_Mode << endl;
#endif
if (! (Operating_Mode & DRAGGING_MODE))
	{
	if (Operating_Mode & SATURATION_MODE)
		{
		/*	selection codes:

			x - upper (1) or lower (-1) bound selected (0 if no selection)
			y - band SELECTED_XXX flags.
		*/
		This_Node = selected_bounds (position);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
		clog << "    selected_bounds = " << This_Node << endl;
		#endif
		Node_Bands_Selected = This_Node.ry ();

		QRgb
			color = 0;
		QCursor
			*cursor;
		if (Node_Bands_Selected)
			{
			if (Node_Bands_Selected & SELECTED_RED)
				color |= DISPLAY_BAND_COLORS[0];
			if (Node_Bands_Selected & SELECTED_GREEN)
				color |= DISPLAY_BAND_COLORS[1];
			if (Node_Bands_Selected & SELECTED_BLUE)
				color |= DISPLAY_BAND_COLORS[2];

			if (This_Node.rx () < 0)
				//	Lower bound.
				cursor = Greater_Than_Cursors[Node_Bands_Selected];
			else
				//	Upper bound.
				cursor = Less_Than_Cursors[Node_Bands_Selected];
			}
		else
			{
			color = BLACK_COLOR;
			cursor = Move_Horizontal_Cursor;
			}
		if ((Bound_Marker->linePen ()).color () != color)
			{
			Bound_Marker->setLinePen (QPen (color));
			Tracker->set_cursor (*cursor);
			}

		if (! Bound_Marker->isVisible ())
			Bound_Marker->show ();

		Bound_Marker->setValue (position);
		}
	else
		{
		This_Node = selected_data_point (position);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
		clog << "    selected_data_point = " << This_Node << endl;
		#endif
		Node_Bands_Selected = This_Node.ry ();
		if (Node_Bands_Selected)
			{
			//	Determine the data coordinates of This_Node.
			if (Node_Bands_Selected & SELECTED_RED)
				This_Node.ry () = Functions[0]->y (This_Node.rx ());
			else
			if (Node_Bands_Selected & SELECTED_GREEN)
				This_Node.ry () = Functions[1]->y (This_Node.rx ());
			else
			if (Node_Bands_Selected & SELECTED_BLUE)
				This_Node.ry () = Functions[2]->y (This_Node.rx ());
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
			clog << "                        = " << This_Node << endl;
			#endif

			if (This_Node.rx () != (int)(Selection_Marker->xValue ()))
				Selection_Marker->setXValue (This_Node.rx ());
			if (This_Node.ry () != (int)(Selection_Marker->yValue ()))
				Selection_Marker->setYValue (This_Node.ry ());

			QRgb
				color = 0;
			if (Node_Bands_Selected & SELECTED_RED)
				color |= DISPLAY_BAND_COLORS[0];
			if (Node_Bands_Selected & SELECTED_GREEN)
				color |= DISPLAY_BAND_COLORS[1];
			if (Node_Bands_Selected & SELECTED_BLUE)
				color |= DISPLAY_BAND_COLORS[2];
			if ((Selection_Symbol->brush ()).color () != color)
				{
				Selection_Symbol->setBrush (QColor (color));
				Selection_Marker->setSymbol (Selection_Symbol);
				}

			if (! Selection_Marker->isVisible ())
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
				clog << "    show Selection_Marker" << endl
					 << "    set TRACKER_DRAG_MODE" << endl;
				#endif
				Selection_Marker->show ();
				//Tracker->setSelectionFlags (TRACKER_DRAG_MODE);
				}
			}
		else
		if (Selection_Marker->isVisible ())
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
			clog << "    hide Selection_Marker" << endl
				 << "    set TRACKER_TRACK_MODE" << endl;
			#endif
			Selection_Marker->hide ();
			//Tracker->setSelectionFlags (TRACKER_TRACK_MODE);
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
clog << "<<< Data_Mapper_Tool::graph_position" << endl;
#endif
}


void
Data_Mapper_Tool::mouse_down
	(
	const QPointF& point
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">>> Data_Mapper_Tool::mouse_down: " << point << endl
	 << "    This_Node = " << This_Node << endl;
#endif
if (Node_Bands_Selected)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
	clog << "    Node_Bands_Selected = " << Node_Bands_Selected << endl;
	#endif
	Operating_Mode |= DRAGGING_MODE;

	if (Operating_Mode & SATURATION_MODE)
		{
		Function_Nodes
			*function_nodes = NULL;
		if (Node_Bands_Selected & SELECTED_RED)
			function_nodes = Nodes[0];
		else
		if (Node_Bands_Selected & SELECTED_GREEN)
			function_nodes = Nodes[1];
		else
		if (Node_Bands_Selected & SELECTED_BLUE)
			function_nodes = Nodes[2];

		//	Carry forward the Upper/Lower bound flag.
		Last_Node_Index[0] = This_Node.rx ();
		if (This_Node.rx () < 0)
			{
			This_Node.rx () = function_nodes->lower_bound ();
			This_Node.ry () = 0;
			}
		else
			{
			This_Node.rx () = function_nodes->upper_bound ();
			This_Node.ry () = max_y ();
			}
		Tracker->set_cursor_position (QPoint
			(This_Node.rx (), (int)(point.y () + 0.5)));
		}
	else
		{
		QPolygon
			*nodes = NULL;
		int
			index,
			total_nodes,
			band = -1;
		while (++band < 3)
			{
			if (Node_Bands_Selected & (1 << band))
				{
				nodes = Nodes[band]->nodes ();
				total_nodes = nodes->size ();
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "    band " << band << " total nodes = "
						<< total_nodes << endl;
				#endif
				index = -1;
				while (++index < total_nodes &&
						(nodes->at (index)).x () <= This_Node.rx ()) ;

				if (index == total_nodes)
					//	This_Node is at max_x.
					--index;
				Next_Node_Index[band] = index;
				--index;
				if (index &&
					nodes->at (index) == This_Node)
					//	Existing node selected.
					--index;
				Last_Node_Index[band] = index;
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "    last node[" << band << "][" << Last_Node_Index[band]
						<< "] = " << nodes->at (Last_Node_Index[band]) <<endl
					 << "    next node[" << band << "][" << Next_Node_Index[band]
			 			<< "] = " << nodes->at (Next_Node_Index[band]) << endl;
				#endif
				}
			else
				{
				Next_Node_Index[band] = -1;
				Last_Node_Index[band] = -1;
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "    last node[" << band << "][" << Last_Node_Index[band]
						<< "]" <<endl
					 << "    next node[" << band << "][" << Next_Node_Index[band]
			 			<< "]" << endl;
				#endif
				}
			}

		Tracker->set_cursor_position (This_Node);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << "<<< Data_Mapper_Tool::mouse_down" << endl;
#endif
}


void
Data_Mapper_Tool::mouse_drag
	(
	const QPointF&	point
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">>> Data_Mapper_Tool::mouse_drag: " << point << endl
	 << "    Node_Bands_Selected = " << Node_Bands_Selected << endl;
#endif
if (Node_Bands_Selected &&
	(Operating_Mode & DRAGGING_MODE))
	{
	int
		band;
	QPoint
		node,
		here (This_Node);
	This_Node.rx () = static_cast<int>(point.x () + 0.5);
	This_Node.ry () = static_cast<int>(point.y () + 0.5);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
	clog << "    selection_distance = " << selection_distance () << endl
		 << "       old This_Node = " << here << endl
		 << "       new This_Node = " << This_Node << endl;
	QPoint
		there (This_Node);
	#endif

	//	Constrain the point vertically to within the graph data range.
	if (This_Node.ry () < 0)
		This_Node.ry () = 0;
	else
	if (This_Node.ry () > max_y ())
		This_Node.ry () = max_y ();
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
	if (This_Node != there)
		{
		clog << "        This_Node = " << This_Node
				<< " constrained vertically" << endl;
		there = This_Node;
		}
	#endif

	if (Operating_Mode & SATURATION_MODE)
		{
		//	Constrain the point horizontally to within the graph data range.
		if (This_Node.rx () < 0)
			This_Node.rx () = 0;
		else
		if (This_Node.rx () > max_x ())
			This_Node.rx () = max_x ();
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
		if (This_Node != there)
			clog << "        This_Node = " << This_Node
					<< " constrained horizontally" << endl;
		#endif

		if (This_Node != here)
			{
			QVector<int>
				values (3, This_Node.rx ());
			band = -1;
			while (++band < 3)
				if (! (Node_Bands_Selected & (1 << band)))
					values[band] = -1;

			//	Upper/Lower bound flag carried in Last_Node_Index[0].
			if (Last_Node_Index[0] < 0)
				lower_bound_values (values);
			else
				upper_bound_values (values);

			//	Move the Selection_Marker to the new position.
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
			clog << "    move Selection_Marker to " << This_Node << endl;
			#endif
			Bound_Marker->setValue (This_Node);
			}
		}
	else
		{
		//	Constrain/merge horizontally within the bounding nodes range.
		int
			above = This_Node.ry () + selection_distance (),
			below = This_Node.ry () - selection_distance ();

		//	Possible saturation bounds changes.
		int
			bounds_changed = 0;
		QVector<int>
			changed_bounds (3, -1);
		band = -1;
		while (++band < 3)
			{
			if (Node_Bands_Selected & (1 << band))
				{
				/*	>>> CAUTION << Check the next node first.
					If it gets merge with this node it will not affect
					the value of the Last_Node_Index. Merging with the last
					node will change the value of the Next_Node_Index.
				*/
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "     last node[" << band << "]["
						<< Last_Node_Index[band] << "]" << endl
					 << "     next node[" << band << "]["
						<< Next_Node_Index[band] << "]" << endl
					 << "    total nodes = " << Nodes[band]->size () << endl;
				#endif
				node = Nodes[band]->nodes ()->at (Next_Node_Index[band]);
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "     next node[" << band << "]["
						<< Next_Node_Index[band] << "] = " << node << endl;
				#endif
				if (This_Node.rx () >= node.rx ())
					{
					//	This_Node is at or beyond next node.
					if (Next_Node_Index[band]
							== (Last_Node_Index[band] + 1))
						//	This_Node is next node at max_x; constrain.
						This_Node.rx () = node.rx ();
					else
					if (node.ry () > above ||
						node.ry () < below)
						//	Far from next node; constrain.
						This_Node.rx () = node.rx () - 1;
					else
						{
						//	Merge with next node.
						This_Node.rx () = node.rx ();
						#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
						clog << "           This_Node = " << This_Node
								<< " merge with next node" << endl;
						#endif
						if (Next_Node_Index[band]
								== Nodes[band]->last_node_index ())
							{
							Next_Node_Index[band]--;
							#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
							clog << "     next node[" << band << "]["
									<< Next_Node_Index[band] << "] = "
									<< Nodes[band]->nodes ()->at
										(Next_Node_Index[band]) << endl;
							#endif
							}
						}
					}
				else
					{
					node = Nodes[band]->nodes ()->at (Last_Node_Index[band]);
					#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
					clog << "     last node[" << band << "]["
							<< Last_Node_Index[band] << "] = " << node << endl;
					#endif
					if (This_Node.rx () <= node.rx ())
						{
						//	This_Node is at or below last node.
						if (Next_Node_Index[band]
								== (Last_Node_Index[band] + 1))
							//	This_Node is last node at 0; constrain.
							This_Node.rx () = 0;
						else
						if (node.ry () > above ||
							node.ry () < below)
							//	Far from last node; constrain.
							This_Node.rx () = node.rx () + 1;
						else
							{
							//	Merge with last node.
							This_Node.rx () = node.rx ();
							#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
							clog << "           This_Node = " << This_Node
									<< " merge with last node" << endl;
							#endif
							Next_Node_Index[band]--;
							#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
							clog << "     next node[" << band << "]["
									<< Next_Node_Index[band] << "] = "
									<< Nodes[band]->nodes ()->at
										(Next_Node_Index[band]) << endl;
							#endif
							if (Last_Node_Index[band] > 0)
								{
								Last_Node_Index[band]--;
								#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
								clog << "     last node[" << band << "]["
										<< Last_Node_Index[band] << "] = "
										<< Nodes[band]->nodes ()->at
											(Last_Node_Index[band]) << endl;
								#endif
								}
							}
						}
					}

				//	Check for change to saturation bound.
				if (Next_Node_Index[band] == Nodes[band]->last_node_index ())
					{
					if (This_Node.ry () == max_y ())
						{
						//	This_Node is upper bound.
						bounds_changed = 1;
						changed_bounds[band] = This_Node.rx ();
						}
					else
					if (here.ry () == max_y ())
						{
						//	This_Node was upper bound, but not now.
						bounds_changed = 1;
						changed_bounds[band] = max_x ();
						}
					}
				if (Last_Node_Index[band] == 0)
					{
					if (This_Node.ry () == 0)
						{
						//	This_Node is lower bound.
						bounds_changed = -1;
						changed_bounds[band] = This_Node.rx ();
						}
					else
					if (here.ry () == 0)
						{
						//	This_Node was upper bound, but not now.
						bounds_changed = -1;
						changed_bounds[band] = 0;
						}
					}
				}
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
			if (This_Node != there)
				clog << "        This_Node = " << This_Node
						<< " constrained horizontally" << endl;
			#endif
			}

		if (This_Node != here)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
			clog << "    node moved from " << here
					<< " to " << This_Node << endl;
			#endif
			bool
				function_changed = false;
			band = -1;
			while (++band < 3)
				{
				if (Node_Bands_Selected & (1 << band))
					{
					//	Move the node.
					#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
					clog << "    move node for band " << band << endl;
					#endif
					below = Nodes[band]->move_node (here, This_Node);
					if ((below & Function_Nodes::ADDED_NODE) &&
						! (below & Function_Nodes::REMOVED_NODES_MASK))
						{
						//	This_Node was added to Nodes for the first time.
						if (Next_Node_Index[band]
								< Nodes[band]->last_node_index ())
							{
							Next_Node_Index[band]++;
							#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
							clog << "     next node[" << band << "]["
									<< Next_Node_Index[band] << "] = "
									<< Nodes[band]->nodes ()->at
										(Next_Node_Index[band])
									<< " bumped by node insertion" << endl;
							#endif
							}
						}
					function_changed |= (below != 0);
					#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
					clog << "    total nodes = "
							<< Nodes[band]->size () << endl;
					#endif
					}
				}

			//	Move the Selection_Marker to the new position.
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
			clog << "    move Selection_Marker to " << This_Node << endl;
			#endif
			Selection_Marker->setValue (This_Node);

			if (bounds_changed)
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
				clog << "    saturation "
						<< (This_Node.ry () ? "upper" : "lower")
						<< " bounds changed: "
						<< changed_bounds[0] << ", "
						<< changed_bounds[1] << ", "
						<< changed_bounds[2] << endl;
				#endif
				if (bounds_changed > 0)
					upper_bound_values (changed_bounds);
				else
					lower_bound_values (changed_bounds);
				//	Note: The bounds change will signal data_maps_changed.
				}
			else
			if (function_changed)
				{
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING | DEBUG_SIGNALS))
				clog << "^^^ Data_Mapper_Tool::mouse_drag: "
						"emit data_maps_changed" << endl;
				#endif
				//	>>> SIGNAL <<<
				emit data_maps_changed (Data_Maps);
				}
			}
		}	//	Function node manipulation.
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << "<<< Data_Mapper_Tool::mouse_drag" << endl;
#endif
}


void
Data_Mapper_Tool::mouse_up
	(
	const QPointF&
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">-< Data_Mapper_Tool::mouse_up" << endl;
#endif
Operating_Mode &= ~DRAGGING_MODE;

if (Operating_Mode & SATURATION_MODE)
	{
	Bound_Marker->setLinePen (QPen (BLACK_COLOR));
	Tracker->set_cursor (*Move_Horizontal_Cursor);
	}
}


void
Data_Mapper_Tool::leave_graph ()
{
if (Selection_Marker->isVisible ())
	Selection_Marker->hide ();
if (Bound_Marker->isVisible ())
	Bound_Marker->hide ();
}

/*==============================================================================
	Helpers
*/
void
Data_Mapper_Tool::linear_interpolation
	(
	int			x0,
	int			y0,
	int			x1,
	int			y1,
	int			band
	)
{
if (band < 0)
	{
	linear_interpolation (x0, y0, x1, y1, Data_Maps[0]);
	linear_interpolation (x0, y0, x1, y1, Data_Maps[1]);
	linear_interpolation (x0, y0, x1, y1, Data_Maps[2]);
	}
else
if (band < 3)
	linear_interpolation (x0, y0, x1, y1, Data_Maps[band]);
}


void
Data_Mapper_Tool::linear_interpolation
	(
	const QPoint&	start,
	const QPoint&	end,
	int				band
	)
{linear_interpolation (start.x (), start.y (), end.x (), end.y (), band);}


void
Data_Mapper_Tool::linear_interpolation
	(
	int				band_selections,
	const QPoint&	start,
	const QPoint&	end
	)
{
if (band_selections)
	{
	if (band_selections & SELECTED_RED)
		linear_interpolation
			(start.x (), start.y (), end.x (), end.y (), Data_Maps[0]);
	if (band_selections & SELECTED_GREEN)
		linear_interpolation
			(start.x (), start.y (), end.x (), end.y (), Data_Maps[1]);
	if (band_selections & SELECTED_BLUE)
		linear_interpolation
			(start.x (), start.y (), end.x (), end.y (), Data_Maps[2]);
	}
}

QPoint
Data_Mapper_Tool::selected_data_point
   (
   const QPoint&  position
   ) const
{
QPoint
   selection (-1, SELECTED_NONE);
double
   min_distance = selection_distance () * selection_distance ();
int
   band,
   band_count,
   bands_selected,
   max_bands = 0,
   active_bands = selected_bands (),
   active_band_mask,
   index = -1;

   for (band = 0,
         band_count = 0,
         bands_selected = SELECTED_NONE,
         active_band_mask = 1;
       band < 3;
      ++band,
         active_band_mask <<= 1)
      {
      if (active_bands & active_band_mask)
      {
         index = Function_Plots[band]->closestPoint(position, &min_distance);
      }
      else
            {
            bands_selected |= active_band_mask;
            ++band_count;
            }
      }

   if (band_count > max_bands)
      {
      // Best selection.
      selection.rx () = Function_Plots[band]->sample(index).x();
      selection.ry () = bands_selected;
      max_bands = band_count;
      }

    return selection;
}
/*
QPoint
Data_Mapper_Tool::selected_data_point
	(
	const QPoint&	position
	) const
{
QPoint
	selection (-1, SELECTED_NONE);
double
	min_distance = selection_distance () * selection_distance (),
	x_distance,
	y_distance;
int
	band,
	band_count,
	bands_selected,
	max_bands = 0,
	active_bands = selected_bands (),
	active_band_mask,
	x_position = position.x (),
	y_position = position.y (),
	x = max_x ();

while (x--)
	{
	x_distance = x - x_position;
	x_distance *= x_distance;

	for (band = 0,
			band_count = 0,
			bands_selected = SELECTED_NONE,
			active_band_mask = 1;
		 band < 3;
	   ++band,
			active_band_mask <<= 1)
		{
		if (active_bands & active_band_mask)
			{
			y_distance = Function_Plots[band]->y (x) - y_position;
			y_distance *= y_distance;
			y_distance += x_distance;
			if (y_distance < min_distance)
				{
				//	New minimum distance.
				min_distance = y_distance;
				bands_selected |= active_band_mask;
				band_count = 1;
				max_bands = 0;	//	Will force a selection update.
				}
			else
			if (y_distance == min_distance)
				{
				bands_selected |= active_band_mask;
				++band_count;
				}
			}
		}

	if (band_count > max_bands)
		{
		//	Best selection.
		selection.rx () = x;
		selection.ry () = bands_selected;
		max_bands = band_count;
		}
	}

return selection;
} */


QPoint
Data_Mapper_Tool::selected_bounds
	(
	const QPoint&	position
	) const
{
/*	selection codes:

	x - upper (1) or lower (-1) bound selected (0 if no selection)
	y - band SELECTED_XXX flags.
*/
QPoint
	selection (0, SELECTED_NONE);
int
	band = 3,
	active_bands = selected_bands (),
	active_band_mask,
	min_distance = selection_distance (),
	x_distance,
	x_position = position.x ();

for (band = 0,
		active_band_mask = 1;
	 band < 3;
   ++band,
   		active_band_mask <<= 1)
	{
	if (active_bands & active_band_mask)
		{
		//	Lower bound.
		x_distance = Nodes[band]->lower_bound () - x_position;
		if (x_distance < 0)
			x_distance = -x_distance;
		if (x_distance < min_distance)
			{
			min_distance = x_distance;
			selection.rx () = -1;
			selection.ry () = active_band_mask;
			}
		else
		if (x_distance == min_distance)
			{
			selection.rx () = -1;
			selection.ry () |= active_band_mask;
			}

		//	Upper bound
		x_distance = Nodes[band]->upper_bound () - x_position;
		if (x_distance < 0)
			x_distance = -x_distance;
		if (x_distance < min_distance)
			{
			min_distance = x_distance;
			selection.rx () = 1;
			selection.ry () = active_band_mask;
			}
		else
		if (x_distance == min_distance &&
			selection.rx () >= 0)	//	Lower bound has precedence.
			{
			selection.rx () = 1;
			selection.ry () |= active_band_mask;
			}
		}
	}

return selection;
}

/*------------------------------------------------------------------------------
	Save and Load graph data.
*/
void
Data_Mapper_Tool::save ()
{
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> Data_Mapper_Tool::save" << endl;
#endif
if (! Save_File_Dialog)
	{
	#if ((DEBUG_SECTION) & DEBUG_FILE)
	clog << "    constructing the Save_File_Dialog" << endl;
	#endif
	QStringList
		formats (HiView_Utilities::image_writer_formats ());
	//	Put the CSV format at the top of the list
	QString
		file_filters
			(HiView_Utilities::file_filter_for (Graph_Data_Format) + ";; ");
	file_filters += HiView_Utilities::image_writer_formats_file_filters ();

	/*	>>> Qt implementation used instead of native implementation <<<

		The Mac OS X native file dialog does not always allow links to files
		to be opened: the Open button remains disabled, and the display of the
		link is often incorrect. Though the native dialog is to be preferred,
		the strange and inconsistent behavior with links makes it unacceptable
		for use with HiView.
	*/
	Save_File_Dialog = new QFileDialog (this, tr ("Save Data Mapper File"),
		Graph_Directory, file_filters);
	Save_File_Dialog->setAcceptMode (QFileDialog::AcceptSave);
	Save_File_Dialog->setFileMode (QFileDialog::AnyFile);
	Save_File_Dialog->setOptions
		(QFileDialog::DontUseNativeDialog |
		 QFileDialog::DontResolveSymlinks);
	Save_File_Dialog->setDefaultSuffix (Graph_Data_Format);
	Save_File_Dialog->setConfirmOverwrite (true);
	connect (Save_File_Dialog,
				SIGNAL (filterSelected (const QString&)),
				SLOT (graph_data_format (const QString&)));
	}
if (Save_File_Dialog->exec ())
	{
	QString
		pathname (Save_File_Dialog->selectedFiles ().value (0));
	if (! pathname.isEmpty ())
		{
		if (pathname.length () > 2 &&
			pathname.at (0) == '~' &&
			pathname.at (1) == QDir::separator ())
			//	Substitute '~' with the user's home directory pathname.
			pathname.replace (0, 1, QDir::homePath ());

		Graph_Directory = QFileInfo (pathname).absolutePath ();
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << "    Graph_Directory = " << Graph_Directory << endl;
		#endif

		if (Graph_Data_Format.toLower () == "csv")
			save_CSV_file (pathname);
		else
			save_graph_image (pathname);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << "<<< Data_Mapper_Tool::save" << endl;
#endif
}


bool
Data_Mapper_Tool::save_CSV_file
	(
	const QString&	pathname
	)
{
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> save_CSV_file: " << pathname << endl;
#endif
bool
	saved = false;
if (! pathname.isEmpty ())
	{
	QFile
		file (pathname);
	if (file.open
			(QIODevice::WriteOnly |
			 QIODevice::Truncate |
			 QIODevice::Text))
		{
		QTextStream
			stream (&file);
		stream
			<< "# HiView Data Mapper Graph Nodes\n"
			<< "# " << ID << '\n';

		QPolygon
			*nodes;
		QPoint
			node;
		int
			index,
			total;
		for (int
				band = 0;
				band < 3;
			  ++band)
			{
			stream << "#\n";

			nodes = Nodes[band]->nodes ();
			if (! nodes)
				continue;
			stream << DISPLAY_BAND_NAMES[band] << '\n';
			total = nodes->size ();
			index = -1;
			while (++index < total)
				{
				node = nodes->at (index);
				stream << node.x () << ", " << node.y () << '\n';
				}
			}
		stream.flush ();
		file.close ();
		saved = true;

		last_CSV_file (pathname);
		}
	else
		{
		QString
			report
			("The data mapper graph file could not be opened -\n");
		report += pathname;
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << report << endl;
		#endif
		if (Error_Message)
			Error_Message->showMessage (report.replace ("\n", "<br>"));
		}
	}
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << "<<< save_CSV_file: " << boolalpha << saved << endl;
#endif
return saved;
}


bool
Data_Mapper_Tool::save_graph_image
	(
	const QString&	pathname
	)
{
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> save_graph_image: " << pathname << endl;
#endif
bool
	saved = false;
if (! pathname.isEmpty ())
	{
	QImage
		rendered_image (Graph->size (), QImage::Format_RGB32);
	Graph->render (&rendered_image);
	if (rendered_image.save (pathname, Graph_Data_Format.toLatin1()/*.toAscii ()*/))
		saved = true;
	else
		{
		QString
			report
			("Could not write the data mapper graph image file -\n");
		report += pathname;
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << report << endl;
		#endif
		if (Error_Message)
			Error_Message->showMessage (report.replace ("\n", "<br>"));
		}
	}
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << "<<< save_graph_image: " << boolalpha << saved << endl;
#endif
return saved;
}


void
Data_Mapper_Tool::graph_data_format
	(
	const QString&	format
	)
{
Graph_Data_Format = format.left (format.indexOf (' ')).toLower ();
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">-< Data_Mapper_Tool::graph_data_format: " << format
		<< " (" << Graph_Data_Format << ')' << endl;
#endif
Save_File_Dialog->setDefaultSuffix (Graph_Data_Format);
}


void
Data_Mapper_Tool::load ()
{
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> Data_Mapper_Tool::load" << endl;
#endif
if (! Load_File_Dialog)
	{
	QString
		file_filters ("All Files (*);; CSV (*.CSV *.csv)");

	/*	>>> Qt implementation used instead of native implementation <<<

		The Mac OS X native file dialog does not always allow links to files
		to be opened: the Open button remains disabled, and the display of the
		link is often incorrect. Though the native dialog is to be preferred,
		the strange and inconsistent behavior with links makes it unacceptable
		for use with HiView.
	*/
	Load_File_Dialog = new QFileDialog (this, tr ("Load Data Mapper File"),
		Graph_Directory, file_filters);
	Load_File_Dialog->setAcceptMode (QFileDialog::AcceptOpen);
	Load_File_Dialog->setFileMode (QFileDialog::ExistingFile);
	Load_File_Dialog->setOptions
		(QFileDialog::DontUseNativeDialog |
		 QFileDialog::DontResolveSymlinks);
	}
if (Load_File_Dialog->exec ())
	{
	QString
		pathname (Load_File_Dialog->selectedFiles ().value (0));
	if (! pathname.isEmpty ())
		{
		Graph_Directory = QFileInfo (pathname).absolutePath ();

		if (pathname.at (0) == '~' &&
			pathname.at (1) == QDir::separator ())
			//	Substitute '~' with the user's home directory pathname.
			pathname.replace (0, 1, QDir::homePath ());

		load_CSV_file (pathname);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << "<<< Data_Mapper_Tool::load" << endl;
#endif
}


bool
Data_Mapper_Tool::load_CSV_file
	(
	const QString&	pathname
	)
{
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> Data_Mapper_Tool::load_CSV_file: " << pathname << endl;
#endif
if (! pathname.isEmpty ())
	{
	QFile
		file (pathname);
	if (! file.open
			(QIODevice::ReadOnly |
			 QIODevice::Text))
		{
		QString
			report
			("The data mapper graph file could not be opened -\n");
		report += pathname;
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << report << endl;
		#endif
		if (Error_Message)
			Error_Message->showMessage (report.replace ("\n", "<br>"));
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << "<<< Data_Mapper_Tool::load_CSV_file: false" << endl;
		#endif
		return false;
		}

	QTextStream
		stream (&file);
	QString
		line;
	QStringList
		words;
	int
		line_count = 0,
		index,
		band = -1,
		entry,
		x, y;
	bool
		OK;
	QPolygon
		nodes[3];

	while (! stream.atEnd ())
		{
		line = stream.readLine ();
		line_count++;
		#if ((DEBUG_SECTION) & DEBUG_FILE)
		clog << "    " << line_count << ": \"" << line << '"' << endl;
		#endif
		line = line.trimmed ();
		if (line.startsWith ('#') ||
			line.length () == 0)
			continue;

		if (line.startsWith (DISPLAY_BAND_NAMES[0], Qt::CaseInsensitive))
			band = 0;
		else
		if (line.startsWith (DISPLAY_BAND_NAMES[1], Qt::CaseInsensitive))
			band = 1;
		else
		if (line.startsWith (DISPLAY_BAND_NAMES[2], Qt::CaseInsensitive))
			band = 2;
		else
		if (band >= 0)
			{
			if (! (Selected_Bands & (1 << band)))
				//	Don't load nodes for a band that is not selected.
				continue;

			words =
				line.split (QRegExp ("[\\s,]"), QString::SkipEmptyParts);
			if (! words.size ())
				continue;

			index = -1;
			while (++index < words.size ())
				{
				if (words.at (index).startsWith ('#'))
					break;
				if ((words.size () - index) == 1)
					{
					Invalid_Contents:
					file.close ();
					if (line.length () > 256)
						line.replace (252, line.length () - 252, "...");
					QString
						report = QString
						("Invalid data mapper graph file contents -\n"
						 "%1\n\n"
						 "at word \"%2\" of line %3: \"%4\"\n")
						.arg (pathname)
						.arg (words.at (index))
						.arg (line_count)
						.arg (line);
					#if ((DEBUG_SECTION) & DEBUG_FILE)
					clog << report << endl;
					#endif
					if (Error_Message)
						Error_Message->showMessage (report);
					#if ((DEBUG_SECTION) & DEBUG_FILE)
					clog << "<<< Data_Mapper_Tool::load_CSV_file: false"
							<< endl;
					#endif
					return false;
					}
				x = words.at (index).toInt (&OK, 0);
				if (! OK)
					goto Invalid_Contents;
				y = words.at (++index).toInt (&OK, 0);
				if (! OK)
					goto Invalid_Contents;
				if (x <= max_x () &&
					y <= max_y ())
					{
					//	Insert in increasing x order.
					for (entry = nodes[band].size () - 1;
						 entry >= 0 &&
						 	 x <= nodes[band].at (entry).x ();
						 entry--) ;
					if (nodes[band].size () &&
						entry > 0 &&
						nodes[band].at (entry).x () == x)
						//	Duplicate x position.
						continue;
					nodes[band].insert (++entry, QPoint (x, y));
					#if ((DEBUG_SECTION) & DEBUG_FILE)
					clog << "    nodes[" << band << "][" << entry << "] = "
							<< nodes[band].at (entry) << endl;
					#endif
					}
				#if ((DEBUG_SECTION) & DEBUG_FILE)
				else
					clog << "    out of range: "
							<< x << "x, " << y << 'y' << endl;
				#endif
				}
			}
		else
			{
			file.close ();
			if (line.length () > 256)
				line.replace (252, line.length () - 252, "...");
			QString
				report = QString
				("Invalid data mapper graph file contents -\n"
				 "%1\n\n"
				 "at line %2: \"%3\"\n")
				.arg (pathname)
				.arg (line_count)
				.arg (line);
			#if ((DEBUG_SECTION) & DEBUG_FILE)
			clog << report << endl;
			#endif
			if (Error_Message)
				Error_Message->showMessage (report.replace ("\n", "<br>"));
			#if ((DEBUG_SECTION) & DEBUG_FILE)
			clog << "<<< Data_Mapper_Tool::load_CSV_file: false" << endl;
			#endif
			return false;
			}
		}	//	EOF
	file.close ();

	//	Apply the new function nodes.
	OK = false;
	Signal_Changes = false;
	QVector<int>
		upper_values (3, -1),
		lower_values (3, -1);
	for (band = 0;
		 band < 3;
	   ++band)
		{
		if (nodes[band].size ())
			{
			//	Ensure the required anchor nodes are present.
			if (nodes[band].at (0).x () != 0)
				nodes[band].insert (0, QPoint (0, 0));
			if (nodes[band].last ().x () != max_x ())
				nodes[band].append (QPoint (max_x (), max_y ()));

			if (*(Nodes[band]->nodes ()) != nodes[band])
				{
				OK = true;
				//	Reset the function nodes.
				*(Nodes[band]->nodes ()) = nodes[band];
				Nodes[band]->interpolate ();

				//	Saturation Bounds.
				upper_values[band] = Nodes[band]->upper_bound ();
				lower_values[band] = Nodes[band]->lower_bound ();

				#if ((DEBUG_SECTION) & DEBUG_FILE)
				clog << "    reset Nodes[" << band << "] -" << endl
					 << "      lower bound = "
						<< lower_values[band] << endl;
				for (entry = 0;
					 entry < nodes[band].size ();
					 entry++)
					 clog << "      " << entry << ": "
						 	<< nodes[band].at (entry) << endl;
				clog << "      upper bound = "
						<< upper_values[band] << endl;
				#endif
				}
			}
		}

	//	Reset the saturation bounds.
	upper_bound_values (upper_values);
	lower_bound_values (lower_values);

	Signal_Changes = true;
	if (OK)
		{
		Graph->replot ();

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_FILE | DEBUG_SIGNALS))
		clog << "^^^ Data_Mapper_Tool::load: "
				"emit data_maps_changed" << endl;
		#endif
		emit data_maps_changed (Data_Maps);
		}

	last_CSV_file (pathname);
	}
#if ((DEBUG_SECTION) & DEBUG_FILE)
clog << ">>> Data_Mapper_Tool::load_CSV_file: true" << endl;
#endif
return true;
}


void
Data_Mapper_Tool::last_CSV_file
	(
	const QString&	pathname
	)
{
QFileInfo
	file (Last_CSV_Pathname = pathname);
if (! file.exists () ||
	! file.isFile ())
	Last_CSV_Pathname.clear ();

if (Last_CSV_Pathname.isEmpty ())
	{
	Load_Last_CSV_File_Action->setEnabled (false);
	Save_Last_CSV_File_Action->setEnabled (false);
	File_Menu_Button->setToolTip ("");
	}
else
	{
	Load_Last_CSV_File_Action->setEnabled (true);
	Save_Last_CSV_File_Action->setEnabled (true);
	File_Menu_Button->setToolTip (tr ("Last file: ") + Last_CSV_Pathname);
	}
}


void
Data_Mapper_Tool::last_file_action ()
{
if (Last_CSV_Pathname.isEmpty ())
	{
	Load_Last_CSV_File_Action->setEnabled (false);
	Save_Last_CSV_File_Action->setEnabled (false);
	}
else
	{
	QObject
		*source = sender ();
	if (source == Load_Last_CSV_File_Action)
		load_CSV_file (Last_CSV_Pathname);
	else
	if (source == Save_Last_CSV_File_Action)
		save_CSV_file (Last_CSV_Pathname);
	}
}

/*==============================================================================
	Utilities
*/
void
Data_Mapper_Tool::print_data_map
	(
	std::ostream&	stream,
	Data_Map*		map
	)
{
stream << "    Data_Map @ " << (void*)map << hex << setfill ('0') << endl;
if (map)
	{
	for (int
			index = 0;
			index < map->size ();
		  ++index)
		{
		if (! (index % 16))
			{
			if (index)
				stream << endl;
			stream << dec << setfill (' ')
				 << setw (5) << index << ": "
				 << hex << setfill ('0');
			}
		stream << "  " << setw (2) << ((unsigned int)(map->at (index)) & 0xFF);
		}
	}
stream << dec << setfill (' ') << endl;
}


void
Data_Mapper_Tool::print_data_maps
	(
	std::ostream&	stream
	)
{
stream << "    Data_Maps @ " << (void*)Data_Maps << endl;
if (Data_Maps)
	{
	for (int
			band = 0;
			band < 3;
		  ++band)
		{
		stream << "    Data_Maps[" << band << "] -" << endl;
		print_data_map (stream, Data_Maps[band]);
		}
	}
}



}	//	namespace UA::HiRISE
