/*	Distance_Line

HiROC CVS ID: $Id: Distance_Line.hh,v 2.1 2013/10/14 18:31:42 stephens Exp $

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

#ifndef HiView_Distance_Line_hh
#define HiView_Distance_Line_hh

#include	<QWidget>
#include	<QPoint>
#include	<QString>
#include	<QPen>


namespace UA
{
	namespace HiRISE
	{
		/*
			The Distance Line allows a line to be painted on top of a parent
			widget. It will hold 2 QPoints, which will represent a straight
			line, as well as a String field which will be displayed adjacent
			to the line. It is suggested that the string field be used to report
			the distance of the line. The functions are  named in accordance with
			the QLine function names.
		*/
		class Distance_Line : public QWidget
		{
			Q_OBJECT
			
			public:
			/***************************************************
			 *                 Constructors                    *
			 ***************************************************/
				Distance_Line(QWidget *parent);
				
			/***************************************************
			 *                    Setters                      *
			 ***************************************************/
				void setP1(const QPoint & p);
				void setP2(const QPoint & p);
				void setText(QString str);
				void setColor(const QColor & color);
				
			/***************************************************
			 *                   Accessors                     *
			 ***************************************************/
				QPoint p1();
				QPoint p2();
				int dx();
				int dy();
			/***************************************************
			 *         Overloaded Protected Functions          *
			 ***************************************************/	
			protected:
				virtual void paintEvent(QPaintEvent *);
				
			/***************************************************
			 *                    Helpers                      *
			 ***************************************************/
			private:
				void update_rect();
			
			/***************************************************
			 *                     Data                        *
			 ***************************************************/
				QString Line_Text;
				QPoint P1, P2;
				int Dx, Dy, BottomRight_X, BottomRight_Y;
				QPen Line_Style;
				
		};
	}
}

#endif