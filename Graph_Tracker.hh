/*	Graph_Tracker

HiROC CVS ID: $Id: Graph_Tracker.hh,v 1.5 2011/04/24 08:17:38 castalia Exp $

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

#ifndef HiView_Graph_Tracker_hh
#define HiView_Graph_Tracker_hh

#include <qwt_plot_picker.h>
#include <qwt_plot_canvas.h>

class QPoint;
class QCursor;
class QMouseEvent;


namespace UA::HiRISE
{
/**	A <i>Graph_Tracker</i> tracks the position of the cursor on a Graph.

	The Graph_Tracker is a subclass of the QwtPlotPicker. The
	QwtPlotPicker tracks the position of the cursor on the QwtPlotCanvas
	to which it is bound, and intercepts specific mouse and keyboard
	events (QwtEventPattern) that signal a transition of the
	QwtPickerMachine state machine employed by the QwtPicker - base class
	of the QwtPlotPicker - to control cursor position acquisition in
	various modes.

	The QwtPicker provides a cursor position display capability for its
	QWidget (a QwtPlotCanvas for a QwtPlotPicker) that can be selectively
	enabled. The Graph_Tracker overrides the display text method to
	provide integer coordinate display values and prevent coordinate
	display when the cursor is outside the bounds of the graph axes. In
	addition, a position signal is generated using integer graph
	coordinates for in-bounds coordinates.

	<b>Selection Mode and Signals:</b>

	<b>ClickSelection -</b>

	Mouse down (button 1) emits an appended signal and a selected signal.
	No other signals are emitted during dragging or mouse button up.

	<b>DragSelection -</b>

	Mouse down (button 1) emits an appended signal. Dragging (mouse
	button remains down) the cursor emits moved signals whenever the
	cursor display location changes. During dragging horizontal and
	vertical lines across the graph centered on the cursor
	(CrossRubberBand) are displayed. Dragging may extend outside the
	graph (and the application window). Mouse up emits a selected signal.

	Note: The appended, moved and selected signals have two forms:
	display - integer values - and a graph - floating point values -
	coordinates.

	All operations, above are in PointSelection mode with NoRubberBand.

	Activation events may be changed using the setMousePattern and
	setKeyPattern methods of the QwtEventPattern base class.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.5 $
*/
class Graph_Tracker
:	public QwtPlotPicker
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
/**	Constructs a Graph_Tracker bound to a QwtPlotCanvas.

	The base QwtPlotPicker class will be initialized in
	PointSelection/ClickSelection mode with NoRubberBand and the
	coordinate display AlwaysOn. The bottom x-axis and left y-axis
	are used for obtaining graph coordinates.

	@param	canvas	A pointer to a QwtPlotCanvas to which this
		Graph_Tracker will be bound.
*/
Graph_Tracker (QwtPlotCanvas *canvas);

/*==============================================================================
	Helpers
*/
void set_cursor (const QCursor& cursor);

void set_cursor_position (const QPoint& graph_position);

/*==============================================================================
	Signals
*/
signals:

/**	Signals the position of the cursor on the graph.

	The signal is emitted as the cursor position is being {@link
	widgetMouseMoveEvent(QMouseEvetn*) tracked} but only when the cursor
	is within the graph data limits.

	@param	point	A QPoint specifying valid graph data coordinates in
		integer values.
*/
void position (const QPoint& point) const;

void leave_widget ();

protected:
/*==============================================================================
	QwtPlotPicker override
*/
/**	Provides integer graph data coordinate display text.

	If the coordinate is outside the bounds of the graph's axes an empty
	text value is returned immediately.

	The floating point graph data coordinate is rounded to integer
	values. Coordinate display text is generated from the position
	values. If the tracker is in HLineRubberBand or VLineRubberBand
	rubber band mode then only the y or x coordinate, respectively, is
	included in the display text. Otherwise the two coordinate values
	separated by a comma and a space are included in the display text.

	@param	coordinate	A QwtDoublePoint containing the graph coordinate.
	@return	A QwtText coordinate display text. This will be empty if
		the specified coordinate is outside the bounds of the graph
		data limits.
	@see	position(const QPoint&)
*/
virtual QwtText trackerText (const QPoint& coordinate) const;

/**	Provides integer graph data coordinate position signal.

	The base class method that is being overridden is called first.

	The floating point graph data coordinate is rounded to integer
	values. If the coordinate is withing the bounds of the graph's axes
	it is emitted with a {@link position(const QPoint&) position} signal.

	@param	event	The QMouseEvent from the widget to which this
		Graph_Tracker has been bound.
*/
virtual void widgetMouseMoveEvent (QMouseEvent* event);

virtual void widgetLeaveEvent (QEvent* event);

};


}	//	namespace UA::HiRISE
#endif
