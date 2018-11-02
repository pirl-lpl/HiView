/*	Statistics_Tools

HiROC CVS ID: $Id: Statistics_Tools.hh,v 1.4 2011/05/14 02:34:22 castalia Exp $

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

#ifndef HiView_Statistics_Tools_hh
#define HiView_Statistics_Tools_hh

#include	<QDockWidget>

//	Forward references.
class QWidget;
class QTabWidget;
class QResizeEvent;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Statistics_and_Bounds_Tool;
class Statistics_Tool;

/**	The <i>Statistics_Tools</i> for the HiView application.

	The Statistics_Tools provides a QDockWidget container for the
	Statistics_Tool and the Statistics_and_Bounds_Tool.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.4 $
*/
class Statistics_Tools
:	public QDockWidget
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


enum
	{
	SOURCE_STATISTICS_INDEX		= 0,
	DISPLAY_STATISTICS_INDEX	= 1
	};

/*==============================================================================
	Constructor
*/
explicit Statistics_Tools (QWidget* parent = NULL);

virtual ~Statistics_Tools ();

/*==============================================================================
	Accessors
*/
inline Statistics_and_Bounds_Tool* source_statistics () const
	{return Source_Statistics;}
inline Statistics_Tool* display_statistics () const
	{return Display_Statistics;}

inline QTabWidget* sections () const
	{return Sections;}

virtual QSize minimumSizeHint () const;
virtual QSize sizeHint () const;

/*	Get the previous size of the Statistics_Tools before the last
	{@link resizeEvent(QResizeEvent*) resize event}.

	@return	A QSize for the previous size of the Statistics_Tools. This
		will be an invalid size if a previous size is not yet known.
*/
QSize previous_size () const
	{return Previous_Size;}

/*==============================================================================
	Manipulators
*/
inline bool visible_graph () const
	{return Visible_Graph;}
bool visible_graph (bool enabled);

/*==============================================================================
	Qt signals:
*/
signals:

void section_changed (int index);

void tool_context_menu_requested (QDockWidget* tool, QContextMenuEvent* event);

/*==============================================================================
	Qt slots
*/
public slots:

void select_section (int index);
void select_section (QWidget* section);

/*==============================================================================
	Event Handlers
*/
protected:

virtual void resizeEvent (QResizeEvent* event);
virtual void contextMenuEvent (QContextMenuEvent* event);

/*==============================================================================
	Data
*/
private:

QTabWidget
	*Sections;

Statistics_and_Bounds_Tool
	*Source_Statistics;
Statistics_Tool
	*Display_Statistics;

//	Dynamic size control.
QSize
	Minimum_Size;
bool
	Visible_Graph;
int
	Graph_Width_Increment;
QSize
	Previous_Size;

};

}	//	namespace HiRISE
}	//	namespace UA
#endif
