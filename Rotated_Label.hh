/*	Rotated_Label

HiROC CVS ID: $Id: Rotated_Label.hh,v 1.1 2011/04/09 06:31:15 castalia Exp $

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

#ifndef HiView_Rotated_Label_hh
#define HiView_Rotated_Label_hh

#include <QFrame>


namespace UA
{
namespace HiRISE
{
/**	A <i>Rotated_Label</i> is a text label widget in which the text
	may be rotated in 90 degree increments.

	The Rotated_Label class is a simplification of the QxtLabel class
	implementation from the Qxt library (http://libqxt.org;
	foundation@libqxt.org).

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.1 $
*/
class Rotated_Label
:	public QFrame
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


enum Text_Rotation
	{
	NO_ROTATION			= 0,
	CLOCKWISE			= 90,
	INVERTED			= 180,
	COUNTER_CLOCKWISE	= 270
	};

//	Bit set in CLOCKWISE and COUNTER_CLOCKWISE but not NO_ROTATION and INVERTED.
#define IS_VERTICAL		2

/*==============================================================================
	Constructors
*/
explicit Rotated_Label
	(QWidget* parent = NULL, Qt::WindowFlags flags = 0);
explicit Rotated_Label
	(const QString& text, QWidget* parent = NULL, Qt::WindowFlags flags = 0);

virtual ~Rotated_Label ();

/*==============================================================================
	Accessors
*/
inline QString text () const
	{return Text;}

inline Qt::Alignment alignment () const
	{return Align;}
void alignment (Qt::Alignment alignment);

inline Text_Rotation text_rotation () const
	{return Rotation;}
void text_rotation (Text_Rotation rotation);

virtual QSize sizeHint() const;
virtual QSize minimumSizeHint() const;

/*==============================================================================
	Qt signals:
*/
signals:

void textChanged (const QString& text);

/*==============================================================================
	Qt slots
*/
public slots:

void text (const QString& text);

/*==============================================================================
	Event Handlers
*/
protected:

virtual void changeEvent (QEvent* event);
virtual void paintEvent (QPaintEvent* event);

/*==============================================================================
	Data
*/
private:

QString
	Text;

Qt::Alignment
	Align;

Text_Rotation
	Rotation;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
