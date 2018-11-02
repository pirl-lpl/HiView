/*	Statistics_and_Bounds_Tool

HiROC CVS ID: $Id: Statistics_and_Bounds_Tool.cc,v 1.20 2012/03/09 02:13:59 castalia Exp $

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

#include	"Statistics_and_Bounds_Tool.hh"

#include	"HiView_Config.hh"
#include	"Graph_Tracker.hh"
#include	"Histogram_Plot.hh"

//	Qt
#include	<QWidget>
#include	<QPen>

//	Qwt
#include	<qwt_plot_marker.h>
#include	<qwt_symbol.h>
#include <qwt_picker_machine.h>

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SLOTS				(1 << 1)
#define DEBUG_SIGNALS			(1 << 2)
#define DEBUG_BOUNDS			(1 << 3)
#define DEBUG_REFRESH			(1 << 4)
#define DEBUG_GRAPH_POSITION	(1 << 5)
#define DEBUG_DRAGGING			(1 << 6)

#define DEBUG_DEFAULT			DEBUG_ALL

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
	Defaults
*/
#ifndef DEFAULT_SELECTION_DISTANCE
#define DEFAULT_SELECTION_DISTANCE		5
#endif
int
	Statistics_and_Bounds_Tool::Default_Selection_Distance =
		DEFAULT_SELECTION_DISTANCE;

/*==============================================================================
	Application configuration parameters
*/
#define Crosshair_Cursor \
	HiView_Config::Crosshair_Cursor
#define Greater_Than_Cursors \
	HiView_Config::Greater_Than_Cursors
#define Less_Than_Cursors \
	HiView_Config::Less_Than_Cursors

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

#define PERCENT_DECIMAL_PLACES \
	HiView_Config::PERCENT_DECIMAL_PLACES

/*==============================================================================
	Local constants
*/
#ifndef DOXYGEN_PROCESSING
enum
	{
	SELECTED_LOWER_BOUND = 1 << ((sizeof (int) << 3) - 1)
	};

enum
	{
	DRAGGING_MODE	= (1 << 0)
	};

#define MARKER_DEPTH	(Histogram_Plot::Default_Plot_Z + 10)
#endif

/*==============================================================================
	Constructors
*/
Statistics_and_Bounds_Tool::Statistics_and_Bounds_Tool
	(
	const QString&	title,
	QWidget*		parent,
	Qt::WindowFlags	flags
	)
	:	Statistics_Tool (title, parent, flags),
		Selected_Bounds (SELECTED_NONE),
		Bounds_Moving (false),
		Selected_Bands (SELECTED_ALL),
		Selection_Distance (Default_Selection_Distance)
{
if (title.isEmpty ())
	setObjectName ("Statistics_and_Bounds_Tool");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Statistics_and_Bounds_Tool: " << object_pathname (this) << endl;
#endif
//	Enable limits controls.
limits (true);

Lower_Limit_Marker = new QwtPlotMarker;
Lower_Limit_Marker->setLineStyle (QwtPlotMarker::VLine);
Lower_Limit_Marker->setLinePen (QPen (Qt::black));
Lower_Limit_Marker->setZ (MARKER_DEPTH);
Lower_Limit_Marker->setXValue (0);
Lower_Limit_Marker->attach (Graph);

Upper_Limit_Marker = new QwtPlotMarker;
Upper_Limit_Marker->setLineStyle (QwtPlotMarker::VLine);
Upper_Limit_Marker->setLinePen (QPen (Qt::black));
Upper_Limit_Marker->setZ (MARKER_DEPTH);
Upper_Limit_Marker->setXValue (max_x () + 1);
Upper_Limit_Marker->attach (Graph);

//	Initial saturation bounds.
Upper_Bound_Value[0] =
Upper_Bound_Value[1] =
Upper_Bound_Value[2] = max_x ();
Lower_Bound_Value[0] =
Lower_Bound_Value[1] =
Lower_Bound_Value[2] = 0;
Upper_Bound_Percent[0] =
Upper_Bound_Percent[1] =
Upper_Bound_Percent[2] =
Lower_Bound_Percent[0] =
Lower_Bound_Percent[1] =
Lower_Bound_Percent[2] = 0;

//	Saturation bounds selection marker.
Bound_Marker = new QwtPlotMarker;
Bound_Marker->setLineStyle (QwtPlotMarker::VLine);
Bound_Marker->setLinePen (QPen (Qt::black));
Bound_Marker->setZ (MARKER_DEPTH);
Bound_Marker->hide ();
Bound_Marker->attach (Graph);

/* TODO ???
Bounder = new QwtPickerDragRectMachine(TRACKER_BOUND_MODE);
Tracker = new QwtPickerDragRectMachine(TRACKER_TRACK_MODE);
*/
//	Saturation bounds markers.
int
	band = 3;
while (band--)
	{
	Upper_Bound_Marker[band] = new QwtPlotMarker;
	Upper_Bound_Marker[band]->setSymbol
		(new QwtSymbol (QwtSymbol::Rect,
		 QBrush (DISPLAY_BAND_COLORS[band]),
		 QPen (Qt::white),
		 QSize (2, (3 - band) * 20)));
	Upper_Bound_Marker[band]->setZ (MARKER_DEPTH + band);
	Upper_Bound_Marker[band]->setValue (Upper_Bound_Value[band], 0);
	Upper_Bound_Marker[band]->attach (Graph);

	Lower_Bound_Marker[band] = new QwtPlotMarker;
	Lower_Bound_Marker[band]->setSymbol
		(new QwtSymbol (QwtSymbol::Rect,
		 QBrush (DISPLAY_BAND_COLORS[band]),
		 QPen (Qt::black),
		 QSize (2, (3 - band) * 20)));
	Lower_Bound_Marker[band]->setZ (MARKER_DEPTH + band);
	Lower_Bound_Marker[band]->setValue (Lower_Bound_Value[band], 0);
	Lower_Bound_Marker[band]->attach (Graph);
	}

connect (Tracker,
	SIGNAL (position (const QPoint&)),
	SLOT   (graph_position (const QPoint&)));
connect (Tracker,
	SIGNAL (appended (const QwtDoublePoint&)),
	SLOT   (mouse_down (const QwtDoublePoint&)));
connect (Tracker,
	SIGNAL (moved (const QwtDoublePoint&)),
	SLOT   (mouse_drag (const QwtDoublePoint&)));
connect (Tracker,
	SIGNAL (selected (const QwtDoublePoint&)),
	SLOT   (mouse_up (const QwtDoublePoint&)));
connect (Tracker,
	SIGNAL (leave_widget ()),
	SLOT (leave_graph ()));
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Statistics_and_Bounds_Tool" << endl;
#endif
}


Statistics_and_Bounds_Tool::~Statistics_and_Bounds_Tool ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Statistics_and_Bounds_Tool: @ " << (void*)this << endl;
#endif
}

/*==============================================================================
	Accessors
*/
void
Statistics_and_Bounds_Tool::bands_selected
	(
	int		band_selections
	)
{Selected_Bands = band_selections;}


void
Statistics_and_Bounds_Tool::selection_distance
	(
	int		distance
	)
{
if (distance < 0)
	distance = 0;
Selection_Distance= distance;
}


bool
Statistics_and_Bounds_Tool::refresh
	(
	const QRect&	selected_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
clog << ">>> Statistics_and_Bounds_Tool::refresh: " << selected_region << endl;
#endif
if (! Statistics_Tool::refresh (selected_region))
	{
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
	clog << "    Statistics_Tool::refresh false" << endl
		 << "<<< Statistics_and_Bounds_Tool::refresh: false" << endl;
	#endif
	return false;
	}


#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
clog << "    upper percents -" << endl;
#endif
bool
	changed = false;
double
	percent;
QVector<double>
	percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	percent =
		Statistics.upper_percent_at_value (Upper_Bound_Value[band],
			//	N.B.: Display band mapped to Statistics set index.
			Display_to_Stats_Map[band]);
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
	clog << "    " << band << ": "
			<< Upper_Bound_Percent[band] << " -> " << percent << endl;
	#endif
	if (percent != Upper_Bound_Percent[band])
		{
		changed = true;
		percents[band] = percent;
		}
	}
if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::refresh: "
			"emit lower_bound_percents_changed "
			<< percents[0] << ", "
			<< percents[1] << ", "
			<< percents[2] << endl;
	#endif
	emit upper_bound_percents_changed (percents);
	}

#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
clog << "    lower percents -" << endl;
#endif
changed = false;
percents.fill (-1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	percent =
		Statistics.lower_percent_at_value (Lower_Bound_Value[band],
			//	N.B.: Display band mapped to Statistics set index.
			Display_to_Stats_Map[band]);
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
	clog << "    " << band << ": "
			<< Lower_Bound_Percent[band] << " -> " << percent << endl;
	#endif
	if (percent != Lower_Bound_Percent[band])
		{
		changed = true;
		percents[band] = percent;
		}
	}
if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::refresh: "
			"emit lower_bound_percents_changed "
			<< percents[0] << ", "
			<< percents[1] << ", "
			<< percents[2] << endl;
	#endif
	emit lower_bound_percents_changed (percents);
	}
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_BOUNDS))
clog << "<<< Statistics_and_Bounds_Tool::refresh: true" << endl;
#endif
return true;
}

/*==============================================================================
	Saturation bounds controls
*/
void
Statistics_and_Bounds_Tool::upper_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << ">>> Statistics_and_Bounds_Tool::upper_bound_percents: "
		<< percents[0] << ", " << percents[1] << ", " << percents[2] << endl;
#endif
bool
	changed = false;
int
	value;
QVector<int>
	changed_values (3, -1);
QVector<double>
	changed_percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (percents[band] < 0.0)
		continue;

	//	Find the value at which the percent occurs.
	value = Statistics.upper_value_at_percent (percents[band],
			//	N.B.: Display band mapped to Statistics set index.
			Display_to_Stats_Map[band]);
	if (value < 0)
		value = max_x ();
	#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
	clog << "    " << band << ": upper_value_at_percent = " << value << endl;
	#endif
	if (upper_bound_value (value, band))
		{
		changed = true;
		changed_values[band] = value;
		changed_percents[band] = Upper_Bound_Percent[band];
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
		clog << "    changed_percents[" << band << "] = "
				<< changed_percents[band] << endl;
		#endif
		}
	}

if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::upper_bound_values: "
			"emit upper_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit upper_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::upper_bound_values: "
			"emit upper_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit upper_bound_percents_changed (changed_percents);
	}
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << "<<< Statistics_and_Bounds_Tool::upper_bound_percent"
		<< endl;
#endif
}


void
Statistics_and_Bounds_Tool::lower_bound_percents
	(
	const QVector<double>&	percents
	)
{
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << ">>> Statistics_and_Bounds_Tool::lower_bound_percents: "
		<< percents[0] << ", " << percents[1] << ", " << percents[2] << endl;
#endif
bool
	changed = false;
int
	value;
QVector<int>
	changed_values (3, -1);
QVector<double>
	changed_percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (percents[band] < 0.0)
		continue;

	//	Find the value at which the percent occurs.
	value = Statistics.lower_value_at_percent (percents[band],
			//	N.B.: Display band mapped to Statistics set index.
			Display_to_Stats_Map[band]);
	if (value < 0)
		value = max_x ();
	#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
	clog << "    " << band << ": lower_value_at_percent = " << value << endl;
	#endif
	if (lower_bound_value (value, band))
		{
		changed = true;
		changed_values[band] = value;
		changed_percents[band] = Lower_Bound_Percent[band];
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
		clog << "    percent[" << band << "] = "
				<< changed_percents[band] << endl;
		#endif
		}
	}

if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::lower_bound_values: "
			"emit lower_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit lower_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::lower_bound_values: "
			"emit lower_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit lower_bound_percents_changed (changed_percents);
	}
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << "<<< Statistics_and_Bounds_Tool::lower_bound_percent"
		<< endl;
#endif
}


void
Statistics_and_Bounds_Tool::upper_bound_values
	(
	const QVector<int>&	values
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << ">>> Statistics_and_Bounds_Tool::upper_bound_values: "
		<< values[0] << ", " << values[1] << ", " << values[2] << endl;
#endif
bool
	changed = false;
QVector<int>
	changed_values (3, -1);
QVector<double>
	changed_percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (upper_bound_value (values[band], band))
		{
		changed = true;
		changed_values[band] = values[band];
		changed_percents[band] = Upper_Bound_Percent[band];
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
		clog << "    percent[" << band << "] = "
				<< changed_percents[band] << endl;
		#endif
		}
	}

if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::upper_bound_values: "
			"emit upper_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit upper_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::upper_bound_values: "
			"emit upper_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit upper_bound_percents_changed (changed_percents);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << "<<< Statistics_and_Bounds_Tool::upper_bound_values" << endl;
#endif
}


bool
Statistics_and_Bounds_Tool::upper_bound_value
	(
	int		value,
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << ">>> Statistics_and_Bounds_Tool::upper_bound_value: value "
		<< value << ", band " << band << endl;
#endif
bool
	changed = false;
if (value >= 0)
	{
	if (value < 1)
		value = 1;
	else
	if (value > max_x ())
		value = max_x ();
	#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
	clog << "    previous Upper_Bound_Value[" << band << "] = "
			<< Upper_Bound_Value[band] << endl;
	#endif
	if (Upper_Bound_Value[band] != value)
		{
		changed = true;
		Upper_Bound_Value[band] = value;
		Upper_Bound_Marker[band]->setXValue (value);

		//	If the value changed then the percent is different.
		#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
		clog << "    previous Upper_Bound_Percent[" << band << "] = "
				<< Upper_Bound_Percent[band] << endl;
		#endif
		Upper_Bound_Percent[band]
			= Statistics.upper_percent_at_value (value,
				//	N.B.: Display band mapped to Statistics set index.
				Display_to_Stats_Map[band]);
		#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
		clog << "         new Upper_Bound_Percent[" << band << "] = "
				<< Upper_Bound_Percent[band] << endl;
		#endif
		}
	}
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << "<<< Statistics_and_Bounds_Tool::upper_bound_value: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
Statistics_and_Bounds_Tool::lower_bound_values
	(
	const QVector<int>&	values
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << ">>> Statistics_and_Bounds_Tool::lower_bound_values: "
		<< values[0] << ", " << values[1] << ", " << values[2] << endl;
#endif
bool
	changed = false;
QVector<int>
	changed_values (3, -1);
QVector<double>
	changed_percents (3, -1.0);
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (lower_bound_value (values[band], band))
		{
		changed = true;
		changed_values[band] = values[band];
		changed_percents[band] = Lower_Bound_Percent[band];
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
		clog << "    percent[" << band << "] = "
				<< changed_percents[band] << endl;
		#endif
		}
	}

if (changed)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::lower_bound_values: "
			"emit lower_bound_values_changed "
			<< changed_values[0] << ", "
			<< changed_values[1] << ", "
			<< changed_values[2] << endl;
	#endif
	emit lower_bound_values_changed (changed_values);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS | DEBUG_SIGNALS))
	clog << "^^^ Statistics_and_Bounds_Tool::lower_bound_values: "
			"emit lower_bound_percents_changed "
			<< changed_percents[0] << ", "
			<< changed_percents[1] << ", "
			<< changed_percents[2] << endl;
	#endif
	emit lower_bound_percents_changed (changed_percents);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_BOUNDS))
clog << "<<< Statistics_and_Bounds_Tool::lower_bound_values" << endl;
#endif
}


bool
Statistics_and_Bounds_Tool::lower_bound_value
	(
	int		value,
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << ">>> Statistics_and_Bounds_Tool::lower_bound_value: value "
		<< value << ", band " << band << endl;
#endif
bool
	changed = false;
if (value >= 0)
	{
	if (value >= max_x ())
		value  = max_x () - 1;
		#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
		clog << "    Lower_Bound_Value = "
				<< Lower_Bound_Value[band] << endl;
		#endif
	if (Lower_Bound_Value[band] != value)
		{
		changed = true;
		Lower_Bound_Value[band] = value;
		Lower_Bound_Marker[band]->setXValue (value);

		//	If the value changed then the percent is different.
		Lower_Bound_Percent[band]
			= Statistics.lower_percent_at_value (value,
				//	N.B.: Display band mapped to Statistics set index.
				Display_to_Stats_Map[band]);
		#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
		clog << "    Lower_Bound_Percent = "
				<< Lower_Bound_Percent[band] << endl;
		#endif
		}
	}
#if ((DEBUG_SECTION) & DEBUG_BOUNDS)
clog << "<<< Statistics_and_Bounds_Tool::lower_bound_value: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
Statistics_and_Bounds_Tool::upper_limit
	(
	int		limit
	)
{
Statistics_Tool::upper_limit (limit);

Upper_Limit_Marker->setXValue (limit + 1);
}


void
Statistics_and_Bounds_Tool::lower_limit
	(
	int		limit
	)
{
Statistics_Tool::lower_limit (limit);

Lower_Limit_Marker->setXValue (limit);
}

/*------------------------------------------------------------------------------
	Graph Tracker
*/
void
Statistics_and_Bounds_Tool::graph_position
	(
	const QPoint&	position
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
clog << ">>> Statistics_and_Bounds_Tool::graph_position: " << position << endl;
#endif
if (! Bounds_Moving)
	{
	Selected_Bounds = selected_bounds (position);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
	clog << "    selected_bounds = " << Selected_Bounds << endl;
	#endif
	if (Selected_Bounds)
		{
		QRgb
			color = 0;
		QCursor
			*cursor;
		if (Selected_Bounds & SELECTED_RED)
			color |= DISPLAY_BAND_COLORS[0];
		if (Selected_Bounds & SELECTED_GREEN)
			color |= DISPLAY_BAND_COLORS[1];
		if (Selected_Bounds & SELECTED_BLUE)
			color |= DISPLAY_BAND_COLORS[2];

		if (Selected_Bounds & SELECTED_LOWER_BOUND)
			//	Lower bound.
			cursor = Greater_Than_Cursors
				[Selected_Bounds & SELECTED_ALL];
		else
			//	Upper bound.
			cursor = Less_Than_Cursors
				[Selected_Bounds & SELECTED_ALL];

		if (Bound_Marker->linePen ().color () != color)
			Bound_Marker->setLinePen (QPen (color));
		if (! Bound_Marker->isVisible ())
			{
			Bound_Marker->show ();
			//Tracker->setStateMachine/*setSelectionFlags*/(QwtPickerTrackerMachine(QwtPickerMachine::SelectionType::RectSelection));
			Tracker->setRubberBand (QwtPicker::NoRubberBand);
			}
		Tracker->set_cursor (*cursor);

		Last_Position.rx () = saturation_bound (Selected_Bounds);
		Last_Position.ry () = position.y ();
		Bound_Marker->setValue (Last_Position);
		}
	else
	if (Bound_Marker->isVisible ())
		{
		//	Moved away from bounds location.
		Bound_Marker->hide ();
		//Tracker->setStateMachine/*setSelectionFlags*/(QwtPickerTrackerMachine(QwtPickerMachine::SelectionType::PointSelection));
		Tracker->setRubberBand (QwtPicker::CrossRubberBand);
		Tracker->set_cursor (*Crosshair_Cursor);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_GRAPH_POSITION))
clog << "<<< Statistics_and_Bounds_Tool::graph_position" << endl;
#endif
}


void
Statistics_and_Bounds_Tool::mouse_down
	(
	const QwtDoublePoint&
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
		point
		#endif
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">>> Statistics_and_Bounds_Tool::mouse_down: " << point << endl
	 << "    Selected_Bounds = " << Selected_Bounds << endl;
#endif
if (Selected_Bounds)
	{
	Bounds_Moving = true;
	Tracker->set_cursor_position (Last_Position);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << "<<< Statistics_and_Bounds_Tool::mouse_down" << endl;
#endif
}


void
Statistics_and_Bounds_Tool::mouse_drag
	(
	const QwtDoublePoint&	point
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">>> Statistics_and_Bounds_Tool::mouse_drag: " << point << endl;
#endif
if (Selected_Bounds &&
	Bounds_Moving)
	{
	int
		band;
	QPoint
		position
			(static_cast<int>(point.x () + 0.5),
			 static_cast<int>(point.y () + 0.5));
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
	clog << "    Last_Position = " << Last_Position << endl
		 << "         position = " << position << endl;
	QPoint
		there (position);
	#endif

	//	Constrain the point within the graph data range.
	if (position.ry () < 0)
		position.ry () = 0;
	else
	if (position.ry () > max_y ())
		position.ry () = max_y ();
	if (position.rx () < 0)
		position.rx () = 0;
	else
	if (position.rx () > max_x ())
		position.rx () = max_x ();
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
	if (position != there)
		clog << "        position = " << position << " constrained" << endl;
	#endif

	if (position != Last_Position)
		{
		QVector<int>
			values (3, (int)position.rx ());
		band = -1;
		while (++band < 3)
			if (! (Selected_Bounds & (1 << band)))
				values[band] = -1.0;
		
		//	Move the bound.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
		clog << "    move selection bounds to "
				<< position.rx () << endl;
		#endif
		if (Selected_Bounds & SELECTED_LOWER_BOUND)
			lower_bound_values (values);
		else
			upper_bound_values (values);

		//	Move the Selection_Marker to the new position.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
		clog << "    move Selection_Marker to " << position << endl;
		#endif
		Bound_Marker->setValue (Last_Position = position);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << "<<< Statistics_and_Bounds_Tool::mouse_drag" << endl;
#endif
}


void
Statistics_and_Bounds_Tool::mouse_up
	(
	const QwtDoublePoint&
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DRAGGING))
clog << ">-< Statistics_and_Bounds_Tool::mouse_up" << endl;
#endif
Bounds_Moving = false;
}


void
Statistics_and_Bounds_Tool::leave_graph ()
{
if (Bound_Marker->isVisible ())
	Bound_Marker->hide ();
}


int
Statistics_and_Bounds_Tool::selected_bounds
	(
	const QPoint&	position
	) const
{
int
	selection = SELECTED_NONE,
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
		x_distance = Lower_Bound_Value[band] - x_position;
		if (x_distance < 0)
			x_distance = -x_distance;
		if (x_distance < min_distance)
			{
			min_distance = x_distance;
			selection = (active_band_mask | SELECTED_LOWER_BOUND);
			}
		else
		if (x_distance == min_distance)
			selection |= (active_band_mask | SELECTED_LOWER_BOUND);

		//	Upper bound
		x_distance = Upper_Bound_Value[band] - x_position;
		if (x_distance < 0)
			x_distance = -x_distance;
		if (x_distance < min_distance)
			{
			min_distance = x_distance;
			selection = active_band_mask;
			}
		else
		if (x_distance == min_distance &&
			//	Lower bound has precedence.
			! (selection & SELECTED_LOWER_BOUND))
			selection |= active_band_mask;
		}
	}

return selection;
}


int
Statistics_and_Bounds_Tool::saturation_bound
	(
	int		selection
	)
{
int
	band = 3;
while (band--)
	if (selection & (1 << band))
		return (selection & SELECTED_LOWER_BOUND) ?
			Lower_Bound_Value[band] :
			Upper_Bound_Value[band];
return -1;
}


}	//	namespace HiRISE
}	//	namespace UA
