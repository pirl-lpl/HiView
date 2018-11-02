/*	Statistics_Tools

HiROC CVS ID: $Id: Statistics_Tools.cc,v 1.10 2012/03/09 02:13:58 castalia Exp $

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

#include	"Statistics_Tools.hh"
#include	"Statistics_Tool.hh"
#include	"Statistics_and_Bounds_Tool.hh"

//	Qt
#include	<QWidget>
#include	<QTabWidget>
#include	<QHBoxLayout>
#include	<QResizeEvent>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_LAYOUT			(1 << 2)

#define DEBUG_DEFAULT			DEBUG_OFF

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Statistics_Tools::ID =
		"UA::HiRISE::Statistics_Tools ($Revision: 1.10 $ $Date: 2012/03/09 02:13:58 $)";

/*==============================================================================
	Defaults
*/
#ifndef DEFAULT_STATISTICS_GRAPHS_VISIBLE
#define DEFAULT_STATISTICS_GRAPHS_VISIBLE	true
#endif

/*==============================================================================
	Constructor
*/
Statistics_Tools::Statistics_Tools
	(
	QWidget*	parent
	)
	:	QDockWidget (tr ("Statistics"), parent),
		Visible_Graph (true)
{
setObjectName ("Statistics_Tools");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << ">>> Statistics_Tools: " << object_pathname (this) << endl;
#endif
setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

//	Enable keyboard focus on the tool.
setFocusPolicy (Qt::StrongFocus);

//	Tabbed sections.
Sections = new QTabWidget (this);
Sections->setObjectName (windowTitle () + " Tabs");
Sections->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
Sections->setTabShape (QTabWidget::Rounded);
Sections->setTabPosition (QTabWidget::South);
Sections->setTabsClosable (false);
QWidget
	*section;
QHBoxLayout
	*layout;

//	SOURCE_STATISTICS_INDEX (0)
Source_Statistics = new Statistics_and_Bounds_Tool (tr ("Source Data"));
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "     Source_Statistics sizeHint = "
		<< Source_Statistics->sizeHint () << endl;
#endif
section = new QWidget;
layout = new QHBoxLayout (section);
section->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
layout->addWidget (Source_Statistics);
Sections->addTab (section, Source_Statistics->windowTitle ());

//	DISPLAY_STATISTICS_INDEX (1)
Display_Statistics = new Statistics_Tool (tr ("Display Data"));
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "    Display_Statistics sizeHint = "
		<< Display_Statistics->sizeHint () << endl;
#endif
Display_Statistics->data_structure (3, 8);	//	bands, precision.
section = new QWidget;
layout = new QHBoxLayout (section);
section->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
layout->addWidget (Display_Statistics);
Sections->addTab (section, Display_Statistics->windowTitle ());
Sections->setCurrentIndex (SOURCE_STATISTICS_INDEX);
connect (Sections,
	SIGNAL (currentChanged (int)),
	SIGNAL (section_changed (int)));

setWidget (Sections);

//	Dynamic size control.
Minimum_Size = QDockWidget::sizeHint ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "              tool Minimum_Size = " << Minimum_Size << endl;
#endif
//	Incremental width to subtract/add when graph area is hidden/visible.
Graph_Width_Increment = Statistics_Tool::GRAPH_MIN_SIZE.width () + 5;
visible_graph (DEFAULT_STATISTICS_GRAPHS_VISIBLE);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
clog << "          adj tool Minimum_Size = " << Minimum_Size << endl
	 << "<<< Statistics_Tools" << endl;
#endif
}


Statistics_Tools::~Statistics_Tools ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< ~Statistics_Tools: @ " << (void*)this << endl;
#endif
}

/*==============================================================================
	Accessors
*/
QSize
Statistics_Tools::minimumSizeHint () const
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">-< Statistics_Tools::minimumSizeHint: " << Minimum_Size << endl;
#endif
return Minimum_Size;
}


QSize
Statistics_Tools::sizeHint () const
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">-< Statistics_Tools::sizeHint: " << Minimum_Size << endl;
#endif
return Minimum_Size;
}

/*==============================================================================
	Event Handlers
*/
void
Statistics_Tools::resizeEvent
	(
	QResizeEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
clog << ">-< Statistics_Tools::resizeEvent: from "
		<< event->oldSize () << " to " << event->size () << endl;
#endif
Previous_Size = event->oldSize ();
QDockWidget::resizeEvent (event);
}


void
Statistics_Tools::contextMenuEvent
	(
	QContextMenuEvent* event
	)
{
//	>>> SIGNAL <<<
emit tool_context_menu_requested (this, event);
}

/*==============================================================================
	Manipulators
*/
bool
Statistics_Tools::visible_graph
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">>> Statistics_Tools::visible_graph: " << boolalpha << enabled << endl
	 << "     Source_Statistics visible_graph = "
	 	<< Source_Statistics->visible_graph () << endl
	 << "    Display_Statistics visible_graph = "
	 	<< Display_Statistics->visible_graph () << endl
	 << "                       Visible_Graph = " << Visible_Graph << endl;
#endif
bool
	visible = Visible_Graph;
Source_Statistics->visible_graph (enabled);
Display_Statistics->visible_graph (enabled);
Visible_Graph = enabled;
if (visible != Visible_Graph)
	{
	Visible_Graph = enabled;
	if (Visible_Graph)
		Minimum_Size.rwidth () += Graph_Width_Increment;
	else
		Minimum_Size.rwidth () -= Graph_Width_Increment;
	#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
	clog << "    Minimum_Size = " << Minimum_Size << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << "<<< Statistics_Tools::visible_graph: " << visible << endl;
#endif
return visible;
}

/*==============================================================================
	Slots
*/
void
Statistics_Tools::select_section
	(
	int		index
	)
{Sections->setCurrentIndex (index);}


void
Statistics_Tools::select_section
	(
	QWidget*	section
	)
{Sections->setCurrentWidget (section);}


}	//	namespace HiRISE
}	//	namespace UA
