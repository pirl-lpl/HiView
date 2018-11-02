/*	Distance_Line
 
 HiROC CVS ID: $Id: Distance_Line.cc,v 2.1 2013/10/14 18:31:42 stephens Exp $
 
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

#include "Distance_Line.hh"

#include <QPoint>
#include <QPainter>
#include <QtCore/qmath.h>
#include <QFont>

namespace UA
{
	namespace HiRISE
	{
		Distance_Line::Distance_Line(QWidget *parent)
			: QWidget(parent)
		{
			setAttribute(Qt::WA_TransparentForMouseEvents);
			Line_Style = QPen(Qt::red, 1);
			P1.setX(0);
			P1.setY(0);
			P2.setX(0);
			P2.setY(0);
		}
		
		void Distance_Line::update_rect() {
			Dx = (P1.x()-P2.x());
			Dy = (P1.y()-P2.y());
			if(Dx < 0)
				BottomRight_X = P2.x();
			else
				BottomRight_X = P1.x();
			if(Dy < 0)
				BottomRight_Y = P2.y();
			else
				BottomRight_Y = P1.y();
			Dx = qAbs(Dx);
			Dy = qAbs(Dy);
		}
		
		void Distance_Line::setP1(const QPoint & p) {
			P1 = p;
			update_rect();
		}
		
		void Distance_Line::setP2(const QPoint & p) {
			P2 = p;
			update_rect();
		}
		
		void Distance_Line::setText(QString str) {
			Line_Text = str;
		}
		
		void Distance_Line::setColor(const QColor & color) {
			Line_Style.setColor(color);
		}
		
		QPoint Distance_Line::p2() {
			return P2;
		}
		
		QPoint Distance_Line::p1() {
			return P1;
		}
		
		int Distance_Line::dx() {
			return Dx;
		}
		
		int Distance_Line::dy() {
			return Dy;
		}
		
		void Distance_Line::paintEvent(QPaintEvent *) {
			QPainter painter(this);
			//Antialiasing - renders lines so you dont see pixels, looks straight not jagged.
			painter.setRenderHint(QPainter::Antialiasing, true); 
			painter.setPen(Line_Style);
			QFont font;
			font.setPointSize(10);
			painter.setFont(font);
			
			//need to get text width and height to make sure it is in geomerty
			QFontMetrics metrics = painter.fontMetrics();
			int text_width = metrics.width(Line_Text);
			int text_height = metrics.height();
			
			int text_pos_x = (BottomRight_X - Dx/2 - text_width - 2 < 0) ? BottomRight_X - Dx/2 + 2 : BottomRight_X - Dx/2 - text_width - 2;
			int text_pos_y = (BottomRight_Y - Dy/2 - text_height - 2 < 0) ? BottomRight_Y - Dy/2 + text_height + 2 : BottomRight_Y - Dy/2 - 2;
			
			//if setGeometry falls below zero the line won't render again
			if(BottomRight_X > 0 && BottomRight_Y > 0) {
				//need small geometry offset, or there will be a small error rendering straight lines
				setGeometry(0, 0, BottomRight_X + text_width + 2, Dy + BottomRight_Y + text_height + 2);
				painter.drawLine(P1, P2);
				painter.drawText(text_pos_x, text_pos_y, Line_Text);
			}
			else {
				setVisible(false);
			}
			/**********debug************
			QFont font("Times", 10);
			
			painter.setFont(font);
			painter.setPen(QPen(Qt::blue,1));
			
			setGeometry(0,0,200,200);
			QString str = QString("(%1,%2)").arg(P1.x()).arg(P1.y());
			QString str1 = QString("(%1,%2)").arg(P2.x()).arg(P2.y());
			QString str2 = QString("(%1,%2)").arg(TopLeft_X).arg(TopLeft_Y);
			QString str3 = QString("(%1,%2)").arg(Dx).arg(Dy);
			
			painter.drawText(50, 50, str);
			painter.drawText(50, 60, str1);
			painter.drawText(50, 70, str2);
			painter.drawText(50, 80, str3);
			 
			painter.setPen(QPen(Qt::green,1));
			setGeometry(TopLeft_X, TopLeft_Y, Dx + 1, Dy + 1);
			QRect rect(0, 0, Dx, Dy);
			painter.drawRect(rect);
			***********end debug************/
			
		}
	}
}