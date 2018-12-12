/*	HiView_Window

HiROC CVS ID: $Id: HiView_Window.hh,v 1.90 2013/10/14 18:31:42 stephens Exp $

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

#ifndef HiView_Window_hh
#define HiView_Window_hh

#include	<QMainWindow>

#include	"Data_Mapper_Tool.hh"
#include	"Image_Viewer.hh"
#include	"Distance_Line.hh"

#ifdef __APPLE__
#include "Mac_Voice_Adapter.hh"
#endif

//	Forward references.
class QAction;
class QMenu;
class QComboBox;
class QFileDialog;
class QRubberBand;
class QDockWidget;
class QUrl;
class QNetworkAccessManager;
class QNetworkReply;
class QErrorMessage;
class QResizeEvent;
class QPaintEvent;
class QCloseEvent;
//class QPainter;

namespace idaeim::PVL {
class Aggregate;
}


namespace UA::HiRISE
{
//	Forward references.
class About_HiView_Dialog;
class Preferences_Dialog;
class Activity_Indicator;
class Image_Info_Panel;
class Location_Mapper;
class Metadata_Dialog;
class PDS_Metadata;
class Image_Viewer;
class Navigator_Tool;
class Statistics_Tools;
class Save_Image_Dialog;
class Save_Image_Thread;
class Plastic_Image;

/**	The <i>HiView_Window</i> implements the QMainWindow for the HiView
	application.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.90 $
*/
class HiView_Window
:	public QMainWindow
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types:
*/
typedef Image_Viewer::Shared_Image	Shared_Image;
typedef Data_Mapper_Tool::Data_Map	Data_Map;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

enum Layout_Restoration
	{
	DO_NOT_RESTORE_LAYOUT,
	RESTORE_LAYOUT,
	PREFERENCES_RESTORE_LAYOUT
	};

/*==============================================================================
	Constructors
*/
HiView_Window (const QString& source_name, const QSizeF& scaling = QSizeF (),
	Layout_Restoration restore_layout = PREFERENCES_RESTORE_LAYOUT,
	QWidget* parent = NULL, Qt::WindowFlags flags = 0);

virtual ~HiView_Window ();

/*==============================================================================
	Accessors
*/
public:

QString source_name ()
	{return Source_Name;}

/**	Get the selected image source region.

	If a user selected region of the image display has been determined
	that is returned. If image region selection is in progress then
	an empty rectangle is returned. Otherwise the entire image display
	viewport region is returned.

	@return	A QRect specifiying the effective selected image region in
		source image coordinates.
	@see	selected_display_region() const
*/
QRect selected_source_region () const;

/**	Get the selected image display region.

	If a user selected region of the image display has been determined
	that is returned. If image region selection is in progress then
	an empty rectangle is returned. Otherwise the entire image display
	viewport region is returned.

	@return	A QRect specifiying the effective selected image region in
		display coordinates.
	@see	selected_source_region() const
*/
QRect selected_display_region () const;

/**	Test if a region of the image has been selected.

	@return	true if a region of the image has been selected; false otherwise.
	@see	selected_source_region() const
	@see	selected_display_region() const
*/
inline bool has_selected_region () const
	{return ! Selected_Image_Region.isEmpty () || Selection_Modification;}


protected:

/**	Initiate loading of a named image source with an intitial scaling.

	If the initial scaling is invalid  - i.e. either the
	height or width is nagative - and the Preferences initial scale is
	not INITIAL_SCALE_AUTO_FIT, then the Preferences scaling is used. If
	the scaling remains invalid then the source will be {@link
	image_load(const QString&, const QSize&) loaded to fit the display
	viewport}.

	@param	source_name	A QString that names the image source, which may be
		a file pathname or JPIP URL. If this is empty nothing is done.
	@param	scaling	The desired initial scaling to be applied to the image.
		If this is invalid, and valid scaling is not provided to use by the
		Preferences initial scale, then the image will be fit to the
		display viewport.
	@return	true if the load request to the Image_Viewer was accepted;
		false otherwise.
*/
bool load_image (const QString& source_name, const QSizeF& scaling = QSizeF ());

/**	Initiate loading of a named image source fit to a display size.

	@param	source_name	A QString that names the image source, which may be
		a file pathname or JPIP URL. If this is empty nothing is done.
	@param	display_size	The desired size, in screen pixels, of the
		displayed image. If this is an invalid size - i.e. either the
		height or width is nagative - then the image size will be fit to
		the current image display viewport (within contraints applied by
		the Image_Viewer).
	@return	true if the load request to the Image_Viewer was accepted;
		false otherwise.
*/
bool load_image (const QString& source_name, const QSize& display_size);

/**	Initiate loading of an in-memory image with an initial scaling.

	If the initial scaling is invalid  - i.e. either the
	height or width is nagative - and the Preferences initial scale is
	not INITIAL_SCALE_AUTO_FIT, then the Preferences scaling is used. If
	the scaling remains invalid then the source will be {@link
	image_load(const QImage&, const QSize&) loaded to fit the display
	viewport}.

	@param	image	A QImage to be loaded into the display.
	@param	scaling	The desired initial scaling to be applied to the image.
		If this is invalid, and valid scaling is not provided to use by the
		Preferences initial scale, then the image will be fit to the
		display viewport.
	@return	true if the load request to the Image_Viewer was accepted;
		false otherwise.
	@see	load_image(const QString&, const QSizeF&)
*/
bool load_image (const QImage& image, const QSizeF& scaling = QSizeF ());

/**	Initiate loading of an in-memory image fit to a display size.

	@param	image	A QImage to be loaded into the display.
	@param	display_size	The desired size, in screen pixels, of the
		displayed image. If this is an invalid size - i.e. either the
		height or width is nagative - then the image size will be fit to
		the current image display viewport (within contraints applied by
		the Image_Viewer).
	@return	true if the load request to the Image_Viewer was accepted;
		false otherwise.
*/
bool load_image (const QImage& image, const QSize& display_size);

/**	Initiate loading of an image from an HTTP URL.

	An asynchronous fetch of the source into an in-memory image via a
	QNetworkReply is intiated. If this completes successfully the
	resulting image is {@link load_image(const QImage&, const QSizeF&)
	loaded} with default (auto-fit) scaling.

	@param	source_name	A QString that provides an HTTP URL. It is presumed
		that the URL has already been vetted as not for a JP2 source.
*/
void load_URL (const QString& source_name);

/**	Get the image rendering status.

	@return An {@link Image_Viewer::rendering_status() image rendering
		status code}.
*/
inline int rendering_status () const
	{return Image_View->rendering_status ();}

/*==============================================================================
	GUI elements
*/
protected:

void create_menus ();
void update_window_fit_action ();

void create_source_selections ();
void display_image_name (const QString& source_name);
void add_source_selection (const QString& name);
void remove_source_selection (const QString& name);

void create_image_viewer ();

void create_navigator ();

void create_statistics_panels ();

void create_data_mapper ();

void create_toolbar ();

/*==============================================================================
	Event Handlers
*/
virtual void resizeEvent (QResizeEvent* event);

void dragEnterEvent (QDragEnterEvent* event);
void dropEvent (QDropEvent* event);

virtual void mousePressEvent (QMouseEvent* event);
virtual void mouseMoveEvent (QMouseEvent* event);
virtual void mouseReleaseEvent (QMouseEvent* event);
virtual void mouseDoubleClickEvent (QMouseEvent* event);

virtual void closeEvent (QCloseEvent* event);

bool eventFilter (QObject* object, QEvent* event);

/*==============================================================================
	Qt slots
*/
public slots:

void open (const QString& source_name);
void open_file ();
void open_URL ();

bool save_image ();


private slots:

void source_selections (const QStringList& source_list);

bool load_image (QNetworkReply*	network_reply);
void continue_startup ();

/**	Receives notification when the {@link
	PDS_Metadata::fetched(idaeim::PVL::Aggregate*) PDS metadata fetch
	completes}.

	If metadata is NULL, but a network request resulted in a {@link
	Network_Status::redirected_URL() redirected URL}, then an
	asychnronous {@link PDS_Metadata::fetch(const QUrl&, bool) fetch} of the
	metadata from the redirected URL is requested. If no redirected URL is
	found nothing more is done.

	If non-NULL metadata is obtained the parameters are added to the
	{@metadata() basic metadata} with the {@link
	#PDS_Metadata::PDS_METADATA_GROUP} name. Then the {@link
	Plastic_Image::notify_metadata_monitors() registered metadata
	monitors are notified} of the change to the metadata.

	@param	metadata	An idaeim::PVL::Aggregate pointer to the PDS
		metadata that was fetched. This will be NULL if no metadata was
		obtained.
*/
void PDS_metadata (idaeim::PVL::Aggregate* metadata);

void save_image_done (bool completed);

/**	Receives the {@link Image_Viewer::rendering_status(int) rendering
	status} signal from the Image_Viewer.

	@param	status	A rendering status condition.
	@see	Image_Viewer::rendering_status(int)
	@see	image_viewer_state_change(int)
*/
void rendering_status (int status);
void activity_indicator_clicked (int status);
void image_loaded (bool successful);

void image_moved (const QPoint& image_position, int band);
void image_cursor_moved
	(const QPoint& display_position, const QPoint& image_position);
void displayed_image_region_resized (const QSize& region_size);

/**	Handles the {@link Image_Viewer::state_change(int) state change}
	signal from the Image_Viewer.

	If startup splash image scaling has completed then the {@link
	load_initial_source() initial source image is loaded}.

	If image loading is not in progress the Activity_Indicator state
	and possible statistics refresh are checked against the changed
	Image_Viewer state.

	@param	state	The {@link Image_Viewer::state_change(int) state
		change} signal from the Image_Viewer.
*/
void image_viewer_state_change (int state);

void view_image_info (bool enable);

void view_image_metadata (bool enable);

void view_navigator (bool enabled);
void navigator_visibility_changed (bool visible);

void view_statistics (bool enable);
void statistics_visibility_changed (bool visible);
void statistics_section_changed (int panel_index);

void view_data_mapper (bool enable);
void data_mapper_visibility_changed (bool visible);

void tool_location_changed ();

void auto_resize (bool enable);
void fit_window_to_image ();

void view_status_bar (bool enabled);
void show_status_message (const QString& message);
void status_message_changed (const QString& message);

void help ();
void help_documentation (const QString& location);
void about ();

void tool_context_menu_requested (QDockWidget* tool, QContextMenuEvent* event);
void tool_position ();

void toggle_distance_tool(bool enable);
void line_color(const QColor & color);
#ifdef __APPLE__
void recognizer_toggled(bool enable);
#endif

/*==============================================================================
	Helpers
*/
private:

void save_layout ();
void restore_layout ();

bool load_initial_source ();
void load_image_start ();
void load_image_failed (const QString& reason = QString ());

void reset_metadata ();

void configure ();
void save_configuration ();
void image_load_actions (bool enabled);

void reset_selected_region ();
void reset_region_overlay ();
void update_line();
void reset_line();
int selection_modification (const QPoint& image_position);
void set_selection_cursor (int selection);

/**	Conditionally refresh the Statistics Tools.

	Refreshing the image statistics requires updating the histograms for
	the source and display images. The display image statistics need only
	be refreshed when the display statistics tool is visible. However,
	the source statistics need to be kept current in case they are needed
	by the Data Mapper tool for determining contrast stretch saturation
	bounds based on histogram percentages.

	If the {@link refresh_source_statistics() source statistics are
	refreshed} then the {@link refresh_display_statistics() display
	statistics are refreshed}. Note that if the source statistics can not
	be refreshed the display statistics also can not be refreshed. In
	this case the Statistics_Refresh_Needed flag is set to ensure a
	refresh at the next opportunity.
*/
void refresh_statistics ();

/**	Refresh the source statistics of the Statistics Tools.

	If a Statistics_Tools object does not exist nothing is done.

	If the {@selected_source_region() selected source region} intersects
	the region of the image in the Image_Viewer that is within the
	viewport, then the histograms of the source statistics tool are
	updated from the source data histograms obtained from the
	Image_Viewer. Updating the histograms data will fail if rendering of
	an applicable tile image is in progress. If the histograms update
	succeeded then the source statistics tool is refreshed for the
	selected image region. If the refresh of the source statistics tool
	completes successfully then the return value is true.

	@return true if the statistics were refreshed; false if the
		statistics refresh was not done for any reason.
*/
bool refresh_source_statistics ();

/**	Refresh the display statistics of the Statistics Tools.

	If a Statistics_Tools object exists, is visible, and the {@ink
	selected_display_region() selected display region} intersects the
	region of the image in the Image_Viewer that is within the viewport,
	then the histograms of the display statistics tool are updated with
	the display data histograms obtained from the Image_Viewer. If the
	histograms update succeeded then the display statistics tool is
	refreshed for the selected image region.
*/
void refresh_display_statistics ();

/**	Test if the Statistics Tools are visible.

	@return true if the Statistics_Tools object exists and it is visible;
		false otherwise.
*/
bool statistics_are_visible () const;

void make_way_for (QDockWidget* tool);
bool vertically_overlapping_docks
	(Qt::DockWidgetArea this_dock, Qt::DockWidgetArea that_dock) const;
void resize_tool (QDockWidget* tool);

static bool URL_source (QString& source_name);
bool JPIP_source (QString& source_name) const;

/*==============================================================================
	Data
*/
private:

//	Configuration/Preferences.
Preferences_Dialog
	*Preferences;

//	Menus.
QMenu
	*File_Menu;
QAction
	*Open_File_Action,
	*Open_URL_Action,
	*Save_Action,
	*Preferences_Action,
	*Quit_Action;

QMenu
	*Tools_Menu,
	*View_Menu,
	*Scale_Menu;
QAction
	*Distance_Tool_Action,
	*View_Image_Info_Action,
	*View_Image_Metadata_Action,
	*View_Navigator_Action,
	*View_Statistics_Action,
	*View_Data_Mapper_Action,
	*Auto_Resize_Action,
	*Fit_to_Image_Action,
	*View_Status_Bar_Action,
	*View_Tooltips_Action;

#ifdef __APPLE__
QAction
    *View_SpeechRecog_Action;
#endif

QMenu
	*Data_Map_Menu;

QMenu
	*Help_Menu;
QAction
	*Help_Action,
	*About_Action;
About_HiView_Dialog
	*About_Dialog;

QMenu
	*Tool_Position_Menu;
QAction
	*Floating_Position,
	*Left_Position,
	*Right_Position,
	*Top_Position,
	*Bottom_Position,
	*Close_Position;
QDockWidget
	*Selected_Tool;

//	Source selection list.
QComboBox
	*Source_Selections;

QString
	Source_Directory,
	Source_Name,
	Source_Name_Loading;

int
	Startup_Stage;
QString
	Initial_Source;
QSizeF
	Initial_Scaling;
bool
	Image_Loading;


//	Network access for image loading from HTTP URL.
QNetworkAccessManager
	*Network_Access_Manager;
QNetworkReply
	*Network_Reply;


//	Rendering activity indicator.
Activity_Indicator
	*Image_Activity_Indicator;


//	Image info panel.
Image_Info_Panel
	*Image_Info;
Location_Mapper
	*Location;


//	Image viewer.
Image_Viewer
	*Image_View;

//	Region selection.
QRectF
	Selected_Image_Region;
//	A selection modifiction mode, or zero if no modification in progress.
int
	Selection_Modification;
QPointF
	Selection_Start;
QRubberBand
	*Region_Overlay;
QLabel
	*Selected_Area;
QString
	Selected_Area_Text;

//	Image metadata dialog.
Metadata_Dialog
	*Image_Metadata_Dialog;
PDS_Metadata
	*Metadata;

// distance tool
QColor Line_Color;
Distance_Line *Line;
QPoint Image_Line_X;
QLine Image_Line;
bool Distance_Tool;
bool P1_Set;

//	Image navigator tool.
Navigator_Tool
	*Navigator;
bool
	Navigator_Fit;


//	Image statistics tools.
Statistics_Tools
	*Statistics;
bool
	Statistics_Refresh_Needed;


//	Source-to-Display data mapping tool.
Data_Mapper_Tool
	*Data_Mapper;
Data_Map
	**Data_Maps;


//	Open file selection dialog.
QFileDialog
	*Open_File_Dialog;

//	Save image dialog.
Save_Image_Dialog
	*Image_Save_Dialog;
Save_Image_Thread
	*Image_Save_Thread;
Plastic_Image
	*Image_Saved;

#ifdef __APPLE__
Mac_Voice_Adapter*	adapter;
#endif

static bool
	Restore_Layout;

//	Shared error message dialog.
static QErrorMessage
	*Error_Message;
};

}	//	namespace UA::HiRISE
#endif
