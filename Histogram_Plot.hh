/*	Histogram_Plot

HiROC CVS ID: $Id: Histogram_Plot.hh,v 1.5 2012/07/21 22:34:32 castalia Exp $

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

#ifndef HiView_Histogram_Plot_hh
#define HiView_Histogram_Plot_hh

//	Qwt
#include	"qwt_plot_item.h"

//	Qt
#include	<QColor>

//	Forward references.
class QString;
template<typename T> class QVector;


namespace UA::HiRISE
{
//	Forward reference.
class Count_Sequence;

/**	A <i>Histogram_Plot</i> is a QwtPlotItem that produces a histogram
	style plot for a graph (QwtPlot) widget.

	Data, in the form of a Count_Sequence, is provided for histogram
	plotting. Methods are provided to manage the color of the bars,
	in addition to the QwtPlotItem controls.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.5 $
*/
class Histogram_Plot
:	public QwtPlotItem
{
public:

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Histogram plotting attributes.
enum Attribute
	{
    VERTICAL_BARS		= 0,
    HORIZONTAL_BARS		= (1 << 0)
	};

/*==============================================================================
	Defaults
*/
//!	Default {@link bar_color(const QColor&) bar color}.
static QColor
	Default_Bar_Color;

//!	Default layering depth of the plot relative to other plotting layers.
static double
	Default_Plot_Z;

/*==============================================================================
	Constructors
*/
/**	Construct a Histogram_Plot with an optional title.

	The initial {@link #Attribute} setting is {@link #VERTICAL_BARS}.
	The initial {@link bar_color(const QColor&) bar color} is gray.

	@param	title	A QString providing the title of the plot.
*/
explicit Histogram_Plot (const QString& title = QString::null);

/**	Construct a Histogram_Plot with a title.

	The inititial {@link #Attribute} setting is {@link #VERTICAL_BARS}.
	The initial {@link bar_color(const QColor&) bar color} is gray.

	@param	title	A QwtText providing the title of the plot.
*/
explicit Histogram_Plot (const QwtText& title);

//!	Destructor.
virtual ~Histogram_Plot ();

/*==============================================================================
	Accessors
*/
/**	Assigns a data Count_Sequence to the Histogram_Plot.

	This method will result in a change to the item that may cause the
	parent plot to be refreshed.

	@param	count_sequence	A Count_Sequence to provide data content. The
		Count_Sequence that is assigned is used to replace the current data
		content.
	@return	This Histogram_Plot.
*/
Histogram_Plot& data (const Count_Sequence& count_sequence);

/**	Assigns a data Count_Sequence to the Histogram_Plot.

	If the specified Count_Sequence is NULL or identical to the
	current Count_Sequence data content nothing is done.

	If the data content Count_Sequence changes the base class is notified
	that a change to the item occurred which may cause the parent plot to
	be refreshed.

	<b>WARNING</b>: The ownership of the assigned Count_Sequence
	is transferred to this Histogram_Plot. The assigned Count_Sequence will
	be deleted if this method is used with a different Count_Sequence

	@param	count_sequence	A Count_Sequence to provide data content.
		<b>N.B.</b>: The ownership of the Count_Sequence is transferred
		to this Histogram_Plot. Any previous data content is deleted.
	@return	This Histogram_Plot.
	@see	data(const Count_Sequence&)
*/
Histogram_Plot& data (Count_Sequence* count_sequence);

/**	Assigns a data vector to the Histogram_Plot.

	The data vector is assigned to the data content Count_Sequence
	which replace its data values.

	This method will result in a change to the item that may cause the
	parent plot to be refreshed.

	@param	count_sequence	A QVector of unsigned long long values that
		will replace the current data content.
	@return	This Histogram_Plot.
*/
Histogram_Plot& data (QVector<unsigned long long>& data_sequence);

/**	Gets a reference to the Count_Sequence data content.

	@return	A Count_Sequence reference to data content.
*/
inline Count_Sequence& data ()
	{return *Data;}

/**	Gets a pointer to the data content vector.

	<b>WARNING</b>: Direct writable access is provided to the data
	content of the Histogram_Plot.

	@return	A QVector<unsigned long long> pointer to data content of this
		Count_Sequence. <b>N.B.</b>: The data content can be externally
		modified; however, unless the {@link data() data Count_Sequence}
		has its {@link Count_Sequence::calculate_max_count() max count
		value updated} the {@link Count_Sequence::max_count() reported
		max count value} may be incorrect.
*/
QVector<unsigned long long>* data_vector ();

/**	Sets the default color to use for histogram bars.

	@param	color	The default bar color.
*/
inline static void default_bar_color (const QColor& color)
	{Default_Bar_Color = color;}

/**	Gets the default color to use for histogram bars.

	@return	The default bar color.
*/
inline static QColor default_bar_color ()
	{return Default_Bar_Color;}

/**	Sets the color to use for the bars of this Histogram_Plot.

	@param	color	The bar color.
*/
Histogram_Plot& bar_color (const QColor& color);

/**	Gets the color being uses for the bars of this Histogram_Plot.

	@return	The bar color.
*/
inline QColor bar_color () const
	{return Bar_Color;}

/**	Sets the default plot depth for histogram bars.

	@param	z_depth	The default plotting depth for histogram bars.
*/
inline static void default_plot_z (double z_depth)
	{Default_Plot_Z = z_depth;}

/**	Gets the default plot depth for histogram bars.

	@return	The default plotting depth for histogram bars.
*/
inline static double default_plot_z ()
	{return Default_Plot_Z;}

/**	Sets an attribute of the Histogram_Plot.

	@param	attribute	The Attribute to be set.
	@param	enabled		Whether to enable or disable the attribute.
	@return	This Histogram_Plot.
*/
Histogram_Plot& attribute_set (Attribute attribute, bool enabled = true);

/**	Tests the setting of a Histogram_Plot attribute.

	@param	attribute	The Attribute to be tested.
	@return	Whether the attribute is enabled (true) or disabled (false).
*/
bool attribute_is_set (Attribute attribute) const;

/*==============================================================================
	QwtPlotItem virtual methods implementations
*/
/**	Gets the run time type information code for a Histogram_Plot.

	@return	QwtPlotItem::Rtti_PlotHistogram
*/
virtual int rtti () const;

/**	Gets the rectange that bounds the data content.

	The data content {@link Count_Sequence::boundingRect() bounding box}
	is reoriented if the {@link #HORIZONTAL_BARS} attribute is set.

	@return	A QRectF that describes the bounding box of the
		data content.
*/
virtual QRectF boundingRect () const;

/**	Paints the histogram bars on a canvas region using horizontal
	and vertical scaling maps.

	@param	painter	The QPainter to use for painting the histogram
		bars.
	@param	x_scale_map	A QwtScaleMap to use for mapping horizontal (x)
		postions to painter positions.
	@param	y_scale_map	A QwtScaleMap to use for mapping vertical (y)
		data content values to painter positions.
	@param	canvas_region	A QRect specifying the limits of the
		painter canvas to use.
*/
virtual void draw (QPainter* painter,
	const QwtScaleMap& x_scale_map, const QwtScaleMap& y_scale_map,
	const QRectF& canvas_region) const;

/*==============================================================================
	Helpers
*/
private:

void initialize ();

/*==============================================================================
	Data
*/
Count_Sequence
	*Data;

int
	Attributes;

QColor
	Bar_Color;
};


}	//	namespace UA::HiRISE
#endif	//	HISTOGRAM_PLOT_HH
