/*	Activity_Indicator

HiROC CVS ID: $Id: Activity_Indicator.hh,v 1.6 2011/11/27 00:06:03 castalia Exp $

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

#ifndef HiView_Activity_Indicator_hh
#define HiView_Activity_Indicator_hh

#include	<QWidget>
#include	<QMutex>

//	Forward references.
class	QSize;
class	QColor;
class	QPixmap;
class	QTimer;
class	QPaintEvent;
class	QMouseEvent;


namespace UA
{
namespace HiRISE
{

class Activity_Indicator
:	public QWidget
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


//!	State conditions.
enum
	{
	STATE_OFF,
	STATE_1,
	STATE_2,
	TOTAL_STATE_CONDITIONS
	};

/*==============================================================================
	Constructors
*/
explicit Activity_Indicator (int size = 0, QWidget* parent = NULL);
explicit Activity_Indicator (QWidget* parent);

~Activity_Indicator ();

/*==============================================================================
	Accessors
*/
static void default_indicator_size (int size);
static int default_indicator_size ()
	{return Default_Indicator_Size;}
Activity_Indicator& indicator_size (int size);
inline int indicator_size () const
	{return Indicator_Size;}
inline QSize button_size () const
{return QSize (Indicator_Size >> 1, Indicator_Size >> 1);}
virtual QSize sizeHint () const;

static void default_start_delay (int delay);
static int default_start_delay ()
	{return Default_Start_Delay;}
Activity_Indicator& start_delay (int delay);
inline int start_delay () const
	{return Start_Delay;}

static void default_update_interval (int interval);
static int default_update_interval ()
	{return Default_Update_Interval;}
Activity_Indicator& update_interval (int interval);
inline int update_interval () const
	{return Update_Interval;}

static void default_interval_degrees (int degrees);
static int default_interval_degrees ()
	{return Default_Interval_Degrees;}
Activity_Indicator& interval_degrees (int degrees);
inline int interval_degrees () const
	{return Interval_Degrees;}

static void default_state_color (int condition, QColor color);
static QColor default_state_color (int condition);
Activity_Indicator& state_color (int condition, QColor color);
QColor state_color (int condition) const;

inline int state () const
	{return State;}

/*==============================================================================
	Qt signals
*/
signals:

void state_changed
	(int old_condition, int new_condition);

void button_clicked (int condition);

/*==============================================================================
	Qt slots
*/
public slots:

void state (int condition);
void cancel ();


private slots:

void interval ();

/*==============================================================================
	Event Handlers
*/
protected:

virtual void paintEvent (QPaintEvent* event);
virtual void mousePressEvent (QMouseEvent* event);
virtual void mouseReleaseEvent (QMouseEvent* event);

/*==============================================================================
	Helpers
*/
protected:

QPixmap* indicator ();

void set_indicator_state
	(QPixmap* pixmap, int condition, bool pressed = false);


private:

void initialize ();

/*==============================================================================
	Data Members
*/
private:

static int
	Default_Indicator_Size,
	Default_Start_Delay,
	Default_Update_Interval,
	Default_Interval_Degrees;
int
	Indicator_Size,
	Start_Delay,
	Update_Interval,
	Interval_Degrees;

static QColor
	Default_State_Color[];
QColor
	*State_Color;

QPixmap
	*Indicator;
QTimer
	*Update_Timer;

int
	State;
int
	Progress;

QMutex
	State_Lock;

};

}	//	namespace HiRISE
}	//	namespace UA
#endif
