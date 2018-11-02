/*	Rotated_Label

HiROC CVS ID: $Id: Rotated_Label.cc,v 1.1 2011/04/09 06:31:15 castalia Exp $

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


The Rotated_Label class is a simplification of the QxtLabel class
implementation from the Qxt library (http://libqxt.org;
foundation@libqxt.org).

*******************************************************************************/

#include "Rotated_Label.hh"

#include <QFontMetrics>
#include <QPainter>
#include <QEvent>
#include <QApplication>


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Rotated_Label::ID =
		"UA::HiRISE::Rotated_Label ($Revision: 1.1 $ $Date: 2011/04/09 06:31:15 $)";

/*==============================================================================
	Constructors
*/
Rotated_Label::Rotated_Label
	(
	QWidget*		parent,
	Qt::WindowFlags	flags
	)
	:	QFrame (parent, flags),
		Align (Qt::AlignLeft | Qt::AlignVCenter),
		Rotation (NO_ROTATION)
{}


Rotated_Label::Rotated_Label
	(
	const QString&	text,
	QWidget*		parent,
	Qt::WindowFlags	flags
	)
	:	QFrame (parent, flags),
		Text (text),
		Align (Qt::AlignLeft | Qt::AlignVCenter),
		Rotation (NO_ROTATION)
{}


Rotated_Label::~Rotated_Label ()
{}


void
Rotated_Label::text
	(
	const QString&	text
	)
{
if (Text != text)
	{
	Text = text;
	updateGeometry ();
	update ();

	//	>>> SIGNAL <<<
	emit textChanged (text);
	}
}


void
Rotated_Label::alignment
	(
	Qt::Alignment	alignment
	)
{
if (Align != alignment)
	{
	Align = alignment;
	update(); //	No geometry change, update is sufficient.
	}
}


void
Rotated_Label::text_rotation
	(
	Text_Rotation	rotation
	)
{
if (Rotation != rotation)
	{
	Text_Rotation
		previous = Rotation;
	Rotation = rotation;

	switch (rotation)
		{
		case NO_ROTATION:
		case INVERTED:
			if (previous & IS_VERTICAL)
				updateGeometry ();
			break;
		case CLOCKWISE:
		case COUNTER_CLOCKWISE:
			if (! (previous & IS_VERTICAL))
				updateGeometry ();
			break;
		default:
			break;
		}
	update ();
	}
}


QSize
Rotated_Label::sizeHint () const
{
const QFontMetrics&
	font_metrics = fontMetrics ();
QSize
	size (font_metrics.width (Text), font_metrics.height ());
if (Rotation & IS_VERTICAL)
	size.transpose ();
return size;
}


QSize
Rotated_Label::minimumSizeHint () const
{return sizeHint ();}


void
Rotated_Label::paintEvent
	(
	QPaintEvent*	event
	)
{
QFrame::paintEvent (event);

QPainter
	painter (this);
painter.rotate (Rotation);
QRect
	rect = contentsRect ();
switch (Rotation)
	{
	case INVERTED:
		painter.translate (-rect.width(), -rect.height());
		break;
	case CLOCKWISE:
		painter.translate (0, -rect.width());
		break;
	case COUNTER_CLOCKWISE:
		painter.translate (-rect.height(), 0);
		break;
	default:
		break;
	}

if (Rotation & IS_VERTICAL)
	//	Swap width and height.
	rect.setRect (rect.x (), rect.y (), rect.height (), rect.width ());

painter.drawText (rect, Align, Text);
}


void
Rotated_Label::changeEvent
	(
	QEvent*		event
	)
{
QFrame::changeEvent (event);

switch (event->type ())
	{
	case QEvent::FontChange:
	case QEvent::ApplicationFontChange:
		updateGeometry ();
		update ();
		break;
	default:
		break;
	}
}


}	//	namespace HiRISE
}	//	namespace UA
