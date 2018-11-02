/*	Statistics_and_Bounds_Tool

HiROC CVS ID: $Id: Statistics_and_Bounds_Tool.hh,v 1.13 2012/07/21 22:34:32 castalia Exp $

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

#ifndef HiView_Statistics_and_Bounds_Tool_hh
#define HiView_Statistics_and_Bounds_Tool_hh

#include	"Statistics_Tool.hh"

//	Qt
#include	<QPoint>
template<typename T> class QVector;
class QWidget;

//	Qwt
#include <qwt_compat.h>
class QwtPlotMarker;


namespace UA
{
namespace HiRISE
{
//	Forward References:
class Graph_Tracker;
class Histogram_Plot;

/**	The <i>Statistics_and_Bounds_Tool</i> for the HiView application.

	The Statistics_and_Bounds_Tool adds saturation bounds controls to
	the base Statistics_Tool.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.13 $
*/
class Statistics_and_Bounds_Tool
:	public Statistics_Tool
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
	Constructors
*/
Statistics_and_Bounds_Tool (const QString& title,
	QWidget* parent = NULL, Qt::WindowFlags flags = 0);

virtual ~Statistics_and_Bounds_Tool ();

/*==============================================================================
	Accessors
*/
//	Saturations Bounds:

inline bool band_selected (int band) const
	{return Selected_Bands & (1 << band);}

/**	Get the map of currently selected bands.

	@return	A Band_Selection map in which the bit flag for each selected
		band is set. The bits for unselected bands are not set.
*/
inline int selected_bands () const
	{return Selected_Bands;}

inline int selection_distance () const
	{return Selection_Distance;}
static int default_selection_distance (int distance);
inline static int default_selection_distance ()
	{return Default_Selection_Distance;}

inline int upper_bound_value (int band) const
	{return Upper_Bound_Value[band];}
inline int lower_bound_value (int band) const
	{return Lower_Bound_Value[band];}

inline int upper_bound_percent (int band) const
	{return Upper_Bound_Percent[band];}
inline int lower_bound_percent (int band) const
	{return Lower_Bound_Percent[band];}

/*==============================================================================
	Manipulators
*/
virtual bool refresh (const QRect& selected_region);

/*==============================================================================
	Qt signals:
*/
signals:

void upper_bound_values_changed (const QVector<int>& values);
void lower_bound_values_changed (const QVector<int>& values);

void upper_bound_percents_changed (const QVector<double>& percents);
void lower_bound_percents_changed (const QVector<double>& percents);

/*==============================================================================
	Qt slots
*/
public slots:

void bands_selected (int band_selections);

void selection_distance (int distance);

void upper_bound_values (const QVector<int>& values);
void lower_bound_values (const QVector<int>& values);

void upper_bound_percents (const QVector<double>& percents);
void lower_bound_percents (const QVector<double>& percents);

virtual void upper_limit (int limit);
virtual void lower_limit (int limit);


private slots:

/**	Reports the current position of the cursor within the graph.

	@param	point	A QPoint containing a graph data coordinate. The
		coordinate is guaranteed to be within the bounds of the graph
		data.
*/
void graph_position (const QPoint& point);

/**	Reports pressing of the mouse button on the graph.

	Pressing mouse button 1 (the left button of multi-button devices; a.k.a
	MouseSelect1) causes this slot to be called.

	@param	point	A QPoint containing a graph data coordinate. The
		coordinate is guaranteed to be within the bounds of the graph
		display but may be outside the bounds of the graph data.
*/
void mouse_down (const QwtDoublePoint& point);

/**	Reports dragging the mouse.

	Dragging the mouse cursor after a {@link mouse_down(const
	QwtDoublePoint&) mouse down} action causes this slot to be called.
	The slot will be called whenever the cursor display position changes.
	Once dragging has been initiated the reported display position may
	move anywhere inside or outside the graph, including outside the
	application window.

	@param	point	A QPoint containing a graph data coordinate. The
		coordinate values are unbounded.
*/
void mouse_drag (const QwtDoublePoint&);

/**	Reports releasing of the mouse button.

	Releasing the mouse button after a {@link mouse_down(const
	QwtDoublePoint&) mouse down} action - possibly with intervening
	{@link mouse_drag(const QwtDoublePoint&) mouse drag} action(s) -
	causes this slot to be called. The reported position will be the last
	mouse drag position or the mouse down position if no dragging
	occurred.

	@param	point	A QPoint containing a graph data coordinate. The
		coordinate values are unbounded.
*/
void mouse_up (const QwtDoublePoint& point);

void leave_graph ();

/*==============================================================================
	Helpers
*/
private:

/**	Get the {@link selected_bounds(const QPoint&) selected bounds} value.

	The selection bit flags are scanned for a set band selection bit and
	the {@link upper_bound_value(int) upper bound value} or {@link
	lower_bound_value(band) lower bound value} (if the
	SELECTED_LOWER_BOUND flag is set) for the corresponding band is
	returned.

	@param	A saturation bound selection SELECTED_XXX bit flags value.
	@return A saturation bound value, or -1 if no bound was selected.
*/
int saturation_bound (int selection);

/**	Select the saturation bounds that are nearest a graph position.

	The bands are selected with the smallest distance of the saturation
	lower or upper bounds values from the position x location. The
	distance must be within the {@link selection_distance() selection
	distance} for a band to be selected. All bands at the closest
	distance are selected. When both lower and upper bounds are at the
	minimum distance, the lower bounds takes precedence.

	@param	position	A QPoint containing a graph data position.
	@return	A bands SELECTED_XXX bit flags value. If selection is for
		lower bound(s) the SELECTED_LOWER_BOUND flag will also bet set.
		If no bounds location was selected the value will be zero
		(SELECTED_NONE).
*/
int selected_bounds (const QPoint& position) const;

bool upper_bound_value (int value, int band);
bool lower_bound_value (int value, int band);

/*==============================================================================
	Data
*/
private:

QwtPlotMarker
	*Bound_Marker,
	*Lower_Limit_Marker,
	*Upper_Limit_Marker,
	*Upper_Bound_Marker[3],
	*Lower_Bound_Marker[3];

int
	Lower_Bound_Value[3],
	Upper_Bound_Value[3];
double
	Lower_Bound_Percent[3],
	Upper_Bound_Percent[3];
int
	Selected_Bounds;
bool
	Bounds_Moving;
QPoint
	Last_Position;

//	Band selection.
int
	Selected_Bands;
int
	Selection_Distance;
static int
	Default_Selection_Distance;

//QwtPickerMachine *Bounder, *Tracker;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
