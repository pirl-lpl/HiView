/*	Data_Mapper_Tool

HiROC CVS ID: $Id: Data_Mapper_Tool.hh,v 1.52 2013/09/09 22:11:55 stephens Exp $

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

#ifndef HiView_Data_Mapper_Tool_hh
#define HiView_Data_Mapper_Tool_hh

#include	"Function_Nodes.hh"

#include	<QDockWidget>

//	Qt
#include	<QVector>
#include	<QSize>

//	Forward references.
class QWidget;
class QIcon;
class QLabel;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QPoint;
class QAction;
class QFileDialog;
class QErrorMessage;
class QResizeEvent;

//	Qwt
#include	<qwt.h>

//	Forward references.
class QwtPlot;
class QwtPlotCurve;
//class QwtPlotZoomer;
//class QwtPlotPanner;
class QwtSlider;
class QwtSymbol;
class QwtPlotMarker;

#include	<iosfwd>


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Icon_Button;
class Function;
class Function_Nodes;
class Graph_Tracker;

/**	The <i>Data_Mapper_Tool</i> for the HiView application.

	The Data_Mapper_Tool provides an interactive graph of source image
	data to display image data mapping functions and a control panel for
	user management of the mappings.

	A data mapping is provided for each of the three - red, green, blue -
	display image bands. Each display band data mapping is represented by
	a lookup table vector - called a Data_Map - with an 8-bit display
	image pixel sample value entry for each possible source image data
	value. Thus the size of the Data_Maps is determined by the source
	image pixel sample precision: the Data_Map index corresponds to a
	source image pixel sample value and the Data_Map entry provides the
	display image pixel sample value for the corresponding source value.
	The Data_Maps are provided by the user - they are owned externally
	(e.g. by a Plastic_Image) - and are managed in placed to avoid the
	cost of constantly copying these data structures.

	Each Data_Map is associated with Function_Nodes containing function
	node points that define the contents of its Data_Map. Each function
	will have at least two anchor nodes: one at source value zero and the
	other at the maximum source value. Additional function nodes may be
	provided between the anchor nodes. The function nodes may be
	interactively manipulated - moved, added and removed - on the
	tool's graph of the mapping functions.

	A special characteristic of the function nodes are the saturation
	bounds nodes. Each function has a lower and upper bound node: The
	lower bound node is the first node (lowest source value location) at
	or above the lower anchor node that has a zero display value; and the
	upper bound node is the last node (highest source value location) at
	or below the upper anchor node that has a maximum display value.
	There are no nodes between the saturation bounds nodes and their
	corresponding anchor nodes. The data mapping function between an
	anchor node and its saturation bounds node is a flat line at the
	minimum or maximum display value; thus the data mapping function
	saturates at these bounds nodes. Saturation bounds are typically used
	to apply a simple contrast stretch to the source data where the
	display data is linearly mapped from the minimum to the maximum
	values  between the lower and upper bound source values and source
	values outside the bounds being saturated. Of course, additial
	contrast stretching may be applied between the saturation bounds by
	adding additional function nodes.

	The location of saturation bounds nodes in their functions may be
	controlled by directly specifying their source value locations.
	Alternatively, the location of saturation bounds nodes may be
	specified in terms of the percentage of the source image histogram
	area for the band that is below or above the bounds node. This
	requires coordination with the Statistics_and_Bounds tool that
	maintains the histograms and can convert histogram percentages to
	source value locations.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.52 $
*/
class Data_Mapper_Tool
:	public QDockWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types
*/
//!	A lookup table that maps source data values to display image values.
typedef Function_Nodes::Data_Map		Data_Map;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Minimum size of the graph area.
static const QSize
	GRAPH_MIN_SIZE;

//!	Selects {@link remap_data(int, bool) remapping} bounds.
enum
	{
	REMAP_VALUES	= 0,
	REMAP_PERCENTS	= 1
	};

//!	Controls the extent of {@link linear_mapping(Mapping_Extent)}.
enum Mapping_Extent
	{
	UNBOUNDED_MAPPING,
	BOUNDED_MAPPING,
	LIMITED_MAPPING
	};

/*==============================================================================
	Constructor
*/
explicit Data_Mapper_Tool (Data_Map** data_maps, QWidget* parent = NULL);

virtual ~Data_Mapper_Tool ();

/*==============================================================================
	Accessors
*/
/**	Get the data maps being managed.

	<b>>>> WARNING <<<</b> Modifying the data maps can produce
	unexpected, and possibly catastrophic, results. The structure of the
	data maps are checked when they are provided to the Data_Mapper_Tool
	and their contents are expected to be managed in a consistent state
	by the Data_Mapper_Tool. It is safe to read, but not modify, the data
	maps.

	@return	A pointer to the array of three Data_Map pointers.
	@see	data_map(Data_Map**)
*/
inline Data_Map** data_maps () const
	{return Data_Maps;}

/**	Set the data maps to be managed.

	Each map must not be empty and all be the same size. The size
	determines the {@link max_x() maximum source value} which must have a
	precision of no more than {@link HiView_Config::MAX_SOURCE_DATA_PRECISION}.

	The contents of the data maps are reset to the default linear
	interpolation. The saturation bounds are silently reset to to the
	{@link upper_limit() upper} and {@link lower_limit() lower} limits of
	valid image data, or the maximum range of the data if no limits have
	been set. The data maps are then {@link remap_data(int, bool) remapped}
	unconditionally.

	@param	maps	An array of three Data_Map pointers in red, green,
		blue display band order. If the maps are identical to the current
		{@link data_maps() data maps} nothing is done. <b>N.B.</b>:
		Ownership of the data maps is not transferred to the Data_Mapper;
		they are managed in place, not copied.
	@throws invalid_argument if any of the maps pointers are NULL, a
		map is empty or larger than 1 << MAX_SOURCE_DATA_PRECISION, or
		they are not all the same size.
*/
void data_maps (Data_Map** maps);

/**	Test if a display band has been selected for modification.

	@param	band	A display band index (0-based).
	@return	true if the band has been selected for modification; false
		otherwise.
	@see	selected_bands()
*/
inline bool band_selected (int band) const
	{return Selected_Bands & (1 << band);}

/**	Get the map of currently selected bands.

	@return	A Band_Selection map in which the bit flag for each selected
		band is set. The bits for unselected bands are not set.
	@see	selected_bands(int)
*/
inline int selected_bands () const
	{return Selected_Bands;}

/**	Set the map of currently selected bands.

	@param	selection	A Band_Selection map in which the bit flag for
		each selected band is set. The bits for unselected bands are not
		set.
	@see	selected_bands()
	@see	select_band (bool, int)
*/
void selected_bands (int selection);

inline int selection_distance () const
	{return Selection_Distance;}
static int default_selection_distance (int distance);
inline static int default_selection_distance ()
	{return Default_Selection_Distance;}

/**	Get the maximum possible image value.

	@return	The maximum possible image value as determined from
		the size of the {@link data_maps() data maps}.
*/
inline int max_x () const
	{return Max_Source_Value;}
int max_y () const;


//	Saturuation Bounds - Values:
public:

/**	Get a saturation upper bound value.

	@param	band	A display band index (0-based).
	@return	The saturation upper bound value for the band.
	@see	lower_bound_value(int)
*/
int upper_bound_value (int band) const;

/**	Get a saturation lower bound value.

	@param	band	A display band index (0-based).
	@return	The saturation lower bound value for the band.
	@see	upper_bound_value(int)
*/
int lower_bound_value (int band) const;


private:

/**	Set the saturation upper bound value.

	The specified value is clipped to the range 1 - {@link max_x() max x}.

	If the currently indicated {@link upper_bound_value(int)
	current value} is different than the specified value, the
	Upper_Bound_Value for the band is set while its signals are blocked.

	If the Upper_Bound_Value is the same as the specified value for all
	{@link selected_bands() selected bands} the Upper_Bound_Value_Slider
	is set while its signals are blocked.

	If the {@link lower_bound_value(int) lower bound value} is
	greater than or equal to the specified value the lower value is reset
	to specified value - 1 to prevent range overlap.

	If the value has changed the Nodes for the band has its upper bound
	reset to the specified value. In addition, if the Signal_Changes flag
	is set, then the graph is replotted, the
	upper_bound_value_changed signal is emitted, and the
	data_maps_changed signal is emitted.

	@param	value	The new saturation upper bound value.
	@param	band	The band to which the value applies. If the value
		is not in the valid range of 0-2 nothing is done.
	@param	force_change	If false the upper bound node value will be
		set to the specified value only if the currently indicated value
		is different or not. If true the upper bound node is
		unconditionally set.
	@return	true if the specified value is different than the previous
		value.
*/
bool upper_bound_value (int value, int band,
		bool force_change = false);

/**	Set the saturation lower bound value.

	@param	value	The new saturation upper bound value.
	@param	band	The band to which the value applies. If the value
		is not in the valid range of 0-2 nothing is done.
	@param	force_change	If false the upper bound node value will be
		set to the specified value only if the currently indicated value
		is different or not. If true the upper bound node is
		unconditionally set.
	@return	true if the specified value is different than the previous
		value.
	@see	upper_bound_value(int, int, bool)
*/
bool lower_bound_value (int value, int band,
		bool force_change = false);

//	Saturuation Bounds - Percents:
public:

/**	Set the saturation bounds percent values but do not apply them.

	The upper and lower saturation bounds percent values are changed
	without {link apply_bound_percents(bool) applying} them.

	<b>N.B.</b>: The percent values are rounded to {@link
	#HiView_Config::PERCENT_DECIMAL_PLACES} decimal places.

	@param	upper	A QVector of three doubles in display band order
		(Red, Green, Blue) specifying the upper saturation bound percent
		values. A negative value does not change the existing value; a
		non-negative value is the new setting value clipped to be no
		greater than 100.
	@param	lower	A QVector of three doubles in display band order
		(Red, Green, Blue) specifying the lower saturation bound percent
		values. A negative value does not change the existing value; a
		non-negative value is the new setting value clipped to be no
		greater than 100.
*/
void bound_percents
	(const QVector<double>& upper, const QVector<double>& lower);

/**	Get a saturation upper bound percent.

	@param	band	A display band index (0-based).
	@return	The saturation upper bound percent for the band.
	@see	lower_bound_percent(int)
	@see	bound_percents(const QVector<double>&, const QVector<double>&)
*/
double upper_bound_percent (int band) const;

/**	Get a saturation lower bound percent.

	@param	band	A display band index (0-based).
	@return	The saturation lower bound percent for the band.
	@see	upper_bound_percent(int)
	@see	bound_percents(const QVector<double>&, const QVector<double>&)
*/
double lower_bound_percent (int band) const;

/**	Get a default contrast stretch saturation upper bound percent.

	@param	band	A display band index (0-based).
	@return	The default contrast stretch saturation upper bound percent
		for the band.
	@see	upper_default_contrast_stretch(double, int)
	@see	lower_default_contrast_stretch(int)
*/
inline double upper_default_contrast_stretch (int band) const
	{return (band >= 0 && band < 3) ?
		Upper_Default_Contrast_Stretch[band] : -1.0;}

/**	Get a default contrast stretch saturation lower bound percent.

	@param	band	A display band index (0-based).
	@return	The default contrast stretch saturation lower bound percent
		for the band.
	@see	lower_default_contrast_stretch(double, int)
	@see	upper_default_contrast_stretch(int)
*/
inline double lower_default_contrast_stretch (int band) const
	{return (band >= 0 && band < 3) ?
		Lower_Default_Contrast_Stretch[band] : -1.0;}


private:

bool upper_bound_percent (double percent, int band, bool force_change = false);
bool lower_bound_percent (double percent, int band, bool force_change = false);


public:
//	Data Limits:

/**	Get the upper limit, inclusive, of valid image data.

	The Data_Mapper does not manage the image data limits, but may
	receive notice when the {@link upper_limit(int) limit has changed}
	which the Data_Mapper uses to adjust the saturation bounds as needed.
	Thus the returned value is the last value for which a notice was
	received.

	<b>N.B.</b>: The upper limit is expressed in terms of a non-negative
	offset from the {@link max_x() maximum image value}.

	@return	The upper limit, inclusive, of valid image data expressed as
		a non-negative offset below the {@link max_x() maximum possible
		image value. The default value, if no limit report has been
		received, will be zero.
	@see	data_maps(Data_Map**)
*/
inline int upper_limit () const
	{return Upper_Limit;}
inline int lower_limit () const
	{return Lower_Limit;}

//	Graph File:

bool load_CSV_file (const QString& pathname);
bool save_CSV_file (const QString& pathname);
bool save_graph_image (const QString& pathname);

void last_CSV_file (const QString& pathname);
inline QString last_CSV_file () const
	{return Last_CSV_Pathname;}

inline static QErrorMessage* error_message ()
	{return Error_Message;}
//	Ownership of the QErrorMesage is NOT transferred.
inline static void error_message (QErrorMessage* dialog)
	{Error_Message = dialog;}

/*==============================================================================
	GUI elements
*/
virtual QSize minimumSizeHint () const;
virtual QSize sizeHint () const;

/*	Get the previous size of this Navigator_Tool before the last
	{@link resizeEvent(QResizeEvent*) resize event}.

	@return	A QSize for the previous size of this Navigator_Tool. This
		will be an invalid size if a previous size is not yet known.
*/
QSize previous_size () const
	{return Previous_Size;}

inline bool visible_graph () const
	{return Visible_Graph;}
bool visible_graph (bool enabled);

void refresh_graph ();

//	Actions for external use:

QList<QAction*> band_selection_actions () const;
QList<QAction*> presets_actions () const;
QList<QAction*> file_actions () const;

inline QAction* default_contrast_stretch_action () const
	{return Default_Contrast_Stretch_Action;}
inline QAction* restore_original_contrast_stretch_action () const
	{return Restore_Original_Contrast_Stretch_Action;}
inline QAction* apply_percents_action () const
	{return Apply_Percents_Action;}
inline QAction* percents_actual_to_settings_action () const
	{return Actual_to_Settings_Action;}


private:

QWidget* control_panel ();
QWidget* graph_panel ();

/*==============================================================================
	Utilities
*/
public:

inline static void linear_interpolation
	(int x0, int y0, int x1, int y1, Data_Map* data_map)
	{Function_Nodes::linear_interpolation (x0, y0, x1, y1, data_map);}
inline static void linear_interpolation
	(const QPoint& start, const QPoint& end, Data_Map* data_map)
	{Function_Nodes::linear_interpolation (start, end, data_map);}
void linear_interpolation
	(int x0, int y0, int x1, int y1, int band = -1);
void linear_interpolation
	(const QPoint& start, const QPoint& end, int band = -1);
void linear_interpolation
	(int band_selections, const QPoint& start, const QPoint& end);

/**	Apply the current saturation bounds to the data maps.

	@param	bounds_selection	If {@link REMAP_VALUES} the current
		saturation bounds values are applied; if {@link REMAP_PERCENTS}
		the current saturation bounds percent settings are applied.
	@param	unconditional	If true, saturation bounds and data map
		change signals are emitted even if no change has been detected.
*/
void remap_data (int bounds_selection = REMAP_VALUES,
	bool unconditional = false);

static void print_data_map (std::ostream& stream, Data_Map* map);
void print_data_maps (std::ostream& stream);

/*==============================================================================
	Qt signals:
*/
signals:

void data_maps_changed (Data_Map** maps);

void selected_bands_changed (int band_selections);

void saturation_mode (bool enabled);

void upper_bound_values_changed (const QVector<int>& values);
void lower_bound_values_changed (const QVector<int>& values);

void upper_bound_percents_changed (const QVector<double>& percents);
void lower_bound_percents_changed (const QVector<double>& percents);

void tool_context_menu_requested (QDockWidget* tool, QContextMenuEvent* event);

/*==============================================================================
	Qt slots
*/
public slots:

void band_map (const unsigned int* bands);
void refresh_band_numbers ();

void select_band (bool enabled, int band = -1);
void selection_distance (int distance);

bool linear_mapping (Mapping_Extent extent = UNBOUNDED_MAPPING);
bool limited_linear_mapping ();
bool bounded_linear_mapping ();

//	Bounds Values:

bool upper_bound_values (const QVector<int>& values);
bool lower_bound_values (const QVector<int>& values);
void apply_bound_values ();

//	Bounds Percents:

bool upper_bound_percents (const QVector<double>& percents);
bool lower_bound_percents (const QVector<double>& percents);

/**	Set the actual upper bound percent values.

	This slot is expected to be used by the Statistics_and_Bounds_Tool
	to provide the actual upper bound percent values in response to
	a signal from the Data_Mapper_Tool that the upper bound values
	or percents have been changed.

	<b>N.B.</b>: The upper bound percent settings are not changed;
	i.e. there will be no change to the data mapping.

	@param	percents	A QVector of three doubles. A negative value
		does not change the existing value; a non-negative value is
		applied.
*/
void actual_upper_bound_percents (const QVector<double>& percents);

/**	Set the actual lower bound percent values.

	This slot is expected to be used by the Statistics_and_Bounds_Tool
	to provide the actual lower bound percent values in response to
	a signal from the Data_Mapper_Tool that the lower bound values
	or percents have been changed.

	<b>N.B.</b>: The lower bound percent settings are not changed;
	i.e. there will be no change to the data mapping.

	@param	percents	A QVector of three doubles. A negative value
		does not change the existing value; a non-negative value is
		applied.
*/
void actual_lower_bound_percents (const QVector<double>& percents);

/**	Apply the current saturation bound percent settings.

	Both the {@link upper_bound_percent(int) upper} and {@link
	lower_bound_percent(int) lower} saturation bound percents settings
	are applied to the currently {@link selected_bands() selected bands}.

	@see	bound_percents
		(const QVector<double>&, const QVector<double>&)
*/
void apply_bound_percents ();

/**	Apply the default constrast stretch saturation bound percents.

	@see	upper_default_contrast_stretch(double, int)
	@see	lower_default_contrast_stretch(double, int)
*/
void default_contrast_stretch ();

/** Restore the Original contrast strech saturation bound percents

	Original Upper Bound Values = [0.0,0.0,0.0]
	Original Lower Bound Values = [0.0,0.0,0.0]
*/
void restore_original_contrast_stretch();

/**	Set the upper default contrast stretch saturation bound percent
	for a display band.

	@param	percent	The default contrast stretch saturation upper bound
		percent.
	@param	band	A display band index (0-based).
	@see	upper_default_contrast_stretch(int)
	@see	lower_default_contrast_stretch(double, int)
	@see	default_contrast_stretch()
*/
void upper_default_contrast_stretch (double percent, int band);

/**	Set the lower default contrast stretch saturation bound percent
	for a display band.

	@param	percent	The default contrast stretch saturation lower bound
		percent.
	@param	band	A display band index (0-based).
	@see	lower_default_contrast_stretch(int)
	@see	upper_default_contrast_stretch(double, int)
	@see	default_contrast_stretch()
*/
void lower_default_contrast_stretch (double percent, int band);

//	Data Limits:

/**	The upper limit of valid image data that has been set.

	Any {@link upper_bound_value(int) saturation upper bound value} that
	is greater than the limit is reset to the limit.

	@param	limit	The upper limit, inclusive, of valid image data
		expressed as a non-negative offset below the maximum possible
		image value.
*/
void upper_limit (int limit);

/**	The lower limit of valid image data that has been set.

	Any {@link lower_bound_value(int) saturation lower bound value} that
	is lower than the limit is reset to the limit.

	@param	limit	The lower limit, inclusive, of valid image data.
*/
void lower_limit (int limit);

void canvas_color (QRgb color);

void save ();
void load ();


private slots:

void select_band ();

void saturation_controls (bool enabled);

/**	Router for all saturation bounds widgets.

	The {Upper,Lower}_Bound_{Value,Percent}{_Slider} widgets are all
	connected to this slot. Depending on the object that generated
	the signal the widget's value is checked and the appropriate
	specific slot called.
*/
void bound ();

//	Interactive Graphing:

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
void mouse_down (const QPointF& point);

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
void mouse_drag (const QPointF&);

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
void mouse_up (const QPointF& point);

void leave_graph ();

//	Graph File:

/**	Set the file format for a saved graph.

	A filter has been selected from the Save_File_Dialog. The first word
	(space separated) of the filter format string is used to set the
	Graph_Data_Format and the default filename suffix to be applied by
	the Save_File_Dialog.

	@param	format	A file format filter QString.
*/
void graph_data_format (const QString& format);

void last_file_action ();

/*==============================================================================
	Event Handlers
*/
protected:

virtual void resizeEvent (QResizeEvent* event);
virtual void contextMenuEvent (QContextMenuEvent* event);

/*==============================================================================
	Helpers
*/
public:

private:

/**	Set new Data_Maps.

	Ownership of the data maps is not transferred to the Data_Mapper;
	they are managed in place.

	Each map must not be empty and all be the same size. The size
	determines the {@link max_x() maximum source value} which must have a
	precsion of no more than {@link
	HiView_Config::MAX_SOURCE_DATA_PRECISION}.

	@param	maps	An array of three Data_Map pointers.
	@throws invalid_argument if any of the maps pointers are NULL, a
		map is empty or larger than 1 << MAX_SOURCE_DATA_PRECISION, or
		they are not all the same size.
	@see	data_maps(Data_Map**)
*/
void reset_data_maps (Data_Map** maps);

/**	Select the graphed data coordinate that is nearest a graph position.

	A Function data point must be within the {@link selection_distance()
	selection distance} from the graph position to be selected. The
	closest data point to the graph position is selected.

	When there is more than one data point at the same closest distance
	to the graph position the data point that occurs most often - i.e.
	is present on the most bands that were specified to be search - is
	selected. When the number of band occurances is the same, however,
	the first data point encountered - which will be the one with the
	highest x value - will be selected.

	Only Function data points for {@link selected_bands() selected bands}
	are searched.

	@param	position	A QPoint containing a graph data position.
	@return	A QPoint containing the x position of the selected data
		point as the x value, and the Band_Selection where the data
		point occurs as the y value. <b>N.B.</b>: If no data point was
		selected the x value will be -1 and the y value will be
		SELECTED_NONE.
*/
QPoint selected_data_point (const QPoint& position) const;

/**	Select a saturation bound that is nearest a graph position.

	The bands are selected with the smallest distance of their Nodes'
	lower or upper bounds from the position x location. The distance must
	be within the {@link selection_distance() selection distance} for a
	band to be selected. All bands at the closest distance are selected.
	When both lower and upper bounds are at the minimum distance, the
	lower bounds takes precedence.

	Only Function data points for {@link selected_bands() selected bands}
	are searched.

	@param	position	A QPoint containing a graph data position.
	@return	A QPoint containing a lower or upper bound flag as the x
		value: negative indicates lower bound; positive indicates upper
		bound. The y value is the Band_Selection for the selected bands.
		<b>N.B.</b>: If no bounds location was selected the x value will
		be zero and the y value will be SELECTED_NONE (0).
*/
QPoint selected_bounds (const QPoint& position) const;

//void enableZoomMode (bool);

/*==============================================================================
	Data
*/
private:

Data_Map
	**Data_Maps;
int
	Max_Source_Value;

//	Control Panel:

//		Band selection.
int
	Selected_Bands;

Icon_Button
	*Band_Selection_Buttons[3];
QAction
	*Band_Selection_Actions[4];	//	The extra entry selects all bands.
QIcon
	*Band_Selection_Icons[4],
	*Not_Selected_Icon;
QSize
	Band_Selection_Icon_Size;
QLabel
	*Band_Mapping_Labels[3];
unsigned int
	Band_Map[3];

//		Selection distance.
int
	Selection_Distance;
static int
	Default_Selection_Distance;

//		Saturation Bounds.
Icon_Button
	*Bounds_Button;
QIcon
	*Bounds_Icon;
QwtSlider
	*Upper_Bound_Percent_Slider,
	*Lower_Bound_Percent_Slider;
QwtSlider
	*Upper_Bound_Value_Slider,
	*Lower_Bound_Value_Slider;
QSpinBox
	*Upper_Bound_Value[3],
	*Lower_Bound_Value[3];
QDoubleSpinBox
	*Upper_Bound_Percent_Setting[3],
	*Lower_Bound_Percent_Setting[3];
QLabel
	*Upper_Bound_Percent_Actual[3],
	*Lower_Bound_Percent_Actual[3];
QVector<double>
	Upper_Default_Contrast_Stretch,
	Lower_Default_Contrast_Stretch;
QAction
	*Default_Contrast_Stretch_Action,
	*Restore_Original_Contrast_Stretch_Action,
	*Apply_Percents_Action,
	*Actual_to_Settings_Action;
QwtPlotMarker
	*Bound_Marker;
bool
	Signal_Changes;


//!	Valid image data limits as non-negative offset from max and min values.
int
	Upper_Limit,
	Lower_Limit;

//	Graph Panel:

/*	The current graph manipulation mode.

	This will be one of the bit flags specifed by the Operating_Modes -
	TRACKING_MODE or SATURATION_MODE - with the DRAGGING_MODE bit set
	when a selection has been made and it is being dragging on the graph.
*/
int
	Operating_Mode;

QwtPlot
	*Graph;

QwtPlotCurve
	*Function_Plots[3],
	*Node_Plots[3];

//	The internal Function class wraps a Data_Map in a QwtData interface.
Function
	*Functions[3];

//	The Function_Nodes class manages a Data_Map based on function node points.
Function_Nodes
	*Nodes[3];

QwtSymbol
	*Selection_Symbol;
QwtPlotMarker
	*Selection_Marker;

//	Bands selected during node dragging.
int
	Node_Bands_Selected;

/*	Node position being dragged.

	When the Operating_Mode is TRACKING_MODE this value is set in the
	graph_postion handler to the value returned by selected_data_point,
	used to set the cursor position when the mouse goes down, and updated
	as the mouse is dragged.

	When the Operating_Mode is SATURATION_MODE this value is set in the
	graph_position handler to the value returned by selected_bounds with
	its y coordinate transferred to Node_Bands_Selected and its x
	coordinate indicating which bound(s) - upper or lower - were
	selected, converted to a bounding node position when the mouse goes
	down, and updated as the mouse is dragged.
*/
QPoint
	This_Node;

/*	The indices of the nodes immediately before and after This_Node.

	When the Operating_Mode is SATURATION_MODE only Last_Node_Index[0] is
	used to carry forward the upper/lower bound indicator from This_Node
	before it is converted to a bounding node position.

	When the Operating_Mode is TRACKING_MODE the values are set when the
	mouse goes down: They will be -1 if the corresponding band was not a
	Node_Bands_Selected. Otherwise the values are set to the indices of
	the nodes immediately before and after the position of This_Node,
	unless This_Node is at an end-point anchor in which case the index of
	the anchor node is used. These values are used during node dragging
	to determine when node merging is to occur. When node merging occurs
	the corresponding values are updated to the new last/next node
	indices.
*/
int
	Last_Node_Index[3],
	Next_Node_Index[3];

Graph_Tracker
	*Tracker;
/*
QwtPlotPanner
	*Panner;
QwtPlotZoomer
	*Zoomer;
*/

//	Mapping presets.
QAction
	*Limited_Linear_Action,
	*Bounded_Linear_Action,
	*Unbounded_Linear_Action;

//	Load/Save files.
QPushButton
	*File_Menu_Button;
QFileDialog
	*Load_File_Dialog,
	*Save_File_Dialog;
QAction
	*Load_CSV_File_Action,
	*Load_Last_CSV_File_Action,
	*Save_File_Action,
	*Save_Last_CSV_File_Action;
QString
	Graph_Directory;
QString
	Graph_Data_Format;
static QString
	Default_Graph_Data_Format;
QString
	Last_CSV_Pathname;

//	Dynamic size control.
QSize
	Minimum_Size;
bool
	Visible_Graph;
int
	Graph_Width_Increment;
QSize
	Previous_Size;

//!	Shared error message dialog.
static QErrorMessage
	*Error_Message;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
