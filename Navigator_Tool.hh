/*	Navigator_Tool

HiROC CVS ID: $Id: Navigator_Tool.hh,v 1.43 2012/06/15 01:16:07 castalia Exp $

Copyright (C) 2009-2011  Arizona Board of Regents on behalf of the
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

#ifndef HiView_Navigator_Tool_hh
#define HiView_Navigator_Tool_hh

#include	<QDockWidget>

#include	"Image_Viewer.hh"
#include	"Plastic_Image.hh"

class QWidget;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QComboBox;
class QErrorMessage;
class QSize;
class QPoint;
class QRubberBand;
class QResizeEvent;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Icon_Button;

/**	The <i>Navigator_Tool</i> for the HiView application.

	The Navigator_Tool provides an Image_Viewer pane with an (initially)
	scaled down display of the image to provide an overview of the image
	in the main display area. The overview image contains a rectangle
	locating the main display image region.

	Another pane provides text fields describing the image
	characteristics and display values. Those display values that can be
	changed by the user are present in user editable fields. The display
	values are constantly updated to remain in sync with the main and
	overview displays.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.43 $
*/
class Navigator_Tool
:	public QDockWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types:
*/
typedef Image_Viewer::Shared_Image	Shared_Image;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Minimum size of the image area.
static const QSize
	IMAGE_MIN_SIZE;

/*==============================================================================
	Class data members
*/
static bool
	Default_Immediate_Mode;

/*==============================================================================
	Constructors
*/
Navigator_Tool (QWidget* parent = NULL);

virtual ~Navigator_Tool ();

/*==============================================================================
	Accessors
*/
//	Ownership of the QImage is NOT transferred.
bool image (const Shared_Image& source_image, const QString& name = "");

void image_name (const QString& name);
QString image_name () const;

Image_Viewer* overview_image () const
	{return Image_View;}

static void default_show_all_region_origins (bool enabled)
	{Default_Show_All_Region_Origins = enabled;}
static void default_show_all_scalings (bool enabled)
	{Default_Show_All_Scalings = enabled;}
static void default_scaling_X_Y_distinct (bool enabled)
	{Default_Scaling_X_Y_Distinct = enabled;}
static void default_immediate_mode (bool enabled)
	{Default_Immediate_Mode = enabled;}
static bool default_immediate_mode ()
	{return Default_Immediate_Mode;}
void immediate_mode (bool enabled);
bool immediate_mode () const;

inline static QErrorMessage* error_message ()
	{return Error_Message;}

//	Ownership of the QErrorMesage is NOT transferred.
static void error_message (QErrorMessage* dialog);

/*==============================================================================
	GUI elements
*/
public:

virtual QSize minimumSizeHint () const;
virtual QSize sizeHint () const;

/**	Get the previous size of this Navigator_Tool before the last
	{@link resizeEvent(QResizeEvent*) resize event}.

	@return	A QSize for the previous size of this Navigator_Tool. This
		will be an invalid size if a previous size is not yet known.
*/
QSize previous_size () const
	{return Previous_Size;}


private:

QWidget* image_panel ();
QWidget* info_panel ();
void reset_info ();

/*==============================================================================
	Qt signals
*/
signals:

void region_moved (const QPoint& origin, int band);

void image_scaled (const QSizeF& scaling, const QPoint& center, int band);

void bands_mapped (const unsigned int* band_map);

void tool_context_menu_requested (QDockWidget* tool, QContextMenuEvent* event);

/*==============================================================================
	Qt slots
*/
public slots:

/*	The public slots are informative.

	They may change the state of various GUI components
	but these changes will not result in any signals being emitted.
*/
void refresh_band_numbers ();

void image_cursor_moved
	(const QPoint& display_position, const QPoint& image_position);

void image_pixel_value
	(const Plastic_Image::Triplet& display_pixel,
	 const Plastic_Image::Triplet& image_pixel);

void move_region (const QPoint& origin, int band = -1);
void show_all_region_origins (bool enabled);

void displayed_image_region_resized (const QSize& region_size);
void display_viewport_resized (const QSize& viewport_size);

void scale_image (const QSizeF& scaling, int band = -1);
void show_all_scalings (bool enabled);
void scaling_X_Y_distinct (bool enabled, int band = -1);


private slots:

void image_loaded (bool successful);

/**	Navigator image viewer cursor position.

	This slot receives the cursor position information from the local
	Image_Viewer and passes it on to the common image_cursor_moved slot
	for position reporting. Then it sets the local Image_Viewer cursor to
	the Shift_Region_Cursor if the cursor location is within the
	Region_Overlay; otherwise the default cursor is restored.

	@param	display_position	A QPoint containing the position of the
		cursor in the local Image_Viewer display viewport.
	@param	image_position		A QPoint containing the position of the
		cursor in the local Image_Viewer image. This will be -1,-1 if
		the cursor position is not within the displayed image region.
*/
void nav_image_cursor_moved
	(const QPoint& display_position, const QPoint& image_position);

/*	These private slots are declarative.

	They receive signals directly from the GUI components
	and will result in signals being emitted
	if any value changes have occured or are pending
	and the changes are not due to Received_Knowledge.
*/
void region_origin_changed ();

void image_scaling_changed ();

void band_mapping_changed ();
void band_map_reset ();

void apply ();

void apply_when_changed (int index);

void overview_image_moved (const QPoint& origin, int band);

void overview_image_scaled (const QSizeF& scaling, int band);

/*==============================================================================
	Event Handlers
*/
protected:

virtual void mousePressEvent (QMouseEvent* event);
virtual void mouseMoveEvent (QMouseEvent* event);
virtual void mouseReleaseEvent (QMouseEvent* event);
virtual void mouseDoubleClickEvent (QMouseEvent* event);

virtual void resizeEvent (QResizeEvent* event);

virtual void contextMenuEvent (QContextMenuEvent* event);

/*==============================================================================
	Helpers
*/
private:

void reset_region_overlay ();

/*==============================================================================
	Data
*/
private:

Image_Viewer
	*Image_View;

QLabel
	*Source_Name,
	*Source_Bands,
	*Source_Values,
	*Source_Value[3],
	*Display_Value[3],
	*Source_Size_X,
	*Source_Size_Y,
	*Display_Location_X,
	*Display_Location_Y,
	*Source_Location_X,
	*Source_Location_Y,
	*Region_Size_X,
	*Region_Size_Y,
	*Display_Size_X,
	*Display_Size_Y;
QSize
	Region_Size;

QSpinBox
	*Image_Band[3],
	*Region_Origin_X[3],
	*Region_Origin_Y[3];

Icon_Button
	*Band_Map_Reset_Button;
int
	Initial_Band_Map[3];
static bool
	Default_Show_All_Region_Origins;
bool
	Show_All_Region_Origins;

QDoubleSpinBox
	*Scaling_X[3],
	*Scaling_Y[3];
static bool
	Default_Show_All_Scalings,
	Default_Scaling_X_Y_Distinct;
bool
	Show_All_Scalings,
	Scaling_X_Y_Distinct[3];

QPushButton
	*Apply;
QComboBox
	*Apply_When;
unsigned int
	Changes_Pending;

/**	Flags information received from an external source
	that is not to result in a signal being emitted
	if it changes a current GUI component value.
*/
bool
	Received_Knowledge;

QRubberBand
	*Region_Overlay;
QPoint
	Region_Drag_Offset;

QSize
	Previous_Size;

static QErrorMessage
	*Error_Message;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
