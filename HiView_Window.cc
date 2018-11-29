/*	HiView_Window

HiROC CVS ID: $Id: HiView_Window.cc,v 1.235 2014/08/05 17:58:08 stephens Exp $

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

#include	"HiView_Window.hh"

#include	"HiView_Application.hh"
#include	"HiView_Config.hh"
#include	"Preferences_Dialog.hh"
#include	"About_HiView_Dialog.hh"
#include	"Help_Docs.hh"
#include	"Activity_Indicator.hh"
#include	"Icon_Button.hh"
#include	"Image_Info_Panel.hh"
#include	"Location_Mapper.hh"
#include	"Metadata_Dialog.hh"
#include	"PDS_Metadata.hh"
#include	"Navigator_Tool.hh"
#include	"Statistics_Tools.hh"
#include	"Statistics_and_Bounds_Tool.hh"
#include	"Statistics_Tool.hh"
#include	"Data_Mapper_Tool.hh"
#include	"Image_Viewer.hh"
#include	"Plastic_Image_Factory.hh"
#include	"Plastic_Image.hh"
#include	"JP2_Image.hh"
#include	"Save_Image_Dialog.hh"
#include	"Save_Image_Thread.hh"
#include	"HiView_Utilities.hh"
#include	"Coordinate.hh"

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS
#include	<QString>

#include	<QDesktopWidget>
#include	<QSettings>
#include	<QAction>
#include	<QMenu>
#include	<QMenuBar>
#include	<QToolBar>
#include	<QToolButton>
#include	<QComboBox>
#include	<QLineEdit>
#include	<QCompleter>
#include	<QFileDialog>
#include	<QImageReader>
#include	<QFileInfo>
#include	<QDir>
#include	<QUrl>
#include	<QHostInfo>
#include	<QNetworkAccessManager>
#include	<QNetworkReply>
#include	<QVBoxLayout>
#include	<QHBoxLayout>
#include	<QRubberBand>
#include	<QCursor>
#include	<QStatusBar>
#include	<QResizeEvent>
#include	<QTimer>
#include	<QMessageBox>
#include	<QErrorMessage>
#include	<QCloseEvent>
#include <QPainter>
#include	<QLabel>
#include	<QLineF>
#include	<QtCore/qmath.h>
#include <QMimeData>

//	Image metadata parameters.
#include	"PVL.hh"
using idaeim::PVL::Aggregate;
using idaeim::PVL::Parameter;

#include	<string>
using std::string;

#ifdef __APPLE__
#include "Mac_Voice_Adapter.hh"
#endif

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_INITIALIZE		(1 << 1)
#define DEBUG_SLOTS				(1 << 2)
#define DEBUG_SIGNALS			(1 << 3)
#define DEBUG_MENUS				(1 << 4)
#define DEBUG_HELP				(1 << 5)
#define	DEBUG_SOURCE_SELECTIONS	(1 << 6)
#define DEBUG_IMAGE_VIEWER		(1 << 7)
#define DEBUG_NAVIGATOR			(1 << 8)
#define DEBUG_STATISTICS		(1 << 9)
#define DEBUG_DATA_MAPPER		(1 << 10)
#define DEBUG_TOOLS_POSITION	(1 << 11)
#define DEBUG_LAYOUT			(1 << 12)
#define DEBUG_LOAD_IMAGE		(1 << 13)
#define DEBUG_DROP_IMAGE		(1 << 14)
#define DEBUG_OPEN_FILE			(1 << 15)
#define DEBUG_SAVE_IMAGE		(1 << 16)
#define DEBUG_MOUSE_EVENTS		(1 << 17)
#define DEBUG_STATE				(1 << 18)
#define DEBUG_RENDERING_STATUS	(1 << 19)
#define DEBUG_HELPERS			(1 << 20)
#define DEBUG_OVERVIEW			(1 << 21)
#define DEBUG_METADATA			(1 << 22)
#define DEBUG_METADATA_PARAMETERS	(1 << 23)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OVERVIEW
#endif

#include	"Projection.hh"

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
using std::showbase;
using std::uppercase;
using std::hex;
using std::dec;

#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	HiView_Window::ID =
		"UA::HiRISE::HiView_Window ($Revision: 1.235 $ $Date: 2014/08/05 17:58:08 $)";


#ifndef IMAGE_DISPLAY_MIN_WIDTH
#define IMAGE_DISPLAY_MIN_WIDTH			300
#endif
#ifndef IMAGE_DISPLAY_MIN_HEIGHT
#define IMAGE_DISPLAY_MIN_HEIGHT		200
#endif

#ifndef SPLASH_IMAGE
#define SPLASH_IMAGE					":/Images/HiView_Splash.png"
#endif

#ifndef SPLASH_IMAGE_DWELL_MS
#define SPLASH_IMAGE_DWELL_MS			700
#endif

#ifndef DEFAULT_VIEW_TOOLTIPS
#define DEFAULT_VIEW_TOOLTIPS			true
#endif

#ifndef DEFAULT_VIEW_SPEECH_RECOG
#ifdef __APPLE__
#define DEFAULT_VIEW_SPEECH_RECOG			true
#else
#define DEFAULT_VIEW_SPEECH_RECOG			false
#endif
#endif

#ifndef DEFAULT_VIEW_IMAGE_INFO
#define DEFAULT_VIEW_IMAGE_INFO			false
#endif

#ifndef DEFAULT_VIEW_IMAGE_METADATA
#define DEFAULT_VIEW_IMAGE_METADATA		false
#endif
#ifndef DEFAULT_METADATA_ROOT_VISIBLE
#define DEFAULT_METADATA_ROOT_VISIBLE	false
#endif
#ifndef DEFAULT_METADATA_STRIPED
#define DEFAULT_METADATA_STRIPED		true
#endif

#ifndef DEFAULT_VIEW_STATUS_BAR
#define DEFAULT_VIEW_STATUS_BAR			false
#endif

#ifndef DEFAULT_AUTO_RESIZE
#define DEFAULT_AUTO_RESIZE				false
#endif

/*==============================================================================
	Application configuration parameters
*/
#define MAX_DISPLAY_VALUE \
	HiView_Config::MAX_DISPLAY_VALUE

#define Shift_Region_Cursor \
	HiView_Config::Shift_Region_Cursor
#define Shift_Region_Horizontal_Cursor \
	HiView_Config::Shift_Region_Horizontal_Cursor
#define Shift_Region_Vertical_Cursor \
	HiView_Config::Shift_Region_Vertical_Cursor
#define Shift_Region_FDiag_Cursor \
	HiView_Config::Shift_Region_FDiag_Cursor
#define Shift_Region_BDiag_Cursor \
	HiView_Config::Shift_Region_BDiag_Cursor

/*==============================================================================
	Class data members
*/
bool
	HiView_Window::Restore_Layout;

QErrorMessage
	*HiView_Window::Error_Message	= NULL;

#ifndef X_BUTTON_ICON
#define X_BUTTON_ICON					":/Images/X_button.png"
#endif

/*==============================================================================
	Local constants
*/
namespace
{
//	Activity_Indicator states.
enum
	{
	ACTIVITY_OFF					= Activity_Indicator::STATE_OFF,
	ACTIVITY_VISIBLE_RENDERING		= Activity_Indicator::STATE_1,
	ACTIVITY_BACKGROUND_RENDERING	= Activity_Indicator::STATE_2
	};

//	Startup_Stages.
enum
	{
	STARTUP_COMPLETE,
	STARTUP_SPLASH,
	STARTUP_SPLASH_SCALING
	};

//	Tool_Position_Menu action indices.
enum
	{
	TOOL_POSITION_FLOATING	= 0,
	TOOL_POSITION_LEFT,
	TOOL_POSITION_RIGHT,
	TOOL_POSITION_TOP,
	TOOL_POSITION_BOTTOM,
	TOOL_POSITION_CLOSED
	};


#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
QString
dock_areas_names (Qt::DockWidgetAreas areas)
{
if (areas == Qt::NoDockWidgetArea)
	return "none";

QString
	name;

if (areas & Qt::LeftDockWidgetArea)
	name = "left";
if (areas & Qt::RightDockWidgetArea)
	{
	if (! name.isEmpty ())
		name += ", ";
	name += "right";
	}
if (areas & Qt::TopDockWidgetArea)
	{
	if (! name.isEmpty ())
		name += ", ";
	name += "top";
	}
if (areas & Qt::BottomDockWidgetArea)
	{
	if (! name.isEmpty ())
		name += ", ";
	name += "bottom";
	}
return name;
}
#endif
}

/*==============================================================================
	Constructors
*/
HiView_Window::HiView_Window
	(
	const QString&		source,
	const QSizeF&		scaling,
	Layout_Restoration	restore_layout,
	QWidget*			parent,
	Qt::WindowFlags		flags
	)
	:	QMainWindow (parent, flags),
		About_Dialog (NULL),
		Selected_Tool (NULL),
		Source_Name (source),
		Startup_Stage (STARTUP_SPLASH),
		Image_Loading (false),
		Network_Reply (NULL),
		Image_Activity_Indicator (NULL),
		Image_Info (NULL),
		Location (new Location_Mapper),
		Image_View (NULL),
		Selected_Image_Region (0, 0, 0, 0),
		Selection_Modification (0),
		Selection_Start (-1, -1),
		Region_Overlay (NULL),
		Selected_Area (NULL),
		Image_Metadata_Dialog (NULL),
		Metadata (NULL),
		Image_Line(0,0,0,0),
		Distance_Tool (false),
		Navigator (NULL),
		Navigator_Fit (true),
		Statistics (NULL),
		Statistics_Refresh_Needed (false),
		Data_Mapper (NULL),
		Open_File_Dialog (NULL),
		Image_Save_Dialog (NULL),
		Image_Saved (NULL)
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
clog << boolalpha;
#endif
if (qApp->applicationName ().isEmpty ())
	qApp->setApplicationName("HiView");
setObjectName (qApp->applicationName ());
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << ">>> HiView_Window: " << object_pathname (this) << endl;
#endif
setWindowTitle (qApp->applicationName ());

//	Shared error message dialog.
Error_Message = new QErrorMessage ();
Image_Viewer::error_message (Error_Message);
Navigator_Tool::error_message (Error_Message);
Data_Mapper_Tool::error_message (Error_Message);
Metadata_Dialog::error_message (Error_Message);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    shared Error_Message @ " << (void*)Error_Message << endl;
#endif

/*	Use the text file configuration settings format.

	N.B.: It is presumed that the application-wide (QCoreApplication)
	applicationName and organizationName have been set.
*/
QSettings::setDefaultFormat (QSettings::IniFormat);

//	Initialize the application configuration parameters.
HiView_Config::initialize ();

//	Load the user-modifiable preferences (must be done before menus created).
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    construct Preferences ..." << endl;
#endif
Preferences = new Preferences_Dialog (this);

//	Application menus (must be done before configuration).
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_menus ..." << endl;
#endif
create_menus ();

//	Configuration settings.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    configure ..." << endl;
#endif
configure ();

//	Disable image rendering auto-update.
Plastic_Image::default_auto_update (false);

//	Central widget.
QWidget
	*central_area = new QWidget (this);
central_area->setObjectName ("Central_Area");
QVBoxLayout
	*vertical_layout = new QVBoxLayout (central_area);
central_area->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

//	Image source selection bar.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_source_selections ..." << endl;
#endif
create_source_selections ();
QHBoxLayout
	*horizontal_layout = new QHBoxLayout;
horizontal_layout->addWidget (Source_Selections, 100);
connect (Preferences,
			SIGNAL (source_list_changed (const QStringList&)),
			SLOT (source_selections (const QStringList&)));

//	Image loading activity indicator.
int
	indicator_size = Source_Selections->sizeHint ().rheight () + 2;
if (! (indicator_size & 1))
	//	The size should be odd.
	++indicator_size;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    construct Image_Activity_Indicator ..." << endl;
#endif
Image_Activity_Indicator = new Activity_Indicator (indicator_size);
horizontal_layout->addWidget (Image_Activity_Indicator);
horizontal_layout->addStretch ();
vertical_layout->addLayout (horizontal_layout);

//	Brief image info panel.
horizontal_layout = new QHBoxLayout;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    construct Image_Info_Panel ..." << endl;
#endif
Image_Info = new Image_Info_Panel;
Image_Info->setVisible (false);
horizontal_layout->addWidget (Image_Info);
horizontal_layout->addStretch ();
vertical_layout->addLayout (horizontal_layout);
connect(Preferences,
			SIGNAL(longitude_direction_changed (int)),
		Image_Info,
			SLOT(longitude_direction (int)));
connect(Preferences,
			SIGNAL(longitude_units_changed (int)),
		Image_Info,
			SLOT(longitude_units (int)));
connect(Preferences,
			SIGNAL(latitude_units_changed (int)),
		Image_Info,
			SLOT(latitude_units (int)));
connect(Preferences,
			SIGNAL(script_changed(const QString&)),
		Image_Info,
			SLOT(script_changed(const QString&)));
connect(Preferences,
			SIGNAL(show_script_changed(bool)),
		Image_Info,
			SLOT(show_script_changed(bool)));
connect(Image_Info,
			SIGNAL(variables_updated(QStringList&)),
		Preferences,
			SIGNAL(variables_updated(QStringList&)));
//	Image metadata dialog.
Image_Metadata_Dialog = new Metadata_Dialog (NULL, this, Qt::Window);
Image_Metadata_Dialog->root_name ("Metadata");
Image_Metadata_Dialog->root_visible (DEFAULT_METADATA_ROOT_VISIBLE);
Image_Metadata_Dialog->alternating_row_colors (DEFAULT_METADATA_STRIPED);
connect (Image_Metadata_Dialog,
			SIGNAL (visibility_changed (bool)),
			SLOT (view_image_metadata (bool)));

//	Image viewer pane.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_image_viewer ..." << endl;
#endif
create_image_viewer ();
vertical_layout->addWidget (Image_View);

//	Set the central layout.
setCentralWidget (central_area);


//Distance Line
Line = new Distance_Line(Image_View->image_display ());
Line->setColor(Line_Color);
Line->setVisible(false);



//	Selected region overlay.
Region_Overlay =
	new QRubberBand (QRubberBand::Rectangle, Image_View->image_display ());

//	Connect to the status bar message.
/*
connect (Image_View,
			SIGNAL (rendering_status_notice (const QString&)),
		 	SLOT (show_status_message (const QString&)));
*/
connect (statusBar (),
			SIGNAL (messageChanged (const QString&)),
		 	SLOT (status_message_changed (const QString&)));
Icon_Button
	*X_button = new Icon_Button (QIcon (X_BUTTON_ICON));
connect (X_button,
			SIGNAL (clicked (bool)),	//	Always emits false.
		 	SLOT (view_status_bar (bool)));
statusBar ()->addPermanentWidget (X_button);
statusBar ()->setVisible (false);

//	Image drag-and_drop.
setAcceptDrops (true);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE | DEBUG_DROP_IMAGE))
clog << "    drop events enabled = " << acceptDrops () << endl;
#endif
Network_Access_Manager = new QNetworkAccessManager (this);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE | DEBUG_DROP_IMAGE))
clog << "    Network_Access_Manager @ " << (void*)Network_Access_Manager << endl;
#endif
connect (Network_Access_Manager,
			SIGNAL (finished (QNetworkReply*)),
		 	SLOT (load_image (QNetworkReply*)));

//------------------------------------------------------------------------------
//	Dockable tools.

setDockOptions (QMainWindow::AnimatedDocks);
//	Left corners to left side.
setCorner (Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
setCorner (Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
//	Right corners to right side.
setCorner (Qt::TopRightCorner, Qt::RightDockWidgetArea);
setCorner (Qt::BottomRightCorner,Qt::RightDockWidgetArea);

/*	Create the Navigator panel.

	The creation of the Navigator can safely be commented out to prevent
	any Navigator activity. This can be useful when debugging lower level
	operations associated with the Image_Viewer that would otherwise
	occur twice.
*/
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_navigator ..." << endl;
#endif
create_navigator ();

/*	Create the Statistics_Tool.

	The creation of the Statistics_Tool can safely be commented out to
	prevent any Statistics_Tool activity.
*/
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_statistics_panels ..." << endl;
#endif
create_statistics_panels ();
Image_Info->update_statistics(&Statistics->source_statistics ()->stats ());
/*	Create the Data_Mapper_Tool.

	The creation of the Data_Mapper_Tool can safely be commented out to
	prevent any Data_Mapper_Tool activity.
*/
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    create_data_mapper ..." << endl;
#endif
create_data_mapper ();

//	Create the Toolbar.
create_toolbar ();

//------------------------------------------------------------------------------
//	Startup sequence.

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    Startup_Stage set to STARTUP_SPLASH ("
		<< STARTUP_SPLASH << ')' << endl;
#endif
Startup_Stage = STARTUP_SPLASH;

//	Layout restoration flag.
if (restore_layout == PREFERENCES_RESTORE_LAYOUT)
	Restore_Layout = Preferences->restore_layout ();
else
	Restore_Layout = restore_layout;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    Restore_Layout = " << Restore_Layout << " - "
		<< ((restore_layout == PREFERENCES_RESTORE_LAYOUT) ?
			"from Preferences" : "user specified") << endl;
#endif

//	Initial source to load after the splash image (may be empty).
Initial_Source = source;
//	Initial scaling to apply to the initial source.
if (scaling.isEmpty () &&
	Preferences->initial_scale () != Preferences->INITIAL_SCALE_AUTO_FIT)
	//	Use the Preferences setting for the initial scaling.
	Initial_Scaling.rwidth () =
	Initial_Scaling.rheight () = Preferences->initial_scale ();
else
	Initial_Scaling = scaling;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
clog << "    Initial_Scaling = " << Initial_Scaling << " - from "
		<< ((scaling.isEmpty () &&
			Preferences->initial_scale ()
				!= Preferences->INITIAL_SCALE_AUTO_FIT) ?
			"Preferences" : "command line") << endl;
#endif

//	Begin with the splash image.
QImage
	default_image (SPLASH_IMAGE);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
	clog << "    load SPLASH_IMAGE \"" << SPLASH_IMAGE
			<< "\" image @ " << (void*)&default_image << endl;
	#endif
if (! load_image (default_image, QSizeF (1.0, 1.0)))
	{
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
	clog << "    initial load failed" << endl
		 << "    show ..." << endl;
	#endif
	show ();
	Source_Selections->setFocus (Qt::OtherFocusReason);
	Startup_Stage = STARTUP_COMPLETE;
	}

if (Error_Message->isVisible ())
	Error_Message->raise ();

//	Filter tooltip events.
qApp->installEventFilter (this);


HiView_Application
	*application = dynamic_cast<HiView_Application*>(qApp);
if (application)
	{
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
	clog << "    connect to HiView_Application file_open_request" << endl;
	#endif
	connect (application,
		SIGNAL (file_open_request (const QString&)),
		SLOT (open (const QString&)));
	}

#ifdef __APPLE__
//if (View_SpeechRecog_Action->isChecked ())
//{
    adapter = new Mac_Voice_Adapter
    (
        Image_View,
        Statistics,
        Data_Mapper
     );
    adapter->toggle(View_SpeechRecog_Action->isChecked ());
//}
//else
//{
//    adapter = NULL;
//
/*connect (SpeechRecog_Action,
			SIGNAL (toggle()),
		 	SLOT (load_image (QNetworkReply*)));*/
#endif

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_INITIALIZE))
LOCKED_LOGGING ((
clog << "<<< HiView_Window" << endl));
#endif
}


HiView_Window::~HiView_Window ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< HiView_Window::~HiView_Window" << endl;
#endif
}

#ifdef __APPLE__
void
HiView_Window::recognizer_toggled(bool enable)
{
    if (enable)
    {
        if (adapter)
        {
            adapter->toggle(enable);
        }
        else
        {
            adapter = new Mac_Voice_Adapter
            (
                Image_View,
                Statistics,
                Data_Mapper
            );
        }
    }
    else if (adapter)
    {
            adapter->toggle(enable);
    }
}
#endif

/*==============================================================================
	Configuration
*/
void
HiView_Window::configure ()
{
//	N.B. The menu actions must be created before they are configured.
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << ">>> HiView_Window::configure" << endl
	 << "    Organization - " << qApp->organizationName () << endl
	 << "     Application - " << qApp->applicationName () << endl;
#endif
/*	Use the text file format by default.

	N.B.: It is presumed that the application-wide (QCoreApplication)
	applicationName and organizationName have been set.
*/
QSettings::setDefaultFormat (QSettings::IniFormat);

//	Initialize configuration settings that are not user preferences.

QSettings
	settings;

Source_Directory = settings.value ("Source_Directory",
	QString ()).toString ();
QFileInfo
	file_info (Source_Directory);
if (Source_Directory.isEmpty () ||
	! file_info.exists () ||
	! file_info.isDir ())
	Source_Directory = QDir::currentPath ();

QString
	pathname (settings.value ("Save_Image_Directory",
		QString ()).toString ());
file_info.setFile (pathname);
if (pathname.isEmpty () ||
	! file_info.exists () ||
	! file_info.isDir ())
	pathname = QDir::currentPath ();
Save_Image_Dialog::Default_Directory = pathname;
Save_Image_Dialog::Default_Image_Format = settings.value ("Save_Image_Format",
	QString ()).toString ();

View_Tooltips_Action->setChecked
	(settings.value ("View_Tooltips",
		DEFAULT_VIEW_TOOLTIPS).toBool ());

#ifdef __APPLE__
View_SpeechRecog_Action->setChecked
	(settings.value ("View_SpeechRecog",
		DEFAULT_VIEW_SPEECH_RECOG).toBool ());
#endif

View_Image_Info_Action->setChecked
	(settings.value ("View_Image_Info",
		DEFAULT_VIEW_IMAGE_INFO).toBool ());

View_Image_Metadata_Action->setChecked
	(settings.value ("View_Image_Metadata",
		DEFAULT_VIEW_IMAGE_METADATA).toBool ());

View_Status_Bar_Action->setChecked
	(settings.value ("View_Status_Bar",
		DEFAULT_VIEW_STATUS_BAR).toBool ());

Navigator_Tool::default_immediate_mode
	(settings.value ("Navigator_Immediate_Mode",
		Navigator_Tool::default_immediate_mode ()).toBool ());

/*
Auto_Resize_Action->setChecked
	(settings.value ("Auto_Resize",
		DEFAULT_AUTO_RESIZE).toBool ());
*/


Line_Color = QColor(settings.value("Line_Color", "#ff0000").toString());

//	Clean out obsolete settings.
if (settings.contains ("View_Navigator"))
	settings.remove   ("View_Navigator");
if (settings.contains ("View_Statistics"))
	settings.remove   ("View_Statistics");
if (settings.contains ("View_Data_Mapper"))
	settings.remove   ("View_Data_Mapper");

#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "<<< HiView_Window::configure" << endl;
#endif
}

void
HiView_Window::save_configuration ()
{
#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << ">>> HiView_Window::save_configuration" << endl
	 << "    Organization - " << qApp->organizationName () << endl
	 << "     Application - " << qApp->applicationName () << endl;
#endif
QSettings
	settings (QSettings::IniFormat, QSettings::UserScope,
		qApp->organizationName (), qApp->applicationName ());

settings.setValue ("Source_Directory",
	Source_Directory);
settings.setValue ("View_Tooltips",
	View_Tooltips_Action->isChecked ());
#ifdef __APPLE__
settings.setValue ("View_SpeechRecog",
	View_SpeechRecog_Action->isChecked ());
#endif
settings.setValue ("View_Image_Info",
	View_Image_Info_Action->isChecked ());
settings.setValue ("View_Image_Metadata",
	View_Image_Metadata_Action->isChecked ());
settings.setValue ("View_Status_Bar",
	View_Status_Bar_Action->isChecked ());
/*
settings.setValue ("Auto_Resize",
	Auto_Resize_Action->isChecked ());
*/

if (Image_Save_Dialog)
	{
	QString
		pathname (Image_Save_Dialog->pathname ());
	if (! pathname.isEmpty ())
		{
		QFileInfo
			file_info (pathname);
		if (! file_info.isDir ())
			pathname = file_info.dir ().absolutePath ();
		settings.setValue ("Save_Image_Directory",
			pathname);
		}
	pathname = Image_Save_Dialog->image_format ();
	if (! pathname.isEmpty ())
		settings.setValue ("Save_Image_Format",
			pathname);
	}

if (Navigator)
	settings.setValue ("Navigator_Immediate_Mode",
		Navigator->immediate_mode ());

if (Statistics)
	{
	settings.setValue ("Statistics_Upper_Limit_Offset",
		Statistics->source_statistics ()->stats ().upper_limit ());
	settings.setValue ("Statistics_Lower_Limit_Offset",
		Statistics->source_statistics ()->stats ().lower_limit ());
	}

if (Data_Mapper)
	{
	QList<QVariant>
		values;
	#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
	clog << "    Saturation_Upper_Bound_Percents =";
	#endif
	for (int
			band = 0;
			band < 3;
		  ++band)
		{
		#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
		if (band)
			clog << ',';
		clog << ' ' << Data_Mapper->upper_bound_percent (band);
		#endif
		values.append (QVariant
			(Data_Mapper->upper_bound_percent (band)));
		}
	settings.setValue ("Saturation_Upper_Bound_Percents", values);
	values.clear ();

	#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
	clog << endl
		 << "    Saturation_Lower_Bound_Percents =";
	#endif
	for (int
			band = 0;
			band < 3;
		  ++band)
		{
		#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
		if (band)
			clog << ',';
		clog << ' ' << Data_Mapper->lower_bound_percent (band);
		#endif
		values.append (QVariant
			(Data_Mapper->lower_bound_percent (band)));
		}
	settings.setValue ("Saturation_Lower_Bound_Percents", values);

	#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
	clog << endl
		 << "    Last_CSV_File = \""
		 	<< Data_Mapper->last_CSV_file () << '"' << endl;
	#endif
	settings.setValue ("Last_CSV_File",
			Data_Mapper->last_CSV_file ());
	}

#if ((DEBUG_SECTION) & DEBUG_INITIALIZE)
clog << "<<< HiView_Window::save_configuration" << endl;
#endif
}


void
HiView_Window::save_layout ()
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">>> HiView_Window::save_layout" << endl;
#endif
QString
	application_geometry (qApp->applicationName () + "_Geometry"),
	application_state (qApp->applicationName () + "_State");
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << "    saving " << Preferences_Dialog::LAYOUT_GEOMETRY_SECTION << ' '
		<< application_geometry << " and " << application_state
		<< " settings" << endl;
#endif
QSettings
	settings;
settings.beginGroup (Preferences_Dialog::LAYOUT_GEOMETRY_SECTION);
settings.setValue (application_geometry, saveGeometry ());
settings.setValue (application_state, saveState ());
settings.endGroup ();
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << "<<< HiView_Window::save_layout" << endl;
#endif
}


void
HiView_Window::restore_layout ()
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
				DEBUG_INITIALIZE | \
				DEBUG_MENUS | \
				DEBUG_LOAD_IMAGE))
clog << ">>> HiView_Window::restore_layout" << endl;
#endif
QSettings
	settings;
settings.beginGroup (Preferences_Dialog::LAYOUT_GEOMETRY_SECTION);

//	Cleanup obsolete settings.
if (settings.contains ("Main_Window"))
	settings.remove ("Main_Window");
if (settings.contains ("Image_Display"))
	settings.remove ("Image_Display");
if (settings.contains ("Navigator_Tool"))
	settings.remove ("Navigator_Tool");
if (settings.contains ("Navigator_Tool_Dock"))
	settings.remove ("Navigator_Tool_Dock");
if (settings.contains ("Data_Mapper_Tool"))
	settings.remove ("Data_Mapper_Tool");
if (settings.contains ("Data_Mapper_Tool_Dock"))
	settings.remove ("Data_Mapper_Tool_Dock");
if (settings.contains ("Statistics_Tools"))
	settings.remove ("Statistics_Tools");
if (settings.contains ("Statistics_Tools_Dock"))
	settings.remove ("Statistics_Tools_Dock");

QString
	application_geometry (qApp->applicationName () + "_Geometry"),
	application_state (qApp->applicationName () + "_State");
if (Restore_Layout &&
	Image_Metadata_Dialog &&
	Navigator &&
	Statistics &&
	Data_Mapper &&
	settings.contains (application_geometry) &&
	settings.contains (application_state))
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    restoring geometry and dock states" << endl;
	#endif
	restoreGeometry
		(settings.value (windowTitle () + "_Geometry").toByteArray ());
	restoreState
		(settings.value (windowTitle () + "_State").toByteArray ());
	}
else
	{
	//	Don't restore the image info, metadata, and status bar panels.
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    not restoring image info, metadata, and status bar panels"
			<< endl;
	#endif
	View_Image_Info_Action->setChecked (false);
	View_Image_Metadata_Action->setChecked (false);
	View_Status_Bar_Action->setChecked (false);
	}
settings.endGroup ();

/*	Synchronize menu items.

	All of the following actions are expected to be initialed to their
	unchecked and disabled state. The configure settings may have changed
	their checked state, which may also have been changed during the
	startup sequence.

	Dock tool view actions have their checked state set according to
	whether the corresponding tool is visible. Then all actions are
	enabled. The action slots of non-dock tools (image info panel,
	metadata dialog, and status bar) are called if the actions are
	checked to cause these tools to become visible.
*/
if (Navigator)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    Navigator isVisible = " << Navigator->isVisible () << endl;
	#endif
	View_Navigator_Action->setChecked (Navigator->isVisible ());
	View_Navigator_Action->setEnabled (true);
	}
if (Statistics)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    Statistics isVisible = " << Statistics->isVisible () << endl;
	#endif
	View_Statistics_Action->setChecked (Statistics->isVisible ());
	View_Statistics_Action->setEnabled (true);
	}
if (Data_Mapper)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    Data_Mapper isVisible = " << Data_Mapper->isVisible () << endl;
	#endif
	View_Data_Mapper_Action->setChecked (Data_Mapper->isVisible ());
	View_Data_Mapper_Action->setEnabled (true);
	}
if (Image_Info)
	{
	View_Image_Info_Action->setEnabled (true);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    Image_Info visible = "
			<< View_Image_Info_Action->isChecked () << endl;
	#endif
	if (View_Image_Info_Action->isChecked ())
		view_image_info (true);
	}
if (Image_Metadata_Dialog)
	{
	View_Image_Metadata_Action->setEnabled (true);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
					DEBUG_INITIALIZE | \
					DEBUG_MENUS | \
					DEBUG_LOAD_IMAGE))
	clog << "    Image_Metadta_Dialog visible = "
			<< View_Image_Metadata_Action->isChecked () << endl;
	#endif
	if (View_Image_Metadata_Action->isChecked ())
		view_image_metadata (true);
	}
View_Status_Bar_Action->setEnabled (true);
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
				DEBUG_INITIALIZE | \
				DEBUG_MENUS | \
				DEBUG_LOAD_IMAGE))
clog << "    status bar visible = "
		<< View_Status_Bar_Action->isChecked () << endl;
#endif
if (View_Status_Bar_Action->isChecked ())
	view_status_bar (true);
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | \
				DEBUG_INITIALIZE | \
				DEBUG_MENUS | \
				DEBUG_LOAD_IMAGE))
clog << "<<< HiView_Window::restore_layout" << endl;
#endif
}

/*==============================================================================
	Menus
*/
void
HiView_Window::create_menus ()
{
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_MENUS))
clog << ">>> HiView_Window::create_menus" << endl;
#endif
QAction
	*action;
QKeySequence
	key_sequence;

//	Disable context menu event.
setContextMenuPolicy (Qt::PreventContextMenu);

//	Application shortcuts.
action = new QAction (tr ("Minimize Window"), this);
action->setShortcut (tr ("Ctrl+M"));
connect (action,
			SIGNAL (triggered ()),
			SLOT (showMinimized ()));
addAction (action);

//	File.
File_Menu = menuBar ()->addMenu (tr ("&File"));

Open_File_Action = new QAction (tr ("&Open File..."), this);
key_sequence = QKeySequence (QKeySequence::Open);
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl+O"));
Open_File_Action->setShortcut (key_sequence);
connect (Open_File_Action,
			SIGNAL (triggered ()),
			SLOT (open_file ()));
File_Menu->addAction (Open_File_Action);

Open_URL_Action = new QAction (tr ("Open &Location..."), this);
Open_URL_Action->setShortcut (tr ("Ctrl+L"));
connect (Open_URL_Action,
			SIGNAL (triggered ()),
			SLOT (open_URL ()));
File_Menu->addAction (Open_URL_Action);

Save_Action = new QAction (tr ("&Save..."), this);
key_sequence = QKeySequence (QKeySequence::Save);
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl+S"));
Save_Action->setShortcut (key_sequence);
connect (Save_Action,
			SIGNAL (triggered ()),
			SLOT (save_image ()));
File_Menu->addAction (Save_Action);

File_Menu->addSeparator ();

Preferences_Action = new QAction (tr ("Preferences..."), this);
/*	Broken on OS X with Qt 4.6.2
key_sequence = QKeySequence (QKeySequence::Preferences);
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl+,"));
*/
Preferences_Action->setShortcut (tr ("Ctrl+,"));
connect (Preferences_Action,
			SIGNAL (triggered ()),
		 Preferences,
		 	SLOT (show ()));
Preferences_Action->setMenuRole (QAction::PreferencesRole);
File_Menu->addAction (Preferences_Action);

File_Menu->addSeparator ();

Quit_Action = new QAction (tr ("&Quit"), this);
//	N.B.: QKeySequence::Quit is not present in Qt v4.5.x
#if QT_VERSION >= 0x40600
key_sequence = QKeySequence (QKeySequence::Quit);
#endif
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl+Q"));
Quit_Action->setShortcut (key_sequence);
connect (Quit_Action,
			SIGNAL (triggered ()),
			SLOT (close ()));
Quit_Action->setMenuRole (QAction::QuitRole);
File_Menu->addAction (Quit_Action);

//  Tools
Tools_Menu = menuBar ()->addMenu (tr ("&Tools"));
Distance_Tool_Action = new QAction (tr ("&Distance Tool"), this);
Distance_Tool_Action->setShortcut (tr ("Alt+D"));
Distance_Tool_Action->setCheckable (true);
Distance_Tool_Action->setChecked(false);
Distance_Tool_Action->setEnabled(true);
connect(Distance_Tool_Action, SIGNAL(triggered(bool)), SLOT(toggle_distance_tool(bool)));
Tools_Menu->addAction (Distance_Tool_Action);

//	View.
View_Menu = menuBar ()->addMenu (tr ("&View"));

View_Image_Info_Action = new QAction (tr ("Image &Info"), this);
View_Image_Info_Action->setShortcut (tr ("Ctrl+I"));
View_Image_Info_Action->setCheckable (true);
View_Image_Info_Action->setChecked (false);
View_Image_Info_Action->setEnabled (false);	//	Enabled later (restore_layout).
connect (View_Image_Info_Action,
			/*
				N.B.: The toggled signal is emitted when setChecked is called;
				the triggered signal is not emitted when setChecked is called.
			*/
			SIGNAL (triggered (bool)),
			SLOT (view_image_info (bool)));
View_Menu->addAction (View_Image_Info_Action);

View_Image_Metadata_Action = new QAction (tr ("Image &Metadata"), this);
View_Image_Metadata_Action->setShortcut (tr ("Alt+M"));
View_Image_Metadata_Action->setCheckable (true);
View_Image_Metadata_Action->setChecked (false);
View_Image_Metadata_Action->setEnabled (false);
connect (View_Image_Metadata_Action,
			SIGNAL (triggered (bool)),
			SLOT (view_image_metadata (bool)));
View_Menu->addAction (View_Image_Metadata_Action);

View_Navigator_Action = new QAction (tr ("&Navigator"), this);
View_Navigator_Action->setShortcut (tr ("Ctrl+N"));
View_Navigator_Action->setCheckable (true);
View_Navigator_Action->setChecked (false);
View_Navigator_Action->setEnabled (false);	//	Enabled later.
connect (View_Navigator_Action,
			SIGNAL (triggered (bool)),
			SLOT (view_navigator (bool)));
View_Menu->addAction (View_Navigator_Action);

View_Statistics_Action = new QAction (tr ("S&tatistics"), this);
View_Statistics_Action->setShortcut (tr ("Ctrl+T"));
View_Statistics_Action->setCheckable (true);
View_Statistics_Action->setChecked (false);
View_Statistics_Action->setEnabled (false);	//	Enabled later.
connect (View_Statistics_Action,
			SIGNAL (triggered (bool)),
			SLOT (view_statistics (bool)));
View_Menu->addAction (View_Statistics_Action);

View_Data_Mapper_Action = new QAction (tr ("&Data Mapper"), this);
View_Data_Mapper_Action->setShortcut (tr ("Ctrl+D"));
View_Data_Mapper_Action->setCheckable (true);
View_Data_Mapper_Action->setChecked (false);
View_Data_Mapper_Action->setEnabled (false);	//	Enabled later.
connect (View_Data_Mapper_Action,
			SIGNAL (triggered (bool)),
			SLOT (view_data_mapper (bool)));
View_Menu->addAction (View_Data_Mapper_Action);

View_Menu->addSeparator ();

Scale_Menu = View_Menu->addMenu (tr ("&Scale Image"));
//	The tear-off menu does not properly show the accelerator keys (4.6).
//Scale_Menu->setTearOffEnabled (true);
//	Add menu actions after the Image_Viewer has been constructed....

Fit_to_Image_Action = new QAction (tr ("Resize Window to &Image"), this);
Fit_to_Image_Action->setEnabled (false);
Fit_to_Image_Action->setShortcut(tr ("Ctrl+Shift+I"));
Fit_to_Image_Action->setToolTip (tr ("Resize the window to the image"));
connect (Fit_to_Image_Action,
			SIGNAL (triggered ()),
		 	SLOT (fit_window_to_image ()));
View_Menu->addAction (Fit_to_Image_Action);

View_Menu->addSeparator ();

View_Status_Bar_Action = new QAction (tr ("Status &Bar"), this);
View_Status_Bar_Action->setShortcut (tr ("Ctrl+B"));
View_Status_Bar_Action->setCheckable (true);
View_Status_Bar_Action->setChecked (false);
View_Status_Bar_Action->setEnabled (false);	//	Enabled later.
connect (View_Status_Bar_Action,
			SIGNAL (triggered (bool)),
		 	SLOT (view_status_bar (bool)));
View_Menu->addAction (View_Status_Bar_Action);

View_Tooltips_Action = new QAction (tr ("Tooltips"), this);
View_Tooltips_Action->setCheckable (true);
View_Tooltips_Action->setChecked (true);
View_Menu->addAction (View_Tooltips_Action);

#ifdef __APPLE__
View_SpeechRecog_Action = new QAction (tr ("Speech Recognizer"), this);
View_SpeechRecog_Action->setCheckable (true);
View_SpeechRecog_Action->setChecked (false);
View_Menu->addAction (View_SpeechRecog_Action);
connect (View_SpeechRecog_Action,
            SIGNAL (triggered (bool)),
            SLOT (recognizer_toggled(bool)) );

#endif

Auto_Resize_Action = new QAction (tr ("Auto-Resize"), this);
Auto_Resize_Action->setCheckable (true);
Auto_Resize_Action->setChecked (false);
/*
connect (Auto_Resize_Action, SIGNAL (triggered (bool)),
			this, SLOT (auto_resize (bool)));
View_Menu->addAction (Auto_Resize_Action);
*/

//	Data Map (populated when the Data_Map is created).
Data_Map_Menu = menuBar ()->addMenu (tr ("&Data Map"));

//	Menu bar separator to (possibly) put the Help menu on the right end.
menuBar ()->addSeparator ();

//	Help.
Help_Menu = menuBar ()->addMenu (tr ("&Help"));

Help_Action = new QAction (tr ("Hi&View Help"), this);
Help_Action->setShortcut (tr ("Ctrl+?"));
Help_Action->setEnabled (! Preferences->documentation_location ().isEmpty ());
Help_Menu->addAction (Help_Action);
connect (Help_Action,
			SIGNAL (triggered ()),
			SLOT (help ()));
connect (Preferences,
			SIGNAL (documentation_location_changed (const QString&)),
			SLOT (help_documentation (const QString&)));

action = new QAction (tr ("About &HiView"), this);
connect (action,
			SIGNAL (triggered ()),
			SLOT (about ()));
action->setMenuRole (QAction::AboutRole);
Help_Menu->addAction (action);

action = new QAction (tr ("About &Qt"), this);
connect (action,
			SIGNAL (triggered ()),
		 qApp,
		 	SLOT (aboutQt ()));
action->setMenuRole (QAction::AboutQtRole);
Help_Menu->addAction (action);


//	Dock location for Tools.
Tool_Position_Menu = new QMenu (tr ("Tool &Position"));

//	Always add actions in TOOL_POSITION_XXX enum order.
Floating_Position = new QAction (tr ("Floating"), this);
Floating_Position->setShortcut (tr ("Ctrl+F"));
Tool_Position_Menu->addAction (Floating_Position);
connect (Floating_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));
Left_Position = new QAction (tr ("Left"), this);
Left_Position->setShortcut (Qt::CTRL + Qt::Key_Left);
Tool_Position_Menu->addAction (Left_Position);
connect (Left_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));
Right_Position = new QAction (tr ("Right"), this);
Right_Position->setShortcut (Qt::CTRL + Qt::Key_Right);
Tool_Position_Menu->addAction (Right_Position);
connect (Right_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));
Top_Position = new QAction (tr ("Top"), this);
Top_Position->setShortcut (Qt::CTRL + Qt::Key_Up);
Tool_Position_Menu->addAction (Top_Position);
connect (Top_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));
Bottom_Position = new QAction (tr ("Bottom"), this);
Bottom_Position->setShortcut (Qt::CTRL + Qt::Key_Down);
Tool_Position_Menu->addAction (Bottom_Position);
connect (Bottom_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));
Close_Position = new QAction (tr ("Close"), this);
Close_Position->setShortcut (tr ("Ctrl+W"));
Tool_Position_Menu->addAction (Close_Position);
connect (Close_Position,
			SIGNAL (triggered ()),
			SLOT (tool_position ()));

#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_MENUS))
clog << "<<< HiView_Window::create_menus" << endl;
#endif
}

void HiView_Window::toggle_distance_tool (bool enabled) {
	if(Distance_Tool_Action->isChecked() != enabled)
		Distance_Tool_Action->setChecked(enabled);
	else {
		Distance_Tool = enabled;
		if(Distance_Tool) {
			reset_selected_region ();
		}
		else {
			//turn off line
			Line->setVisible(false);
			//turn off mouse tracking
			setMouseTracking(false);
			centralWidget()->setMouseTracking(false);
			Image_View->setMouseTracking(false);
			Image_Info->set_property_f("distance_length_px", 0);
			Image_Info->set_property_f("distance_length_m", 0);
			Image_Info->evaluate_script();
		}
	}
}

void
HiView_Window::view_image_info
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS))
clog << ">>> HiView_Window::view_image_info: " << enabled << endl;
#endif
if (View_Image_Info_Action->isChecked () != enabled)
	//	Toggle the action, if it is enabled, which is connected here.
	View_Image_Info_Action->setChecked (enabled);
else
if (View_Image_Info_Action->isEnabled () &&
	Image_Info)
	Image_Info->setVisible (enabled);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS))
clog << "<<< HiView_Window::view_image_info" << endl;
#endif
}


void
HiView_Window::view_image_metadata
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS))
clog << ">>> HiView_Window::view_image_metadata: " << enabled << endl;
#endif
if (View_Image_Metadata_Action->isChecked () != enabled)
	//	Toggle the action, which is connected here.
	View_Image_Metadata_Action->setChecked (enabled);
else
if (View_Image_Metadata_Action->isEnabled () &&
	Image_Metadata_Dialog &&
	Image_Metadata_Dialog->isVisible () != enabled)
	Image_Metadata_Dialog->setVisible (enabled);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS))
clog << "<<< HiView_Window::view_image_metadata" << endl;
#endif
}


void
HiView_Window::auto_resize
	(
	bool	enabled
	)
{Auto_Resize_Action->setChecked (enabled);}


void
HiView_Window::fit_window_to_image ()
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << ">>> HiView_Window::fit_window_to_image" << endl;
#endif
//	Desired Image_Viewer size.
QSize
	scaled_size (Image_View->scaled_image_size ());
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << "    scaled image size = " << scaled_size << endl
	 << "      Image_View size = " << Image_View->size () << endl;
#endif
scaled_size.rwidth ()  += (Image_View->frameWidth () << 1);
scaled_size.rheight () += (Image_View->frameWidth () << 1);
if (Image_View->scrollbars ())
	scaled_size.rwidth () += Image_View->scale_slider ()->sizeHint ().width ();

//	Adjusted window size.
QSize
	window_size (size ());
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << "         desired size = " << scaled_size << endl
	 << "          window size = " << window_size << endl;
#endif
window_size -= Image_View->size ();
window_size += scaled_size;
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << "        adjusted size = " << window_size << endl;
#endif
if (window_size.rwidth ()  < minimumWidth ())
	window_size.rwidth ()  = minimumWidth ();
if (window_size.rwidth ()  > maximumWidth ())
	window_size.rwidth ()  = maximumWidth ();
if (window_size.rheight () < minimumHeight ())
	window_size.rheight () = minimumHeight ();
if (window_size.rheight () > maximumHeight ())
	window_size.rheight () = maximumHeight ();

//	Resize the window to the adjusted size.
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << "         limited size = " << window_size << endl;
#endif
resize (window_size);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MENUS | DEBUG_LAYOUT))
clog << "<<< HiView_Window::fit_window_to_image" << endl;
#endif
}


void
HiView_Window::update_window_fit_action ()
{
#if ((DEBUG_SECTION) & DEBUG_MENUS)
clog << ">>> HiView_Window::update_window_fit_action" << endl;
#endif
bool
	fit = false;
if (Image_View)
	fit = Image_View->display_fit_to_image ();
Fit_to_Image_Action->setEnabled (! fit);

#if ((DEBUG_SECTION) & DEBUG_MENUS)
clog << "<<< HiView_Window::update_window_fit_action" << endl;
#endif
return;
}


void
HiView_Window::displayed_image_region_resized
	(
	const QSize&
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << ">>> HiView_Window::displayed_image_region_resized" << endl;
#endif
update_window_fit_action ();
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << "<<< HiView_Window::displayed_image_region_resized" << endl;
#endif
}


void
HiView_Window::help ()
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_HELP))
clog << ">-< HiView_Window::help" << endl
	 << "    documentation_location = "
	 	<< Preferences->documentation_location () << endl
	 << "    default_documentation_filename = "
	 	<< Preferences->default_documentation_filename () << endl;
#endif
if (Preferences->documentation_location().isEmpty ())
	Help_Action->setEnabled (false);
else
	Preferences->help_docs ()->help
		(Preferences->default_documentation_filename ());
}


void
HiView_Window::help_documentation
	(
	const QString&	location
	)
{Help_Action->setEnabled (! location.isEmpty ());}


void
HiView_Window::about ()
{
if (! About_Dialog)
	About_Dialog = new About_HiView_Dialog;
About_Dialog->show ();	//	Run non-modal (exec is modal).
}

/*==============================================================================
	Source Selections
*/
void
HiView_Window::create_source_selections ()
{
#if ((DEBUG_SECTION) & (DEBUG_SOURCE_SELECTIONS | DEBUG_INITIALIZE))
clog << ">>> HiView_Window::create_source_selections -" << endl;
#endif
Source_Selections = new QComboBox ();
Source_Selections->setToolTip (tr ("Image source location"));
Source_Selections->setSizeAdjustPolicy (QComboBox::AdjustToContentsOnFirstShow);
Source_Selections->setDuplicatesEnabled (false);
Source_Selections->setInsertPolicy (QComboBox::InsertAtBottom);
Source_Selections->addItems (Preferences->source_list ());
Source_Selections->setEditable (true);
Source_Selections->setInsertPolicy (QComboBox::InsertAtTop);
Source_Selections->setMinimumContentsLength (32);
Source_Selections->setCurrentIndex (-1);
Source_Selections->clearEditText ();
Source_Selections->completer ()->setCaseSensitivity (Qt::CaseSensitive);
#if ((DEBUG_SECTION) & DEBUG_SOURCE_SELECTIONS)
for (int
		index = 0;
		index < Source_Selections->count ();
		index++)
	clog << "    " << index << ": "
			<< Source_Selections->itemText (index) << endl;
#endif
connect (Source_Selections,
			SIGNAL (activated (const QString&)),
			SLOT (open (const QString&)));
#if ((DEBUG_SECTION) & (DEBUG_SOURCE_SELECTIONS | DEBUG_INITIALIZE))
clog << "<<< HiView_Window::create_source_selections" << endl;
#endif
}


#ifndef DOXYGEN_PROCESSING
namespace
{
bool
operator==
	(
	const QComboBox&	list,
	const QStringList&	strings
	)
{
bool
	matched = false;
if (list.count () == strings.count ())
	{
	int
		count = list.count ();
	while (count--)
		if (list.itemText (count) != strings.at (count))
			break;
	if (count < 0)
		matched = true;
	}
return matched;
}


bool
operator==
	(
	const QStringList&	strings,
	const QComboBox&	list
	)
{return list == strings;}


bool
operator!=
	(
	const QComboBox&	list,
	const QStringList&	strings
	)
{return ! (list == strings);}


bool
operator!=
	(
	const QStringList&	strings,
	const QComboBox&	list
	)
{return ! (list == strings);}

}	//	Anonymous namespace.
#endif


void
HiView_Window::display_image_name
	(
	const QString&	source_name
	)
{
//	Set the source name as used here in the Image_Viewer.
Image_View->image_name (source_name);

QString
	name (source_name);
if (! name.isEmpty () &&
	! HiView_Utilities::is_URL (name))
	name = QFileInfo (name).absoluteFilePath ();
Source_Name = name;

QString
	title (qApp->applicationName ());
if (! source_name.isEmpty ())
	{
	add_source_selection (source_name);

	title += " - ";
	title += QFileInfo (Source_Name).fileName ();
	}
setWindowTitle (title);
}


void
HiView_Window::add_source_selection
	(
	const QString&	name
	)
{
#if ((DEBUG_SECTION) & DEBUG_SOURCE_SELECTIONS)
clog << ">>> HiView_Window::add_source_selection: " << name << endl;
#endif
QStringList
	source_list (Preferences->source_list ());
//	Push the new name on the top of the list as the current item.
source_list.push_front (name);

//	Save the source list.
Preferences->source_list (source_list);
#if ((DEBUG_SECTION) & DEBUG_SOURCE_SELECTIONS)
clog << "<<< HiView_Window::add_source_selection" << endl;
#endif
}


void
HiView_Window::remove_source_selection
	(
	const QString&	name
	)
{
int
	index = Preferences->source_list ().indexOf (name);
if (index >= 0)
	{
	QStringList
		source_list (Preferences->source_list ());
	source_list.removeAt (index);

	//	Save the selection list.
	Preferences->source_list (source_list);
	}
}


void
HiView_Window::source_selections
	(
	const QStringList&	source_list
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_SOURCE_SELECTIONS))
clog << ">>> HiView_Window::source_selections:" << endl;
for (int
		index = 0;
	 	index < source_list.count ();
	  ++index)
	clog << "    " << index << ": "
			<< source_list.at (index) << endl;
#endif
if (&source_list != &Preferences->source_list ())
	{
	//	Not a signal from Preferences; update the list.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_SOURCE_SELECTIONS))
	clog << "    set Preferences source_list" << endl;
	#endif
	Preferences->source_list (source_list);
	}
else
if (! Source_Name.isEmpty () &&
	(source_list.isEmpty () ||
	 source_list.at (0) != Source_Name))
	{
	/*	The Source_Name must be at the top of the list.

		The change to the list will result in this method being called.
		to complete the update of the Source_Selections list.
	*/
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_SOURCE_SELECTIONS))
	clog << "    add \"" << Source_Name << "\" to selections" << endl;
	#endif
	add_source_selection (Source_Name);
	}
else
if (*Source_Selections != source_list)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_SOURCE_SELECTIONS))
	clog << "    reset Source_Selections" << endl;
	#endif
	Source_Selections->clear ();
	Source_Selections->addItems (source_list);
	if (Source_Name.isEmpty ())
		Source_Selections->setCurrentIndex (-1);
	else
		Source_Selections->setCurrentIndex (0);
	}
else
	Source_Selections->setCurrentIndex (0);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_SOURCE_SELECTIONS))
clog << "<<< HiView_Window::source_selections" << endl;
#endif
}


bool
HiView_Window::URL_source
	(
	QString&	name
	)
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> URL_source: " << name << endl;
#endif
bool
	source_is_URL = false;

if (! name.isEmpty ())
	{
	if (name.startsWith ("file:", Qt::CaseInsensitive))
		{
		//	Convert to a filesystem pathname.
		int
			index = 5;
		if (name[index] == '/')
			//	Remove redundant '/' characters.
			while ((index + 1) < name.size () &&
					name[index + 1] == '/')
					++index;
		/*
			On UNIX systems the file URL has the form "file:///foo/bar".
			On MS/Windows the file URL has the form "file:///D:/foo/bar";
			note the extra '/' before the drive letter. This extra delimiter
			must be removed on MS/Windows systems to produce the intended
			pathname.
		*/
		#ifdef WIN32
		++index;
		#endif
		name.remove (0, index);
		}
	else
	if (HiView_Utilities::is_URL (name) ||
		name.startsWith ("http://", Qt::CaseInsensitive) ||
		name.startsWith ("https://", Qt::CaseInsensitive) )
		source_is_URL = true;

	/*
		This abbreviated URL accellerator has been disabled
		because it has been found to produce false positives
		on a single OS X system. No other systems have this problem!?!

	else
	if (! QDir::isAbsolutePath (name))
		{
	*/
		/*	The source name doesn't look like an absolute file pathname.

			This section will check the first part of the source
			specification, up to an optional colon port number delimiter,
			for a possible hostname or IP address using a hostname system
			lookup. If this succeeds the source name is converted to a
			HTTP URL. The source name remains unchanged if no hostname is
			found.
		*/
		/*
		This URL abbreviation accellerator has been disabled
		because it has been found to produce false positives
		on a single OS X system. No other systems have this problem!?!

		int
			pathname_index = name.indexOf (QDir::separator ()),
			colon_index = name.lastIndexOf (':', pathname_index);
		if (colon_index > 0)
			//	Potential hostname ends at the colon delimiter.
			pathname_index = colon_index;

		QString
			hostname (name.mid (0, pathname_index));
		#if ((DEBUG_SECTION) & DEBUG_HELPERS)
		clog << "    checking for hostname - " << hostname << endl;
		#endif
		if (QHostInfo::fromName (hostname).error () == QHostInfo::NoError)
			{
			#if ((DEBUG_SECTION) & DEBUG_HELPERS)
			clog << "    hostname lookup succeeded" << endl;
			#endif
			//	Add the HTTP URL prefix.
			name.insert (0, "http://");
			source_is_URL = true;
			}
		}
	*/

	if (! source_is_URL)
		{
		if (name.at (0) == '~' &&
			name.at (1) == QDir::separator ())
			//	Substitute '~' with the user's home directory pathname.
			name.replace (0, 1, QDir::homePath ());

		/*	Always apply the absoluteFilePath:
			Fully qualifies relative pathnames,
			and sets any MS/Windows drive letter to uppercase.
		*/
		name = QFileInfo (name).absoluteFilePath ();
		}
	}
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "<<< URL_source: "<< source_is_URL << " - " << name << endl;
#endif
return source_is_URL;
}


bool
HiView_Window::JPIP_source
	(
	QString&	source_name
	) const
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> JPIP_source: " << source_name << endl;
#endif
bool
	is_JPIP = false;
if (HiView_Utilities::is_URL (source_name))
	{
	QUrl
		URL (source_name);
	if (URL.scheme () == "jpip")
		{
		is_JPIP = true;
		if (URL.port () < 0 &&
			Preferences->JPIP_server_port () > 0)
			//	Add the default port number.
			URL.setPort (Preferences->JPIP_server_port ());
		}
	else
		{
		//	Search for a JP2 source.
		QString
			path;
		if (URL.path ().endsWith (".JP2", Qt::CaseInsensitive))
			//	The path has a JP2 reference.
			path = URL.path ();
		/*else
		if (URL.hasQuery ())
			{
			QList<QPair<QString, QString> >
				query_items (URL.queryItems ());
			int
				index = query_items.size ();
			while (index--)
				{
				path = query_items.at (index).second;
				if (path.endsWith (".JP2", Qt::CaseInsensitive))
					//	A query value has a JP2 reference.
					break;
				}
			if (index < 0)
				path.clear ();
			}
      */
		if (! path.isEmpty ())
			{
			//	HTTP to JPIP conversion.
			QString
				hostname /*(Preferences->HTTP_to_JPIP_hostname ());
			if (hostname.isEmpty ())
				hostname*/ = URL.host ();
			int
				port = Preferences->JPIP_server_port ();
			if (port <= 0)
				port = URL.port ();

			URL.clear ();
			URL.setScheme ("jpip");
			URL.setHost (hostname);
			URL.setPort (port);
			URL.setPath (path);
			source_name = URL.toString ();
			is_JPIP = true;
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "<<< JPIP_source: " << is_JPIP << " - " << source_name << endl;
#endif
return is_JPIP;
}

/*==============================================================================
	Tools
*/
void
HiView_Window::tool_location_changed ()
{
QDockWidget
	*tool = dynamic_cast<QDockWidget*>(sender ());
if (tool)
	{
	make_way_for (tool);
	resize_tool (tool);
	}
}


void
HiView_Window::make_way_for
	(
	QDockWidget*	tool
	)
{
if (! tool)
	return;
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << ">>> HiView_Window::make_way_for: " << (void*)tool
		<< ' ' << tool->windowTitle () << endl;
#endif
if (! tool->isWindow ())
	{
	//	Check for possible GUI squeeze.
	Qt::DockWidgetArea
		dock_area = dockWidgetArea (tool);

	/*	>>> WARNING <<< The frameSize method may return incorrect values.
		This problem is noted in the Qt Window Geometry section under
		X11 Peculiarities. The Enlightenment (e16) window manager is
		known to behave badly. In this case, punt: take the window height
		plus a frame margin guess of 25.
	*/
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
	bool
		guess = false;
	#endif
	int
		/**	Height of that part of the display screen excluding parts the
			system deems reserved (OS X dock and menu bar or task bar on
			MS/Windows).
		*/
		screen_height = (qApp->desktop ()->availableGeometry (this)).height (),
		current_height = height (),
		max_height = frameSize ().height ();
	if (max_height > screen_height ||
		max_height < current_height)
		{
		max_height = screen_height - 25;
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		guess = true;
		#endif
		}
	else
		max_height = screen_height - (max_height - current_height);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
	clog << "                   tool isWindow = "
			<< tool->isWindow () << endl
		 << "                  tool dock area = "
		 	<< dock_area << endl
		 << "                     tool height = "
		 	<< tool->height () << endl
		 << "              tool minimumHeight = "
		 	<< tool->minimumHeight () << endl
		 << "         available screen height = "
			<< screen_height << endl
		 << "                frameSize height = "
			<< frameSize ().height () << endl
		 << "      main window current height = "
		 	<< current_height << endl
		 << "                      max height = "
		 	<< max_height
			<< (guess ? " (guess)" : "") << endl;
	#endif
	if (current_height > max_height &&
		Statistics &&
		Statistics != tool &&
		Statistics->isVisible () &&
		! Statistics->isWindow () &&
		vertically_overlapping_docks
			(dock_area, dockWidgetArea (Statistics)))
		{
		current_height -= Statistics->height ();
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    hide Statistics" << endl
			 << "               Statistics height = "
			 	<< Statistics->height () << endl
			 << "        Statistics minimumHeight = "
			 	<< Statistics->minimumHeight () << endl
			 << "                  current_height = "
			 	<< current_height << endl;
		#endif
		Statistics->setVisible (false);
		}
	if (current_height > max_height &&
		Data_Mapper &&
		Data_Mapper != tool &&
		Data_Mapper->isVisible () &&
		! Data_Mapper->isWindow () &&
		vertically_overlapping_docks
			(dock_area, dockWidgetArea (Data_Mapper)))
		{
		current_height -= Data_Mapper->height ();
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    hide Data_Mapper" << endl
			 << "              Data_Mapper height = "
			 	<< Data_Mapper->height () << endl
			 << "       Data_Mapper minimumHeight = "
			 	<< Data_Mapper->minimumHeight () << endl
			 << "                  current_height = "
			 	<< current_height << endl;
		#endif
		Data_Mapper->setVisible (false);
		}
	if (current_height > max_height &&
		Navigator &&
		Navigator != tool &&
		Navigator->isVisible () &&
		! Navigator->isWindow () &&
		vertically_overlapping_docks
			(dock_area, dockWidgetArea (Navigator)))
		{
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    hide Navigator" << endl
			 << "                Navigator height = "
			 	<< Navigator->height () << endl
			 << "         Navigator minimumHeight = "
			 	<< Navigator->minimumHeight () << endl;
		#endif
		Navigator->setVisible (false);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << "<<< HiView_Window::make_way_for" << endl;
#endif
}


bool
HiView_Window::vertically_overlapping_docks
	(
	Qt::DockWidgetArea	this_dock_area,
	Qt::DockWidgetArea	that_dock_area
	) const
{
if (this_dock_area == that_dock_area)
	return true;

if ((this_dock_area == Qt::LeftDockWidgetArea &&
	 that_dock_area == Qt::RightDockWidgetArea) ||
	(this_dock_area == Qt::RightDockWidgetArea &&
	 that_dock_area == Qt::LeftDockWidgetArea))
	return false;

if (this_dock_area == corner (Qt::TopLeftCorner))
	return
		that_dock_area == corner (Qt::BottomLeftCorner) ||
		that_dock_area == Qt::LeftDockWidgetArea;
if (this_dock_area == corner (Qt::TopRightCorner))
	return
		that_dock_area == corner (Qt::BottomRightCorner) ||
		that_dock_area == Qt::RightDockWidgetArea;
if (this_dock_area == corner (Qt::BottomLeftCorner))
	return
		that_dock_area == corner (Qt::TopLeftCorner) ||
		that_dock_area == Qt::LeftDockWidgetArea;
if (this_dock_area == corner (Qt::BottomRightCorner))
	return
		that_dock_area == corner (Qt::TopRightCorner) ||
		that_dock_area == Qt::RightDockWidgetArea;

//	this_dock_area does not occupy a corner; check against that_dock_area.
if (that_dock_area == corner (Qt::TopLeftCorner))
	return
		this_dock_area == corner (Qt::BottomLeftCorner) ||
		this_dock_area == Qt::LeftDockWidgetArea;
if (that_dock_area == corner (Qt::TopRightCorner))
	return
		this_dock_area == corner (Qt::BottomRightCorner) ||
		this_dock_area == Qt::RightDockWidgetArea;
if (that_dock_area == corner (Qt::BottomLeftCorner))
	return
		this_dock_area == corner (Qt::TopLeftCorner) ||
		this_dock_area == Qt::LeftDockWidgetArea;
if (that_dock_area == corner (Qt::BottomRightCorner))
	return
		this_dock_area == corner (Qt::TopRightCorner) ||
		this_dock_area == Qt::RightDockWidgetArea;

//	that_dock does not occupy a corner either.
return false;
}


void
HiView_Window::resize_tool
	(
	QDockWidget*	tool
	)
{
if (! tool)
	return;
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << ">>> HiView_Window::resize_tool: " << (void*)tool
		<< ' ' << tool->windowTitle () << endl
	 << "    isWindow = " << tool->isWindow () << endl
	 << "    dockWidgetArea = " << dockWidgetArea (tool) << endl;
#endif
if (tool->isVisible () &&
	(tool != Navigator ||
	Navigator_Fit))
	{
	Qt::DockWidgetArea
		dock_area = dockWidgetArea (tool);
	if (tool->isWindow () ||
		dock_area & (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea))
		{
		//	No constraints.
		if (tool == Statistics)
			Statistics->visible_graph (true);
		else
		if (tool == Data_Mapper)
			Data_Mapper->visible_graph (true);
		}
	else
		{
		//	Apply contraints.
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    constrain -" << endl
			 << "    sizeHint ante = " << tool->sizeHint () << endl;
		#endif
		if (tool == Statistics)
			Statistics->visible_graph (false);
		else
		if (tool == Data_Mapper)
			Data_Mapper->visible_graph (false);

		/*	Force the tool to shrink to minimal size in a side dock.

			Unfortunately, the layout system is using the size of the
			tool from before it is moved into the dock. Changing the
			preferred size to be smaller after the tool has been moved
			into the dock is ignored when the layout is updated. So to
			force the layout system to notice, the tool is set to have a
			maximum width no larger than the tool's reported minimum size
			hint, or the lareger of the previous size - before the resize
			effect of adding this tool to the dock - of any other tools
			in the same dock.
		*/
		QSize
			max_size (tool->sizeHint ());
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    sizeHint post = " << tool->sizeHint () << endl;
		#endif
		if (Navigator->isVisible () &&
			! Navigator->isWindow () &&
			dockWidgetArea (Navigator) == dock_area)
			{
			if (Navigator == tool)
				{
				#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
				clog << "    Navigator previous_size = "
						<< Navigator->previous_size () << endl;
				#endif
				max_size.rwidth () =
					qMax (max_size.rwidth (),
						Navigator->previous_size ().width ());
				}
			else
				{
				#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
				clog << "             Navigator size = "
						<< Navigator->previous_size () << endl;
				#endif
				max_size.rwidth () =
					qMax (max_size.rwidth (),
						Navigator->width ());
				}
			}
		if (tool == Data_Mapper &&
			Data_Mapper->isVisible () &&
			! Data_Mapper->isWindow () &&
			dockWidgetArea (Data_Mapper) == dock_area)
			{
			#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
			clog << "    Data_Mapper previous_size = "
					<< Data_Mapper->previous_size () << endl;
			#endif
			max_size.rwidth () =
				qMax (max_size.rwidth (),
					Data_Mapper->previous_size ().width ());
			}
		if (tool == Statistics &&
			Statistics->isVisible () &&
			! Statistics->isWindow () &&
			dockWidgetArea (Statistics) == dock_area)
			{
			#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
			clog << "    Statistics previous_size = "
					<< Statistics->previous_size () << endl;
			#endif
			max_size.rwidth () =
				qMax (max_size.rwidth (),
					Statistics->previous_size ().width ());
			}
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
		clog << "    intended size = " << max_size << endl;
		#endif
		/*
		tool->widget ()->resize (max_size);
		tool->adjustSize ();
		*/
		tool->setMaximumSize (max_size);

		/*	Process the layout changes so the following will take effect.

			GUI updates are optimized by the event loop by coalescing
			sequences together. If the releasing of the maximum size
			constraints that follows the forced application of the
			constraints were done without the intervening GUI update
			actually occuring - i.e. not being optimized away - then the
			effect of the constraints on the GUI would be lost in the
			optimization. By forcing the event loop to process its
			contents before releasing the contraints with its GUI update
			put on the event loop, the two GUI updates should be done as
			separate, distinct, operations that will each have the
			desired effect on the GUI layout.
		*/
		qApp->processEvents ();
		qApp->sendPostedEvents ();
		qApp->flush ();		//	This may not be necessary.
		//	Release the tool to expand into the available space.
		tool->setMaximumSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		}
	}
if (tool == Navigator)
	Navigator_Fit = false;
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SLOTS))
clog << "<<< HiView_Window::resize_tool" << endl;
#endif
}


void
HiView_Window::tool_position ()
{
#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
clog << ">>> HiView_Window::tool_position" << endl;
#endif
QObject
	//	Expected to be a QAction of the Tool_Position_Menu.
	*source = sender ();

if (! Selected_Tool)
	{
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	clog << "    no Selected_Tool" << endl;
	#endif
	Selected_Tool = dynamic_cast<QDockWidget*>(QApplication::focusWidget ());
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	if (! Selected_Tool)
		clog << "    no tool with focus" << endl;
	#endif
	}

if (Selected_Tool)
	{
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	clog << "    Selected_Tool = " << (void*)Selected_Tool
	 	<< ' ' << Selected_Tool->windowTitle () << endl;
	#endif
	Qt::DockWidgetArea
		dock_area = dockWidgetArea (Selected_Tool);
	Qt::DockWidgetAreas
		allowed_areas = Selected_Tool->allowedAreas ();
	bool
		is_floating = Selected_Tool->isFloating ();
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	clog << "       isFloating = " << is_floating << endl
		 << "        dock_area = " << dock_areas_names (dock_area)
			<< " (" << dock_area << ')' << endl
		 << "    allowed_areas = " << dock_areas_names (allowed_areas)
			<< " (" << allowed_areas << ')' << endl;
	#endif
	if (source == Close_Position)
		{
		#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
		clog << "    Close_Position" << endl;
		#endif
		Selected_Tool->hide ();
		}
	else
	if (source == Floating_Position)
		{
		#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
		clog << "    Floating_Position - setFloating "
				<< (! is_floating) << endl;
		#endif
		Selected_Tool->setFloating (! is_floating);
		}
	else
		{
		Qt::DockWidgetArea
			selected_area = Qt::NoDockWidgetArea;
		if (source == Left_Position)
			{
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			clog << "    Left_Position" << endl;
			#endif
			selected_area = Qt::LeftDockWidgetArea;
			}
		else
		if (source == Right_Position)
			{
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			clog << "    Right_Position" << endl;
			#endif
			selected_area = Qt::RightDockWidgetArea;
			}
		else
		if (source == Top_Position)
			{
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			clog << "    Top_Position" << endl;
			#endif
			selected_area = Qt::TopDockWidgetArea;
			}
		else
		if (source == Bottom_Position)
			{
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			clog << "    Bottom_Position" << endl;
			#endif
			selected_area = Qt::BottomDockWidgetArea;
			}

		if (selected_area == dock_area)
			{
			if (is_floating)
				{
				#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
				clog << "    setFloating false" << endl;
				#endif
				Selected_Tool->setFloating (false);
				}
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			else
				clog << "      no change" << endl;
			#endif
			}
		else
			{
			if (selected_area & allowed_areas)
				addDockWidget (selected_area, Selected_Tool);
			#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
			else
				clog << "      no change" << endl;
			#endif
			}
		}
	Selected_Tool = NULL;
	}
#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
clog << "<<< HiView_Window::tool_position" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Navigator
*/
void
HiView_Window::create_navigator ()
{
#if ((DEBUG_SECTION) & (DEBUG_NAVIGATOR | DEBUG_INITIALIZE))
clog << ">>> HiView_Window::create_navigator" << endl;
#endif
Navigator = new Navigator_Tool (this);
Navigator->setVisible (false);
Navigator->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
Navigator->displayed_image_region_resized
	(round_down ((Image_View->displayed_image_region ()).size ()));
addDockWidget (Qt::LeftDockWidgetArea, Navigator);

//	Tool position actions.
Navigator->addActions (Tool_Position_Menu->actions ());
//	Context menu handling.
connect (Navigator,
	SIGNAL (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)),
	SLOT (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)));

//	View Navigator selection.
connect (Navigator,
			SIGNAL (visibilityChanged (bool)),
		 	SLOT (navigator_visibility_changed (bool)));
//	Dock area change.
connect (Navigator,
			SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
			SLOT (tool_location_changed ()));
connect (Navigator,
			SIGNAL (topLevelChanged (bool)),
			SLOT (tool_location_changed ()));

//	Image pixel value.
connect (Image_View,
			SIGNAL (image_pixel_value
				(const Plastic_Image::Triplet&,
				 const Plastic_Image::Triplet&)),
		 Navigator,
		 	SLOT (image_pixel_value
				(const Plastic_Image::Triplet&,
				 const Plastic_Image::Triplet&)));

//	Band map changed.
connect (Navigator,
			SIGNAL (bands_mapped (const unsigned int*)),
		 Image_View,
		 	SLOT (map_bands (const unsigned int*)));

//	Displayed band numbering changed.
connect (Preferences,
			SIGNAL (band_numbers_indexed_changed (bool)),
		 Navigator,
		 	SLOT (refresh_band_numbers ()));

//	Image cursor moved.
connect (Image_View,
			SIGNAL (image_cursor_moved (const QPoint&, const QPoint&)),
		 Navigator,
		 	SLOT (image_cursor_moved (const QPoint&, const QPoint&)));

//	Image origin moved.
connect (Image_View,
			SIGNAL (image_moved (const QPoint&, int)),
		 Navigator,
		 	SLOT (move_region (const QPoint&, int)));
connect (Navigator,
			SIGNAL (region_moved (const QPoint&, int)),
		 Image_View,
		 	SLOT (move_image (const QPoint&, int)));

//	Image display region size change.
connect (Image_View,
			SIGNAL (displayed_image_region_resized (const QSize&)),
		 Navigator,
		 	SLOT (displayed_image_region_resized (const QSize&)));
connect (Image_View,
			SIGNAL (display_viewport_resized (const QSize&)),
		 Navigator,
		 	SLOT (display_viewport_resized (const QSize&)));

//	Image scaling change.
connect (Image_View,
			SIGNAL (image_scaled (const QSizeF&, int)),
		 Navigator,
		 	SLOT (scale_image (const QSizeF&, int)));
connect (Navigator,
			SIGNAL (image_scaled (const QSizeF&, const QPoint&, int)),
		 Image_View,
		 	SLOT (scale_image (const QSizeF&, const QPoint&, int)));

#if ((DEBUG_SECTION) & (DEBUG_NAVIGATOR | DEBUG_INITIALIZE))
clog << "<<< HiView_Window::create_navigator" << endl;
#endif
}


void
HiView_Window::view_navigator
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_NAVIGATOR | DEBUG_SLOTS | DEBUG_MENUS))
clog << ">>> HiView_Window::view_navigator: " << enabled << endl;
#endif
if (View_Navigator_Action->isChecked () != enabled)
	//	Toggle the action, if it is enabled, which is connected here.
	View_Navigator_Action->setChecked (enabled);
else
if (View_Navigator_Action->isEnabled () &&
	Navigator)
	Navigator->setVisible (enabled);
#if ((DEBUG_SECTION) & (DEBUG_NAVIGATOR | DEBUG_SLOTS | DEBUG_MENUS))
clog << "<<< HiView_Window::view_navigator" << endl;
#endif
}


void
HiView_Window::navigator_visibility_changed
	(
	bool	visible
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_NAVIGATOR))
clog << ">>> HiView_Window::navigator_visibility_changed: " << visible << endl;
#endif
View_Navigator_Action->setChecked (visible);
if (visible)
	{
	make_way_for (Navigator);
	resize_tool (Navigator);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_NAVIGATOR))
clog << "<<< HiView_Window::navigator_visibility_changed" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Statistics
*/
void
HiView_Window::create_statistics_panels ()
{
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_STATISTICS))
clog << ">>> HiView_Window::create_statistics_panels" << endl;
#endif
Statistics = new Statistics_Tools (this);
Statistics->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
Statistics->setAllowedAreas (Qt::AllDockWidgetAreas);
Statistics->setVisible (false);
addDockWidget (Qt::BottomDockWidgetArea, Statistics);

//	Tool position actions.
Statistics->addActions (Tool_Position_Menu->actions ());
//	Context menu handling.
connect (Statistics,
	SIGNAL (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)),
	SLOT (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)));

//	Preferences parameters.
Statistics->source_statistics ()
	->canvas_color (Preferences->canvas_color ());
connect (Preferences,
			SIGNAL (canvas_color_changed (QRgb)),
		 Statistics->source_statistics (),
		 	SLOT (canvas_color (QRgb)));
Statistics->display_statistics ()
	->canvas_color (Preferences->canvas_color ());
connect (Preferences,
			SIGNAL (canvas_color_changed (QRgb)),
		 Statistics->display_statistics (),
		 	SLOT (canvas_color (QRgb)));
Statistics->source_statistics ()
	->selection_distance (Preferences->selection_sensitivity ());
connect (Preferences,
			SIGNAL (selection_sensitivity_changed (int)),
		 Statistics->source_statistics (),
		 	SLOT (selection_distance (int)));
connect (Preferences,
			SIGNAL (band_numbers_indexed_changed (bool)),
		 Statistics->source_statistics (),
		 	SLOT (refresh_band_numbers ()));
connect (Preferences,
			SIGNAL (band_numbers_indexed_changed (bool)),
		 Statistics->display_statistics (),
		 	SLOT (refresh_band_numbers ()));

//	View Statistics selection.
connect (Statistics,
			SIGNAL (visibilityChanged (bool)),
		 	SLOT (statistics_visibility_changed (bool)));
connect (Statistics,
			SIGNAL (section_changed (int)),
		 	SLOT (statistics_section_changed (int)));
//	Dock area change.
connect (Statistics,
			SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
			SLOT (tool_location_changed ()));
connect (Statistics,
			SIGNAL (topLevelChanged (bool)),
			SLOT (tool_location_changed ()));

if (Navigator)
	//	Navigator band mapping.
	connect (Navigator,
				SIGNAL (bands_mapped (const unsigned int*)),
			 Statistics->source_statistics (),
			 	SLOT (band_map (const unsigned int*)));

#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_STATISTICS))
clog << "<<< HiView_Window::create_statistics_panels" << endl;
#endif
}


void
HiView_Window::view_statistics
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS | DEBUG_MENUS))
clog << ">>> HiView_Window::view_statistics: " << enabled << endl;
#endif
if (View_Statistics_Action->isChecked () != enabled)
	//	Toggle the action, if it is enabled, which is connected here.
	View_Statistics_Action->setChecked (enabled);
else
if (View_Statistics_Action->isEnabled () &&
	Statistics)
	Statistics->setVisible (enabled);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS | DEBUG_MENUS))
clog << "<<< HiView_Window::view_statistics" << endl;
#endif
}


void
HiView_Window::statistics_visibility_changed
	(
	bool	visible
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS))
clog << ">-< HiView_Window::statistics_visibility_changed: " << visible << endl;
#endif
View_Statistics_Action->setChecked (visible);
if (visible)
	{
	make_way_for (Statistics);
	resize_tool (Statistics);
	}
else
	reset_selected_region ();
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS))
clog << "<<< HiView_Window::statistics_visibility_changed" << endl;
#endif
}


void
HiView_Window::statistics_section_changed
	(
	int		section_index
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS))
clog << ">-< HiView_Window::statistics_section_changed: " << section_index << endl;
#endif
if (section_index == Statistics_Tools::DISPLAY_STATISTICS_INDEX)
	refresh_display_statistics ();
}


bool
HiView_Window::statistics_are_visible () const
{return Statistics && Statistics->isVisible ();}


void
HiView_Window::refresh_statistics ()
{
#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << ">>> HiView_Window::refresh_statistics" << endl;
#endif
if (! (Statistics_Refresh_Needed = ! refresh_source_statistics ()))
	refresh_display_statistics ();
#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << "<<< HiView_Window::refresh_statistics" << endl;
#endif
}


bool
HiView_Window::refresh_source_statistics ()
{
#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << ">>> HiView_Window::refresh_source_statistics" << endl;
#endif
bool
	refreshed = false;

if (Statistics)
{
	QRect selected_region (selected_source_region ());
	#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
	clog << "    displayed_image_region = "
			<< Image_View->displayed_image_region () << endl
		 << "    selected_source_region = " << selected_region << endl;
	#endif
	if (selected_region.intersects
			(round_down (Image_View->displayed_image_region ())) &&
		//	Refresh the histograms data.
		Image_View->source_data_histograms
			(Statistics->source_statistics ()
				->stats ().histograms (), selected_region))
	{
		#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
		clog << "    refresh source_statistics" << endl;
		#endif
		refreshed = Statistics->source_statistics ()->refresh (selected_region);
	}

	if(refreshed) {
		//send data to engine
		if(!Selected_Image_Region.isEmpty()) {
			Image_Info->update_region_stats();
//			Image_Info->set_avgR(Statistics->source_statistics()->stats().mean_value(0));
//			Image_Info->set_avgG(Statistics->source_statistics()->stats().mean_value(1));
//			Image_Info->set_avgB(Statistics->source_statistics()->stats().mean_value(2));
//			Image_Info->set_property_f("red", Statistics->source_statistics()->stats().mean_value_excluding_exceptions(0));
//			Image_Info->set_property_f("green", Statistics->source_statistics()->stats().mean_value_excluding_exceptions(1));
//			Image_Info->set_property_f("blue", Statistics->source_statistics()->stats().mean_value_excluding_exceptions(2));
		}
		Image_Info->set_property("region_area_px", (unsigned long long)selected_region.width()*selected_region.height());
		Image_Info->set_property("region_width_px", (unsigned int)selected_region.width());
		Image_Info->set_property("region_height_px", (unsigned int)selected_region.height());
		Image_Info->evaluate_script();
	}
}


#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << "<<< HiView_Window::refresh_source_statistics: " << refreshed << endl;
#endif
return refreshed;
}


void
HiView_Window::refresh_display_statistics ()
{
#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << ">>> HiView_Window::refresh_display_statistics" << endl;
#endif
if (Statistics &&
	Statistics->display_statistics ()->isVisible ())
	{
	QRect
		selected_region (selected_display_region ());
	#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
	clog << "       image_display_region = "
			<< Image_View->image_display_region () << endl
		 << "    selected_display_region = " << selected_region << endl;
	#endif
	if (selected_region.intersects (Image_View->image_display_region ()) &&
		//	Refresh the histograms data.
		Image_View->display_data_histograms
			(Statistics->display_statistics ()
				->stats().histograms (), selected_region))
		{
		#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
		clog << "    refresh display_statistics" << endl;
		#endif
		Statistics->display_statistics ()->refresh (selected_region);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_STATISTICS)
clog << "<<< HiView_Window::refresh_display_statistics" << endl;
#endif
}


QRect
HiView_Window::selected_source_region () const
{
if (Selected_Image_Region.isEmpty ())
	{
	if (Selection_Modification)
		//	Selection being modified is empty.
		return QRect ();
	else
		return round_down (Image_View->displayed_image_region ());
	}
return round_down (Selected_Image_Region);
}


QRect
HiView_Window::selected_display_region () const
{
if (Selected_Image_Region.isEmpty ())
	{
	if (Selection_Modification)
		//	Selection being modified is empty.
		return QRect ();
	else
		return Image_View->image_display_region ();
	}
return Image_View->map_image_to_display (Selected_Image_Region);
}

/*------------------------------------------------------------------------------
	Data_Mapper
*/
void
HiView_Window::create_data_mapper ()
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPPER | DEBUG_INITIALIZE))
clog << ">>> HiView_Window::create_data_mapper" << endl;
#endif
//	Used in lieu of source image data maps.
int
	amount = MAX_DISPLAY_VALUE + 1;
Data_Maps = new Data_Map*[3];
Data_Maps[0] = new Data_Map (amount);
Data_Maps[1] = new Data_Map (amount);
Data_Maps[2] = new Data_Map (amount);

Data_Mapper = new Data_Mapper_Tool (Data_Maps, this);
Data_Mapper->setVisible (false);
Data_Mapper->visible_graph (false);
Data_Mapper->setAllowedAreas (Qt::AllDockWidgetAreas);
Qt::DockWidgetArea
	dock_area = Qt::LeftDockWidgetArea;
if (Navigator &&
	(Navigator->sizeHint ().height () + Data_Mapper->sizeHint ().height () + 20)
		> qApp->desktop ()->availableGeometry ().height ())
	{
	dock_area = Qt::RightDockWidgetArea;
	//	Fit Statistics below and to right of Data_Mapper.
	setCorner (Qt::BottomRightCorner,Qt::BottomDockWidgetArea);
	}
addDockWidget (dock_area, Data_Mapper);

//	Tool position actions.
Data_Mapper->addActions (Tool_Position_Menu->actions ());
//	Context menu handling.
connect (Data_Mapper,
	SIGNAL (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)),
	SLOT (tool_context_menu_requested (QDockWidget*, QContextMenuEvent*)));

//	Preferences parameters.
Data_Mapper->canvas_color (Preferences->canvas_color ());
connect (Preferences,
			SIGNAL (canvas_color_changed (QRgb)),
		 Data_Mapper,
		 	SLOT (canvas_color (QRgb)));
Data_Mapper->selection_distance (Preferences->selection_sensitivity ());
connect (Preferences,
			SIGNAL (selection_sensitivity_changed (int)),
		 Data_Mapper,
		 	SLOT (selection_distance (int)));
connect (Preferences,
			SIGNAL (band_numbers_indexed_changed (bool)),
		 Data_Mapper,
		 	SLOT (refresh_band_numbers ()));
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	Data_Mapper->upper_default_contrast_stretch
		(Preferences->contrast_stretch_upper (band), band);
	Data_Mapper->lower_default_contrast_stretch
		(Preferences->contrast_stretch_lower (band), band);
	}
connect (Preferences,
			SIGNAL (contrast_stretch_upper_changed (double, int)),
		 Data_Mapper,
		 	SLOT (upper_default_contrast_stretch (double, int)));
connect (Preferences,
			SIGNAL (contrast_stretch_lower_changed (double, int)),
		 Data_Mapper,
		 	SLOT (lower_default_contrast_stretch (double, int)));

//	View Data_Mapper selection.
connect (Data_Mapper,
			SIGNAL (visibilityChanged (bool)),
		 	SLOT (data_mapper_visibility_changed (bool)));
//	Dock area change.
connect (Data_Mapper,
			SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
			SLOT (tool_location_changed ()));
connect (Data_Mapper,
			SIGNAL (topLevelChanged (bool)),
			SLOT (tool_location_changed ()));

if (Navigator)
	//	Navigator band mapping.
	connect (Navigator,
				SIGNAL (bands_mapped (const unsigned int*)),
			 Data_Mapper,
			 	SLOT (band_map (const unsigned int*)));

//	Persistent settings.
QSettings
	settings;
bool
	OK;

//	Initialize the saturation bound percent values.
QVector<double>
	upper (3, -1.0),
	lower (3, -1.0);
QList<QVariant>
	values;
if (settings.contains ("Saturation_Upper_Bound_Percents"))
	values = settings.value ("Saturation_Upper_Bound_Percents").toList ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (band < values.count ())
		{
		upper[band] = values[band].toDouble (&OK);
		if (! OK)
			{
			QMessageBox::warning ((isVisible () ? this : NULL),
				tr ("HiView Configuration"),
				tr ("The ") + "Saturation_Upper_Bound_Percents["
				+ QString::number (band) + "] \""
				+ values[band].toString ()
				+ tr ("\" value is invalid - a number is required.\n\n")
				+ tr ("This value is ignored."));
			upper[band] = -1.0;
			}
		}
	}
if (settings.contains ("Saturation_Lower_Bound_Percents"))
	values = settings.value ("Saturation_Lower_Bound_Percents").toList ();
else
	values.clear ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (band < values.count ())
		{
		lower[band] = values[band].toDouble (&OK);
		if (! OK)
			{
			QMessageBox::warning ((isVisible () ? this : NULL),
				tr ("HiView Configuration"),
				tr ("The ") + "Saturation_Lower_Bound_Percents["
				+ QString::number (band) + "] \""
				+ values[band].toString ()
				+ tr ("\" value is invalid - a number is required.\n\n")
				+ tr ("This value is ignored."));
			lower[band] = -1.0;
			}
		}
	}
Data_Mapper->bound_percents (upper, lower);

//	Initialize the Last CSV file.
Data_Mapper->last_CSV_file (settings.value ("Last_CSV_File",
	QString ()).toString ());

if (Statistics)
	{
	connect (Data_Mapper,
				SIGNAL (selected_bands_changed (int)),
			 Statistics->source_statistics (),
			 	SLOT (bands_selected (int)));
	connect (Data_Mapper,
				SIGNAL (upper_bound_values_changed
					(const QVector<int>&)),
			 Statistics->source_statistics (),
			 	SLOT (upper_bound_values
					(const QVector<int>&)));
	connect (Data_Mapper,
				SIGNAL (lower_bound_values_changed
					(const QVector<int>&)),
			 Statistics->source_statistics (),
			 	SLOT (lower_bound_values
					(const QVector<int>&)));
	connect (Data_Mapper,
				SIGNAL (upper_bound_percents_changed
					(const QVector<double>&)),
			 Statistics->source_statistics (),
			 	SLOT (upper_bound_percents
					(const QVector<double>&)));
	connect (Data_Mapper,
				SIGNAL (lower_bound_percents_changed
					(const QVector<double>&)),
			 Statistics->source_statistics (),
			 	SLOT (lower_bound_percents
					(const QVector<double>&)));

	//	And back atcha.
	connect (Statistics->source_statistics (),
				SIGNAL (upper_bound_values_changed
					(const QVector<int>&)),
			 Data_Mapper,
			 	SLOT (upper_bound_values
					(const QVector<int>&)));
	connect (Statistics->source_statistics (),
				SIGNAL (lower_bound_values_changed
					(const QVector<int>&)),
			 Data_Mapper,
			 	SLOT (lower_bound_values
					(const QVector<int>&)));
	connect (Statistics->source_statistics (),
				SIGNAL (upper_bound_percents_changed
					(const QVector<double>&)),
			 Data_Mapper,
			 	SLOT (actual_upper_bound_percents
					(const QVector<double>&)));
	connect (Statistics->source_statistics (),
				SIGNAL (lower_bound_percents_changed
					(const QVector<double>&)),
			 Data_Mapper,
			 	SLOT (actual_lower_bound_percents
					(const QVector<double>&)));
	connect (Statistics->source_statistics (),
				SIGNAL (upper_limit_changed (int)),
			 Data_Mapper,
			 	SLOT (upper_limit (int)));
	connect (Statistics->source_statistics (),
				SIGNAL (lower_limit_changed (int)),
			 Data_Mapper,
			 	SLOT (lower_limit (int)));

	//	Initialize the statistics limits.
	int
		upper_limit,
		lower_limit;
	upper_limit =
		settings.value ("Statistics_Upper_Limit_Offset",
			Statistics->source_statistics ()
				->stats ().upper_limit ()).toInt (&OK);
	if (! OK)
		{
		QMessageBox::warning ((isVisible () ? this : NULL),
			tr ("HiView Configuration"),
			tr ("The Statistics_Upper_Limit_Offset \"")
			+ settings.value ("Statistics_Upper_Limit_Offset").toString ()
			+ tr ("\" value is invalid - a number is required.\n\n")
			+ tr ("The default value of 0 is being used."));
		upper_limit = 0;
		}
	lower_limit =
		settings.value ("Statistics_Lower_Limit_Offset",
			Statistics->source_statistics ()
				->stats ().lower_limit ()).toInt (&OK);
	if (! OK)
		{
		QMessageBox::warning ((isVisible () ? this : NULL),
			tr ("HiView Configuration"),
			tr ("The Statistics_Lower_Limit_Offset \"")
			+ settings.value ("Statistics_Lower_Limit_Offset").toString ()
			+ tr ("\" value is invalid - a number is required.\n\n")
			+ tr ("The default value of 0 is being used."));
		lower_limit = 0;
		}
	Statistics->source_statistics ()
		->stats ().limits (lower_limit, upper_limit);
	}

//	Add the Data Map menus.
QMenu
	*menu;
QList<QAction*>
	actions;
int
	index;

menu = Data_Map_Menu->addMenu (tr ("&Band Selections"));
actions = Data_Mapper->band_selection_actions ();
for (index = 0;
	 index < actions.size ();
	 index++)
	menu->addAction (actions.at (index));

menu = Data_Map_Menu->addMenu (tr ("&Presets"));
actions = Data_Mapper->presets_actions ();
for (index = 0;
	 index < actions.size ();
	 index++)
	menu->addAction (actions.at (index));

menu = Data_Map_Menu->addMenu (tr ("&File"));
actions = Data_Mapper->file_actions ();
for (index = 0;
	 index < actions.size ();
	 index++)
	menu->addAction (actions.at (index));

Data_Map_Menu->addAction (Data_Mapper->default_contrast_stretch_action ());
Data_Map_Menu->addAction (Data_Mapper->restore_original_contrast_stretch_action());
Data_Map_Menu->addAction (Data_Mapper->apply_percents_action ());
Data_Map_Menu->addAction (Data_Mapper->percents_actual_to_settings_action ());

//	Done last to avoid unnecessary signal handling.
connect (Data_Mapper,
			SIGNAL (data_maps_changed (Data_Map**)),
		 Image_View,
		 	SLOT (map_data (Data_Map**)));

#if ((DEBUG_SECTION) & (DEBUG_DATA_MAPPER | DEBUG_INITIALIZE))
clog << "<<< HiView_Window::create_data_mapper" << endl;
#endif
}


void
HiView_Window::view_data_mapper
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPPER | DEBUG_MENUS))
clog << ">>> HiView_Window::view_data_mapper: " << enabled << endl;
#endif
if (View_Data_Mapper_Action->isChecked () != enabled)
	//	Toggle the action, if it is enabled, which is connected here.
	View_Data_Mapper_Action->setChecked (enabled);
else
if (View_Data_Mapper_Action->isEnabled () &&
	Data_Mapper)
	Data_Mapper->setVisible (enabled);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPPER | DEBUG_MENUS))
clog << "<<< HiView_Window::view_data_mapper" << endl;
#endif
}


void
HiView_Window::data_mapper_visibility_changed
	(
	bool	visible
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPPER))
clog << ">>> HiView_Window::data_mapper_visibility_changed: "
		<< visible << endl;
#endif
View_Data_Mapper_Action->setChecked (visible);
if (visible)
	{
	make_way_for (Data_Mapper);
	resize_tool (Data_Mapper);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_DATA_MAPPER))
clog << "<<< HiView_Window::data_mapper_visibility_changed" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Toolbar
*/
void
HiView_Window::create_toolbar ()
{
/*	HiView Toolbar.

	Some users have been asking for even more convenient access to the
	Apply Percents action. A top level menu item with a keyboard shortcut
	does not seem to satsify them; they want a button at the top of the
	application that they can press. A toolbar would provide this at the
	expense of more screen space consumed, the need to popluate it with
	more than one button, and the management (hide/show at the least)
	that would be needed. At this point it seems silly and superfluous,
	but that may change ....

QToolBar
	*toolbar = new QToolBar (this);
toolbar->setToolButtonStyle (Qt::ToolButtonIconOnly);
QToolButton
	*button;
if (Data_Mapper)
	{
	QList<QAction*>
		actions = Data_mapper->band_selection_actions ();
	for (int
			index = 0;
			index < actions.size ();
			index++)
		{
		button = new QToolButton (toolbar);
		button->addAction (actions.at (index));
		toolbar->addWidget (button);
		}

	button = new QToolButton (toolbar);
	button->addAction (Data_Mapper->default_contrast_stretch_action ());
	toolbar->addWidget (button);

	button = new QToolButton (toolbar);
	button->addAction (Data_Mapper->apply_percents_action ());
	toolbar->addWidget (button);
	}
addToolBar (toolbar);
*/
}

/*==============================================================================
	Image Viewer
*/
void
HiView_Window::create_image_viewer ()
{
#if ((DEBUG_SECTION) & DEBUG_IMAGE_VIEWER)
clog << ">>> HiView_Window::create_image_viewer" << endl;
#endif
Image_View = new Image_Viewer (this);

Image_View->setMinimumSize (IMAGE_DISPLAY_MIN_WIDTH, IMAGE_DISPLAY_MIN_HEIGHT);
#if ((DEBUG_SECTION) & (DEBUG_IMAGE_VIEWER | DEBUG_LAYOUT))
clog << "    minimum Image_Viewer size = "
		<< Image_View->minimumSize () << endl;
#endif
Image_View->source_image_rendering (true);	//	Source image renderer.
Image_View->default_source_image_rendering (false);

#if ((DEBUG_SECTION) & DEBUG_IMAGE_VIEWER)
clog << "    add scale menu items from Image_Viewer" << endl;
#endif
//	Add the scale menu actions from the Image_Viewer to the main View menu.
QList<QAction*>
	scale_actions (Image_View->scale_menu_actions ());
/*
// Add copy coordinates action from the Image_Viewer to the main File menu.
File_Menu->addAction (Image_View->copy_coordinates_action());
*/

for (int
		index = 0;
		index < scale_actions.size ();
		index++)
	Scale_Menu->addAction (scale_actions.at (index));

//	Connect the Preferences_Dialog to the Image_View.
Image_View->min_scale (Preferences->min_scale ());
connect (Preferences,
			SIGNAL (min_scale_changed (double)),
		 Image_View,
		 	SLOT (min_scale (double)));
Image_View->max_scale (Preferences->max_scale ());
connect (Preferences,
			SIGNAL (max_scale_changed (double)),
		 Image_View,
		 	SLOT (max_scale (double)));
Image_View->scaling_minor_increment (Preferences->scaling_minor_increment ());
connect (Preferences,
			SIGNAL (scaling_minor_increment_changed (double)),
		 Image_View,
		 	SLOT (scaling_minor_increment (double)));
Image_View->scaling_major_increment (Preferences->scaling_major_increment ());
connect (Preferences,
			SIGNAL (scaling_major_increment_changed (double)),
		 Image_View,
		 	SLOT (scaling_major_increment (double)));
Image_View->tile_size (Preferences->tile_size ());
connect (Preferences,
			SIGNAL (tile_size_changed (int)),
		 Image_View,
		 	SLOT (tile_size (int)));
Image_View->rendering_increment_lines
	(Preferences->rendering_increment_lines ());
connect (Preferences,
			SIGNAL (rendering_increment_lines_changed (int)),
		 Image_View,
		 	SLOT (rendering_increment_lines (int)));
Image_View->background_color (Preferences->background_color ());
connect (Preferences,
			SIGNAL (background_color_changed (QRgb)),
		 Image_View,
		 	SLOT (background_color (QRgb)));
connect (Preferences,
			SIGNAL (line_color_changed (const QColor &)),
			SLOT (line_color(const QColor &)));
Image_View->JPIP_proxy (Preferences->JPIP_proxy ());
connect (Preferences,
			SIGNAL (JPIP_proxy_changed (const QString&)),
		 Image_View,
		 	SLOT (JPIP_proxy (const QString&)));
Image_View->JPIP_cache_directory (Preferences->JPIP_cache_directory ());
connect (Preferences,
			SIGNAL (JPIP_cache_directory_changed (const QString&)),
		 Image_View,
		 	SLOT (JPIP_cache_directory (const QString&)));
Image_View->JPIP_request_timeout (Preferences->JPIP_request_timeout ());
connect (Preferences,
			SIGNAL (JPIP_request_timeout_changed (int)),
		 Image_View,
		 	SLOT (JPIP_request_timeout (int)));
Image_View->max_source_image_area (Preferences->max_source_image_area_MB ());
connect (Preferences,
			SIGNAL (max_source_image_area_MB_changed (int)),
		 Image_View,
		 	SLOT (max_source_image_area (int)));

//	Watch for image loaded.
connect (Image_View,
			SIGNAL (image_loaded (bool)),
		 	SLOT (image_loaded (bool)));

//	Track the image cursor location for image region selection modification.
connect (Image_View,
			SIGNAL (image_cursor_moved (const QPoint&, const QPoint&)),
		 	SLOT (image_cursor_moved (const QPoint&, const QPoint&)));

//	Watch for changes that affect image statistics.
connect (Image_View,
			SIGNAL (image_moved (const QPoint&, int)),
		 	SLOT (image_moved (const QPoint&, int)));
connect (Image_View,
			SIGNAL (displayed_image_region_resized (const QSize&)),
		 	SLOT (displayed_image_region_resized (const QSize&)));
connect (Image_View,
			SIGNAL (state_change (int)),
		 	SLOT (image_viewer_state_change (int)));
/*
//	Track rendering status.
	This might be useful for implementing a progress meter.
connect (Image_View,
			SIGNAL (rendering_status (int)),
		 	SLOT (rendering_status (int)));
*/

if (Image_Info)
	{
	connect (Image_View,
				SIGNAL (image_cursor_moved (const QPoint&, const QPoint&)),
			 Image_Info,
		 		SLOT (cursor_location (const QPoint&, const QPoint&)));
	connect (Image_View,
				SIGNAL (image_pixel_value
					(const Plastic_Image::Triplet&,
					 const Plastic_Image::Triplet&)),
			 Image_Info,
		 		SLOT (pixel_value
					(const Plastic_Image::Triplet&,
					 const Plastic_Image::Triplet&)));
	connect (Image_View,
				SIGNAL (image_scaled (const QSizeF&, int)),
			 Image_Info,
		 		SLOT (image_scale (const QSizeF&, int)));
	Image_Info->image_scale (QSizeF (1, 1));
	}

if (Image_Activity_Indicator)
	connect (Image_Activity_Indicator,
				SIGNAL (button_clicked (int)),
		 		SLOT (activity_indicator_clicked (int)));
#if ((DEBUG_SECTION) & DEBUG_IMAGE_VIEWER)
clog << "<<< HiView_Window::create_image_viewer" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Image loading
*/
bool
HiView_Window::load_initial_source ()
{
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_LOAD_IMAGE))
clog << ">>> HiView_Window::load_initial_source" << endl
	 << "    Preferences restore_last_source = "
	 	<< Preferences->restore_last_source () << endl
	 << "    Initial_Source = \"" << Initial_Source << '"' << endl
	 << "    Startup_Stage set to STARTUP_COMPLETE ("
	 	<< STARTUP_COMPLETE << ')' << endl;
#endif
//	Let load_image know that Initial_Scaling can be reset.
Startup_Stage = STARTUP_COMPLETE;
bool
	loading = false;
if (/*Preferences->restore_last_source () &&*/
	! Initial_Source.isEmpty ())
{
	if (URL_source (Initial_Source) &&
		! JPIP_source (Initial_Source) &&
		/*
			The is_URL utility function uses JP2::is_valid_URL which
			requires the presence of a path segment, but an HTTP URL may
			not have a path segment (and still access a default source).
			So a simple HTTP URL syntax test is applied here.
		*/
		(Initial_Source.startsWith ("http://", Qt::CaseInsensitive)||Initial_Source.startsWith ("https://", Qt::CaseInsensitive) ))
	{
		//	HTTP URL.
		#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
		clog << "    load_URL Initial_Source = \"" << Initial_Source << '"'
				<< endl;
		#endif
		/*
			Loading will be done asynchronously to first obtain a QImage
			via network services which will then be handed to load_image
			(const QImage&, const QSizeF). There the Initial_Scaling is
			first used and then reset so it won't be used again.
		*/
		loading = true;
		load_URL (Initial_Source);
	}
	else
	{
		#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
		clog << "    load_image -" << endl
			 << "      Initial_Source = \"" << Initial_Source << '"' << endl
			 << "      Initial_Scaling = " << Initial_Scaling << endl;
		#endif
		QSizeF
			scaling (Initial_Scaling);
		//	Reset Initial_Scaling.
		Initial_Scaling.rwidth () =
		Initial_Scaling.rheight () = -1.0;

		loading = load_image (Initial_Source, scaling);
	}
}
else
{
	#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_LOAD_IMAGE))
	clog << "    Initial_Source not restored" << endl;
	#endif
	//	Re-enable image loading actions.
	image_load_actions (true);
	}
#if ((DEBUG_SECTION) & (DEBUG_INITIALIZE | DEBUG_LOAD_IMAGE))
clog << "<<< HiView_Window::load_initial_source: " << loading << endl;
#endif
return loading;
}


void
HiView_Window::open
	(
	const QString&	source_name
	)
{
if (source_name.isEmpty ())
	return;
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << ">>> HiView_Window::open: " << source_name << endl;
#endif
if (Startup_Stage)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
					DEBUG_DROP_IMAGE | \
					DEBUG_SLOTS | \
					DEBUG_OVERVIEW))
	clog << "    Startup_Stage = " << Startup_Stage << endl
		 << "<<< HiView_Window::open" << endl;
	#endif
	Initial_Source = source_name;
	return;
	}

/*	Interpretation of the source_name.

	A source name string is the name of a source of image data. Image
	data may be loaded from a local file via the host filesystem or from
	a remote file via a network protocol.

	A local file is named by its host filesystem pathname. As an
	accelerator feature, a relative pathname has its first segment
	checked (URL_source) to see if it is a known hostname or IP address;
	if it is the "http://" protocol specification is prepended. Also, a
	pathname that begins with the '~' character has this character
	replaced with the user's home directory pathname.

	A remote file is named by a URL. A URL that specifies the "file"
	protocol is converted to a local host filesystem pathname. A URL that
	specifies the "jpip" protocol will cause the remote file to be
	accessed via a JPIP client-server session connection. A URL that
	specifies the "http" protocol will be scanned for the presence of a
	pathname ending in "jp2" in the path segment or a value section of
	the query segment; if this is found the URL is converted to use the
	"jpip" protocol, using the hostname and port number from the
	Preferences if they have been defined and the JP2 source path that
	was found.

	An HTTP URL that is not converted to a JPIP URL is assumed to be a
	reference to a non-JP2 image source. In this case a
	QNetworkAccessManager get request is posted for the URL and when the
	request is finished an attempt is made to load the data into a QImage
	for viewing.
*/
QString
	name (source_name);
if (URL_source (name) &&
	! JPIP_source (name) &&
	/*
		The is_URL utility function uses JP2::is_valid_URL which requires
		the presence of a path segment, but an HTTP URL may not have a
		path segment (and still access a default source). So a simple
		HTTP URL syntax test is applied here.
	*/
	(name.startsWith ("http://", Qt::CaseInsensitive)||name.startsWith ("https://", Qt::CaseInsensitive)))
	//	HTTP URL.
	load_URL (name);
else
	{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
					DEBUG_DROP_IMAGE | \
					DEBUG_SLOTS | \
					DEBUG_OVERVIEW))
	clog << "    using source name " << name << endl;
	#endif

        if (! JPIP_source (name) && HiView_Application::is_jpip_passthru_link(name))
        {
            QString Requested_Link = HiView_Application::parse_jpip_passthru_link(name);
            if (! Requested_Link.isEmpty() ) name = Requested_Link;
        }

	if (name != source_name)
		/*	Remove the user entered value that was changed.
			The revised name will be added if the image is successfully loaded.
		*/
		remove_source_selection (source_name);

	load_image (name);
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << "<<< HiView_Window::open" << endl;
#endif
}


void
HiView_Window::open_file ()
{
#if ((DEBUG_SECTION) & (DEBUG_OPEN_FILE | DEBUG_MENUS))
clog << ">>> HiView_Window::open_file" << endl;
#endif
if (! Open_File_Dialog)
	{
	QString
		file_filters ("All Files (*);; ");
	file_filters += HiView_Utilities::image_reader_formats_file_filters ();

	/*	>>> Qt implementation used instead of native implementation <<<

		The Mac OS X native file dialog does not always allow links to files
		to be opened: the Open button remains disabled, and the display of the
		link is often incorrect. Though the native dialog is to be preferred,
		the strange and inconsistent behavior with links makes it unacceptable
		for use with HiView.
	*/
	Open_File_Dialog = new QFileDialog (this, tr ("Open Image File"),
		Source_Directory, file_filters);
	Open_File_Dialog->setAcceptMode (QFileDialog::AcceptOpen);
	Open_File_Dialog->setFileMode (QFileDialog::ExistingFile);
	Open_File_Dialog->setOptions
		(QFileDialog::DontUseNativeDialog |
		 QFileDialog::DontResolveSymlinks);
	}
if (Open_File_Dialog->exec ())
	{
	QString
		source_name (Open_File_Dialog->selectedFiles ().value (0));
	if (! source_name.isEmpty ())
		{
		Source_Directory = QFileInfo (source_name).absolutePath ();
                if (HiView_Application::is_jpip_passthru_link(source_name))
                {
                    QString Requested_Link = HiView_Application::parse_jpip_passthru_link(source_name);
                    if (! Requested_Link.isEmpty() ) source_name = Requested_Link;
                }

		load_image (source_name);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_OPEN_FILE | DEBUG_MENUS))
clog << "<<< HiView_Window::open_file" << endl;
#endif
}


void
HiView_Window::open_URL ()
{
Source_Selections->setFocus (Qt::OtherFocusReason);
Source_Selections->lineEdit ()->selectAll ();
}


bool
HiView_Window::load_image
	(
	const QString&	source_name,
	const QSizeF&	scaling
	)
{
//	N.B.: Massaging of the source_name is done by the open method.
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::load_image: " << source_name << endl
	 << "    scaling = " << scaling << endl));
#endif
if (source_name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	clog << "    empty source name" << endl
		 << "<<< HiView_Window::load_image: false" << endl;
	#endif
	return false;
	}

bool
	registered = false;
QSizeF
	image_scaling (scaling);

if (image_scaling.isEmpty () &&
	Preferences->initial_scale () != Preferences->INITIAL_SCALE_AUTO_FIT)
	{
	//	Use the Preferences setting for the initial scaling.
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    using Preferences initial_scale = "
			<< Preferences->initial_scale () << endl));
	#endif
	image_scaling.rwidth () =
	image_scaling.rheight () = Preferences->initial_scale ();
	}

if (image_scaling.isEmpty ())
	registered = load_image (source_name, QSize ());
else
	{
	Source_Name_Loading = source_name;
	load_image_start ();

	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_View request for image ----------------------------"
			<< endl));
	#endif
	registered = Image_View->image (source_name, image_scaling);
	if (! registered)
		load_image_failed ();
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::load_image: " << registered << endl));
#endif
return registered;
}


bool
HiView_Window::load_image
	(
	const QString&	source_name,
	const QSize&	display_size
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::load_image: " << source_name << endl
	 << "    display_size = " << display_size << endl));
#endif
if (source_name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	clog << "    empty source name" << endl
		 << "<<< HiView_Window::load_image: false" << endl;
	#endif
	return false;
	}
Source_Name_Loading = source_name;
load_image_start ();

QSize
	displayed_size (display_size);
if (displayed_size.isEmpty ())
	{
	displayed_size = Image_View->viewport_size ();
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    viewport_size = " << displayed_size << endl));
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    specified displayed size = " << displayed_size << endl
	 << "    Image_View request for image ----------------------------"
		<< endl));
#endif
bool
	registered = Image_View->image (source_name, displayed_size);
if (! registered)
	load_image_failed ();
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::load_image: " << registered << endl));
#endif
return registered;
}


bool
HiView_Window::load_image
	(
	const QImage&	source_image,
	const QSizeF&	scaling
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::load_image: QImage @ "
		<< (void*)&source_image << endl
	 << "    scaling = " << scaling << endl));
#endif
bool
	registered;
QSizeF
	image_scaling (scaling);
if (image_scaling.isEmpty ())
	{
	if (! Initial_Scaling.isEmpty () &&
		Startup_Stage == STARTUP_COMPLETE)
		{
		image_scaling = Initial_Scaling;
		#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
		LOCKED_LOGGING ((
		clog << "    using Initial_Scaling = " << Initial_Scaling << endl));
		#endif
		//	Reset Initial_Scaling so it won't be used again.
		Initial_Scaling.rwidth () =
		Initial_Scaling.rheight () = -1.0;
		}
	else
	if (Preferences->initial_scale () != Preferences->INITIAL_SCALE_AUTO_FIT)
		{
		//	Use the Preferences setting for the initial scaling.
		#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
		LOCKED_LOGGING ((
		clog << "    using Preferences initial_scale = "
				<< Preferences->initial_scale () << endl));
		#endif
		image_scaling.rwidth () =
		image_scaling.rheight () = Preferences->initial_scale ();
		}
	}

if (image_scaling.isEmpty ())
	registered = load_image (source_image, QSize ());
else
	{
	load_image_start ();

	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_View request for image ----------------------------"
			<< endl));
	#endif
	registered = Image_View->image (source_image, image_scaling);
	if (! registered)
		load_image_failed ();
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::load_image: " << registered << endl));
#endif
return registered;
}


bool
HiView_Window::load_image
	(
	const QImage&	source_image,
	const QSize&	display_size
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::load_image: QImage @ "
		<< (void*)&source_image << endl
	 << "                display_size = " << display_size << endl));
#endif
load_image_start ();

QSize
	displayed_size (display_size);
if (displayed_size.isEmpty ())
	displayed_size = Image_View->viewport_size ();
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    preferred displayed size = " << displayed_size << endl
	 << "    Image_View request for image ----------------------------"
		<< endl));
#endif
bool
	registered = Image_View->image (source_image, displayed_size);
if (! registered)
	load_image_failed ();
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::load_image: " << registered << endl));
#endif
return registered;
}


void
HiView_Window::load_URL
	(
	const QString&	source_name
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << ">>> HiView_Window::load_URL: " << source_name << endl;
#endif
Source_Name_Loading = source_name;
load_image_start ();

//	Get the network data, asynchronously.
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS))
clog << "    initiating network data get" << endl;
#endif
Network_Reply = Network_Access_Manager->get
	(QNetworkRequest (QUrl (source_name)));
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << "<<< HiView_Window::load_URL" << endl;
#endif
}


bool
HiView_Window::load_image
	(
	QNetworkReply*	network_reply
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << ">>> HiView_Window::load_image: from network data" << endl;
#endif
bool
	loaded = false;
if (network_reply->error ())
	load_image_failed (network_reply->errorString ());
else
	{
	QImage
		image;
	if (image.load (network_reply, 0))
		//	Pass the image on to the load sequence.
		loaded = load_image (image);
	else
		load_image_failed ();
	}
network_reply->deleteLater ();
Network_Reply = NULL;
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_DROP_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_OVERVIEW))
clog << "<<< HiView_Window::load_image: from network data - " << loaded << endl;
#endif
return loaded;
}


void
HiView_Window::image_loaded
	(
	bool	successful
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_INITIALIZE | \
				DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::image_loaded: " << successful << endl
	 << "    source \"" << Source_Name_Loading << '"' << endl));
#endif
if (successful)
{
	show_status_message (tr ("Loaded image: ") += Source_Name_Loading);

	/*	Set the name of the source as used here.

		This sets the fully qualified source name in the Image_Viewer
		and the short source name in the Source_Name.
	*/
	display_image_name (Source_Name_Loading);

	//	Reset the Image Info Panel.
	if (Image_Info)
	{
		Image_Info->image_bands (Image_View->image_bands ());
		Image_Info->image_scale (Image_View->image_scaling ());
	}

	//	Reset the Image Metadata Dialog.
	if (Image_Metadata_Dialog)
		reset_metadata ();

	//	Reset the Navigator Tool.
	if (Navigator)
	{
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Navigator image assignment  -----------------------------"
				<< endl
			 << "    " << *(Image_View->image ()) << endl));
		#endif
		Navigator->image (Image_View->image (), Source_Name);
		Navigator->displayed_image_region_resized
			(round_down ((Image_View->displayed_image_region ()).size ()));
		Navigator->updateGeometry ();
	}

	//	Reset the source Statistics Tool.
	reset_selected_region ();
	reset_line ();
	if (Statistics)
	{
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Statistics data bands - " << Image_View->image_bands ()
				<< ", precision - " << Image_View->image_data_precision ()
				<< endl));
		#endif
		Statistics->source_statistics ()->data_structure
			(Image_View->image_bands (), Image_View->image_data_precision ());
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Statistics assignment of Image_View band_map @ "
				<< (void*)Image_View->band_map () << endl));
		#endif
		Statistics->source_statistics ()->band_map
			(Image_View->band_map ());
	}

	/*	Reset the Data Mapper Tool.

		WARNING: Reset the Statistics Tools before the Data Mapper, to
		be sure the former is prepared to receive bounds and limits
		change signals from the latter.
	*/
	if (Data_Mapper)
	{
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Data_Mapper selected_bands SELECTED_ALL" << endl));
		#endif
		Data_Mapper->selected_bands (SELECTED_ALL);
		if (Data_Maps)
		{
			//	Replace the temporary maps with those from the source.
			delete Data_Maps[0];
			delete Data_Maps[1];
			delete Data_Maps[2];
			//delete Data_Maps; VALGRIND
			Data_Maps = NULL;
		}
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Data_Mapper assignment of Image_View data_maps @ "
				<< (void*)Image_View->data_maps () << endl));
		#endif
		Data_Mapper->data_maps (Image_View->data_maps ());
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
		LOCKED_LOGGING ((
		clog << "    Data_Mapper assignment of Image_View band_map @ "
				<< (void*)Image_View->band_map () << endl));
		#endif
		Data_Mapper->band_map (Image_View->band_map ());

		if (Statistics)
		{
			#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
			LOCKED_LOGGING ((
			clog << "    Data_Mapper assignment of Statistics limits offsets "
					<< Statistics->source_statistics ()->lower_limit_offset ()
					<< ", "
					<< Statistics->source_statistics ()->upper_limit_offset ()
					<< endl));
			#endif
			Data_Mapper->upper_limit
				(Statistics->source_statistics ()->upper_limit_offset ());
			Data_Mapper->lower_limit
				(Statistics->source_statistics ()->lower_limit_offset ());

			/*
			#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
			LOCKED_LOGGING ((
			clog << "    refresh_statistics" << endl));
			#endif
			refresh_statistics ();
			*/
		}
	} // end if Data_Mapper

	Source_Name_Loading.clear ();
	Image_View->setFocus (Qt::OtherFocusReason);
} // end if successful
else
{
	load_image_failed ();
	Source_Selections->setFocus (Qt::OtherFocusReason);
}

if (! isVisible ())
{
	//	Must show before GUI layout will be valid.
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS))
	clog << "    show ..." << endl;
	#endif
	show ();
}

if (successful &&
	(Startup_Stage ||
	 Auto_Resize_Action->isChecked ()))
	fit_window_to_image ();

if (Startup_Stage)
{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
					DEBUG_SLOTS | \
					DEBUG_INITIALIZE | \
					DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Startup_Stage = " << Startup_Stage << endl));
	#endif
	if (Image_Activity_Indicator)
		Image_Activity_Indicator->state (ACTIVITY_OFF);

	HiView_Application
		*application = dynamic_cast<HiView_Application*>(qApp);

	qApp->flush ();
	qApp->sendPostedEvents ();
	qApp->processEvents ();

	if (Initial_Source.isEmpty ())
	{
		//	No source specified on the command line.
		if (application)
			//	Try for a source passed via the application's FileOpen event.
			Initial_Source = application->Requested_Pathname;
		if (Initial_Source.isEmpty () &&
                        // TODO is this a bug or is it intended to happen only when restoring layout?
			/*Restore_Layout && */
                        Preferences->restore_last_source() &&
			Source_Selections->count ())
			//	Reload the last source used.
			Initial_Source = Source_Selections->itemText (0);
		}

	if (Initial_Source.isEmpty ())
	{
		//	Just the splash image.
		view_image_info  (false);
		view_navigator   (false);
		view_statistics  (false);
		view_data_mapper (false);
		view_status_bar  (false);
	}
	else
	{
		//	Hold the splash screen for a moment.
		if (Image_Activity_Indicator)
		{
			qApp->processEvents ();
			qApp->sendPostedEvents ();
			qApp->flush ();		//	This may not be necessary.
			Image_Activity_Indicator->start_delay (2);
			Image_Activity_Indicator->state_color
				(ACTIVITY_VISIBLE_RENDERING, Qt::yellow);
			Image_Activity_Indicator->state
				(ACTIVITY_VISIBLE_RENDERING);
		}
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
						DEBUG_SLOTS | \
						DEBUG_INITIALIZE | \
						DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    wait " << SPLASH_IMAGE_DWELL_MS << " ms ..." << endl));
		#endif
		//	Delay before continue_startup.
		QTimer::singleShot (SPLASH_IMAGE_DWELL_MS,
			this, SLOT (continue_startup ()));
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "<<< HiView_Window::image_loaded" << endl));
		#endif
		return;
	} // end else (Initial_source is not empty)
	//	Proceed directly to continue_startup.
	continue_startup ();
} // end if (startup stage)
else
	//	Re-enable image loading actions.
	image_load_actions (true);

if (Error_Message->isVisible ())
	Error_Message->raise ();

#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SLOTS | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::image_loaded" << endl));
#endif
}


void
HiView_Window::continue_startup ()
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_INITIALIZE))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::continue_startup" << endl));
#endif
if (Image_Activity_Indicator)
	{
	Image_Activity_Indicator->state (ACTIVITY_OFF);
	Image_Activity_Indicator->start_delay
		(Activity_Indicator::default_start_delay ());
	Image_Activity_Indicator->state_color
		(ACTIVITY_VISIBLE_RENDERING,
		 Activity_Indicator::default_state_color
		(ACTIVITY_VISIBLE_RENDERING));
	}

//	Possibly restore the previous layout.
restore_layout ();

#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_INITIALIZE))
LOCKED_LOGGING ((
clog << "    Startup_Stage set to STARTUP_SPLASH_SCALING ("
	 	<< STARTUP_SPLASH_SCALING << ')' << endl
	 << "    fit_image_to_window ..." << endl));
#endif
/*	Possibly scale the splash image to the restored layout viewport size.

	If scaling is initiated loading of the initial image is deferred
	until the scaling has completed, which will be detected in the
	state_change slot. The Startup_Stage is changed to
	STARTUP_SPLASH_SCALING so only the state change following the
	fit_image_to_window will be effective in triggering the
	load_initial_source.
*/
Startup_Stage = STARTUP_SPLASH_SCALING;
if (! Image_View->fit_image_to_window ())
	{
	/*	No image scaling started; proceed directly to load_initial_source.

		The Startup_Stage will be reset by load_initial_source.
	*/
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
					DEBUG_SLOTS | \
					DEBUG_INITIALIZE))
	LOCKED_LOGGING ((
	clog << "    no image scaling" << endl
		 << "    load_initial_source ..." << endl));
	#endif
	if (Preferences->restore_last_source() ) load_initial_source ();
	}

image_load_actions (true);
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_SLOTS | \
				DEBUG_INITIALIZE))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::continue_startup" << endl));
#endif
}


void
HiView_Window::load_image_start ()
{
//	Disable image loading actions while image loading is in progress.
image_load_actions (false);

if (Image_Activity_Indicator)
	Image_Activity_Indicator->state (ACTIVITY_VISIBLE_RENDERING);

if (Source_Name_Loading.isEmpty ())
	{
	//	Not a named image source.
	Source_Selections->clearEditText ();
	Source_Selections->setCurrentIndex (-1);
	}
else
	show_status_message (tr ("Loading image: ") += Source_Name_Loading);
}


void
HiView_Window::load_image_failed
	(
	const QString&	reason
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
clog << ">>> HiView_Window::load_image_failed:" << endl
	 << "    reason - " << reason << endl;
#endif
if (Image_Activity_Indicator)
	Image_Activity_Indicator->state (ACTIVITY_OFF);

QString
	report (tr ("Failed to load image"));
if (! Source_Name_Loading.isEmpty ())
	{
	//	A source name being loaded is known.
	report += ": ";
	report += Source_Name_Loading;

	//	Remove the source name from the source selections.
	remove_source_selection (Source_Name_Loading);

	//	Set the failed source name as editable text in the selection field.
	Source_Selections->setEditText (Source_Name_Loading);
	Source_Selections->lineEdit ()->selectAll ();
	}
else
	report += '.';

#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
clog << "    status message = " << report << endl;
#endif
show_status_message (report);

QString
	message (report);
if (! reason.isEmpty ())
	{
	message += '\n';
	message += reason;
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	clog << "    error message = " << message << endl;
	#endif
	Error_Message->showMessage (message.replace ("\n", "<br>"));
	}

Source_Name_Loading.clear ();
Startup_Stage = STARTUP_COMPLETE;
image_load_actions (true);
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
clog << "<<< HiView_Window::load_image_failed" << endl;
#endif
}


void
HiView_Window::image_load_actions
	(
	bool	enabled
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
clog << ">>> HiView_Window::image_load_actions: " << enabled <<endl;
#endif
Source_Selections->setEnabled (enabled);
Open_File_Action->setEnabled (enabled);
Open_URL_Action->setEnabled (enabled);
Image_Loading = ! enabled;
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
clog << "<<< HiView_Window::image_load_actions" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Image rendering status and display state
*/
void
HiView_Window::rendering_status
	(
	int		status
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_RENDERING_STATUS))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::rendering_status: " << status << " - "
		<< Image_View->rendering_status_description (status) << endl
	 << "    Image_View pending_state_change = "
	 	<< Image_View->pending_state_change () << " - "
		<< Image_View->state_change_description
			(Image_View->pending_state_change ()) << endl
	 << "    Image_Loading = " << Image_Loading << endl));
#endif
//	Placeholder.
++status;

#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_RENDERING_STATUS))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::rendering_status" << endl));
#endif
}


void
HiView_Window::image_viewer_state_change
	(
	int		state
	)
{
#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::image_viewer_state_change: " << state << " - "
		<< Image_Viewer::state_change_description (state) << endl
	 << "    Startup_Stage = " << Startup_Stage << endl
	 << "    Image_Loading = " << Image_Loading << endl));
#endif
if (Startup_Stage == STARTUP_SPLASH_SCALING &&
	! (state & Image_Viewer::IMAGE_LOAD_STATE) &&
	(state &
		(Image_Viewer::RENDERING_COMPLETED_STATE |
		 Image_Viewer::RENDERING_CANCELED_STATE)))
	{
	#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
	LOCKED_LOGGING ((
	clog << "    load_initial_source ..." << endl));
	#endif
	load_initial_source ();
	}

int
	condition;

if (! Image_Loading)
	{
	if (Image_Activity_Indicator)
		{
		//	Activity indicator state.
		condition = ACTIVITY_OFF;
		if (state & Image_Viewer::RENDERING_BACKGROUND_STATE)
			condition = ACTIVITY_BACKGROUND_RENDERING;
		else
		if (state & Image_Viewer::RENDERING_VISIBLE_TILES_STATE)
			condition = ACTIVITY_VISIBLE_RENDERING;
		Image_Activity_Indicator->state (condition);
		}

	//	Status message.
	condition = state;
	if (state & Image_Viewer::COMPLETED_WITHOUT_RENDERING_STATE)
		condition = (state & Image_Viewer::STATE_TYPE_MASK)
			| Image_Viewer::RENDERING_COMPLETED_STATE;
	show_status_message (Image_Viewer::state_change_description (condition));

	//	Statistics refresh conditions.
	if ((state & Image_Viewer::COMPLETED_WITHOUT_RENDERING_STATE) ||
		(state & Image_Viewer::STATE_QUALIFIER_MASK) ==
			Image_Viewer::RENDERING_VISIBLE_TILES_COMPLETED_STATE ||
		/*
			The image moved, but only background tiles are being rendered.
			There will be no RENDERING_VISIBLE_TILES_COMPLETED_STATE.
			Don't wait for background rendering to complete.
		*/
		 state ==
		 	(Image_Viewer::IMAGE_MOVE_STATE |
			 Image_Viewer::RENDERING_BACKGROUND_STATE))
		{
		condition = state & Image_Viewer::STATE_TYPE_MASK;
		if (condition == Image_Viewer::DATA_MAPPING_STATE)
			{
			#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
			LOCKED_LOGGING ((
			clog << "    refresh_display_statistics ..." << endl));
			#endif
			refresh_display_statistics ();
			}
		else
		if ((condition &
				(Image_Viewer::DATA_MAPPING_STATE |
				 Image_Viewer::BAND_MAPPING_STATE |
				/*
					A statistics refresh is needed when image scaling is done
					because the data resolution may have changed (JP2 source).
				*/
				 Image_Viewer::IMAGE_SCALE_STATE)))
			{
			#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
			LOCKED_LOGGING ((
			clog << "    refresh_statistics ..." << endl));
			#endif
			refresh_statistics ();
			}
		else
		if ((condition &
				(Image_Viewer::DISPLAY_SIZE_STATE |
				 Image_Viewer::IMAGE_MOVE_STATE)) &&
			(! has_selected_region () ||
				Statistics_Refresh_Needed))
			{
			#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
			LOCKED_LOGGING ((
			clog << "    refresh_statistics ..." << endl));
			#endif
			refresh_statistics ();
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SLOTS))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::image_viewer_state_change" << endl));
#endif
}


void
HiView_Window::activity_indicator_clicked
	(
	int
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
clog << ">-< HiView_Window::activity_indicator_clicked" << endl;
#endif
if (Network_Reply)
	Network_Reply->abort ();
else
	Image_View->cancel_rendering ();
}


void
HiView_Window::view_status_bar
	(
	bool	enabled
	)
{
if (View_Status_Bar_Action->isChecked () != enabled)
	//	Toggle the action, if it is enabled, which is connected here.
	View_Status_Bar_Action->setChecked (enabled);
else
if (View_Status_Bar_Action->isEnabled ())
	statusBar ()->setVisible (enabled);
}


void
HiView_Window::show_status_message
	(
	const QString&	message
	)
{
if (View_Status_Bar_Action->isChecked ())
	statusBar ()->showMessage (message);
}


void
HiView_Window::status_message_changed
	(
	const QString&	message
	)
{
if (! View_Status_Bar_Action->isChecked () &&
	message.isEmpty ())
	statusBar ()->hide ();
}
/*==============================================================================
	Metadata
*/
void
HiView_Window::reset_metadata ()
{
#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_LOAD_IMAGE))
LOCKED_LOGGING ((
clog << ">>> HiView_Window::reset_metadata" << endl));
#endif
//	N.B.: The metadata remains owned by the Source_Image.
Aggregate
	*metadata (Image_View->image_metadata ());
#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_LOAD_IMAGE))
LOCKED_LOGGING ((
clog << "    metadata @ " << (void*)metadata << endl));
#endif
Image_Metadata_Dialog->parameters (metadata);

//	Reset the Location_Mapper.
Location->parameters (NULL);
//	Reset the Image_Info_Panel.
Image_Info->set_metadata(NULL);
Image_Info->projection (Location->projection ());
Image_View->projection (Location->projection());

if (metadata &&
	Preferences->get_PDS_metadata () &&
	! Source_Name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_LOAD_IMAGE))
	LOCKED_LOGGING ((
	clog << "    get PDS metadata ..." << endl));
	#endif
	Aggregate
		*JP2_metadata (NULL);
	string
		name;
	int
		index = metadata->size ();
	while (--index >= 0)
		{
		name = (*metadata)[index].name ();
		if (name == JP2_Image::JP2_METADATA_GROUP)
			JP2_metadata = (Aggregate*)(&(*metadata)[index]);
		else
		if (name == PDS_Metadata::PDS_METADATA_GROUP)
			{
			//	PDS metadata already present.
			JP2_metadata = NULL;
			break;
			}
		}
	if (JP2_metadata)
		{
		QUrl
			URL;
		//	Look for a label URL in the metadata.
		Parameter
			*parameter (JP2_metadata->find (HiView_Utilities::PDS_LABEL_URL_PARAMETER));
		if (parameter)
			URL.setUrl (QString::fromStdString (parameter->value ()));
		URL = HiView_Utilities::PDS_metadata_URL (Source_Name, URL);

		if (! Metadata)
			{
			Metadata = new PDS_Metadata;
			connect (Metadata,
				SIGNAL (fetched (idaeim::PVL::Aggregate*)),
				SLOT (PDS_metadata (idaeim::PVL::Aggregate*)),
				Qt::UniqueConnection);
			}
		#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_LOAD_IMAGE))
		clog << "      Metadata fetch " << URL.toString () << endl;
		#endif
		Metadata->fetch (URL, PDS_Metadata::ASYNCHRONOUS);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_LOAD_IMAGE))
LOCKED_LOGGING ((
clog << "<<< HiView_Window::reset_metadata" << endl));
#endif
}


void
HiView_Window::PDS_metadata
	(
	Aggregate*	metadata
	)
{
#if ((DEBUG_SECTION) & DEBUG_METADATA)
LOCKED_LOGGING ((
clog << ">>> HiView_Window::PDS_metadata: metadata @ "
		<< (void*)metadata << endl));
#endif
if (metadata)
	{
	//	Copy out the metadata from the PDS_Metadata fetcher.
	#if ((DEBUG_SECTION) & DEBUG_METADATA_PARAMETERS)
	clog << "    metadata -" << endl
		 << *metadata;
	#endif
	metadata = new Aggregate (*metadata);
	metadata->name (PDS_Metadata::PDS_METADATA_GROUP);
	//	Add the metadata to the root metadata.
	Image_Metadata_Dialog->parameters ()->add (metadata);
	#if ((DEBUG_SECTION) & DEBUG_METADATA)
	clog << "    Image_Metadata_Dialog parameters -" << endl
		 << *(Image_Metadata_Dialog->parameters ());
	#endif
	//	Refresh the Image_Metadata_Dialog view.
	Image_Metadata_Dialog->parameters (Image_Metadata_Dialog->parameters ());
	//	Refresh the Location_Mapper.
	Location->parameters (metadata);
	//	Refresh the Image_Info_Panel.
	#if ((DEBUG_SECTION) & DEBUG_METADATA)
	clog << "    Location projection @ "
			<< (void*)(Location->projection ()) << endl
		 << "      projection = "
		 	<< Location->projection ()->canonical_projection_name () << endl;
	#endif
	Image_Info->set_metadata(metadata);
	Image_Info->projection (Location->projection ());
    Image_View->projection (Location->projection());
	}
/*
else
	{
	QUrl
		URL (Metadata->redirected_URL ());
	if (! URL.isEmpty ())
		{
		//	Retry the metadata fetch with the redirected URL.
		#if ((DEBUG_SECTION) & DEBUG_METADATA)
		LOCKED_LOGGING ((
		clog << "    redirected URL: " << URL.toString () << endl));
		#endif
		Metadata->fetch (URL, PDS_Metadata::ASYNCHRONOUS);
		}
	}
*/
#if ((DEBUG_SECTION) & DEBUG_METADATA)
LOCKED_LOGGING ((
clog << "<<< HiView_Window::PDS_metadata" << endl));
#endif
}

/*==============================================================================
	Image Save
*/
bool
HiView_Window::save_image ()
{
#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
clog << ">>> HiView_Window::save_image" << endl;
#endif
bool
	saved = false;

if (Image_Saved)
	{
	//	Image save already in progress.
	Image_Save_Thread->show_dialog (true);
	#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
	clog << "    image save in progress" << endl
		 << ">>> HiView_Window::save_image: false" << endl;
	#endif
	return saved;
	}

if (! Image_Save_Dialog)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
	clog << "    constructing Save_Image_Dialog" << endl;
	#endif
	Image_Save_Dialog = new Save_Image_Dialog (*Image_View, this);
	Image_Save_Thread = new Save_Image_Thread;
	connect (Image_Save_Thread,
		SIGNAL (done (bool)),
		SLOT (save_image_done (bool)));
	}


if (Image_Save_Dialog->reset () &&
	Image_Save_Dialog->exec ())
	{
	QString
		pathname (Image_Save_Dialog->pathname ()),
		format (Image_Save_Dialog->image_format ());
	QSize
		image_size (Image_Save_Dialog->image_size ());
	#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
	clog << "      pathname = " << pathname << endl
		 << "        format = " << format << endl
		 << "    image_size = " << image_size << endl;
	#endif
	if (! pathname.isEmpty () &&
		! format.isEmpty () &&
		! image_size.isEmpty ())
		{
		QString
			message;
		try
			{
			if (Image_View->image_name ().isEmpty ())
				Image_Saved = Image_View->image ()->clone (image_size);
			else
			if (! (Image_Saved = Plastic_Image_Factory::create
					(Image_View->image_name (), image_size)))
				{
				message =
					tr ("Could not create the %1x%2 output image.\n")
						.arg (image_size.width ())
						.arg (image_size.height ());
				message += Plastic_Image_Factory::error_message ();
				}

			if (Image_Saved)
				{
				#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
				clog << "     real size = " << Image_Saved->size () << endl;
				#endif
				if (Image_Saved->width () != image_size.width () ||
					Image_Saved->height () != image_size.height ())
					message =
						tr ("Sorry, couldn't allocate sufficient memory\n"
							"for the requested %1x%2 image size.")
							.arg (image_size.width ())
							.arg (image_size.height ());
				else
					{
					//	Image configuration.
					Image_Saved->source_band_map
						(Image_View->image ()->source_band_map ());
					Image_Saved->source_data_maps
						(const_cast<const Plastic_Image::Data_Map**>
						(Image_View->image ()->source_data_maps ()));
					Image_Saved->source_origin
						(Image_View->displayed_image_origin ());
					Image_Saved->source_scale
						(Image_Save_Dialog->image_scale ());
					}
				}
			}
		catch (...)
			{
			message =
				tr ("Unable to provide a %1x%2 image.")
				.arg (image_size.width ())
				.arg (image_size.height ());
			}
		if (message.isEmpty ())
			saved = Image_Save_Thread->save_image
				(Image_Saved, pathname, format);
		else
			{
			QMessageBox::warning (this, qApp->applicationName (), message);
			if (Image_Saved)
				{
				delete Image_Saved;
				Image_Saved = NULL;
				}
			#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
			clog << "    Plastic_Image assembly failed" << endl;
			#endif
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SAVE_IMAGE | DEBUG_MENUS))
clog << "<<< HiView_Window::save_image: " << saved << endl;
#endif
return saved;
}


void
HiView_Window::save_image_done
	(
	bool	completed
	)
{
#if ((DEBUG_SECTION) & DEBUG_SAVE_IMAGE)
clog << ">-< HiView_Window::save_image_done: " << completed << endl;
#endif
delete Image_Saved;
Image_Saved = NULL;

if (completed)
	show_status_message
		(tr ("Saved %1 image file: %2")
		.arg (Image_Save_Thread->image_format ())
		.arg (Image_Save_Thread->image_pathname ()));
else
	{
	QString
		message (tr ("Did not save the %1 image file -\n%2")
		.arg (Image_Save_Thread->image_format ())
		.arg (wrapped_pathname
			(Image_Save_Thread->image_pathname (), 0, font ())));
	if (! Image_Save_Thread->failure_message ().isEmpty ())
		{
		message += "\n\n";
		message += Image_Save_Thread->failure_message ();
		}
	QMessageBox::warning (this, Image_Save_Dialog->windowTitle (), message);
	}
}

/*==============================================================================
	Event Handlers
*/
void
HiView_Window::resizeEvent
	(
    QResizeEvent*
	#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
	event
	#endif
	)
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
clog << ">>> HiView_Window::resizeEvent:" << endl
	 << "    from " << event->oldSize () << endl
	 << "      to " << event->size () << endl;
#endif
update_window_fit_action ();
}


void
HiView_Window::closeEvent
	(
	QCloseEvent*	event
	)
{
save_configuration ();
save_layout ();
event->accept ();
}

/*------------------------------------------------------------------------------
	Drag-and-Drop
*/
void
HiView_Window::dragEnterEvent
	(
	QDragEnterEvent*	event
	)
{
const QMimeData
	*mime_data = event->mimeData ();
#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
clog << ">>> HiView_Window::dragEnterEvent: " << mime_data->text () << endl
	 << "    possible actions = " << hex << event->possibleActions () << endl
	 << "     proposed action = " << event->proposedAction () << dec << endl
	 << "    has image = " << mime_data->hasImage () << endl
	 << "     has URLs = " << mime_data->hasUrls () << endl
	 << "    formats -" << endl;
QStringList
	formats (mime_data->formats ());
for (int
		index = 0;
		index < formats.count ();
		index++)
	clog << "    " << index << " - " << formats[index] << endl;
#endif
if (! Image_Loading &&
	(mime_data->hasImage () ||
	 mime_data->hasUrls () ||
	 mime_data->text ().startsWith ("http://", Qt::CaseInsensitive) ||
	 mime_data->text ().startsWith ("https://", Qt::CaseInsensitive) ||
	 mime_data->text ().startsWith ("jpip://", Qt::CaseInsensitive)))
	{
	#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
	clog << "    proposed action accepted" << endl;
	#endif
	event->acceptProposedAction ();
	}
#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
clog << "<<< HiView_Window::dragEnterEvent" << endl;
#endif
}


void
HiView_Window::dropEvent
	(
	QDropEvent*			event
	)
{
const QMimeData
	*mime_data = event->mimeData ();
QString
	source (mime_data->text ());
#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
clog << ">>> HiView_Window::dropEvent: " << source << endl
	 << "    possible actions = " << hex << event->possibleActions () << endl
	 << "     proposed action = " << event->proposedAction () << dec << endl
	 << "    has image = " << mime_data->hasImage () << endl
	 << "     has URLs = " << mime_data->hasUrls () << endl;
#endif
bool
	accepted = false;
if (! Image_Loading)
	{
	if (mime_data->hasUrls () ||
		source.startsWith ("http://", Qt::CaseInsensitive) ||
		source.startsWith ("https://", Qt::CaseInsensitive) ||
		source.startsWith ("jpip://", Qt::CaseInsensitive))
		{
		if (mime_data->hasUrls ())
			{
			QList<QUrl>
				URLs = mime_data->urls ();
			if (URLs.count ())
				{
				source = URLs.at (0).toString ();
				#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
				clog << "    using URL - " << source << endl;
				#endif
				}
			}
		#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
		else
			clog << "    using text" << endl;
		#endif
		accepted = true;
		open (source);
		}
	else
	if (mime_data->hasImage ())
		{
		#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
		clog << "    using image data" << endl;
		#endif
    	QImage
			image = qvariant_cast<QImage>(mime_data->imageData ());
    	load_image (image);
		accepted = true;
		}
	}
if (accepted)
	event->acceptProposedAction ();
#if ((DEBUG_SECTION) & DEBUG_DROP_IMAGE)
clog << "<<< HiView_Window::dropEvent" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Mouse
*/
/*	N.B.: Mouse events may be intercepted by the Image_Viewer.

	Image_Viewer control modes in effect take precedence over
	region selection operations.
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
//	Selection modification modes.
enum
	{
	MOVE_SELECTION	= (1 << 0),
	RESIZE_LEFT		= (1 << 1),
	RESIZE_RIGHT	= (1 << 2),
	RESIZE_TOP		= (1 << 3),
	RESIZE_BOTTOM	= (1 << 4)
	};
}
#endif

void
HiView_Window::mousePressEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << ">>> HiView_Window::mousePressEvent:" << endl
	 << "    mouse position = " << event->pos () << endl;
#endif
bool
	accepted = false;



if (event->buttons () == Qt::LeftButton &&
	Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE
	/* && statistics_are_visible ()*/)
{
	QPoint position (Image_View->image_display ()->mapFromGlobal (event->globalPos ()));

	#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
	clog << "    display position = " << position << endl;
	#endif
	if (Image_View->image_display_region ().contains (position))
	{
		position = round_down (Image_View->map_display_to_image (position));
		if(Distance_Tool) {
			if(!P1_Set) {
				//set first point
				Image_Line.setP1(position);
				//enable mouse tracking
				setMouseTracking(true);
				centralWidget()->setMouseTracking(true);
				Image_View->setMouseTracking(true);
				P1_Set = true;
			}
			else {
				//set second point and wait until another P1 is set
				Image_Line.setP2(position);
				update_line();
				P1_Set = false;
			}
		}
		else {
		//	Possible selection modification.

			Selection_Modification = selection_modification (position);
			if (! Selection_Modification)
			{
				//	Possible new selection.
				Selection_Modification = RESIZE_RIGHT | RESIZE_BOTTOM;
				Selection_Start = position;
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    Selected_Image_Region start = "
						<< position << endl;
				#endif
			}
			else if (Selection_Modification & MOVE_SELECTION)
			{
				//	Change to selection offset.
				Selection_Start = position;
				Selection_Start -= Selected_Image_Region.topLeft ();
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    move selection w/ offset = "
						<< Selection_Start << endl;
				#endif
			}
			set_selection_cursor (Selection_Modification);
		}
		accepted = true;
	}
}
else
	Selection_Start.rx () =
	Selection_Start.ry () = -1;

event->setAccepted (accepted);
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::mousePressEvent" << endl;
#endif
}


/*	N.B.: Mouse tracking is not enabled for the HiView_Window,
	so the HiView_Window::mouseMoveEvent is only called when a mouse
	button is being pressed and the mouse has been moved.

	The exception is when the distance tool is in use, then mouse tracking
	is enabled until the distance tool is turned off.

	Mouse tracking is enabled for the Tiled_Image_Display which it uses
	to report the current image and display positions via the
	image_cursor_moved signal.
*/
void
HiView_Window::mouseMoveEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << ">>> HiView_Window::mouseMoveEvent:" << endl
	 << "    mouse position = " << event->pos () << endl;
#endif
bool
	accepted = false;

if(Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE) {
	QPoint display_position (Image_View->image_display ()->mapFromGlobal (event->globalPos ()));

	if (Image_View->image_display_region ().contains (display_position))
	{
		QPoint image_position (round_down(Image_View->map_display_to_image (display_position)));

		if(Distance_Tool && P1_Set)
		{
			Image_Line.setP2(image_position);
			update_line();
		}

		if (event->buttons () == Qt::LeftButton)
		{

		#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
		clog << "        display position = " << display_position << endl
		 << "    image_display_region = "
			<< Image_View->image_display_region () << endl;
		#endif

		if (! Selection_Modification)
			{
			//	Drag started outside the image area.
			#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
			clog << "    drag into image treated as a mouse press" << endl
				 << "<<< HiView_Window::mouseMoveEvent" << endl;
			#endif
			mousePressEvent (event);
			return;
			}

		QRectF
			selected_image_region (Selected_Image_Region);
		#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
		clog << "           image position = " << image_position << endl
			 << "    Selected_Image_Region = " << Selected_Image_Region << endl;
		#endif
		if (Selection_Modification & MOVE_SELECTION)
			{
			QSize
				image_size (Image_View->image_size ());
			image_position -= round_down (Selection_Start);
			Selected_Image_Region.moveTopLeft (image_position);
			#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
			clog << "    move offset " << Selection_Start
					<< " to " << image_position << endl;
			#endif
			if (Selected_Image_Region.left () < 0)
				{
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    left edge overlap" << endl;
				#endif
				Selection_Start.rx () -= -Selected_Image_Region.left ();
				Selected_Image_Region.moveLeft (0);
				}
			else
			if (Selected_Image_Region.right () >= image_size.rwidth ())
				{
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    right edge overlap" << endl;
				#endif
				--image_size.rwidth ();
				Selection_Start.rx () +=
					(Selected_Image_Region.right () - image_size.rwidth ());
				Selected_Image_Region.moveRight (image_size.rwidth ());
				}
			if (Selected_Image_Region.top () < 0)
				{
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    top edge overlap" << endl;
				#endif
				Selection_Start.ry () -= -Selected_Image_Region.top ();
				Selected_Image_Region.moveTop (0);
				}
			else
			if (Selected_Image_Region.bottom () >= image_size.rheight ())
				{
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    bottom edge overlap" << endl;
				#endif
				--image_size.rheight ();
				Selection_Start.ry () +=
					(Selected_Image_Region.bottom () - image_size.rheight ());
				Selected_Image_Region.moveBottom (image_size.rheight ());
				}
			}
		else
			{
			if (Selection_Start.rx () != -1)
				{
				display_position -=
					Image_View->map_image_to_display (Selection_Start);
				if (display_position.manhattanLength () < 3)
					{
					//	Haven't moved beyond shaking click distance.
					event->ignore ();
					#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
					clog << "    haven't moved far enough" << endl
						 << "<<< HiView_Window::mouseMoveEvent" << endl;
					#endif
					return;
					}
				//	Initialize the selection location.
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    Selected_Image_Region location initialized" << endl;
				#endif
				Selected_Image_Region.setTopLeft (Selection_Start);
				Selected_Image_Region.setBottomRight (Selection_Start);
				Selection_Start.rx () =
				Selection_Start.ry () = -1;
				}

			//	Adjust the selection size.
			int
				selection = Selection_Modification;
			if (Selection_Modification & RESIZE_LEFT)
				{
				if (image_position.rx () > Selected_Image_Region.right ())
					{
					//	Swap sides.
					Selected_Image_Region.setRight (image_position.rx ());
					Selection_Modification &= ~RESIZE_LEFT;
					Selection_Modification |= RESIZE_RIGHT;
					}
				else
					Selected_Image_Region.setLeft (image_position.rx ());
				}
			else
			if (Selection_Modification & RESIZE_RIGHT)
				{
				if (image_position.rx () < Selected_Image_Region.left ())
					{
					//	Swap sides.
					Selected_Image_Region.setLeft (image_position.rx ());
					Selection_Modification &= ~RESIZE_RIGHT;
					Selection_Modification |= RESIZE_LEFT;
					}
				else
					Selected_Image_Region.setRight (image_position.rx ());
				}
			if (Selection_Modification & RESIZE_TOP)
				{
				if (image_position.ry () > Selected_Image_Region.bottom ())
					{
					//	Swap sides.
					Selected_Image_Region.setBottom (image_position.ry ());
					Selection_Modification &= ~RESIZE_TOP;
					Selection_Modification |= RESIZE_BOTTOM;
					}
				else
					Selected_Image_Region.setTop (image_position.ry ());
				}
			else
			if (Selection_Modification & RESIZE_BOTTOM)
				{
				if (image_position.ry () < Selected_Image_Region.top ())
					{
					//	Swap sides.
					Selected_Image_Region.setTop (image_position.ry ());
					Selection_Modification &= ~RESIZE_BOTTOM;
					Selection_Modification |= RESIZE_TOP;
					}
				else
					Selected_Image_Region.setBottom (image_position.ry ());
				}

			if (selection != Selection_Modification)
				{
				#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
				clog << "    Selection_Modification changed from "
						<< selection << " to " << Selection_Modification
						<< endl;
				#endif
				set_selection_cursor (Selection_Modification);
				}
			}
		#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
		clog << "    Selected_Image_Region = " << Selected_Image_Region << endl;
		#endif

		//	Reset the overlay rectangle drawn on the display.
		reset_region_overlay ();

		if (Selected_Image_Region != selected_image_region)
			//	Update the statistics for the selected region.
			refresh_statistics ();

		accepted = true;
		}
	}
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::mouseMoveEvent" << endl;
#endif
}


void
HiView_Window::image_moved
	(
	const QPoint&	/* image_position */,
	int				/* band */
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS))
clog << ">>> HiView_Window::image_moved" << endl;
#endif
reset_region_overlay ();
update_line();
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_STATISTICS))
clog << "<<< HiView_Window::image_moved" << endl;
#endif
}


void
HiView_Window::image_cursor_moved
	(
	const QPoint&	/* display_position */,
	const QPoint&	image_position
	)
{
//	Only because there is only one HiView_Window in the application.
static int
	last_selection = -1;
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::image_cursor_moved:" << endl
	 << "    image_position = " << image_position << endl
	 << "    Selection_Modification = " << Selection_Modification << endl
	 << "    control_mode = " << Image_View->control_mode () << endl;
#endif
if (! Selection_Modification &&
	! Selected_Image_Region.isEmpty () &&
	Image_View->control_mode () == Image_Viewer::NO_CONTROL_MODE)
	{
	int
		selection = selection_modification (image_position);
	#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
	clog << "    last_selection = " << last_selection << endl
		 << "         selection = " << selection << endl;
	#endif
	if (selection != last_selection)
		{
		set_selection_cursor (selection);
		last_selection = selection;
		}
	}
else
	last_selection = -1;
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::image_cursor_moved" << endl;
#endif
}


void
HiView_Window::set_selection_cursor
	(
	int		selection
	)
{
QCursor
	*cursor = NULL;
switch (selection)
	{
	case MOVE_SELECTION:
		cursor = Shift_Region_Cursor;
		break;
	case RESIZE_LEFT:
	case RESIZE_RIGHT:
		cursor = Shift_Region_Horizontal_Cursor;
		break;
	case RESIZE_TOP:
	case RESIZE_BOTTOM:
		cursor = Shift_Region_Vertical_Cursor;
		break;
	case (RESIZE_LEFT  | RESIZE_TOP):
	case (RESIZE_RIGHT | RESIZE_BOTTOM):
		cursor = Shift_Region_FDiag_Cursor;
		break;
	case (RESIZE_RIGHT | RESIZE_TOP):
	case (RESIZE_LEFT  | RESIZE_BOTTOM):
		cursor = Shift_Region_BDiag_Cursor;
		break;
	}
Image_View->default_cursor (cursor);
}


int
HiView_Window::selection_modification
	(
	const QPoint&	image_position
	)
{
int
	mode = 0;
int
	x = image_position.x (),
	y = image_position.y (),
	left   = round_down (Selected_Image_Region.left ()),
	right  = round_down (Selected_Image_Region.right ()),
	top    = round_down (Selected_Image_Region.top ()),
	bottom = round_down (Selected_Image_Region.bottom ());
if (x < left ||
	x > right ||
	y < top ||
	y > bottom)
	//	The Selected_Image_Region does not contain the image_position.
	return mode;

if (x == left)
	mode |= RESIZE_LEFT;
else
if (x == right)
	mode |= RESIZE_RIGHT;
if (y == top)
	mode |= RESIZE_TOP;
else
if (y == bottom)
	mode |= RESIZE_BOTTOM;
if (! mode)
	mode = MOVE_SELECTION;
return mode;
}


void
HiView_Window::mouseReleaseEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << ">>> HiView_Window::mouseReleaseEvent" << endl
	 << "    mouse position = " << event->pos () << endl;
#endif
Selection_Start.rx () =
Selection_Start.ry () = -1;
Selection_Modification = 0;
set_selection_cursor (selection_modification
	(round_down (Image_View->map_display_to_image
		(Image_View->image_display ()->mapFromGlobal (event->globalPos ())))));
event->ignore ();
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::mouseReleaseEvent" << endl;
#endif
}


void
HiView_Window::mouseDoubleClickEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << ">>> HiView_Window::mouseDoubleClickEvent" << endl
	 << "    mouse position = " << event->pos () << endl;
#endif
bool
	accepted = false;

if (event->buttons () == Qt::LeftButton)
	{
	QPoint
		display_position
			(Image_View->image_display ()->mapFromGlobal (event->globalPos ()));
	#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
	clog << "        image display position = " << display_position << endl
		 << "          image_display_region = "
			<< Image_View->image_display_region () << endl;
	#endif
	if (Image_View->image_display_region ().contains (display_position))
		{
		if (! Selected_Image_Region.isEmpty ())
			reset_selected_region ();
		accepted = true;
		}
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::mouseDoubleClickEvent" << endl;
#endif
}


void HiView_Window::update_line() {
	if(Distance_Tool) {
		//draw the line on the image, done this way so when the image is moved, the same point on the image is shown
		Line->setP1(Image_View->map_image_to_display (Image_Line.p1()));
		Line->setP2(Image_View->map_image_to_display (Image_Line.p2()));

		//length of line in pixels
		double length = qSqrt(Image_Line.dx()*Image_Line.dx() + Image_Line.dy()*Image_Line.dy());

		Image_Info->set_property_f("distance_length_px", length);

		if(Location != NULL && Location->projection() != NULL && !Location->projection()->is_identity()) {
			double projected_length = length*Location->projection()->pixel_size();
			Line->setText(QString("%1 m")
				.arg(projected_length));

			Image_Info->set_property_f("distance_length_m", projected_length);
		}
		//if it cant map project, use pixel value
		else {
			Line->setText(QString("%1 px")
				.arg(length));
		}
		if(!Line->isVisible())
			Line->setVisible(true);
		else
			Line->update();

		Image_Info->evaluate_script();
	}
}

void
HiView_Window::reset_region_overlay ()
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << ">>> HiView_Window::reset_region_overlay" << endl;
#endif
if (Selected_Image_Region.isEmpty ())
	{
	if (Region_Overlay)
		Region_Overlay->setVisible (false);
	if (Selected_Area != NULL)
		Selected_Area->setVisible (false);
	Image_Info->use_avg_pixel_value(false);
	if(Location != NULL && Location->projection() != NULL && !Location->projection()->is_identity()) {
		QRect source_region = round_down (Image_View->displayed_image_region ());
		double pixel_size = Location->projection()->pixel_size();
		double projected_width = source_region.width()*pixel_size;
		double projected_height = source_region.height()*pixel_size;

		Image_Info->set_property_qsreal("region_area_m", projected_width*projected_height);
		Image_Info->set_property_f("region_width_m", projected_width);
		Image_Info->set_property_f("region_height_m", projected_height);
	}
	#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
	clog << "    no selected region" << endl
		 << "<<< HiView_Window::reset_region_overlay" << endl;
	#endif
	return;
	}

#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "    Selected_Image_Region = " << Selected_Image_Region << endl
     << "           display region = "
	 	<< Image_View->map_image_to_display (Selected_Image_Region) << endl;
QPointF
	image_position (Selected_Image_Region.topLeft ());
QPoint
	display_position (Image_View->map_image_to_display (image_position));
clog << "        TL image position = " << image_position << endl
	 << "      TL image to display = " << display_position << endl
	 << "      TL display to image = "
	 	<< Image_View->map_display_to_image (display_position) << endl;
#endif
if (Region_Overlay)
	{
	Image_Info->use_avg_pixel_value(true);
	//Map Selected Region to Display
	Region_Overlay->setGeometry
		(Image_View->map_image_to_display (Selected_Image_Region));
	if (! Region_Overlay->isVisible ())
		Region_Overlay->setVisible (true);
	//Get coordinates of the top left and bottom right locations of the box.
	Coordinate topLeftCoord (Selected_Image_Region.topLeft().x(), Selected_Image_Region.topLeft().y());
	Coordinate bottomRightCoord (Selected_Image_Region.bottomRight().x(), Selected_Image_Region.bottomRight().y());
	//Get QPoints for the top left location of the box.
	QPoint topLeft = Image_View->map_image_to_display(round_down(Selected_Image_Region.topLeft()));
	//Get pixel values, and attempt to select a color that will contrast well enough to read
	Plastic_Image::Triplet pixel_value = Image_View->display_pixel(topLeft);
	QPalette color_palette;
	int avg = (static_cast<int>(pixel_value.Datum[0]) + static_cast<int>(pixel_value.Datum[1]) + static_cast<int>(pixel_value.Datum[2]))/3;
	if(avg < 170)
		color_palette.setColor(QPalette::WindowText, QColor(255,255,255));
	else
		color_palette.setColor(QPalette::WindowText, QColor(0,0,0));

	//check to see if the QLabel has been allocated
	if(Selected_Area == NULL) {
		Selected_Area = new QLabel(Image_View->image_display ());
		Selected_Area->setFont(QFont("Arial",9));
	}
	//set the palette color selected earlier so the qlabels will be a high contrast color.
	Selected_Area->setPalette(color_palette);
	//check if a projector exists, if so see if is_identity is false (therefore do not know the pixel size)
	if(Location != NULL && Location->projection() != NULL && !Location->projection()->is_identity()) {
		double pixel_size = Location->projection()->pixel_size();
		double projected_width = (bottomRightCoord.X-topLeftCoord.X)*pixel_size;
		double projected_height = (bottomRightCoord.Y-topLeftCoord.Y)*pixel_size;

		Selected_Area_Text = QString("%1 x %2 m")
			.arg(projected_width)
			.arg(projected_height);

		//this should occur before the avg RGB values are updated from the histogram so
		// we don't need to evaluate the script at this point
		Image_Info->set_property_qsreal("region_area_m", projected_width*projected_height);
		Image_Info->set_property_f("region_width_m", projected_width);
		Image_Info->set_property_f("region_height_m", projected_height);
	}
	//if it cant map project, set to x,y values
	else {
		Selected_Area_Text = QString("%1 x %2 px")
			.arg(bottomRightCoord.X-topLeftCoord.X)
			.arg(bottomRightCoord.Y-topLeftCoord.Y);
	}
	//Set text in label
	Selected_Area->setText(Selected_Area_Text);
	//set location of QLabel (pay attention to edges of the window)
	if(topLeft.y() - Selected_Area->sizeHint().height() < 0)
		Selected_Area->setGeometry(topLeft.x(), topLeft.y(), Selected_Area->sizeHint().width(), Selected_Area->sizeHint().height());
	else
		Selected_Area->setGeometry(topLeft.x(), topLeft.y() - Selected_Area->sizeHint().height(), Selected_Area->sizeHint().width(), Selected_Area->sizeHint().height());

	Selected_Area->setVisible(true);
	}
#if ((DEBUG_SECTION) & DEBUG_MOUSE_EVENTS)
clog << "<<< HiView_Window::reset_region_overlay" << endl;
#endif
}

void HiView_Window::reset_line () {
	Image_Line.setLine(0,0,0,0);
	update_line();
	P1_Set = false;
	Line->setVisible(false);
}

void
HiView_Window::reset_selected_region ()
{
bool region_was_selected = has_selected_region ();
Selected_Image_Region = QRectF/*.setRect*/(0, 0, 0, 0);
Selection_Start.rx () =
Selection_Start.ry () = -1;
Selection_Modification = 0;
reset_region_overlay ();
if (region_was_selected && Statistics)
	refresh_statistics ();
}

void HiView_Window::line_color(const QColor & color) {
	Line_Color = color;
	Line->setColor(color);
	Line->update();
}

/*------------------------------------------------------------------------------
	Tools menu
*/
void
HiView_Window::tool_context_menu_requested
	(
	QDockWidget*		tool,
	QContextMenuEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
clog << ">>> HiView_Window::tool_context_menu_requested:" << endl
	 << "    " << (void*)tool << ' ' << tool->windowTitle () << endl
	 << "    " << event->globalPos () << endl;
#endif
Selected_Tool = tool;
if (Selected_Tool)
	{
	Qt::DockWidgetArea
		dock_area = dockWidgetArea (Selected_Tool);
	Qt::DockWidgetAreas
		allowed_areas = Selected_Tool->allowedAreas ();
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	clog << "    dock_area = " << dock_area
			<< ", allowed_areas = " << allowed_areas << endl;
	#endif
	QList<QAction*>
		actions (Tool_Position_Menu->actions ());
	int
		index;
	#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
	clog << "    actions -" << endl;
	index = actions.count ();
	while (index--)
		clog << "    " << index << ": " << actions.at (index)->text () << endl;
	#endif
	if (Selected_Tool->isFloating ())
		{
		actions.at (TOOL_POSITION_FLOATING)->setText (tr ("Docked"));
		dock_area = Qt::NoDockWidgetArea;
		}
	else
		actions.at (TOOL_POSITION_FLOATING)->setText (tr ("Floating"));

	if (dock_area == Qt::LeftDockWidgetArea ||
		! (allowed_areas & Qt::LeftDockWidgetArea))
		actions.at (TOOL_POSITION_LEFT)->setEnabled (false);
	if (dock_area == Qt::RightDockWidgetArea ||
		! (allowed_areas & Qt::RightDockWidgetArea))
		actions.at (TOOL_POSITION_RIGHT)->setEnabled (false);
	if (dock_area == Qt::TopDockWidgetArea ||
		! (allowed_areas & Qt::TopDockWidgetArea))
		actions.at (TOOL_POSITION_TOP)->setEnabled (false);
	if (dock_area == Qt::BottomDockWidgetArea ||
		! (allowed_areas & Qt::BottomDockWidgetArea))
		actions.at (TOOL_POSITION_BOTTOM)->setEnabled (false);

	//	Pop the menu (actions handled by tool_position).
	Tool_Position_Menu->exec (event->globalPos ());

	index = actions.count ();
	while (index--)
		actions.at (index)->setEnabled (true);

	Selected_Tool = NULL;
	}
#if ((DEBUG_SECTION) & DEBUG_TOOLS_POSITION)
clog << "<<< HiView_Window::tool_context_menu_requested" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Tooltips event filter
*/
bool
HiView_Window::eventFilter
	(
	QObject*	object,
	QEvent*		event
	)
{
if (event->type () == QEvent::ToolTip &&
	View_Tooltips_Action &&
	! View_Tooltips_Action->isChecked ())
	return true;	//	Block event.
return QMainWindow::eventFilter (object, event);
}



}	//	namespace UA::HiRISE
