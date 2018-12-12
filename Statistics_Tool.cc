/*	Statistics_Tool

HiROC CVS ID: $Id: Statistics_Tool.cc,v 1.36 2012/03/09 02:13:58 castalia Exp $

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

#include	"Statistics_Tool.hh"

#include	"HiView_Config.hh"
#include	"Stats.hh"
#include	"Histogram_Plot.hh"
#include	"Count_Sequence.hh"
#include	"Graph_Tracker.hh"
#include	"Drawn_Line.hh"
#include	"HiView_Utilities.hh"

//	Qt
#include	<QApplication>
#include	<QDesktopWidget>
#include	<QWidget>
#include	<QVBoxLayout>
#include	<QHBoxLayout>
#include	<QGridLayout>
#include	<QFrame>
#include	<QLabel>
#include	<QColor>
#include <QBrush>
#include	<QPen>
#include	<QSpinBox>

//	Qwt
#include	<qwt_plot.h>
#include	<qwt_plot_grid.h>

#include	<sstream>
using std::ostringstream;
#include	<iomanip>
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
#define DEBUG_LAYOUT			(1 << 2)
#define DEBUG_DATA				(1 << 3)
#define	DEBUG_HELPERS			(1 << 4)
#define DEBUG_SLOTS				(1 << 5)
#define DEBUG_SIGNALS			(1 << 6)
#define DEBUG_LIMITS			(1 << 7)
#define DEBUG_REFRESH			(1 << 8)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

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
	Statistics_Tool::ID =
		"UA::HiRISE::Statistics_Tool ($Revision: 1.36 $ $Date: 2012/03/09 02:13:58 $)";


#ifndef DEFAULT_STATISTICS_GRAPH_VISIBLE
#define DEFAULT_STATISTICS_GRAPH_VISIBLE	true
#endif

#ifndef STATISTICS_TOOL_GRAPH_MIN_WIDTH
#define STATISTICS_TOOL_GRAPH_MIN_WIDTH		350
#endif
#ifndef STATISTICS_TOOL_GRAPH_MIN_HEIGHT
#define STATISTICS_TOOL_GRAPH_MIN_HEIGHT	200
#endif
const QSize
	Statistics_Tool::GRAPH_MIN_SIZE
		(STATISTICS_TOOL_GRAPH_MIN_WIDTH, STATISTICS_TOOL_GRAPH_MIN_HEIGHT);

const double
	Statistics_Tool::DEFAULT_OPACITY[3] = {1.00, 0.75, 0.50};

#ifndef STATISTICS_TOOL_LIMITS_ENABLED_DEFAULT
#define STATISTICS_TOOL_LIMITS_ENABLED_DEFAULT	false
#endif

/*==============================================================================
	Application configuration parameters
*/
#define MAX_DISPLAY_VALUE \
	HiView_Config::MAX_DISPLAY_VALUE
#define MAX_SOURCE_DATA_PRECISION \
	HiView_Config::MAX_SOURCE_DATA_PRECISION

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

#define Default_Graph_Canvas_Color \
	HiView_Config::Default_Graph_Canvas_Color

#define DISPLAY_BAND_COLORS \
	HiView_Config::DISPLAY_BAND_COLORS

/*==============================================================================
	Constructors
*/
Statistics_Tool::Statistics_Tool
	(
	const QString&	title,
	QWidget*		parent,
	Qt::WindowFlags	flags
	)
	:	QWidget (parent, flags),
		Limits_Enabled (STATISTICS_TOOL_LIMITS_ENABLED_DEFAULT),
		Visible_Graph (true)
{
setObjectName (title.isEmpty () ?  tr ("Statistics_Tool") : title);
setWindowTitle (title.isEmpty () ? tr ("Statistics") : title);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << ">>> Statistics_Tool: " << object_pathname (this) << endl;
#endif

//	Enable keyboard focus on the tool.
setFocusPolicy (Qt::StrongFocus);

Plot[0] =
Plot[1] =
Plot[2] = NULL;

//	Initialize the default display bands to statistics set index map.
Display_to_Stats_Map[0] = 0;
Display_to_Stats_Map[1] = 1;
Display_to_Stats_Map[2] = 2;

QHBoxLayout
	*layout = new QHBoxLayout (this);
layout->setSpacing (5);
setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

//	Graph.
QWidget
	*panel;
panel = graph_panel ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "     graph_panel sizeHint = " << panel->sizeHint () << endl;
#endif
Minimum_Size = panel->sizeHint ();
panel->setObjectName (windowTitle () + " Graph_Panel");
layout->addWidget (panel);
layout->setStretchFactor (panel, 100);

//	Statistics report/controls.
panel = info_panel ();
//	Dynamic size control.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "      info_panel sizeHint = " << panel->sizeHint () << endl;
#endif
Minimum_Size.rwidth () += panel->sizeHint ().width () + 5;
Minimum_Size.rheight () =
	qMax (Minimum_Size.rheight (), panel->sizeHint ().height ());
panel->setObjectName (windowTitle () + " Info_Panel");
layout->addWidget (panel);
layout->setStretchFactor (panel, 0);

//	Incremental width to subtract/add when graph area is hidden/visible.
Graph_Width_Increment = GRAPH_MIN_SIZE.width () + 5;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "           GRAPH_MIN_SIZE = " << GRAPH_MIN_SIZE << endl
	 << "    Graph_Width_Increment = " << Graph_Width_Increment << endl
	 << "        tool Minimum_Size = " << Minimum_Size << endl;
#endif
visible_graph (DEFAULT_STATISTICS_GRAPH_VISIBLE);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "    adj tool Minimum_Size = " << Minimum_Size << endl;
#endif

//	Enable/Disable limits controls after baseline size determined.
limits (Limits_Enabled);

//	Focus tab ordering.
setTabOrder (Count_Scale_Max, Lower_Limit);
setTabOrder (Lower_Limit, Upper_Limit);
setTabOrder (Upper_Limit, Opacity[0]);
setTabOrder (Opacity[0], Opacity[1]);
setTabOrder (Opacity[1], Opacity[2]);
/*
	The focus tab order is held in a single, global list
	so it is not possible to loop back to the first widget in a group.
*/
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "<<< Statistics_Tool" << endl;
#endif
}


Statistics_Tool::~Statistics_Tool ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Statistics_Tool: @ " << (void*)this << endl;
#endif
}

/*==============================================================================
	Accessors
*/
void
Statistics_Tool::data_structure
	(
	int		bands,
	int		precision
	)
{
#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
clog << ">>> Statistics_Tool::data_structure "
		<< object_pathname (this) << ": " << endl
	 << "    bands = " << bands << endl
	 << "    precision = " << precision << "-bit" << endl;
#endif
if (precision > MAX_SOURCE_DATA_PRECISION)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid " << precision << "-bit data precision." << endl
		<< "The maximum supported is " << MAX_SOURCE_DATA_PRECISION
			<< "-bit data precision.";
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "!!! " << message.str () << endl
		 << "<<< Statistics_Tool::data_structure" << endl;
	#endif
	throw invalid_argument (message.str ());
	}	
if (precision <= 0)
	bands = 0;
if (bands < 0)
	bands =
	precision = 0;
else
if (bands > 3)
	bands = 3;

//	Block signals during data structure change.
bool
	blocked = blockSignals (true);

int
	band_count = statistics_sets ();
#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
clog << "    current band count = " << band_count << endl;
#endif
if (bands < band_count)
	//	Remove excess band(s).
	while (band_count > bands)
		view_band_data (--band_count, false);
else
if (bands > band_count)
	//	Add new band(s).
	while (band_count < bands)
		view_band_data (band_count++, true);

if (precision)
	{
	precision = 1 << precision;	//	Convert precision from bits to size.
	while (bands--)
		{
		if (Plot[bands])
			{
			#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
			clog << "    Plot[" << bands << "] data size from "
					<< Plot[bands]->data ().size () << " to "
					<< precision << endl;
			#endif
			Plot[bands]->data ().size (precision);
			}
		}

	//	Get the upper offset first in case range change changes the offset.
	int
		upper_offset = Statistics.upper_limit ();
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "    Upper_Limit maximum " << max_x () << endl;
	#endif
	Upper_Limit->setMaximum (max_x ());
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "    upper_limit " << qMax (0, max_x () - upper_offset) << endl;
	#endif
	upper_limit (qMax (0, max_x () - upper_offset));
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "    Lower_Limit maximum " << max_x () << endl;
	#endif
	Lower_Limit->setMaximum (max_x ());
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "    lower_limit " << qMin (Statistics.lower_limit (), max_x ())
			<< endl;
	#endif
	lower_limit (qMin (Statistics.lower_limit (), max_x ()));
	#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
	clog << "          max_x = " << max_x () << endl
		 << "    lower_limit = " << lower_limit () << endl
		 << "    upper_limit = " << upper_limit ()
		 	<< " (" << Statistics.upper_limit () << ')' << endl;
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
clog << "    X axis maximum value = " << max_x () << endl;
#endif
Graph->setAxisScale (QwtPlot::xBottom, 0.0, max_x ());

blockSignals (blocked);
#if ((DEBUG_SECTION) & (DEBUG_DATA | DEBUG_LIMITS))
clog << "<<< Statistics_Tool::data_structure" << endl;
#endif 
}


int
Statistics_Tool::data_precision () const
{
int
	bits = 0;
if (Plot[0])
	for (unsigned int
			size = Plot[0]->data ().size ();
			size;
			size >>= 1)
		++bits;
return bits;
}


int
Statistics_Tool::max_x () const
{
int
	max = 0;
if (Plot[0])
	max = Plot[0]->data ().size () - 1;
return max;
}


int
Statistics_Tool::max_y () const
{return MAX_DISPLAY_VALUE;}


void
Statistics_Tool::band_map
	(
	const unsigned int*		bands
	)
{
int
	band = 3;
while (band--)
	Band_Names[band]->setText (QString::number
		(HiView_Utilities::band_index_to_number
			(Band_Map[band] = bands[band]))
		.prepend ("<b>Band ").append ("</b>"));

/*	Update the map of display image band numbers to Statistics set numbers.

	Note that display band zero is always mapped to statistics set zero.
*/
if (bands[1] == bands[0])
	Display_to_Stats_Map[1] = 0;
else
	Display_to_Stats_Map[1] = 1;
if (bands[2] == bands[0])
	Display_to_Stats_Map[2] = 0;
else
if (bands[2] == bands[1])
	Display_to_Stats_Map[2] = 1;
else
	Display_to_Stats_Map[2] = 2;
}


void
Statistics_Tool::refresh_band_numbers ()
{band_map (Band_Map);}


void
Statistics_Tool::band_color
	(
	const QColor&	color,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">>> Statistics_Tool::band_color: " << color << ", " << band << endl;
#endif
if (band >= 0 &&
	band < 3 &&
	Plot[band])
	{
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	clog << "     current color = " << Plot[band]->bar_color () << endl;
	#endif
	if (Plot[band]->bar_color () != color)
		{
		Opacity[band]->setValue (static_cast<int>(color.alphaF () * 100));
		Plot[band]->bar_color (color);
		Graph->replot ();
		}
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "<<< Statistics_Tool::band_color" << endl;
#endif
}


QColor
Statistics_Tool::band_color
	(
	int		band
	) const
{
if (band >= 0 &&
	band < 3 &&
	Plot[band])
	return Plot[band]->bar_color ();
return QColor ();
}


void
Statistics_Tool::band_opacity
	(
	int		percent,
	int		band
	)
{
if (band >= 0 &&
	band < 3 &&
	Plot[band])
	{
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		percent = 100;

	QColor
		color (Plot[band]->bar_color ());
	color.setAlphaF (percent / 100.0);
	band_color (color, band);
	}
}


int
Statistics_Tool::band_opacity
	(
	int		band
	) const
{return Opacity[band]->value ();}


void
Statistics_Tool::canvas_color
	(
	QRgb	color
	)
{
Graph->setCanvasBackground (QBrush(color));
Graph->replot ();
}


void
Statistics_Tool::opacity_changed
	(
	int		percent
	)
{
QObject
	*source = sender ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (source == Opacity[band])
		{
		band_opacity (percent, band);
		break;
		}
	}
}

/*==============================================================================
	GUI elements
*/
QSize
Statistics_Tool::minimumSizeHint () const
{return Minimum_Size;}


QSize
Statistics_Tool::sizeHint () const
{return Minimum_Size;}


QWidget*
Statistics_Tool::graph_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Statistics_Tool::graph_panel " << object_pathname (this) << endl;
#endif
//	The plot.
Graph = new QwtPlot;
Graph->setAutoReplot (false);
Graph->setAutoDelete (false);
Graph->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
Graph->setMinimumSize (GRAPH_MIN_SIZE);
Graph->setCanvasBackground (QBrush(Default_Graph_Canvas_Color));
// TODO Graph->setMargin (5);
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

//	Histogram Plots.
Plot[0] =
Plot[1] =
Plot[2] = NULL;

QFont
	axis_title_font (font ());
axis_title_font.setBold (true);
QwtText
	text;
text.setFont (axis_title_font);

Graph->setAxisScale (QwtPlot::xBottom, 0.0, 0.0);
Graph->setAxisFont  (QwtPlot::xBottom, font ());
text.setText (tr ("Values"));
Graph->setAxisTitle (QwtPlot::xBottom, text);

Graph->setAxisScale (QwtPlot::yLeft, 0.0, 0.0);
Graph->setAxisFont  (QwtPlot::yLeft, font ());
text.setText (tr ("Count"));
Graph->setAxisTitle (QwtPlot::yLeft, text);

//	Cursor tracking.
Tracker = new Graph_Tracker(static_cast<QwtPlotCanvas*>(Graph->canvas ()));
Tracker->setRubberBandPen (QColor (Qt::yellow));
Tracker->setRubberBand (QwtPicker::CrossRubberBand);
Tracker->setTrackerPen (QColor (Qt::black));
Tracker->setEnabled (true);
//	Custom crosshair cursor.
Tracker->set_cursor (*Crosshair_Cursor);

//	Count scale max control.
Count_Scale_Max = new QSpinBox;
Count_Scale_Max->setToolTip
	(tr ("Maximum Count axis scale value; zero for auto scaling"));
Count_Scale_Max->setSpecialValueText (tr ("auto"));
QSize
	screen_size (qApp->desktop ()->screen ()->size ());
Count_Scale_Max->setRange (0, screen_size.width () * screen_size.height ());
Count_Scale_Max->setSingleStep (1);
Count_Scale_Max->setValue (0);
Count_Scale_Max->setAccelerated (true);
Count_Scale_Max->setKeyboardTracking (false);
Count_Scale_Max->setAlignment (Qt::AlignRight);
Count_Scale_Max->setFrame (false);
connect (Count_Scale_Max,
	SIGNAL (valueChanged (int)),
	SLOT (vertical_axis_max (int)));

//	Panel to contain the graph and its scale control widget.
Graph_Panel = new QWidget;
QVBoxLayout
	*layout = new QVBoxLayout (Graph_Panel);
Graph_Panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
Graph_Panel->setMinimumSize (GRAPH_MIN_SIZE);
layout->setSpacing (0);
layout->addWidget (Count_Scale_Max, 1, Qt::AlignLeft);
layout->addWidget (Graph, 100);

Graph->setAutoReplot (true);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Statistics_Tool::graph_panel" << endl;
#endif
return Graph_Panel;
}


bool
Statistics_Tool::visible_graph
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">>> Statistics_Tool::visible_graph: " << boolalpha << enabled << endl
	 << "    Visible_Graph = " << Visible_Graph << endl
	 << "    Graph_Width_Increment = " << Graph_Width_Increment << endl;
#endif
bool
	visible = Visible_Graph;
Graph_Panel->setVisible (Visible_Graph = enabled);
if (visible != Visible_Graph)
	{
	if (Visible_Graph)
		Minimum_Size.rwidth () += Graph_Width_Increment;
	else
		Minimum_Size.rwidth () -= Graph_Width_Increment;
	#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
	clog << "         Minimum_Size = " << Minimum_Size << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << "<<< Statistics_Tool::visible_graph: " << visible << endl;
#endif
return visible;
}


void
Statistics_Tool::vertical_axis_max
	(
	int		max
	)
{
if (! max)
	{
	int
		max_count,
		bands = statistics_sets ();
	while (bands--)
		if (Plot[bands] &&
			max < (max_count = Plot[bands]->data ().calculate_max_count ()))
			max = max_count;
	}
Graph->setAxisScale (QwtPlot::yLeft, 0.0, max);
Graph->replot ();
}


QWidget*
Statistics_Tool::info_panel ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Statistics_Tool::info_panel " << object_pathname (this) << endl;
#endif
QFrame
	*panel = new QFrame;
panel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
panel->setFrameStyle (Panel_Frame_Style);
panel->setLineWidth (Panel_Frame_Width);
QGridLayout
	*layout = new QGridLayout (panel);

QLabel
	*label;
int
	band,
	row				= -1,
	first_stats_row	= 2,
	col,
	label_col		= 0,
	x_col			= 1,
	y_col			= 2,
	area_col		= 3,
	red_col			= 1,
	green_col		= 2,
	blue_col		= 3,
	spacing_col		= 4;
layout->setColumnStretch (label_col, 0);
layout->setColumnStretch (red_col, 0);
layout->setColumnStretch (green_col, 0);
layout->setColumnStretch (blue_col, 0);
layout->setColumnStretch (spacing_col, 100);
layout->setVerticalSpacing (1);

//	Region values.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Region value names" << endl;
#endif
layout->addWidget (new QLabel (tr ("<b>X</b>")),
	row, x_col, Qt::AlignRight | Qt::AlignBottom);
layout->addWidget (new QLabel (tr ("<b>Y</b>")),
	row, y_col, Qt::AlignRight | Qt::AlignBottom);
layout->addWidget (new QLabel (tr ("<b>Area</b>")),
	row, area_col, Qt::AlignRight | Qt::AlignBottom);

//	Region value names underlines.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Region value names underlines" << endl;
#endif
layout->addWidget (new Drawn_Line,
	row, x_col);
layout->addWidget (new Drawn_Line,
	row, y_col);
layout->addWidget (new Drawn_Line,
	row, area_col);

//	Selected region origin.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Selected region origin" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Origin")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Selected image region origin"));
Origin[0] = new QLabel ("      ");	//	Occupy expected maximum space
Origin[0]->setAlignment (Qt::AlignRight);
layout->addWidget (Origin[0], row, x_col);
Origin[1] = new QLabel ("      ");
Origin[1]->setAlignment (Qt::AlignRight);
layout->addWidget (Origin[1], row, y_col);
Sampled_Area = new QLabel ("         ");
Sampled_Area->setToolTip (tr ("Sampled image area"));
Sampled_Area->setAlignment (Qt::AlignRight);
layout->addWidget (Sampled_Area, row, area_col);

//	Selected region size.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Selected region size" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Size")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Selected image region size"));
Size[0] = new QLabel;
Size[0]->setAlignment (Qt::AlignRight);
layout->addWidget (Size[0], row, x_col);
Size[1] = new QLabel;
Size[1]->setAlignment (Qt::AlignRight);
layout->addWidget (Size[1], row, y_col);
Area = new QLabel;
Area->setToolTip (tr ("Actual image area"));
Area->setAlignment (Qt::AlignRight);
layout->addWidget (Area, row, area_col);

//	Separator.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Separator" << endl;
#endif
layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, label_col, 1, -1);
layout->setRowMinimumHeight (row, 10);

//	Band names.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Band names" << endl;
#endif
for (band = 0,
		col = red_col;
	 band < 3;
	 band++,
	 	col++)
	{
	Band_Names[band] = new QLabel;
	Band_Names[band]->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
	Band_Names[band]->setFrameStyle (Label_Frame_Style);
	Band_Names[band]->setLineWidth (Label_Frame_Width);
	Band_Names[band]->setMargin (Label_Frame_Margin);
	Band_Names[band]->setPalette (QPalette
		(QColor (DISPLAY_BAND_COLORS[band]).lighter ()));
	Band_Names[band]->setAutoFillBackground (true);
	layout->addWidget (Band_Names[band],
		row, col, Qt::AlignVCenter);
	}
//	Initialize the Band_Names labels.
Band_Map[0] = 0;
Band_Map[1] = 1;
Band_Map[2] = 2;
band_map (Band_Map);

//	Stats.
first_stats_row = ++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << '-' << (row + 4) << ": Stats -" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Values:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Number of image values counted"));

QHBoxLayout
	*horizontal_layout = new QHBoxLayout;
Below_Limit_Label = new QLabel (tr ("Below"));
Below_Limit_Label->setToolTip (tr ("Area below the minimum valid value"));
horizontal_layout->addWidget (Below_Limit_Label,
	0, Qt::AlignRight | Qt::AlignVCenter);
Lower_Limit = new QSpinBox;
Lower_Limit->setToolTip (tr ("Minimum valid value"));
Lower_Limit->setRange (0, max_x ());
Lower_Limit->setSingleStep (1);
Lower_Limit->setValue (qMin (Statistics.lower_limit (), max_x ()));
Lower_Limit->setAccelerated (true);
Lower_Limit->setKeyboardTracking (false);
Lower_Limit->setAlignment (Qt::AlignRight);
Lower_Limit->setFrame (false);
connect (Lower_Limit,
	SIGNAL (valueChanged (int)),
	SLOT (lower_limit (int)));
horizontal_layout->addWidget (Lower_Limit,
	0, Qt::AlignRight | Qt::AlignVCenter);
layout->addLayout (horizontal_layout,
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);

horizontal_layout = new QHBoxLayout;
Above_Limit_Label = new QLabel (tr ("Above"));
Above_Limit_Label->setToolTip (tr ("Area above the maximum valid value"));
horizontal_layout->addWidget (Above_Limit_Label,
	0, Qt::AlignRight | Qt::AlignVCenter);
Upper_Limit = new QSpinBox;
Upper_Limit->setToolTip (tr ("Maximum valid value"));
Upper_Limit->setRange (0, max_x ());
Upper_Limit->setSingleStep (1);
Upper_Limit->setValue (qMax (0, max_x () - Statistics.upper_limit ()));
Upper_Limit->setAccelerated (true);
Upper_Limit->setKeyboardTracking (false);
Upper_Limit->setAlignment (Qt::AlignRight);
Upper_Limit->setFrame (false);
connect (Upper_Limit,
	SIGNAL (valueChanged (int)),
	SLOT (upper_limit (int)));
horizontal_layout->addWidget (Upper_Limit,
	0, Qt::AlignRight | Qt::AlignVCenter);
layout->addLayout (horizontal_layout,
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);

layout->addWidget (label = new QLabel (tr ("Lowest:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Lowest image value counted"));
layout->addWidget (label = new QLabel (tr ("Highest:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Highest image value counted"));
layout->addWidget (label = new QLabel (tr ("Minimum:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Minimum count"));
layout->addWidget (label = new QLabel (tr ("Maximum:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Maximum count"));
layout->addWidget (label = new QLabel (tr ("Mean:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Mean image value"));
layout->addWidget (label = new QLabel (tr ("Median:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Median image value"));
layout->addWidget (label = new QLabel (tr ("Std. Dev.:")),
	row++, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Standard deviation of image values"));

for (band = 0,
		col = red_col;
	 band < 3;
	 band++,
		col++)
	{
	row = first_stats_row;
	layout->addWidget (Values[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Below_Limit[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Above_Limit[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Lowest_Value[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Highest_Value[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Minimum_Count[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Maximum_Count[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Mean_Value[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Median_Value[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget (Std_Dev_of_Values[band] = new QLabel,
		row++, col, Qt::AlignRight | Qt::AlignVCenter);
	}

//	Separator.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Separator" << endl;
#endif
layout->setRowMinimumHeight (row, 10);
layout->setRowStretch (row, 100);

//	Band opacity.
++row;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << row << ": Band opacity" << endl;
#endif
layout->addWidget (label = new QLabel (tr ("Opacity:")),
	row, label_col, Qt::AlignRight | Qt::AlignVCenter);
label->setToolTip (tr ("Histogram plot opacity"));
for (int
		band = 0,
			col = red_col;
		band < 3;
		band++,
			col++)
	{
	Opacity[band] = new QSpinBox;
	Opacity[band]->setRange (0, 100);
	Opacity[band]->setSingleStep (1);
	Opacity[band]->setValue
		(static_cast<int>(DEFAULT_OPACITY[band] * 100.0));
	Opacity[band]->setAccelerated (true);
	Opacity[band]->setKeyboardTracking (false);
	Opacity[band]->setAlignment (Qt::AlignRight);
	Opacity[band]->setFrame (false);
	connect (Opacity[band],
		SIGNAL (valueChanged (int)),
		SLOT (opacity_changed (int)));
	layout->addWidget (Opacity[band],
		row, col, Qt::AlignRight | Qt::AlignVCenter);
	}

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Statistics_Tool::info_panel" << endl;
#endif
return panel;
}


void
Statistics_Tool::limits
	(
	bool	enabled
	)
{
Limits_Enabled = enabled;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	Below_Limit[band]->setVisible (enabled);
	Above_Limit[band]->setVisible (enabled);
	}
Lower_Limit->setVisible (enabled);
Below_Limit_Label->setVisible (enabled);
Upper_Limit->setVisible (enabled);
Above_Limit_Label->setVisible (enabled);
}


int
Statistics_Tool::upper_limit () const
{return Upper_Limit->value ();}


void
Statistics_Tool::upper_limit
	(
	int		limit
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS))
clog << ">>> Statistics_Tool::upper_limit: " << limit << endl;
#endif
if (limit < 0)
	limit = 0;
else
if (limit > max_x ())
	limit = max_x ();

if (Upper_Limit->value () != limit)
	Upper_Limit->setValue (limit);
else
	{
	if (limit < Lower_Limit->value ())
		//	Maintain range integrity.
		lower_limit (limit);

	if (Limits_Enabled)
		{
		//	Reset the Statistics limit.
		Statistics.limits (Statistics.lower_limit (), max_x () - limit);

		if (! signalsBlocked ())
			{
			refresh ();

			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS | DEBUG_SIGNALS))
			clog << "^^^ Statistics_Tool::upper_limit: "
					"emit upper_limit_changed "
					<< (max_x () - limit) << " (" << limit << ')' << endl;
			#endif
			emit upper_limit_changed (max_x () - limit);
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS))
clog << "<<< Statistics_Tool::upper_limit" << endl;
#endif
}


int
Statistics_Tool::lower_limit () const
{return Lower_Limit->value ();}


void
Statistics_Tool::lower_limit
	(
	int		limit
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS))
clog << ">>> Statistics_Tool::lower_limit: " << limit << endl;
#endif
if (limit < 0)
	limit = 0;
else
if (limit > max_x ())
	limit = max_x ();

if (Lower_Limit->value () != limit)
	Lower_Limit->setValue (limit);
else
	{
	if (limit > Upper_Limit->value ())
		//	Maintain range integrity.
		upper_limit (limit);

	if (Limits_Enabled)
		{
		//	Reset the Statistics limit.
		Statistics.limits (limit, Statistics.upper_limit ());

		if (! signalsBlocked ())
			{
			refresh ();

			//	>>> SIGNAL <<<
			#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS | DEBUG_SIGNALS))
			clog << "^^^ Statistics_Tool::lower_limit: "
					"emit lower_limit_changed " << limit << endl;
			#endif
			emit lower_limit_changed (limit);
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_LIMITS | DEBUG_SLOTS))
clog << "<<< Statistics_Tool::lower_limit" << endl;
#endif
}


void
Statistics_Tool::view_band_data
	(
	int		band,
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> Statistics_Tool::view_band_data "
		<< object_pathname (this) << ": "
		<< band << ' ' << boolalpha << enabled << endl;
#endif
if (band >= 0 &&
	band < 3)
	{
	Band_Names[band]->setVisible (enabled);
	Values[band]->setVisible (enabled);
	if (Limits_Enabled)
		Below_Limit[band]->setVisible (enabled);
	Lowest_Value[band]->setVisible (enabled);
	Highest_Value[band]->setVisible (enabled);
	if (Limits_Enabled)
		Above_Limit[band]->setVisible (enabled);
	Minimum_Count[band]->setVisible (enabled);
	Maximum_Count[band]->setVisible (enabled);
	Mean_Value[band]->setVisible (enabled);
	Median_Value[band]->setVisible (enabled);
	Std_Dev_of_Values[band]->setVisible (enabled);
	Opacity[band]->setVisible (enabled);

	if (enabled)
		{
		if (! Plot[band])
			Plot[band] = new_plot (band);
		#if ((DEBUG_SECTION) & DEBUG_HELPERS)
		clog << "    attach Plot @ " << (void*)Plot[band]
				<< " to Graph" << endl;
		#endif
		Plot[band]->attach (Graph);
		//	Bind the histogram vector to the statistics set.
		#if ((DEBUG_SECTION) & DEBUG_HELPERS)
		clog << "    bind plot histogram @ "
				<< (void*)(Plot[band]->data_vector ())
				<< " to Statistics" << endl;
		#endif
		Statistics.histogram (Plot[band]->data_vector (), band);
		}
	else
	if (Plot[band])
		{
		//	Unbind the histogram vector from the statistics set.
		#if ((DEBUG_SECTION) & DEBUG_HELPERS)
		clog << "    unbind plot histogram @ "
				<< (void*)(Statistics.histogram (band))
				<< " from Statistics" << endl;
		#endif
		Statistics.histogram (NULL, band);
		#if ((DEBUG_SECTION) & DEBUG_HELPERS)
		clog << "    detach Plot @ " << (void*)Plot[band]
				<< " from Graph and delete" << endl;
		#endif
		Plot[band]->attach (NULL);
		delete Plot[band];
		Plot[band] = NULL;
		}
	}
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "<<< Statistics_Tool::view_band_data" << endl;
#endif
}


Histogram_Plot*
Statistics_Tool::new_plot
	(
	int		band
	)
{
Histogram_Plot
	*plot = new Histogram_Plot;
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">-< Statistics_Tool::new_plot "
		<< object_pathname (this) << " @ " << (void*)plot << endl;
#endif
plot->attribute_set (Histogram_Plot::VERTICAL_BARS);
QColor
	color (DISPLAY_BAND_COLORS[band]);
color.setAlphaF (band_opacity (band) / 100.0);
plot->bar_color (color);
plot->setZ (Histogram_Plot::Default_Plot_Z + band);
return plot;
}


bool
Statistics_Tool::refresh
	(
	const QRect&	selected_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
clog << ">>> Statistics_Tool::refresh: " << object_pathname (this)
		<< ": " << selected_region << endl;
#endif
//	Check for invalid histogram data.
if (! Statistics.histogram (0) ||
	Statistics.histogram (0)->at (0) == static_cast<Histogram::value_type>(-1))
	{
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
	clog << "    no histogram"
			<< (Statistics.histogram (0) ? " data" : "") << endl
		 << "<<< Statistics_Tool::refresh: false" << endl;
	#endif
	return false;
	}

#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
clog << "    Statistics.calculate" << endl;
#endif
Statistics.calculate ();

unsigned long long
	area,
	//	Sampled region area.
	value = Statistics.area (0);
if (value < (area = Statistics.area (1)))
	value = area;
if (value < (area = Statistics.area (2)))
	value = area;
//	Actual region area.
if (selected_region.isNull ())
	area = Area->text ().toULongLong ();
else
	area =
		(unsigned long long)selected_region.width ()
			* selected_region.height ();
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
clog << "    sampled area = " << value << endl
	 << "     actual area = " << area << endl;
#endif
if (value == 0 &&
	area  != 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
	clog << "    no area measured" << endl
		 << "<<< Statistics_Tool::refresh: false" << endl;
	#endif
	return false;
	}
if (value != area)
	{
	Sampled_Area->setText (QString::number (value).prepend ("[").append ("]"));
	Sampled_Area->setVisible (true);
	}
else
	Sampled_Area->setVisible (false);

if (! selected_region.isNull ())
	{
	Origin[0]->setNum (selected_region.x ());
	Origin[1]->setNum (selected_region.y ());
	Size[0]->setNum (selected_region.width ());
	Size[1]->setNum (selected_region.height ());
	Area->setText (QString::number (area));
	}

unsigned long long
	max_value = 0;
int
	bands = statistics_sets ();
for (int
		band = 0;
		band < bands;
		band++)
	{
	if (Plot[band])
		{
		#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
		clog << "    band " << band << " -" << endl;
		#endif
		if (max_value < (value = Plot[band]->data ().calculate_max_count ()))
			max_value = value;

		Values[band]->setNum
			(Statistics.values_counted (band));
		Below_Limit[band]->setText (QString::number
			(Statistics.area (band, Stats::LOWER_RANGE)));
		Lowest_Value[band]->setNum
			(Statistics.lowest_value (band));
		Highest_Value[band]->setNum
			(Statistics.highest_value (band));
		Above_Limit[band]->setText (QString::number
			(Statistics.area (band, Stats::UPPER_RANGE)));
		Minimum_Count[band]->setText (QString::number
			(Statistics.minimum_count (band)));
		Maximum_Count[band]->setText (QString::number
			(Statistics.maximum_count (band)));
		Mean_Value[band]->setText (QString::number
			(Statistics.mean_value (band), 'f', 1));
		Median_Value[band]->setText (QString::number
			(Statistics.median_value (band), 'f', 1));
		Std_Dev_of_Values[band]->setText (QString::number
			(Statistics.standard_deviation_of_values (band), 'f', 1));
		#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
		clog << "      values_counted = "
				<< Statistics.values_counted (band) << endl
			 << "      below " << Lower_Limit->value () << " = "
			 	<< Statistics.area (band, Stats::LOWER_RANGE) << endl
			 << "        lowest_value = "
		 		<< Statistics.lowest_value (band) << endl
			 << "       highest_value = "
		 		<< Statistics.highest_value (band) << endl
			 << "      above " << Upper_Limit->value () << " = "
			 	<< Statistics.area (band, Stats::UPPER_RANGE) << endl
			 << "       minimum_count = "
		 		<< Statistics.minimum_count (band) << endl
			 << "       maximum_count = "
		 		<< Statistics.maximum_count (band) << endl
			 << "          mean_value = "
		 		<< Statistics.mean_value (band) << endl
			 << "        median_value = "
		 		<< Statistics.median_value (band) << endl
			 << "       std deviation = "
		 		<< Statistics.standard_deviation_of_values (band) << endl
			 << "                area = "
		 		<< Statistics.area (band) << endl;
		#endif
		}
	}

#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
clog << "    Y axis max_value = " << max_value << endl
	 << "    replot and update" << endl;
#endif
if (Count_Scale_Max->value () == 0)
	Graph->setAxisScale (QwtPlot::yLeft, 0.0, max_value);
Graph->replot ();
update ();

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS | DEBUG_SIGNALS))
clog << "^^^ Statistics_Tool::refresh: emit statistics_refreshed" << endl;
emit statistics_refreshed ();
#endif
#if ((DEBUG_SECTION) & (DEBUG_REFRESH | DEBUG_SLOTS))
clog << "<<< Statistics_Tool::refresh: true" << endl;
#endif
return true;
}


}	//	namespace HiRISE
}	//	namespace UA
