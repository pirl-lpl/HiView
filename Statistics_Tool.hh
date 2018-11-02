/*	Statistics_Tool

HiROC CVS ID: $Id: Statistics_Tool.hh,v 1.24 2011/11/27 02:14:01 castalia Exp $

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

#ifndef HiView_Statistics_Tool_hh
#define HiView_Statistics_Tool_hh

#include	<QWidget>

#include	"Stats.hh"

//	Forward references.
class QWidget;
class QColor;
class QLabel;
class QSpinBox;

class QwtPlot;


namespace UA
{
namespace HiRISE
{
//	Forward References:
class Graph_Tracker;
class Histogram_Plot;

/**	The <i>Statistics_Tool</i> for the HiView application.

	The Statistics_Tool displays the results of various measurement
	operations on the image.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.24 $
*/
class Statistics_Tool
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types
*/
typedef Stats::Histogram	Histogram;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Minimum size of the graph area.
static const QSize
	GRAPH_MIN_SIZE;

static const double
	DEFAULT_OPACITY[3];

/*==============================================================================
	Constructors
*/
Statistics_Tool (const QString& title,
	QWidget* parent = NULL, Qt::WindowFlags flags = 0);

virtual ~Statistics_Tool ();

/*==============================================================================
	Accessors
*/
inline Stats& stats ()
	{return Statistics;}
inline int statistics_sets () const
	{return Statistics.sets ();}

/**	Set the structure of the data.

	The image data for which statistics are reported must be defined in
	terms of the number of bands and the pixel sample (datum) precision
	for each band. If the number of bands specified is less than the
	current number of {@link data_bands() statistics sets} additional
	{@link stats() statistics} histograms and data plots are allocated;
	if the number of bands specified is greater than the current number
	of statistics sets the excess histograms and data plots are removed.

	Statistics sets are allocated in increasing set index order. There
	is no implicit mapping between image band numbers and statistics
	set indexes. Setting the {@link band_map(const unsigned int*)
	band map} provides such a {@link display_to_statistics_index_map()
	display band to set index mapping}.

	<b>N.B.</b>: The {@link upper_limit(int) upper data limit} and {@link
	lower_limit(int) lower data limit} values are reset based on the
	current {@link stats() statistics} limits offsets from the data range
	end points. However, signals are bloocked during the structure change
	process to prevent adverse effects on external tools (e.g. the
	Data_Mapper_Tool) listening for these signals that may not yet have
	been reset with new data maps. Thus setting the limits for the
	external tools after they have been reset is recommended.

	<b>N.B.</b>: The histogram data plots and statistics report are not
	{@link refresh(const QRect&) refreshed}. Having redefined the
	structure of the data the user should reload the {@link stats()
	statistics} {@link Stats::histograms() histograms} from the data
	source before refreshing.

	@param	bands	The total number of data bands to be reported. The
		maximum reported bands is three; additional bands are ignored.
	@param	precision	The pixel sample precision in bits. The maximum
		supported precision is 16-bit data (set by
		MAX_SOURCE_DATA_PRECISION in HiView_Config).
*/
void data_structure (int bands, int precision);

/**	Get the current map of display bands to statistics sets.

	The map of display bands to statistics sets provides an index to
	the applicable {@link stats() statistics} set in display band
	order; i.e. entry zero of the map provides the index of the
	statistics set for the red display band, entry 1 for the green
	band, and entry 2 for the blue band. The same statistics set
	may be referenced by more than one display band; e.g. for a
	monochrome image all map entries will be zero.

	@return	An array of three statistics set index values in display
		band order.
	@see band_map(const unsigned int*)
*/
const unsigned int* display_to_statistics_index_map () const
	{return Display_to_Stats_Map;}

int data_precision () const;
int max_x () const;
int max_y () const;

/**	Enable or disable inclusion of the upper and lower limits.

	@param	enable	If true upper and lower limits selection fields and
		their values for each band will be included is the statistics
		summary report; otherwise no limits characteristics will be
		included in the statistics report.
*/
void limits (bool enabled);

/**	Test if upper and lower limits are included in the statistics report.

	@return	true upper and lower limits selection fields and
		their values for each band will be included is the statistics
		summary report; false if no limits characteristics will be
		included in the statistics report.
*/
inline bool limits () const
	{return Limits_Enabled;}

/**	The upper limit of valid image values.

	@return	The upper limit, inclusive, of valid image values.
		<b>N.B.</b>: This is an image value, not the offset from the
		maximum possible image value.
	@see	upper_limit(int)
*/
int upper_limit () const;
int lower_limit () const;

inline int upper_limit_offset () const
	{return (Limits_Enabled ? Statistics.upper_limit () : 0);}
inline int lower_limit_offset () const
	{return (Limits_Enabled ? Statistics.lower_limit () : 0);}

/**	Set the color applied to the histogram data band plots.

	The initial colors for each histogram band are the
	DISPLAY_BAND_COLORS: red, green, blue - with the alpha values set to
	their {@link band_opacity(int) opacity} percentage. The band
	statistics column labels are the DISPLAY_BAND_COLORS in a slightly
	lighter tone to enhance contrast with the label text; the label
	colors are not changed.

	The alpha value of the specified color is used to reset the reported
	{@link band_opacity(int) band opacity}.

	@param	color	A QColor to be applied to the histogram data
		band plot.
	@param	band	A valid band number. The first band is zero; the
		band number must be less than the current {@link data_bands()
		number of data bands}. If the band number is invalid nothing
		is done.
*/
void band_color (const QColor& color, int band);
QColor band_color (int band) const;

void band_opacity (int percent, int band);
int band_opacity (int band) const;

/*==============================================================================
	Manipulators
*/
/**	Refresh the statistics values.

	<b>N.B.</b>: It is presumed that the {@link stats() statistics}
	histograms have been updated from the selected region of the image
	data.

	The {@link stats() statistics} values are recalculated. Then the
	values displayed in the info panel widgets are updated and the graph
	of the histograms are replotted.

	@param	selected_region	The QRect specifying the region of the image
		from which the current contents of the statistics histograms
		were gathered. If a null rectangle is specified the actual area
		field of the statistics report will not be changed.
	@return	true if the statistics were refreshed; false otherwise.
*/
virtual bool refresh (const QRect& selected_region = QRect ());

inline bool visible_graph () const
	{return Visible_Graph;}
bool visible_graph (bool enabled);

/*==============================================================================
	Qt signals:
*/
signals:

void statistics_refreshed ();

/**	Signal a change to the upper limit of valid image data.

	<b>N.B.</b>: The limit value is expressed as the offset from the
	maximum image value; i.e. the limit is relative to the range of
	possible image values.

	@param	limit	The upper limit of valid image data expressed as
		a non-negative offset from the maximum possible image value.
	@see	upper_limit(int)
*/
void upper_limit_changed (int limit);
void lower_limit_changed (int limit);

/*==============================================================================
	Qt slots
*/
public slots:

/**	Set the numbers of the reported image bands.

	Though the context of the data bands is not relevant to statistics
	reporting - the bands may be display image bands or selected source
	image bands - other tools (e.g. the Data_Mapper_Tool) are likely to
	have expectations that the band statistics sets will be in display
	image band order - i.e. red (0), green (1), blue (2) - regardless of
	their source image band mapping. This is reflected in the {@link
	band_color(int) colors} applied by default to the histogram band data
	plots and band name labels.

	The band map is used to update a {@link
	display_to_statistics_index_map() display band to set index mapping}.
	This mapping is initialized to a one-to-one relationship: each
	display band maps to the corresponding set index. Display band zero
	is always mapped to set zero. If band map entry 1 maps to the same
	value as band map entry zero then set index map 1 is also mapped to
	zero, otherwise it is mapped to 1. Likewise for band map entry 2: if
	it maps to the same value as another band map entry set index map
	entry 2 mapped to the corresponding set map entry, otherwise it is
	mapped to 2. Thus duplicate band map entries result in a one-to-many
	relationship that is refected in the set index mapping.

	@param	bands	An array of three integers that specify the
		band numbers of the three image bands that can be reported.
*/
void band_map (const unsigned int* bands);

void refresh_band_numbers ();

/**	Set the upper limit of valid image data.

	The limit value is clipped to be within the range 0 <= limit <=
	{@link max_x() max image value}. To maintain range integrity the
	{@link lower_limit(int) lower limit} is reset to the limit if it is
	currently greater than the new limit value.

	If the {@link limits() limits} reporting is enabled the new limit
	is passed to the {@link stats() statistics} object and then the
	summary statistics are {@link refresh(const QRect&) refreshed) and
	the change to the new limit value is {@link upper_limit_changed(int)
	signalled}.

	@param	limit	The upper limit, inclusive, of valid image values.
		<b>N.B.</b>: This is an image value, not an offset from the
		maximum possible image value.
	@see	upper_limit()
	@see	upper_limit_changed(int)
*/
virtual void upper_limit (int limit);
virtual void lower_limit (int limit);

void canvas_color (QRgb color);


private slots:

void vertical_axis_max (int max);

void opacity_changed (int percent);

/*==============================================================================
	GUI elements
*/
public:

virtual QSize minimumSizeHint () const;
virtual QSize sizeHint () const;


private:

QWidget* graph_panel ();
QWidget* info_panel ();

/*==============================================================================
	Helpers
*/
private:

void view_band_data (int band, bool enabled);
Histogram_Plot* new_plot (int band);

/*==============================================================================
	Data
*/
protected:

Stats
	Statistics;

//!	Map of Statistics set indexes by display image band number.
unsigned int
	Display_to_Stats_Map[3];
//!	Map of source band index to display band index.
unsigned int
	Band_Map[3];

bool
	Limits_Enabled;

QWidget
	*Graph_Panel;
QSpinBox
	*Count_Scale_Max;
QwtPlot
	*Graph;
Histogram_Plot
	*Plot[3];
Graph_Tracker
	*Tracker;

//	Dynamic size control.
QSize
	Minimum_Size;
bool
	Visible_Graph;
int
	Graph_Width_Increment;


private:

QLabel
	*Origin[2],
	*Size[2],
	*Sampled_Area,
	*Area,
	*Band_Names[3],
	*Values[3],
	*Below_Limit_Label,
	*Below_Limit[3],
	*Lowest_Value[3],
	*Highest_Value[3],
	*Above_Limit_Label,
	*Above_Limit[3],
	*Minimum_Count[3],
	*Maximum_Count[3],
	*Mean_Value[3],
	*Median_Value[3],
	*Std_Dev_of_Values[3];

QSpinBox
	*Lower_Limit,
	*Upper_Limit,
	*Opacity[3];

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
