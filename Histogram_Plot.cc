/*	Histogram_Plot

HiROC CVS ID: $Id: Histogram_Plot.cc,v 1.8 2012/03/09 02:13:56 castalia Exp $

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

#include	"Histogram_Plot.hh"

#include	"Count_Sequence.hh"

#include <QString>
#include <QPainter>

#include <qwt_plot.h>
#include <qwt_series_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 2)
#define	DEBUG_DRAW				(1 << 3)
#define DEBUG_SLOTS				(1 << 4)

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

#include	"HiView_Utilities.hh"

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{

/*==============================================================================
	Defaults
*/
#ifndef HISTOGRAM_BAR_COLOR
#define HISTOGRAM_BAR_COLOR				gray
#endif
QColor
	Histogram_Plot::Default_Bar_Color	= Qt::HISTOGRAM_BAR_COLOR;

#ifndef HISTOGRAM_PLOT_Z
#define HISTOGRAM_PLOT_Z				20.0
#endif
double
	Histogram_Plot::Default_Plot_Z		= HISTOGRAM_PLOT_Z;

/*==============================================================================
	Constructors
*/
Histogram_Plot::Histogram_Plot
	(
	const QwtText&	title
	)
	:	QwtPlotItem (title),
		Data (new Count_Sequence),
		Attributes (Histogram_Plot::VERTICAL_BARS),
		Bar_Color (Default_Bar_Color)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Histogram_Plot @ " << (void*)this << ": " << title.text () << endl;
#endif
initialize ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Histogram_Plot" << endl;
#endif
}

Histogram_Plot::Histogram_Plot
	(
	const QString&	title
	)
	:	QwtPlotItem (QwtText (title)),
		Data (new Count_Sequence),
		Attributes (Histogram_Plot::VERTICAL_BARS),
		Bar_Color (Default_Bar_Color)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Histogram_Plot: " << title << endl;
#endif
initialize ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Histogram_Plot" << endl;
#endif
}


Histogram_Plot::~Histogram_Plot()
{
delete Data;
}


void
Histogram_Plot::initialize ()
{
setItemAttribute (QwtPlotItem::AutoScale, true);
setItemAttribute (QwtPlotItem::Legend, true);
setZ (Default_Plot_Z);
}

/*==============================================================================
	Accessors
*/
Histogram_Plot&
Histogram_Plot::data
	(
	const Count_Sequence&	count_sequence
	)
{
*Data = count_sequence;
itemChanged ();	//	Assume the data changed.
return *this;
}


Histogram_Plot&
Histogram_Plot::data
	(
	Count_Sequence*			count_sequence
	)
{
if (count_sequence &&
	count_sequence != Data)
	{
	delete Data;
	Data = count_sequence;
	itemChanged ();	//	Assume the data changed.
	}
return *this;
}

Histogram_Plot&
Histogram_Plot::data
	(
	QVector<unsigned long long>&	data_sequence
	)
{
Data->data (data_sequence);
itemChanged ();	//	Assume the data changed.
return *this;
}


QVector<unsigned long long>*
Histogram_Plot::data_vector ()
{return &(Data->data ());}


Histogram_Plot&
Histogram_Plot::bar_color
	(
	const QColor&	color
	)
{
if (Bar_Color != color )
	{
	Bar_Color = color;
	itemChanged ();
	}
return *this;
}


Histogram_Plot&
Histogram_Plot::attribute_set
	(
	Attribute	attribute,
	bool		enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">>> Histogram_Plot::attribute_set: " << attribute
		<< ' ' << boolalpha << enabled << endl;
#endif
if (attribute_is_set (attribute) != enabled)
	{
	if (attribute == VERTICAL_BARS)
		{
		attribute = HORIZONTAL_BARS;
		enabled = ! enabled;
		}
    if (enabled)
        Attributes |= attribute;
    else
        Attributes &= ~attribute;
    itemChanged ();
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "<<< Histogram_Plot::attribute_set: " << Attributes << endl;
#endif
return *this;
}


bool
Histogram_Plot::attribute_is_set
	(
	Attribute	attribute
	) const
{
if (attribute == VERTICAL_BARS)
	return ! (Attributes & HORIZONTAL_BARS);
return Attributes & attribute;
}

/*==============================================================================
	QwtPlotItem virtual methods implementations
*/
QwtDoubleRect
Histogram_Plot::boundingRect () const
{
QwtDoubleRect
	rectangle = Data->boundingRect ();
if (rectangle.isValid () &&
	attribute_is_set (HORIZONTAL_BARS))
	//	Horizontal values, increasing upwards.
    rectangle.setRect
		(0.0, rectangle.height (), 
		 rectangle.height (), rectangle.width ());
return rectangle;
}


int
Histogram_Plot::rtti () const
{return QwtPlotItem::Rtti_PlotHistogram;}


void
Histogram_Plot::draw
	(
	QPainter*			painter,
	const QwtScaleMap&	xMap, 
    const QwtScaleMap&	yMap,
	const QRectF&
	) const
{
#if ((DEBUG_SECTION) & DEBUG_DRAW)
clog << ">>> Histogram_Plot @ " << (void*)this << "::draw" << endl
	 << "    Attributes = " << Attributes << endl
	 << "    orientation = " << (attribute_is_set (HORIZONTAL_BARS) ?
			"HORIZONTAL_BARS" : "VERTICAL_BARS") << endl;
#endif
int
	bars = (int)Data->size ();
if (! bars)
	{
	#if ((DEBUG_SECTION) & DEBUG_DRAW)
	clog << "    no bars" << endl
		 << "<<< Histogram_Plot::draw" << endl;
	#endif
	return;
	}
bool
	horizontal = attribute_is_set (HORIZONTAL_BARS);
double
	base = Data->base (),
	increment = Data->increment ();
#if ((DEBUG_SECTION) & DEBUG_DRAW)
clog << "         base = " << base << endl
	 << "    increment = " << increment << endl;
#endif

/*	Bar rectangle.

	bottom -

		The position of the "bottom" edge - at data value zero - of
		all bars.

	last_edge -

		The position of the "leading" edge - starting with the base
		position - of the bar being pending painting. For a vertical bar
		this is the x position of the edge; for a horizontal bar this is
		the y position of the edge. This is not necessarily less than the
		next_edge, depending on the increment direction: a positive
		increment is rightwards for vertical bars or upwards for
		horizontal bars.

	last_top -

		The position of the "top" edge - as opposed to the bottom edge -
		of the bar pending painting. For a vertical bar this is the y
		position of the edge; for a horizontal bar this is the x position
		of the edge. 

	next_edge -

		The position of the "leading" edge of the next bar. This will be
		the "trailing" edge for the last bar. A bar that is eligible for
		painting (because its last_top is different from the next_top)
		must have a last_edge different from the next_edge; i.e. it must
		have a non-zero width. A zero width bar is "absorbed" into the
		next bar.

	next_top -

		The position of the "top" edge of the next bar. This will be the
		same as the last_top for the last bar. A bar is only eligible for
		painting if the next_top is different than the last_top; except
		for the last bar which is always eligible. This assumes that bars
		are contiguous - there is no painting gap between bars - so bars
		of the same length can be merged into a single bar for painting.

	last_length, next_length -

		The length of the last and next bars being compared. The length
		of a bar is the absolute value of the top and bottom edge
		positions. When a zero width bar is encountered its last_top set
		to the next_top if the next_length is greater than the
		last_length.

	The bar rectangle that is painted is always specified in terms of the
	top-left corner, in x,y raster coordinates, and the width and height
	of the bar. The height of a vertical bar, or the width of a
	horizontal bar, is increased by one pixel because the painter fill of
	the rectangle omits the bottom right edge. The width of a vertical
	bar is not expanded, nor the height of a horizontal bar, because a
	bar is inclusive of the leading edge but exclusive of the trailing;
	i.e. when transitioning from the last bar painted to the next bar for
	consideration the next_edge becomes the last_edge, which relies on
	the bottom right edge being excluded from bar rectangle painting. 
*/
QRect
	bar_rect;
int
	bottom,
	last_edge, last_top, last_length = 0,
	next_edge, next_top, next_length;

if (horizontal)
	{
	bottom = xMap.transform (0.0);
	last_edge = yMap.transform (base);
	last_top = xMap.transform (Data->y (0));
	}
else
	{
	bottom = yMap.transform (0.0);
	last_edge = xMap.transform (base);
	last_top = yMap.transform (Data->y (0));
	}
last_length = qAbs (last_top - bottom);
#if ((DEBUG_SECTION) & DEBUG_DRAW)
clog << "       bottom = " << bottom << endl
	 << "         bars = " << bars << endl;
#endif

for (int bar = 0;
		 bar <= bars;
		 bar++)
	{
	#if ((DEBUG_SECTION) & DEBUG_DRAW)
	clog << "    bar " << bar << " -" << endl
		 << "      value = " << Data->y (bar) << endl;
	#endif
	if (horizontal)
		{
		next_edge = yMap.transform (base);
		if (bar < bars)
			next_top = xMap.transform (Data->y (bar));
		else
			next_top = last_top;
		}
	else
		{
		next_edge = xMap.transform (base);
		if (bar < bars)
			next_top = yMap.transform (Data->y (bar));
		else
			next_top = last_top;
		}
	#if ((DEBUG_SECTION) & DEBUG_DRAW)
	clog << "      last = "
			<< last_edge << " edge, " << last_top << " top" << endl
		 << "      next = "
			<< next_edge << " edge, " << next_top << " top" << endl;
	#endif

	base += increment;
	if (last_top == next_top &&
		bar < bars)
		{
		#if ((DEBUG_SECTION) & DEBUG_DRAW)
		clog << "      no length change" << endl
			 << "        no bar drawn" << endl;
		#endif
		continue;
		}
	next_length = qAbs (next_top - bottom);

	if (last_edge == next_edge)
		{
		#if ((DEBUG_SECTION) & DEBUG_DRAW)
		clog << "      no bar width" << endl
			 << "      last length = " << last_length << endl
			 << "      next length = " << next_length << endl;
		#endif
		if (next_length > last_length)
			{
			//	The longest length of overlapping bars will be used.
			last_top = next_top;
			#if ((DEBUG_SECTION) & DEBUG_DRAW)
			clog << "      new last top = " << last_top << endl;
			#endif
			}
		if (bar < bars)
			{
			#if ((DEBUG_SECTION) & DEBUG_DRAW)
			clog << "        no bar drawn" << endl;
			#endif
			continue;
			}
		}

	if (horizontal)
		bar_rect.setRect
			(qMin (last_top,  bottom), qMin (next_edge,  last_edge),
			 qAbs (last_top - bottom) + 1, qAbs (next_edge - last_edge));
	else
		bar_rect.setRect
			(qMin (next_edge,  last_edge), qMin (last_top,  bottom),
			 qAbs (next_edge - last_edge), qAbs (last_top - bottom) + 1);
	#if ((DEBUG_SECTION) & DEBUG_DRAW)
	clog << "      rectangle = " << bar_rect << endl;
	#endif
	painter->fillRect (bar_rect, Bar_Color);

	last_edge = next_edge;
	last_top = next_top;
	last_length = next_length;
	}
#if ((DEBUG_SECTION) & DEBUG_DRAW)
clog << "<<< Histogram_Plot::draw" << endl;
#endif
}

}	//	namespace HiRISE
}	//	namespace UA
