/*	Drawn_Line

HiROC CVS ID: $Id: Drawn_Line.cc,v 1.1 2011/03/13 21:14:19 castalia Exp $

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

#include	"Drawn_Line.hh"

#include	<QPainter>


namespace UA
{
namespace HiRISE
{
Drawn_Line::Drawn_Line
	(
	int			weight,
	QWidget*	parent
	)
	:	QWidget (parent),
		Weight (weight),
		Orientation (Qt::Horizontal),
		Alignment (Qt::AlignCenter),
		Brush (palette ().windowText ())
{
if (Weight < 0)
	Weight = 0;
setMinimumHeight (Weight);
}


Drawn_Line&
Drawn_Line::operator=
	(
	const Drawn_Line&	drawn_line
	)
{
if (this != &drawn_line)
	{
	Weight		= drawn_line.Weight;
	Orientation	= drawn_line.Orientation;
	Alignment	= drawn_line.Alignment;
	Brush		= drawn_line.Brush;

	if (Orientation == Qt::Horizontal)
		setMinimumHeight (Weight);
	else
		setMinimumWidth (Weight);
	}
return *this;
}


Drawn_Line::~Drawn_Line ()
{}


Drawn_Line&
Drawn_Line::weight
	(
	int		weight
	)
{
if (weight < 0)
	weight = 0;
if (Weight != weight)
	{
	Weight = weight;
	if (Orientation == Qt::Horizontal)
		setMinimumHeight (Weight);
	else
		setMinimumWidth (Weight);
	update ();
	}
return *this;
}


Drawn_Line&
Drawn_Line::orientation
	(
	Qt::Orientation	orient
	)
{
if (Orientation != orient)
	{
	Orientation = orient;
	if (Orientation == Qt::Horizontal)
		setMinimumHeight (Weight);
	else
		setMinimumWidth (Weight);
	update ();
	}
return *this;
}


Drawn_Line&
Drawn_Line::alignment
	(
	Qt::Alignment	align
	)
{
if (Alignment != align)
	{
	Alignment = align;
	update ();
	}
return *this;
}


Drawn_Line&
Drawn_Line::brush
	(
	const QBrush&	brush
	)
{
if (Brush != brush)
	{
	Brush = brush;
	update ();
	}
return *this;
}


void
Drawn_Line::paintEvent
	(
	QPaintEvent*
	)
{
if (Weight)
	{
	QSize
		extent (size ());
	if (! extent.isEmpty ())
		{
		QRect
			rectangle;
		int
			alignment;
		if (Orientation == Qt::Horizontal)
			{
			if (Alignment & Qt::AlignTop)
				alignment = 0;
			else
				{
				alignment = extent.height () - Weight;
				if (! (Alignment & Qt::AlignBottom))
					alignment >>= 1;
				}
			rectangle.setRect (0, alignment, extent.width (), Weight);
			}
		else
			{
			if (Alignment & Qt::AlignLeft)
				alignment = 0;
			else
				{
				alignment = extent.width () - Weight;
				if (! (Alignment & Qt::AlignRight))
					alignment >>= 1;
				}
			rectangle.setRect (alignment, 0, Weight, extent.height ());
			}
		QPainter
			painter (this);
		painter.fillRect (rectangle, Brush);
		}
	}
}


}	//	namespace HiRISE
}	//	namespace UA
