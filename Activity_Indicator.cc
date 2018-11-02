/*	Activity_Indicator

HiROC CVS ID: $Id: Activity_Indicator.cc,v 1.15 2012/03/09 02:13:55 castalia Exp $

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

#include	"Activity_Indicator.hh"

#include	<QPixmap>
#include	<QPointF>
#include	<QRectF>
#include	<QPainter>
#include	<QPen>
#include	<QConicalGradient>
#include	<QTimer>
#include	<QPaintEvent>
#include	<QMouseEvent>

#include	<stdexcept>
using std::invalid_argument;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF			0
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_INDICATOR		(1 << 2)
#define DEBUG_EVENTS		(1 << 3)
#define DEBUG_UPDATE		(1 << 4)
#define DEBUG_STATE			(1 << 5)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<string>
using std::string;
#include	<iostream>
using std::clog;
using std::boolalpha;
using std::hex;
using std::dec;

#endif	//	DEBUG_SECTION

namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Activity_Indicator::ID =
		"UA::HiRISE::Activity_Indicator ($Revision: 1.15 $ $Date: 2012/03/09 02:13:55 $)";

/*==============================================================================
	Defaults
*/
#ifndef PROGRESS_INDICATOR_MIN_SIZE
#define PROGRESS_INDICATOR_MIN_SIZE					15
#endif
#ifndef PROGRESS_INDICATOR_SIZE
#define PROGRESS_INDICATOR_SIZE						21
#endif
#ifndef PROGRESS_INDICATOR_START_DELAY
#define PROGRESS_INDICATOR_START_DELAY				50
#endif
#ifndef PROGRESS_INDICATOR_UPDATE_INTERVAL
#define PROGRESS_INDICATOR_UPDATE_INTERVAL			10
#endif
#ifndef PROGRESS_INDICATOR_INTERVAL_DEGREES	
#define PROGRESS_INDICATOR_INTERVAL_DEGREES			2
#endif
int
	Activity_Indicator::Default_Indicator_Size
		= PROGRESS_INDICATOR_SIZE,
	Activity_Indicator::Default_Start_Delay
		= PROGRESS_INDICATOR_START_DELAY,
	Activity_Indicator::Default_Update_Interval
		= PROGRESS_INDICATOR_UPDATE_INTERVAL,
	Activity_Indicator::Default_Interval_Degrees
		= PROGRESS_INDICATOR_INTERVAL_DEGREES;

#ifndef PROGRESS_INDICATOR_HEAD_COLOR
#define PROGRESS_INDICATOR_HEAD_COLOR		0xFFFFFF
#endif
#ifndef PROGRESS_INDICATOR_TAIL_COLOR
#define PROGRESS_INDICATOR_TAIL_COLOR		0x777777
#endif
#ifndef PROGRESS_INDICATOR_RIPPLES
#define PROGRESS_INDICATOR_RIPPLES			7
#endif

#ifndef PROGRESS_INDICATOR_OFF_COLOR
#define PROGRESS_INDICATOR_OFF_COLOR		0
#endif
#ifndef PROGRESS_INDICATOR_1_COLOR
#define PROGRESS_INDICATOR_1_COLOR			0xFF0000
#endif
#ifndef PROGRESS_INDICATOR_2_COLOR
#define PROGRESS_INDICATOR_2_COLOR			0x7777FF
#endif
QColor
	Activity_Indicator::Default_State_Color[TOTAL_STATE_CONDITIONS] =
		{
		QColor ((QRgb)PROGRESS_INDICATOR_OFF_COLOR),
		QColor ((QRgb)PROGRESS_INDICATOR_1_COLOR),
		QColor ((QRgb)PROGRESS_INDICATOR_2_COLOR)
		};

#define NO_PROGRESS	-999

/*==============================================================================
	Constructors
*/
Activity_Indicator::Activity_Indicator
	(
	int			indicator_size,
	QWidget*	parent
	)
	:	QWidget (parent),
		Indicator_Size (indicator_size < PROGRESS_INDICATOR_MIN_SIZE ?
			Default_Indicator_Size : indicator_size),
		Start_Delay (Default_Start_Delay),
		Update_Interval (Default_Update_Interval),
		Interval_Degrees (Default_Interval_Degrees),
		State (STATE_OFF),
		Progress (NO_PROGRESS),
		State_Lock ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Activity_Indicator: " << indicator_size << endl;
#endif
initialize ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Activity_Indicator" << endl;
#endif
}


Activity_Indicator::Activity_Indicator
	(
	QWidget*	parent
	)
	:	QWidget (parent),
		Indicator_Size (Default_Indicator_Size),
		Start_Delay (Default_Start_Delay),
		Update_Interval (Default_Update_Interval),
		Interval_Degrees (Default_Interval_Degrees),
		State (STATE_OFF),
		Progress (NO_PROGRESS)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Activity_Indicator: " << endl;
#endif
initialize ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Activity_Indicator" << endl;
#endif
}


void
Activity_Indicator::initialize ()
{
setSizePolicy (QSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed));

State_Color = new QColor[TOTAL_STATE_CONDITIONS];
for (int index = 0;
		 index < TOTAL_STATE_CONDITIONS;
		 index++)
	State_Color[index] = Default_State_Color[index];

Update_Timer = new QTimer (this);
Update_Timer->setSingleShot (true);
connect (Update_Timer,
	SIGNAL (timeout ()),
	SLOT (interval ()));

Indicator = indicator ();
}


Activity_Indicator::~Activity_Indicator ()
{
if (State_Color)
	delete[] State_Color;
if (Indicator)
	delete Indicator;
if (Update_Timer)
	delete Update_Timer;
}

/*==============================================================================
	Accessors
*/
void
Activity_Indicator::default_indicator_size
	(
	int		size
	)
{
if (size < PROGRESS_INDICATOR_MIN_SIZE)
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid default indicator size: " << size << endl
			<< "The minimum size is " << PROGRESS_INDICATOR_MIN_SIZE;
	throw invalid_argument (message.str ());
	}
Default_Indicator_Size = size;
}


Activity_Indicator&
Activity_Indicator::indicator_size
	(
	int		size
	)
{
if (size < PROGRESS_INDICATOR_MIN_SIZE)
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid indicator size: " << size << endl
			<< "The minimum size is " << PROGRESS_INDICATOR_MIN_SIZE;
	throw invalid_argument (message.str ());
	}
if (size != Indicator_Size)
	{
	State_Lock.lock ();
	Indicator_Size = size;
	delete Indicator;
	Indicator = indicator ();
	State_Lock.unlock ();

	updateGeometry ();
	}
return *this;
}


QSize
Activity_Indicator::sizeHint () const
{return QSize (Indicator_Size, Indicator_Size);}


void
Activity_Indicator::default_start_delay
	(
	int		delay
	)
{
if ((Default_Start_Delay = delay) < 0)
	 Default_Start_Delay = 0;
}


Activity_Indicator&
Activity_Indicator::start_delay
	(
	int		delay
	)
{
State_Lock.lock ();
if ((Start_Delay = delay) < 0)
	 Start_Delay = 0;
State_Lock.unlock ();
return *this;
}


void
Activity_Indicator::default_update_interval
	(
	int		interval
	)
{
if ((Default_Update_Interval = interval) < 0)
	 Default_Update_Interval = 0;
}


Activity_Indicator&
Activity_Indicator::update_interval
	(
	int		interval
	)
{
State_Lock.lock ();
if ((Update_Interval = interval) < 0)
	 Update_Interval = 0;
if (State > STATE_OFF &&
	Update_Timer->isActive ())
	{
	Update_Timer->stop ();
	Update_Timer->setInterval (Update_Interval);
	Update_Timer->start ();
	}
State_Lock.unlock ();
return *this;
}


void
Activity_Indicator::default_interval_degrees
	(
	int		degrees
	)
{
if ((Default_Interval_Degrees = degrees) < 1)
	 Default_Interval_Degrees = 1;
}


Activity_Indicator&
Activity_Indicator::interval_degrees
	(
	int		degrees
	)
{
State_Lock.lock ();
if ((Interval_Degrees = degrees) < 1)
	 Interval_Degrees = 1;
State_Lock.unlock ();
return *this;
}


void
Activity_Indicator::default_state_color
	(
	int		condition,
	QColor	color
	)
{
if (condition >= 0 &&
	condition < TOTAL_STATE_CONDITIONS)
	Default_State_Color[condition] = color;
else
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid default state color selection: " << condition << endl
			<< "The maximum state selection is "
				<< (TOTAL_STATE_CONDITIONS - 1);
	throw invalid_argument (message.str ());
	}
}


 
QColor
Activity_Indicator::default_state_color
	(
	int		condition
	)
{
if (condition >= 0 &&
	condition < TOTAL_STATE_CONDITIONS)
	return Default_State_Color[condition];

ostringstream
	message;
message << ID << endl
		<< "Invalid default state color selection: " << condition << endl
		<< "The maximum state selection is "
			<< (TOTAL_STATE_CONDITIONS - 1);
throw invalid_argument (message.str ());
}


Activity_Indicator&
Activity_Indicator::state_color
	(
	int		condition,
	QColor	color
	)
{
#if ((DEBUG_SECTION) & DEBUG_STATE)
clog << ">>> Activity_Indicator::state_color: " << condition
		<< " color 0x" << hex << color.rgba () << dec << endl;
#endif
if (condition < 0 ||
	condition >= TOTAL_STATE_CONDITIONS)
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid state color selection: " << condition << endl
			<< "The maximum state selection is "
				<< (TOTAL_STATE_CONDITIONS - 1);
	throw invalid_argument (message.str ());
	}
if (State_Color[condition] != color)
	{
	State_Lock.lock ();
	State_Color[condition] = color;
	State_Lock.unlock ();
	if (State == condition)
		{
		set_indicator_state (Indicator, State, false);
		update ();
		}
	}
#if ((DEBUG_SECTION) & DEBUG_STATE)
clog << "<<< Activity_Indicator::state_color" << endl;
#endif
return *this;
}


QColor
Activity_Indicator::state_color
	(
	int		condition
	) const
{
if (condition >= 0 &&
	condition < TOTAL_STATE_CONDITIONS)
	return State_Color[condition];

ostringstream
	message;
message << ID << endl
		<< "Invalid state color selection: " << condition << endl
		<< "The maximum state selection is "
			<< (TOTAL_STATE_CONDITIONS - 1);
throw invalid_argument (message.str ());
}

/*==============================================================================
	Qt slots
*/
void
Activity_Indicator::state
	(
	int		condition
	)
{
#if ((DEBUG_SECTION) & DEBUG_STATE)
clog << ">>> Activity_Indicator::state: " << condition << endl;
#endif
if (condition < 0)
	condition = 0;
else
if (condition >= TOTAL_STATE_CONDITIONS)
	condition  = TOTAL_STATE_CONDITIONS - 1;
if (condition != State)
	{
	int
		old_condition = State;
	State = condition;
	#if ((DEBUG_SECTION) & DEBUG_STATE)
	clog << "    old state " << old_condition << endl;
	#endif
	//	>>> SIGNAL <<<
	emit state_changed (old_condition, condition);

	if (condition == STATE_OFF)
		{
		//	Stop the indicator.
		#if ((DEBUG_SECTION) & DEBUG_STATE)
		clog << "    stop update timer" << endl;
		#endif
		Update_Timer->stop ();
		Progress = NO_PROGRESS;
		update ();
		}
	else
		{
		//	Start the indicator.
		set_indicator_state (Indicator, condition);
		if (old_condition == STATE_OFF)
			{
			#if ((DEBUG_SECTION) & DEBUG_STATE)
			clog << "    start update timer with delay " << Start_Delay << endl;
			#endif
			if (Start_Delay)
				{
				Update_Timer->setSingleShot (true);
				Update_Timer->start (Start_Delay);
				}
			else
				//	Begin without delay.
				interval ();
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_STATE)
clog << "<<< Activity_Indicator::state" << endl;
#endif
}


void
Activity_Indicator::cancel ()
{state (STATE_OFF);}


void
Activity_Indicator::interval ()
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">>> Activity_Indicator::interval" << endl;
#endif
State_Lock.lock ();
int
	interval_degrees = Interval_Degrees,
	update_interval = Update_Interval;
State_Lock.unlock ();

if (Progress == NO_PROGRESS)
	{
	//	First startup delay interval completed.
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "    NO_PROGRESS - starting timer with update interval "
			<< update_interval << endl;
	#endif
	Progress = -interval_degrees;
	Update_Timer->setSingleShot (false);
	Update_Timer->setInterval (update_interval);
	Update_Timer->start ();
	}
else
	{
	//	In progress interval.
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "    progress = " << Progress << endl;
	#endif
	if ((Progress += interval_degrees) >= 360)
		Progress = 0;
	update ();
	}
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "<<< Activity_Indicator::interval" << endl;
#endif
}

/*==============================================================================
	Event Handlers
*/
void
Activity_Indicator::paintEvent
	(
	QPaintEvent*
	)
{
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_EVENTS))
clog << ">>> Activity_Indicator::paintEvent" << endl;
#endif
if (Progress < 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_EVENTS))
	clog << "    No progress" << endl
		 << "<<< Activity_Indicator::paintEvent" << endl;
	#endif
	return;
	}

QPainter
	painter (this);
painter.setViewport
	(0, 0, Indicator_Size, Indicator_Size);
int
	origin = -Indicator_Size >> 1;
painter.setWindow
	(origin, origin, Indicator_Size, Indicator_Size);
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_EVENTS))
clog << "    window = "
		<< origin << "x, " << origin << "y, "
		<< Indicator_Size << "w, " << Indicator_Size << "h " << endl;
#endif
if (Progress > 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_EVENTS))
	clog << "       rotate = " << Progress << endl;
	#endif
	painter.rotate (Progress);
	}

//	Guard the Inidicator from changes.
State_Lock.lock ();
painter.drawPixmap (origin, origin, *Indicator);
State_Lock.unlock ();
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_EVENTS))
clog << "    pixmap drawn at origin "
		<< origin << "x, " << origin << "y" << endl
	 << "<<< Activity_Indicator::paintEvent" << endl;
#endif
}


void
Activity_Indicator::mousePressEvent
	(
	QMouseEvent*	event
	)
{
if (State == STATE_OFF)
	return;

#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">>> Activity_Indicator::mousePressEvent: " << event->pos () << endl;
#endif
int
	diameter = Indicator_Size >> 1,
	radius = diameter >> 1;
if (QRect (radius, radius, diameter, diameter).contains (event->pos ()))
	{
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "    indicator button pressed" << endl;
	#endif
	set_indicator_state (Indicator, State, true);
	update ();
	}
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "<<< Activity_Indicator::mousePressEvent" << endl;
#endif
}


void
Activity_Indicator::mouseReleaseEvent
	(
	QMouseEvent*	event
	)
{
if (State == STATE_OFF)
	return;

#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">>> Activity_Indicator::mouseReleaseEvent: " << event->pos () << endl;
#endif
set_indicator_state (Indicator, State, false);
update ();
int
	diameter = Indicator_Size >> 1,
	radius = diameter >> 1;
if (QRect (radius, radius, diameter, diameter).contains (event->pos ()))
	{
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	clog << "    indicator button released" << endl;
	#endif
	//	>>> SIGNAL <<<
	emit button_clicked (State);
	}
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << "<<< Activity_Indicator::mouseReleaseEvent" << endl;
#endif
}

/*==============================================================================
	Indicator
*/
QPixmap*
Activity_Indicator::indicator ()
{
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << ">>> Activity_Indicator::indicator" << endl
	 << "    Indicator_Size = " << Indicator_Size << endl;
#endif
QPixmap
	*pixmap = new QPixmap (Indicator_Size, Indicator_Size);
pixmap->fill (Qt::transparent);

QPainter
	painter (pixmap);
painter.setRenderHint (QPainter::Antialiasing, true);
painter.setPen (QPen (Qt::transparent));
QPointF
	origin (Indicator_Size / 2.0, Indicator_Size / 2.0);
QConicalGradient
	conical_gradient (origin, -90.0);
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "    " << PROGRESS_INDICATOR_RIPPLES << " ripples" << endl;
#endif
if (PROGRESS_INDICATOR_RIPPLES > 0)
	{
	double
		increment = 1.0 / (PROGRESS_INDICATOR_RIPPLES << 1),
		stop_point = 0.0;
	#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
	clog << "      increment = " << increment << endl;
	#endif
	while (true)
		{
		conical_gradient.setColorAt (stop_point,
			QColor ((QRgb)PROGRESS_INDICATOR_HEAD_COLOR));
		if ((stop_point += increment) > 1.0)
			break;
		conical_gradient.setColorAt (stop_point,
			QColor ((QRgb)PROGRESS_INDICATOR_TAIL_COLOR));
		if ((stop_point += increment) > 1.0)
			break;
		}
	conical_gradient.setColorAt (1.0,
		QColor ((QRgb)PROGRESS_INDICATOR_HEAD_COLOR));
	}
else
	{
	conical_gradient.setColorAt (0.0,
		QColor ((QRgb)PROGRESS_INDICATOR_HEAD_COLOR));
	conical_gradient.setColorAt (1.0,
		QColor ((QRgb)PROGRESS_INDICATOR_TAIL_COLOR));
	}
painter.setBrush (conical_gradient);
QRectF
	region (1.0, 1.0, Indicator_Size - 2, Indicator_Size - 2);
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "    indicator region = " << region << endl;
#endif
painter.drawEllipse (region);

#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "<<< Activity_Indicator::indicator" << endl;
#endif
return pixmap;
}


void
Activity_Indicator::set_indicator_state
	(
	QPixmap*	pixmap,
	int			condition,
	bool		pressed
	)
{
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << ">>> Activity_Indicator::set_indicator_state: "
		<< condition << ", pressed = " << boolalpha << pressed << endl;
#endif
State_Lock.lock ();

if (condition < 0)
	condition = 0;
else
if (condition >= TOTAL_STATE_CONDITIONS)
	condition =  TOTAL_STATE_CONDITIONS - 1;

QPainter
	painter (pixmap);
painter.setRenderHint (QPainter::Antialiasing, true);
painter.setPen (QPen (Qt::transparent));
painter.setBrush (Qt::white);
int
	side = qMin (pixmap->width (), pixmap->height ());
//	Intended state circle expanded by one pixel.
double
	diameter = (side / 2.0) + 2.0;
QRectF
	region
		((pixmap->width ()  - diameter) / 2.0,
		 (pixmap->height () - diameter) / 2.0,
		 diameter, diameter);
//	Clear the existing state circle.
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "      pixmap size = " << pixmap->size () << endl
	 << "     clear circle = " << region << endl;
#endif
painter.drawEllipse (region);

QColor
	color = State_Color[condition];
if (pressed)
	color = color.lighter ();
painter.setBrush (color);
//	Intended size.
diameter -= 2.0;
region.setRect
	((pixmap->width ()  - diameter) / 2.0,
	 (pixmap->height () - diameter) / 2.0,
	 diameter, diameter);
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "    state circle = " << region
		<< ", color 0x" << hex << color.rgba () << dec << endl;
#endif
painter.drawEllipse (region);

State_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_INDICATOR)
clog << "<<< Activity_Indicator::set_indicator_state" << endl;
#endif
}


}	//	namespace HiRISE
}	//	namespace UA
