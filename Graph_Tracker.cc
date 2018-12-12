/*	Graph_Tracker

HiROC CVS ID: $Id: Graph_Tracker.cc,v 1.8 2013/09/24 18:43:16 guym Exp $

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

#include	"Graph_Tracker.hh"

#include	<QPoint>
#include	<QCursor>
#include	<QMouseEvent>

#include	<qwt_plot.h>
#include	<qwt_scale_div.h>
#include <qwt_compat.h>

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_SIGNALS			(1 << 2)
#define DEBUG_EVENTS			(1 << 3)

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
#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Graph_Tracker::ID =
		"UA::HiRISE::Graph_Tracker ($Revision: 1.8 $ $Date: 2013/09/24 18:43:16 $)";

#ifndef DEFAULT_GRAPH_TRACKER_SELECTION_MODE
#define DEFAULT_GRAPH_TRACKER_SELECTION_MODE	QwtPicker::PointSelection | \
												QwtPicker::ClickSelection
#endif

/*==============================================================================
	Constructors
*/
Graph_Tracker::Graph_Tracker
	(
	QwtPlotCanvas	*canvas
	)
	//	Default picker modes:
	:	QwtPlotPicker
			//	Axes used for coordinate system
			(QwtPlot::xBottom, QwtPlot::yLeft,
			//	SelectionType and SelectionMode
    		//DEFAULT_GRAPH_TRACKER_SELECTION_MODE,
			//	RubberBand
    		QwtPlotPicker::NoRubberBand,
			//	DisplayMode
			QwtPicker::AlwaysOn,
    		canvas)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< Graph_Tracker @ " << (void*)this << endl;
#endif
}

/*==============================================================================
	Helpers
*/
void
Graph_Tracker::set_cursor
	(
	const QCursor&	cursor
	)
{parentWidget ()->setCursor (cursor);}


void
Graph_Tracker::set_cursor_position
	(
	const QPoint&	graph_position
	)
{
QCursor::setPos (parentWidget ()->mapToGlobal
	(transform (QwtDoublePoint (graph_position))));
}

/*==============================================================================
	QwtPlotPicker override
*/
QwtText
Graph_Tracker::trackerText
	(
	const QPoint&	coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">>> Graph_Tracker::trackerText: " << coordinate << endl;
#endif
if (coordinate.x () < plot ()->axisScaleDiv (xAxis ()).lowerBound () ||
	coordinate.x () > plot ()->axisScaleDiv (xAxis ()).upperBound () ||
	coordinate.y () < plot ()->axisScaleDiv (yAxis ()).lowerBound () ||
	coordinate.y () > plot ()->axisScaleDiv (yAxis ()).upperBound ())
	return QwtText ();

//QPoint point (coordinate);

QString
	text;
switch (rubberBand ())
	{
    case HLineRubberBand:
		text = QString::number (coordinate.y ());
        break;
    case VLineRubberBand:
        text = QString::number (coordinate.x ());
        break;
    default:
		text  = QString::number (coordinate.x ());
		text += ", ";
		text += QString::number (coordinate.y ());
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "<<< Graph_Tracker::trackerText: " << text << endl;
#endif
return QwtText (text);
}


void
Graph_Tracker::widgetMouseMoveEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Graph_Tracker::widgetMouseMoveEvent: position = "
		<< event->pos () << endl;
#endif
QwtPlotPicker::widgetMouseMoveEvent (event);

QPoint
	point (invTransform (event->pos ()).toPoint ());
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "    graph data position = " << point << endl;
#endif
if (point.rx () >= int (plot ()->axisScaleDiv (xAxis ()).lowerBound ()) &&
	point.rx () <= int (plot ()->axisScaleDiv (xAxis ()).upperBound ()) &&
	point.ry () >= int (plot ()->axisScaleDiv (yAxis ()).lowerBound ()) &&
	point.ry () <= int (plot ()->axisScaleDiv (yAxis ()).upperBound ()))
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "^^^ Graph_Tracker::widgetMouseMoveEvent: "
			"emit position " << point << endl;
	#endif
	emit position (point);
	}
}


void
Graph_Tracker::widgetLeaveEvent
	(
	QEvent*		event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Graph_Tracker::widgetLeaveEvent" << endl;
#endif
QwtPlotPicker::widgetLeaveEvent (event);

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "^^^ Graph_Tracker::widgetLeaveEvent " << endl;
#endif
emit leave_widget ();
}


}	//	namespace UA::HiRISE
