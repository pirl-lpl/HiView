/*	Drawn_Line

HiROC CVS ID: $Id: Drawn_Line.hh,v 1.1 2011/03/13 21:14:19 castalia Exp $

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

#ifndef HiView_Drawn_Line_hh
#define HiView_Drawn_Line_hh

#include	<QWidget>
#include	<QBrush>


namespace UA
{
namespace HiRISE
{
class Drawn_Line
:	public QWidget
{
public:
/*==============================================================================
	Constructor
*/
Drawn_Line (int weight = 1, QWidget* parent = NULL);

Drawn_Line& operator= (const Drawn_Line& drawn_line);

virtual ~Drawn_Line ();

/*==============================================================================
	Accessors
*/
Drawn_Line& weight (int weight);
inline int weight () const
	{return Weight;}

Drawn_Line& orientation (Qt::Orientation orient);
inline Qt::Orientation orientation () const
	{return Orientation;}

Drawn_Line& alignment (Qt::Alignment align);
inline Qt::Alignment alignment () const
	{return Alignment;}

Drawn_Line& brush (const QBrush& brush);
inline QBrush brush () const
	{return Brush;}

/*==============================================================================
	Events
*/
virtual void paintEvent (QPaintEvent*);

/*==============================================================================
	Data
*/
private:

int
	Weight;

Qt::Orientation
	Orientation;

Qt::Alignment
	Alignment;

QBrush
	Brush;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
