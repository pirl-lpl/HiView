/*	Preferences_Dialog

HiROC CVS ID: $Id: Preferences_Dialog.cc,v 1.60 2016/01/07 22:13:14 guym Exp $

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

#include	"Preferences_Dialog.hh"

#include	"HiView_Config.hh"
#include	"HiView_Utilities.hh"
#include	"Icon_Button.hh"
#include	"Drawn_Line.hh"
#include	"Help_Docs.hh"

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS
#include	<QString>

#include	<QApplication>
#include	<QAction>
#include	<QEvent>
#include	<QCloseEvent>
#include	<QTabWidget>
#include	<QIcon>
#include	<QSizePolicy>
#include	<QSettings>
#include	<QMessageBox>
#include	<QDialogButtonBox>
#include	<QCheckBox>
#include	<QButtonGroup>
#include	<QRadioButton>
#include	<QStringList>
#include	<QGridLayout>
#include	<QLabel>
#include	<QSpinBox>
#include	<QDoubleSpinBox>
#include	<QListWidget>
#include	<QVBoxLayout>
#include	<QPushButton>
#include	<QHBoxLayout>
#include	<QLineEdit>
#include	<QGroupBox>
#include	<QColor>
#include	<QColorDialog>
#include	<QHostInfo>
#include	<QUrl>
#include	<QFileInfo>
#include	<QDir>
#include	<QFileDialog>
#include	<QComboBox>

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
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_GENERAL			(1 << 1)
#define DEBUG_SOURCES			(1 << 2)
#define DEBUG_RENDERING			(1 << 3)
#define DEBUG_JPIP				(1 << 4)
#define DEBUG_GRAPHS			(1 << 5)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<iostream>
#include	<iomanip>
using std::clog;
using std::hex;
using std::dec;
using std::boolalpha;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Preferences_Dialog::ID =
		"UA::HiRISE::Preferences_Dialog ($Revision: 1.60 $ $Date: 2016/01/07 22:13:14 $)";


#ifndef DEFAULT_VIEW_TOOLTIPS
#define DEFAULT_VIEW_TOOLTIPS			true
#endif

#define	HORIZONTAL_SPACING				5

const char
	*Preferences_Dialog::LAYOUT_GEOMETRY_SECTION	= "Layout_Geometry";

const double
	Preferences_Dialog::INITIAL_SCALE_AUTO_FIT		= 0.0;

/*==============================================================================
	Statics
*/
Help_Docs
	*Preferences_Dialog::Docs_Helper;

/*==============================================================================
	Application configuration parameters
*/
#define Heading_Line_Weight \
	HiView_Config::Heading_Line_Weight
#define DISPLAY_BAND_NAMES \
	HiView_Config::DISPLAY_BAND_NAMES
#define PERCENT_DECIMAL_PLACES \
	HiView_Config::PERCENT_DECIMAL_PLACES

#define Apply_Button_Icon \
	HiView_Config::Apply_Button_Icon
#define Defaults_Button_Icon \
	HiView_Config::Defaults_Button_Icon
#define Reset_Button_Icon \
	HiView_Config::Reset_Button_Icon

/*==============================================================================
	Constructor
*/
Preferences_Dialog::Preferences_Dialog
	(
	QWidget*		parent
	)
	:	QDialog (parent),
		Show_Tooltips (DEFAULT_VIEW_TOOLTIPS)
{
setObjectName ("Preferences_Dialog");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Preferences_Dialog for " << qApp->applicationName () << endl;
#endif
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
clog << boolalpha;
#endif
setWindowTitle (qApp->applicationName () + ' ' + tr ("Preferences"));

QSizePolicy
	size_policy (QSizePolicy::Expanding, QSizePolicy::Expanding);
size_policy.setHorizontalStretch (0);
size_policy.setVerticalStretch (0);
setSizePolicy (size_policy);

//	Make sure the HiView_Config components have been initialized.
HiView_Config::initialize ();

//	Help Documentation.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Help_Docs" << endl;
#endif
Docs_Helper = new Help_Docs ();

//	Tabbed sections.
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    QTabWidget sections -" << endl;
#endif
QTabWidget
	*sections = new QTabWidget (this);
sections->setTabShape (QTabWidget::Rounded);

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    General ..." << endl;
#endif
General = new General_Section (this);
General->installEventFilter (this);
sections->addTab (General, tr ("General"));

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Sources ..." << endl;
#endif
Sources = new Sources_Section (this);
Sources->installEventFilter (this);
sections->addTab (Sources, tr ("Sources"));

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Rendering ..." << endl;
#endif
Rendering = new Rendering_Section (this);
Rendering->installEventFilter (this);
sections->addTab (Rendering, tr ("Rendering"));

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    JPIP ..." << endl;
#endif
JPIP = new JPIP_Section (this);
JPIP->installEventFilter (this);
sections->addTab (JPIP, tr ("JPIP"));

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Graphs ..." << endl;
#endif
Graphs = new Graphs_Section (this);
Graphs->installEventFilter (this);
sections->addTab (Graphs, tr ("Graphs"));

#if((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "	Scripts ..." << endl;
#endif
Scripts = new Scripts_Section (this);
Scripts->installEventFilter (this);
sections->addTab(Scripts, tr ("Scripts"));

sections->setCurrentIndex (0);

QVBoxLayout
	*layout = new QVBoxLayout;
layout->addWidget (sections);
setLayout (layout);

/*	Signal propagation.

	Connect the signals after the section objects have been instantiated
	to avoid unnecessary signals being seen from setup operations.
*/
connect (General, SIGNAL (band_numbers_indexed_changed (bool)),
	SIGNAL (band_numbers_indexed_changed (bool)));
connect (General, SIGNAL (documentation_location_changed (const QString&)),
	SIGNAL (documentation_location_changed (const QString&)));
connect (General, SIGNAL(longitude_direction_changed (int)), 
	SIGNAL(longitude_direction_changed (int)));
connect (General, SIGNAL(longitude_units_changed (int)), 
	SIGNAL(longitude_units_changed (int)));
connect (General, SIGNAL(latitude_units_changed (int)),
	SIGNAL(latitude_units_changed (int)));

connect (Sources, SIGNAL (source_list_capacity_changed (int)),
	SIGNAL (source_list_capacity_changed (int)));
connect (Sources, SIGNAL (source_list_changed (const QStringList&)),
	SIGNAL (source_list_changed (const QStringList&)));

connect (Rendering, SIGNAL (min_scale_changed (double)),
	SIGNAL (min_scale_changed (double)));
connect (Rendering, SIGNAL (max_scale_changed (double)),
	SIGNAL (max_scale_changed (double)));
connect (Rendering, SIGNAL (scaling_minor_increment_changed (double)),
	SIGNAL (scaling_minor_increment_changed (double)));
connect (Rendering, SIGNAL (scaling_major_increment_changed (double)),
	SIGNAL (scaling_major_increment_changed (double)));
connect (Rendering, SIGNAL (contrast_stretch_upper_changed (double, int)),
	SIGNAL (contrast_stretch_upper_changed (double, int)));
connect (Rendering, SIGNAL (contrast_stretch_lower_changed (double, int)),
	SIGNAL (contrast_stretch_lower_changed (double, int)));
connect (Rendering, SIGNAL (background_color_changed (QRgb)),
	SIGNAL (background_color_changed (QRgb)));
connect (Rendering, SIGNAL (line_color_changed (const QColor &)),
	SIGNAL (line_color_changed (const QColor &)));
connect (Rendering, SIGNAL (tile_size_changed (int)),
	SIGNAL (tile_size_changed (int)));
connect (Rendering, SIGNAL (rendering_increment_lines_changed (int)),
	SIGNAL (rendering_increment_lines_changed (int)));

//connect (JPIP, SIGNAL (HTTP_to_JPIP_hostname_changed (const QString&)),
//	SIGNAL (HTTP_to_JPIP_hostname_changed (const QString&)));
connect (JPIP, SIGNAL (JPIP_server_port_changed (int)),
	SIGNAL (JPIP_server_port_changed (int)));
connect (JPIP, SIGNAL (JPIP_proxy_changed (const QString&)),
	SIGNAL (JPIP_proxy_changed (const QString&)));
connect (JPIP, SIGNAL (JPIP_cache_directory_changed (const QString&)),
	SIGNAL (JPIP_cache_directory_changed (const QString&)));
connect (JPIP, SIGNAL (JPIP_request_timeout_changed (int)),
	SIGNAL (JPIP_request_timeout_changed (int)));
connect (JPIP, SIGNAL (max_source_image_area_MB_changed (int)),
	SIGNAL (max_source_image_area_MB_changed (int)));

connect (Graphs, SIGNAL (selection_sensitivity_changed (int)),
	SIGNAL (selection_sensitivity_changed (int)));
connect (Graphs, SIGNAL (canvas_color_changed (QRgb)),
	SIGNAL (canvas_color_changed (QRgb)));
	
connect(Scripts, SIGNAL (script_changed(const QString&)), 
	SIGNAL (script_changed(const QString&))); 
connect(Scripts, SIGNAL (show_script_changed(bool)),
	SIGNAL (show_script_changed(bool)));
connect(this, SIGNAL(variables_updated(QStringList&)),
	Scripts, SLOT(variables_updated(QStringList&)));

QAction
	*action = new QAction (tr ("Close Window"), this);
action->setShortcut (tr ("Ctrl+W"));
connect (action, SIGNAL (triggered ()),
	SLOT (close ()));
addAction (action);

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Preferences_Dialog" << endl;
#endif
}


Preferences_Dialog::~Preferences_Dialog ()
{delete Docs_Helper;}

/*==============================================================================
	Accessors
*/
bool
Preferences_Dialog::has_changed () const
{
return
	General->has_changed ()		||
	Sources->has_changed ()		||
	Rendering->has_changed ()	||
	JPIP->has_changed ()		||
	Graphs->has_changed ();
}


void
Preferences_Dialog::apply ()
{
General->apply ();
Sources->apply ();
Rendering->apply ();
JPIP->apply ();
Graphs->apply ();
}


void
Preferences_Dialog::reset ()
{
General->reset ();
Sources->reset ();
Rendering->reset ();
JPIP->reset ();
Graphs->reset ();
}

/*==============================================================================
	Utilities
*/
void
Preferences_Dialog::save
	(
	const QString&	key,
	const QVariant&	value
	)
{
QSettings
	settings;
settings.setValue (key, value);
}


QString
Preferences_Dialog::color_text
	(
	QRgb	color
	)
{
if (color == 0)
	return "transparent";
return QColor (color).name ();
}


QRgb
Preferences_Dialog::color_value
	(
	const QString&	text
	)
{return QColor (text).rgba ();}

/*==============================================================================
	Event Handlers
*/
bool
Preferences_Dialog::eventFilter
	(
	QObject*	object,
	QEvent*		event
	)
{
if (event->type () == QEvent::ToolTip &&
	! Show_Tooltips)
	return true;	//	Block event.
return QDialog::eventFilter (object, event);
}


void
Preferences_Dialog::closeEvent
	(
	QCloseEvent*	event
	)
{
bool
	accepted = true;
if (has_changed ())
	{
	switch (QMessageBox::question (this->parentWidget (), windowTitle (),
			tr ("Apply unsaved changes?"),
			QMessageBox::Discard |
			QMessageBox::Apply   |
			QMessageBox::Cancel,
			QMessageBox::Cancel))
		{
		case QMessageBox::Cancel:
			accepted = false;
			break;
		case QMessageBox::Apply:
			apply ();
			break;
		case QMessageBox::Discard:
			reset ();
		default:
			break;
		}
	}
event->setAccepted (accepted);
}

/*=*****************************************************************************
	General_Section
*/
const char
	*General_Section::RESTORE_LAYOUT_KEY			= "Restore_Layout";
#ifndef DEFAULT_RESTORE_LAYOUT
#define DEFAULT_RESTORE_LAYOUT				true
#endif
bool
	General_Section::Default_Restore_Layout
		= DEFAULT_RESTORE_LAYOUT;

//	Obsolete.
const char
	*General_Section::RESTORE_WINDOW_POSITIONS_KEY	= "Restore_Window_Positions";

const char
	*General_Section::RESTORE_LAST_SOURCE_KEY		= "Restore_Last_Source";

#ifndef DEFAULT_RESTORE_LAST_SOURCE
#define DEFAULT_RESTORE_LAST_SOURCE			true
#endif
bool
	General_Section::Default_Restore_Last_Source
		= DEFAULT_RESTORE_LAST_SOURCE;


const char
	*General_Section::BAND_NUMBERS_INDEXED_KEY		= "Band_Numbers_Indexed";
#ifndef DEFAULT_BAND_NUMBERS_INDEXED
#define DEFAULT_BAND_NUMBERS_INDEXED		true
#endif
bool
	General_Section::Default_Band_Numbers_Indexed
		= DEFAULT_BAND_NUMBERS_INDEXED;


const char
	*General_Section::GET_PDS_LABEL_KEY				= "Get_PDS_Metadata";
#ifndef DEFAULT_GET_PDS_LABEL
#define DEFAULT_GET_PDS_LABEL				true
#endif
bool
	General_Section::Default_Get_PDS_Metadata
		= DEFAULT_GET_PDS_LABEL;
		
const char
	*General_Section::RESTORE_LONGITUDE_FORMAT_KEY				= "Restore_Longitude_Format";
const char
	*General_Section::RESTORE_LATITUDE_FORMAT_KEY				= "Restore_Latitude_Format";
const char
	*General_Section::RESTORE_LONGITUDE_DIRECTION_KEY			= "Restore_Longitude_Direction";	
#ifndef DEFAULT_COORDINATE_FORMAT
#define DEFAULT_COORDINATE_FORMAT	0
#endif
int
	General_Section::Default_Coordinate_Format
		= DEFAULT_COORDINATE_FORMAT;

const char
	*General_Section::DOCUMENTATION_LOCATION_KEY	= "Documentation_Location";
#ifndef DOCUMENTATION_SEARCH_LOCATIONS
#define DOCUMENTATION_SEARCH_LOCATIONS \
	"../docs/Users_Guide", \
	"../Resources/Users_Guide", \
	"docs/Users_Guide", \
	"http://pirlwww.lpl.arizona.edu/software/HiView/Users_Guide"
#endif
const char
	*General_Section::Default_Documentation_Search_Locations[] =
		{
		DOCUMENTATION_SEARCH_LOCATIONS,
		NULL
		};
QStringList
	General_Section::Documentation_Search_Locations;

#ifndef DEFAULT_DOCUMENTATION_FILENAME
#define DEFAULT_DOCUMENTATION_FILENAME		"HiView_Users_Guide.html"
#endif
const QString
	General_Section::Default_Documentation_Filename
		= DEFAULT_DOCUMENTATION_FILENAME;


General_Section::General_Section
	(
	QWidget*	parent
	)
	:	QWidget (parent),
		Documentation_Location_lineEdit (NULL)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << ">>> General_Section" << endl;
#endif
if (parent)
	Title = parent->windowTitle ();
else
	Title = tr ("Preferences");
Title += tr (": General");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "    Title = \"" << Title << '"' << endl;
#endif

//	Initialize values.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "    settings -" << endl;
#endif
QSettings
	settings;

//	LAYOUT_GEOMETRY_SECTION.
settings.beginGroup (Preferences_Dialog::LAYOUT_GEOMETRY_SECTION);

if (settings.contains (RESTORE_LAYOUT_KEY))
	Restore_Layout = settings.value (RESTORE_LAYOUT_KEY,
		Default_Restore_Layout).toBool ();
else
	settings.setValue (RESTORE_LAYOUT_KEY,
		Restore_Layout = Default_Restore_Layout);

if (settings.contains (RESTORE_WINDOW_POSITIONS_KEY))
	//	Remove obsolete setting.
	settings.remove (RESTORE_WINDOW_POSITIONS_KEY);

if (settings.contains (RESTORE_LAST_SOURCE_KEY))
	Restore_Last_Source = settings.value (RESTORE_LAST_SOURCE_KEY,
		Default_Restore_Last_Source).toBool ();
else
	settings.setValue (RESTORE_LAST_SOURCE_KEY,
		Restore_Last_Source = Default_Restore_Last_Source);

//	End of LAYOUT_GEOMETRY_SECTION.
settings.endGroup ();

//	Band Numbering.
if (settings.contains (BAND_NUMBERS_INDEXED_KEY))
	Band_Numbers_Indexed = settings.value (BAND_NUMBERS_INDEXED_KEY,
		Default_Band_Numbers_Indexed).toBool ();
else
	settings.setValue (BAND_NUMBERS_INDEXED_KEY,
		Band_Numbers_Indexed = Default_Band_Numbers_Indexed);
HiView_Utilities::Band_Numbering_Indexed = Band_Numbers_Indexed;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Band_Numbers_Indexed = " << Band_Numbers_Indexed << endl;
#endif

//	Get PDS Label.
if (settings.contains (GET_PDS_LABEL_KEY))
	Get_PDS_Metadata = settings.value (GET_PDS_LABEL_KEY,
		Default_Get_PDS_Metadata).toBool ();
else
	settings.setValue (GET_PDS_LABEL_KEY,
		Get_PDS_Metadata = Default_Get_PDS_Metadata);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Get_PDS_Metadata = " << Get_PDS_Metadata << endl;
#endif

//	Help Documentation.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Documentation_Search_Locations -" << endl;
#endif
for (int
		index = 0;
		Default_Documentation_Search_Locations[index];
	  ++index)
	{
	Documentation_Search_Locations.append
		(Default_Documentation_Search_Locations[index]);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
	clog << "      " << index << ": "
			<< Documentation_Search_Locations.last () << endl;
	#endif
	}
Documentation_Search_Locations.removeDuplicates ();

if (settings.contains (DOCUMENTATION_LOCATION_KEY))
	{
	Documentation_Location = settings.value (DOCUMENTATION_LOCATION_KEY,
		QString ()).toString ();
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
	clog << "      Documentation_Location setting = "
			<< Documentation_Location << endl;
	#endif
	}
else
	{
	if (find_documentation_location ())
		Documentation_Location = Documentation_Location_Pending;
	else
		QMessageBox::information ((isVisible () ? this : NULL), Title,
			tr ("The ") + Default_Documentation_Filename
			+ tr (" documentation file was not found.\n")
			+ tr ("Preferences can be used to specify its location."));

	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
	clog << "      Documentation_Location = "
			<< Documentation_Location << endl;
	#endif
	settings.setValue (DOCUMENTATION_LOCATION_KEY,
		Documentation_Location);
	}

//	Get Coordinate Display Format Settings
if (settings.contains (RESTORE_LONGITUDE_FORMAT_KEY))
	Longitude_Units = settings.value (RESTORE_LONGITUDE_FORMAT_KEY,
		Default_Coordinate_Format).toInt();
else
	settings.setValue (RESTORE_LONGITUDE_FORMAT_KEY,
		Longitude_Units = Default_Coordinate_Format);
if (settings.contains (RESTORE_LONGITUDE_DIRECTION_KEY))
	Longitude_Direction = settings.value (RESTORE_LONGITUDE_DIRECTION_KEY,
		Default_Coordinate_Format).toInt();
else
	settings.setValue (RESTORE_LONGITUDE_DIRECTION_KEY,
		Longitude_Direction = Default_Coordinate_Format);
if (settings.contains (RESTORE_LATITUDE_FORMAT_KEY))
	Latitude_Units =  settings.value (RESTORE_LATITUDE_FORMAT_KEY,
		Default_Coordinate_Format).toInt();
else
	settings.setValue (RESTORE_LATITUDE_FORMAT_KEY,
		Latitude_Units = Default_Coordinate_Format);

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "    panel layout -" << endl;
#endif
QGridLayout
	*grid_layout = new QGridLayout (this);
QVBoxLayout
	*vertical_layout;
QHBoxLayout
	*horizontal_layout;
Drawn_Line
	*line;
int
	row = -1;

//	Layout Restoration.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Layout section" << endl;
#endif

//		Heading Label.
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Layout</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

vertical_layout = new QVBoxLayout;
//		Restore Geometry.
Restore_Layout_CheckBox = new QCheckBox (tr ("Restore Geometry"));
Restore_Layout_CheckBox->setChecked (Restore_Layout);
connect (Restore_Layout_CheckBox, SIGNAL (toggled (bool)),
	SLOT (restore_layout (bool)));
vertical_layout->addWidget (Restore_Layout_CheckBox,
	0, Qt::AlignLeft | Qt::AlignVCenter);

//		Restore Last Source.
Restore_Last_Source_CheckBox = new QCheckBox (tr ("Restore Last Source"));
Restore_Last_Source_CheckBox->setChecked (Restore_Last_Source);
connect (Restore_Last_Source_CheckBox, SIGNAL (toggled (bool)),
	SLOT (restore_last_source (bool)));
vertical_layout->addWidget (Restore_Last_Source_CheckBox,
	0, Qt::AlignLeft | Qt::AlignVCenter);

++row;
grid_layout->addLayout (vertical_layout,
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Band Numbering.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Band Numbering section" << endl;
#endif
//		Heading line.
++row;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, 0, 1, -1, Qt::AlignBottom);
//		Heading Label.
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Band Numbering</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

vertical_layout = new QVBoxLayout;
QButtonGroup
	*button_group = new QButtonGroup (this);
//		By index.
Band_Numbers_Indexed_Button = new QRadioButton (tr ("By index: 0-based"));
Band_Numbers_Indexed_Button->setChecked (Band_Numbers_Indexed);
connect (Band_Numbers_Indexed_Button,
	SIGNAL (toggled (bool)), SLOT (band_numbers_indexed (bool)));
button_group->addButton (Band_Numbers_Indexed_Button);
vertical_layout->addWidget (Band_Numbers_Indexed_Button,
	0, Qt::AlignLeft | Qt::AlignVCenter);

//		By count.
QRadioButton
	*button = new QRadioButton (tr ("By count: 1-based"));
button->setChecked (! Band_Numbers_Indexed);
button_group->addButton (button);
vertical_layout->addWidget (button,
	0, Qt::AlignLeft | Qt::AlignVCenter);

++row;
grid_layout->addLayout (vertical_layout,
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Metadata.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Metadata section" << endl;
#endif
//		Heading line.
++row;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, 0, 1, -1, Qt::AlignBottom);
//		Heading Label.
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Metadata</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

vertical_layout = new QVBoxLayout;
//		Get PDS Label.
Get_PDS_Metadata_CheckBox = new QCheckBox
	(tr ("Try to get a PDS metdata label file for JP2 sources"));
Get_PDS_Metadata_CheckBox->setChecked (Get_PDS_Metadata);
connect (Get_PDS_Metadata_CheckBox, SIGNAL (toggled (bool)),
	SLOT (get_PDS_metadata (bool)));
vertical_layout->addWidget (Get_PDS_Metadata_CheckBox,
	0, Qt::AlignLeft | Qt::AlignVCenter);

++row;
grid_layout->addLayout (vertical_layout,
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Help Documentation.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Help Documentation section" << endl;
#endif
//		Heading line.
++row;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, 0, 1, -1, Qt::AlignBottom);
//		Heading Label.
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Help Documentation</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);

horizontal_layout = new QHBoxLayout;
//		Location.
//			Label.
QLabel
	*label = new QLabel (tr ("&Location:"));
horizontal_layout->addWidget (label,
	0, Qt::AlignLeft | Qt::AlignVCenter);
//			Value.
Documentation_Location_lineEdit = new QLineEdit (Documentation_Location);
Documentation_Location_lineEdit->setToolTip
	(tr ("Pathname or URL for the directory containing the ")
		+ Default_Documentation_Filename + tr (" file"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Documentation_Location_lineEdit);
#endif
//	N.B.: textEdited is not emitted when setText is used.
connect (Documentation_Location_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (documentation_location (const QString&)));
//	N.B.: editingFinished emitted when CR entered or widget loses focus.
connect (Documentation_Location_lineEdit, SIGNAL (editingFinished ()),
	SLOT (documentation_location_changed ()));
horizontal_layout->addWidget (Documentation_Location_lineEdit,
	100, Qt::AlignVCenter);
//			Find button.
QPushButton
	*push_button = new QPushButton (tr ("Find"), this);
QString
	tooltip (tr ("Try to find the ") + Default_Documentation_Filename
		+ tr (" documentation file in these locations:"));
for (int
		index = 0;
		index < Documentation_Search_Locations.size ();
	  ++index)
	{
	tooltip += "\n";
	tooltip += Documentation_Search_Locations[index];
	}
push_button->setToolTip (tooltip);
connect (push_button, SIGNAL (clicked ()),
	SLOT (find_documentation_location ()));
horizontal_layout->addWidget (push_button,
	0, Qt::AlignVCenter);
++row;
grid_layout->addLayout (horizontal_layout,
	row, 0, Qt::AlignLeft | Qt::AlignVCenter);
grid_layout->setColumnStretch (0, 100);

//		Heading line.
++row;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, 0, 1, -1, Qt::AlignBottom);

//		Change Coodinates Display Format Heading
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Coordinate Display Format</b>")),
	row, 0, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);
	
//		Longitude Coordinate Display Format Options
++row;
horizontal_layout = new QHBoxLayout;
horizontal_layout->addWidget(new QLabel (tr ("Longitude:")));

Longitude_Units_ComboBox = new QComboBox ();
Longitude_Units_ComboBox->setFrame (false);
Longitude_Units_ComboBox->setEditable (false);
Longitude_Units_ComboBox->addItem (tr ("degrees"));
Longitude_Units_ComboBox->addItem (tr ("h/m/s"));
Longitude_Units_ComboBox->addItem (tr ("radians"));
Longitude_Units_ComboBox->setCurrentIndex (Longitude_Units);
connect (Longitude_Units_ComboBox, 
	SIGNAL (currentIndexChanged (int)),
	SLOT (longitude_format (int)));

Longitude_Units_ComboBox->setFixedHeight (Longitude_Units_ComboBox->sizeHint().height ());
Longitude_Units_ComboBox->setFixedWidth (Longitude_Units_ComboBox->sizeHint ().width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Longitude_Units_ComboBox->setAutoFillBackground (true);
clog << "    Longitude_Units_ComboBox sizeHint = "
		<< Longitude_Units_ComboBox->sizeHint () << endl;
#endif
horizontal_layout->addWidget (Longitude_Units_ComboBox);

Longitude_Direction_ComboBox = new QComboBox ();
Longitude_Direction_ComboBox->setFrame (false);
Longitude_Direction_ComboBox->setEditable (false);
Longitude_Direction_ComboBox->addItem (tr ("east"));
Longitude_Direction_ComboBox->addItem (tr ("west"));
Longitude_Direction_ComboBox->setCurrentIndex (Longitude_Direction);
connect (Longitude_Direction_ComboBox,
	SIGNAL (currentIndexChanged (int)),
	SLOT (longitude_direction (int)));

Longitude_Direction_ComboBox->setFixedHeight (Longitude_Direction_ComboBox->sizeHint().height ());
Longitude_Direction_ComboBox->setFixedWidth (Longitude_Direction_ComboBox->sizeHint ().width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Longitude_Direction_ComboBox->setAutoFillBackground (true);
clog << "    Longitude_Direction_ComboBox sizeHint = "
		<< Longitude_Direction_ComboBox->sizeHint () << endl;
#endif
horizontal_layout->addWidget (Longitude_Direction_ComboBox);

grid_layout->addLayout (horizontal_layout,
	row, 0, Qt::AlignLeft | Qt::AlignVCenter);
	
//		Latitude Coordinates Display Format Options
++row;
horizontal_layout = new QHBoxLayout;
horizontal_layout->addWidget(new QLabel (tr ("Latitude:")));

Latitude_Units_ComboBox = new QComboBox ();
Latitude_Units_ComboBox->setFrame (false);
Latitude_Units_ComboBox->setEditable (false);
Latitude_Units_ComboBox->addItem (tr ("degrees"));
Latitude_Units_ComboBox->addItem (tr ("h/m/s"));
Latitude_Units_ComboBox->addItem (tr ("radians"));
Latitude_Units_ComboBox->setCurrentIndex (Latitude_Units);
connect (Latitude_Units_ComboBox,
	SIGNAL (currentIndexChanged (int)),
	SLOT (latitude_format (int)));
Latitude_Units_ComboBox->setFixedHeight (Latitude_Units_ComboBox->sizeHint().height ());
Latitude_Units_ComboBox->setFixedWidth (Latitude_Units_ComboBox->sizeHint ().width ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
Latitude_Units_ComboBox->setAutoFillBackground (true);
clog << "    Latitude_Units_ComboBox sizeHint = "
		<< Latitude_Units_ComboBox->sizeHint () << endl;
#endif
horizontal_layout->addWidget (Latitude_Units_ComboBox);

grid_layout->addLayout (horizontal_layout,
	row, 0, Qt::AlignLeft | Qt::AlignVCenter);



//			Reset button.
Documentation_Location_Reset_Button =
	new Icon_Button (*Reset_Button_Icon, this);
Documentation_Location_Reset_Button->setVisible (false);
Documentation_Location_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Documentation_Location_Reset_Button, SIGNAL (clicked ()),
	SLOT (documentation_location_reset ()));
grid_layout->addWidget (Documentation_Location_Reset_Button,
	row, 1, Qt::AlignLeft);
grid_layout->setColumnMinimumWidth
	(1, Documentation_Location_Reset_Button->iconSize ().width ());

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);
grid_layout->setRowStretch (row, 100);

//	Defaults/Apply buttons.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Defaults/Apply section" << endl;
#endif
++row;
QDialogButtonBox
	*buttons = new QDialogButtonBox (Qt::Horizontal, this);
Defaults_Button = buttons->addButton
	(tr ("Defaults"), QDialogButtonBox::ResetRole);
if (Defaults_Button_Icon)
	Defaults_Button->setIcon (*Defaults_Button_Icon);
connect (Defaults_Button, SIGNAL (clicked ()), SLOT (defaults ()));
QAction
	*action = new QAction (tr ("Defaults"), this);
action->setShortcut (tr ("Ctrl+Shift+D"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
addAction (action);
connect (action, SIGNAL (triggered ()),
		 Defaults_Button, SLOT (click ()));

Apply_Button = buttons->addButton
	(tr ("Apply"), QDialogButtonBox::ApplyRole);
if (Apply_Button_Icon)
	Apply_Button->setIcon (*Apply_Button_Icon);
connect (Apply_Button, SIGNAL (clicked ()), SLOT (apply ()));
action = new QAction (tr ("Apply"), this);
action->setShortcut (tr ("Ctrl+Shift+A"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
connect (action, SIGNAL (triggered ()),
		 Apply_Button, SLOT (click ()));
addAction (action);
grid_layout->addWidget (buttons,
	row, 0, 1, -1);

//	Initialize the documentation location.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "     initialize documentation location" << endl;
#endif
Preferences_Dialog::Docs_Helper->location (Documentation_Location);
documentation_location_is_valid (Documentation_Location);

//	Apply any changes.
apply ();

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "<<< General_Section" << endl;
#endif
}


void
General_Section::restore_layout
	(
	bool	enabled
	)
{
if (Restore_Layout_CheckBox->isChecked () != enabled)
	Restore_Layout_CheckBox->setChecked (enabled);
else
	reset_modifier_buttons ();
}


void
General_Section::restore_last_source
	(
	bool	enabled
	)
{
if (Restore_Last_Source_CheckBox->isChecked () != enabled)
	Restore_Last_Source_CheckBox->setChecked (enabled);
else
	reset_modifier_buttons ();
}


void
General_Section::band_numbers_indexed
	(
	bool	indexed
	)
{
if (Band_Numbers_Indexed_Button->isChecked () != indexed)
	Band_Numbers_Indexed_Button->setChecked (indexed);
else
	reset_modifier_buttons ();
}

void
General_Section::longitude_format 
	(
	int format
	)
{
if (format < 0 || format > 2)
	return;
if (Longitude_Units_ComboBox->currentIndex() != format)
	Longitude_Units_ComboBox->setCurrentIndex(format);
else
	reset_modifier_buttons ();
}

void
General_Section::longitude_direction
	(
	int direction
	)
{
if(direction < 0 || direction > 1)
	return;
if (Longitude_Direction_ComboBox->currentIndex() != direction)
	Longitude_Direction_ComboBox->setCurrentIndex(direction);
else
	reset_modifier_buttons();
}

void
General_Section::latitude_format
	(
	int format
	)
{
if (format < 0 || format > 2)
	return;
if (Latitude_Units_ComboBox->currentIndex() != format)
	Latitude_Units_ComboBox->setCurrentIndex(format);
else
	reset_modifier_buttons ();
}

void
General_Section::get_PDS_metadata
	(
	bool	enabled
	)
{
if (Get_PDS_Metadata_CheckBox->isChecked () != enabled)
	Get_PDS_Metadata_CheckBox->setChecked (enabled);
else
	reset_modifier_buttons ();
}


void
General_Section::documentation_location
	(
	const QString&	location
	)
{
if (Documentation_Location_lineEdit)
	{
	if (Documentation_Location_lineEdit->text () != location)
		Documentation_Location_lineEdit->setText (location);
	Documentation_Location_Reset_Button->setVisible
		(location != Documentation_Location);
	reset_modifier_buttons ();
	}
}


bool
General_Section::documentation_location_is_valid
	(
	const QString&	location
	)
{
bool
	accepted = true;
if (location.isEmpty ())
	Documentation_Location_Pending = location;
else
	{
	QString
		source (location);
	source += QDir::separator ();
	source += Default_Documentation_Filename;
	if (Preferences_Dialog::Docs_Helper->is_accessible (source) ||
		QMessageBox::question ((isVisible () ? this : NULL), Title,
			tr ("The ") + Default_Documentation_Filename
				+ tr (" documentation file was not found\n")
			+ tr ("at the ") + location + tr (" location.\n\n")
			+ tr ("Use the location anyway?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No)
		== QMessageBox::Yes)
		Documentation_Location_Pending = location;
	else
		accepted = false;
	}

documentation_location (Documentation_Location_Pending);

return accepted;
}


void
General_Section::documentation_location_changed ()
{
//	Avoid redundant editingFinished signals.
bool
	blocked = Documentation_Location_lineEdit->blockSignals (true);
if (Documentation_Location_lineEdit->text ()
		!= Documentation_Location_Pending &&
	! Defaults_Button->hasFocus ())
	documentation_location_is_valid (Documentation_Location_lineEdit->text ());
Documentation_Location_lineEdit->blockSignals (blocked);
}


bool
General_Section::find_documentation_location ()
{
#if ((DEBUG_SECTION) & DEBUG_GENERAL)
clog << ">>> General_Section::find_documentation_location" << endl
	 << "    searching for " << Default_Documentation_Filename << endl;
#endif
Documentation_Location_Pending = Preferences_Dialog::Docs_Helper->find
	(Default_Documentation_Filename, Documentation_Search_Locations);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GENERAL))
clog << "      Documentation_Location_Pending = "
		<< Documentation_Location_Pending << endl;
#endif
if (! Documentation_Location_Pending.isEmpty ())
	documentation_location (Documentation_Location_Pending);
#if ((DEBUG_SECTION) & DEBUG_GENERAL)
clog << "<<< General_Section::find_documentation_location: "
		<< (! Documentation_Location_Pending.isEmpty ()) << endl;
#endif
return ! Documentation_Location_Pending.isEmpty ();
}


void
General_Section::documentation_location_reset ()
{documentation_location (Documentation_Location);}


void
General_Section::defaults ()
{
restore_layout (Default_Restore_Layout);
restore_last_source (Default_Restore_Last_Source);
band_numbers_indexed (Default_Band_Numbers_Indexed);
get_PDS_metadata(Default_Get_PDS_Metadata);
longitude_format(Default_Coordinate_Format);
longitude_direction(Default_Coordinate_Format);
latitude_format(Default_Coordinate_Format);
//	No Default_Documentation_Location.
}


void
General_Section::reset_defaults_button ()
{
Defaults_Button->setEnabled
	(
	(Restore_Layout_CheckBox->isChecked ()
		!= Default_Restore_Layout)				||
	(Restore_Last_Source_CheckBox->isChecked ()
		!= Default_Restore_Last_Source)			||
	(Band_Numbers_Indexed_Button->isChecked ()
		!= Default_Band_Numbers_Indexed)		||
	(Get_PDS_Metadata_CheckBox->isChecked ()
		!= Default_Get_PDS_Metadata)			||
	(Longitude_Units_ComboBox->currentIndex()
		!= Default_Coordinate_Format)			||
	(Longitude_Direction_ComboBox->currentIndex()
		!= Default_Coordinate_Format)			||
	(Latitude_Units_ComboBox->currentIndex()
		!= Default_Coordinate_Format)
	//	No Default_Documentation_Location.
	);
#if ((DEBUG_SECTION) & DEBUG_GENERAL)
clog << ">-< General_Section::reset_defaults_button:" << endl
	 << "            Restore_Layout_CheckBox isChecked = "
	 	<< Restore_Layout_CheckBox->isChecked () << endl
	 << "                       Default_Restore_Layout = "
	 	<< Default_Restore_Layout << endl
	 << "                    Default_Restore_Last_Source = "
	 	<< Default_Restore_Last_Source << endl
	 << "                   Default_Band_Numbers_Indexed = "
	 	<< Default_Band_Numbers_Indexed << endl
	 << "                       Default_Get_PDS_Metadata = "
	 	<< Default_Get_PDS_Metadata << endl
	 << "                      Defaults_Button isEnabled = "
	 	<< Defaults_Button->isEnabled () << endl;
#endif
}


void
General_Section::reset_modifier_buttons ()
{
reset_defaults_button ();
Apply_Button->setEnabled (has_changed ());
}


bool
General_Section::has_changed () const
{
#if ((DEBUG_SECTION) & DEBUG_GENERAL)
bool
	changed =
#else
return
#endif
	(Restore_Layout_CheckBox->isChecked ()
		!= Restore_Layout)						||
	(Restore_Last_Source_CheckBox->isChecked ()
		!= Restore_Last_Source)					||
	(Band_Numbers_Indexed_Button->isChecked ()
		!= Band_Numbers_Indexed)				||
	(Get_PDS_Metadata_CheckBox->isChecked ()
		!= Get_PDS_Metadata)						||
	(Documentation_Location_lineEdit->text ()
		!= Documentation_Location)				||
	(Longitude_Units_ComboBox->currentIndex()
		!= Longitude_Units)						||
	(Longitude_Direction_ComboBox->currentIndex()
		!= Longitude_Direction)					||
	(Latitude_Units_ComboBox->currentIndex()
		!= Latitude_Units);
#if ((DEBUG_SECTION) & DEBUG_GENERAL)
clog << ">-< General_Section::has_changed:" << endl
	 << "    Restore_Layout_CheckBox isChecked = "
	 	<< Restore_Layout_CheckBox->isChecked () << endl
	 << "                       Restore_Layout = "
	 	<< Restore_Layout << endl
	 << "                  Restore_Last_Source = "
	 	<< Restore_Last_Source << endl
	 << "                 Band_Numbers_Indexed = "
	 	<< Band_Numbers_Indexed << endl
	 << "                     Get_PDS_Metadata = "
	 	<< Get_PDS_Metadata << endl
	 << "               Documentation_Location = "
	 	<< Documentation_Location << endl
	 << "                              changed = "
	 	<< changed << endl;
return changed;
#endif
}


void
General_Section::reset ()
{
restore_layout (Restore_Layout);
restore_last_source (Restore_Last_Source);
band_numbers_indexed (Band_Numbers_Indexed);
get_PDS_metadata(Get_PDS_Metadata);
latitude_format(Latitude_Units);
longitude_format(Longitude_Units);
longitude_direction(Longitude_Direction);
}


void
General_Section::apply ()
{
QSettings
	settings;
settings.beginGroup (Preferences_Dialog::LAYOUT_GEOMETRY_SECTION);
if (Restore_Layout != Restore_Layout_CheckBox->isChecked ())
	{
	Restore_Layout = Restore_Layout_CheckBox->isChecked ();
	settings.setValue (RESTORE_LAYOUT_KEY, Restore_Layout);
	}
if (Restore_Last_Source != Restore_Last_Source_CheckBox->isChecked ())
	{
	Restore_Last_Source = Restore_Last_Source_CheckBox->isChecked ();
	settings.setValue (RESTORE_LAST_SOURCE_KEY, Restore_Last_Source);
	}
settings.endGroup ();	
if (Band_Numbers_Indexed != Band_Numbers_Indexed_Button->isChecked ())
	{
	Band_Numbers_Indexed = Band_Numbers_Indexed_Button->isChecked ();
	settings.setValue (BAND_NUMBERS_INDEXED_KEY, Band_Numbers_Indexed);

	//	Set the global variable.
	HiView_Utilities::Band_Numbering_Indexed = Band_Numbers_Indexed;

	//	>>> SIGNAL <<<
	emit band_numbers_indexed_changed (Band_Numbers_Indexed);
	}

if (Latitude_Units != Latitude_Units_ComboBox->currentIndex()) {
	Latitude_Units = Latitude_Units_ComboBox->currentIndex();
	settings.setValue (RESTORE_LATITUDE_FORMAT_KEY, Latitude_Units);
	emit latitude_units_changed (Latitude_Units);
}
if (Longitude_Units != Longitude_Units_ComboBox->currentIndex()) {
	Longitude_Units = Longitude_Units_ComboBox->currentIndex();
	settings.setValue (RESTORE_LONGITUDE_FORMAT_KEY, Longitude_Units);
	emit longitude_units_changed(Longitude_Units);
}
if (Longitude_Direction != Longitude_Direction_ComboBox->currentIndex()) {
	Longitude_Direction = Longitude_Direction_ComboBox->currentIndex();
	settings.setValue (RESTORE_LONGITUDE_DIRECTION_KEY, Longitude_Direction);
	emit longitude_direction_changed (Longitude_Direction);
}
if (Get_PDS_Metadata != Get_PDS_Metadata_CheckBox->isChecked ())
	{
	Get_PDS_Metadata = Get_PDS_Metadata_CheckBox->isChecked ();
	settings.setValue (GET_PDS_LABEL_KEY, Get_PDS_Metadata);
	}
if (Documentation_Location != Documentation_Location_lineEdit->text ())
	{
	Documentation_Location = Documentation_Location_lineEdit->text ();
	Documentation_Location_Reset_Button->setVisible (false);
	settings.setValue (DOCUMENTATION_LOCATION_KEY, Documentation_Location);
	//	Set the documentation location in the Docs_Helper.
	Preferences_Dialog::Docs_Helper->location (Documentation_Location);
	//	>>> SIGNAL <<<
	emit documentation_location_changed (Documentation_Location);
	}

reset_modifier_buttons ();
}

/*=*****************************************************************************
	Sources_Section
*/
const char
	*Sources_Section::SOURCE_LIST_CAPACITY_KEY	= "Source_List_Capacity",
	*Sources_Section::SOURCE_LIST_KEY			= "Source_List";

#ifndef	DEFAULT_SOURCE_LIST_CAPACITY
#define DEFAULT_SOURCE_LIST_CAPACITY		32
#endif
int
	Sources_Section::Default_Source_List_Capacity
		= DEFAULT_SOURCE_LIST_CAPACITY;

#ifndef	DEFAULT_SOURCE_LIST_CAPACITY_MAX
#define DEFAULT_SOURCE_LIST_CAPACITY_MAX	100
#endif
int
	Sources_Section::Default_Source_List_Capacity_Max
		= DEFAULT_SOURCE_LIST_CAPACITY_MAX;


Sources_Section::Sources_Section
	(
	QWidget*	parent
	)
	:	QWidget (parent)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << ">>> Sources_Section" << endl;
#endif
if (parent)
	Title = parent->windowTitle ();
else
	Title = tr ("Preferences");
Title += tr (": Sources");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "    Title = \"" << Title << '"' << endl;
#endif

//	Initialize values.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "    settings -" << endl;
#endif
QSettings
	settings;
bool
	OK;

int
	capacity =
	Capacity =
		settings.value (SOURCE_LIST_CAPACITY_KEY,
			Default_Source_List_Capacity).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "    " << SOURCE_LIST_CAPACITY_KEY << " = " << Capacity << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + SOURCE_LIST_CAPACITY_KEY + " \""
		+ settings.value (SOURCE_LIST_CAPACITY_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Source_List_Capacity)
		+ tr (" is being used."));
	capacity = Default_Source_List_Capacity;
	}
else
if (! settings.contains (SOURCE_LIST_CAPACITY_KEY))
	settings.setValue (SOURCE_LIST_CAPACITY_KEY, Default_Source_List_Capacity);

//	Start with an empty list to cause widget loading of non-empty list.
Source_List = new QStringList;
QStringList
	list (settings.value (SOURCE_LIST_KEY).value<QStringList> ());
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "    " << SOURCE_LIST_KEY << " =" << endl;
for (int
		index = 0;
		index < list.size ();
		index++)
	clog << "    " << index << ": " << list.at (index) << endl;
#endif
if (! settings.contains (SOURCE_LIST_KEY))
	settings.setValue (SOURCE_LIST_KEY, *Source_List);

//	Layout controls.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "    layout controls -" << endl;
#endif
QGridLayout
	*grid_layout = new QGridLayout (this);
QHBoxLayout
	*horizontal_layout = new QHBoxLayout;
QLabel
	*label;
int
	row = -1;

//	Row 0:
++row;
//		Entries.
label = new QLabel (tr ("Entries:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
horizontal_layout->addWidget (label);

Entries = new QLabel (this);
horizontal_layout->addWidget (Entries);

horizontal_layout->addSpacing (10);

//		Capacity.
//			Label.
label = new QLabel (tr ("&Capacity:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
horizontal_layout->addWidget (label);
//			Value.
Capacity_spinBox = new QSpinBox (this);
Capacity_spinBox->setAlignment (Qt::AlignRight);
Capacity_spinBox->setMinimum (1);
Capacity_spinBox->setMaximum (Default_Source_List_Capacity_Max);
Capacity_spinBox->setValue (Capacity);
Capacity_spinBox->setKeyboardTracking (true);
Capacity_spinBox->setToolTip (tr ("Maximum source list entries"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Capacity_spinBox);
#endif
connect (Capacity_spinBox, SIGNAL (valueChanged (int)),
	SLOT (reset_modifier_buttons ()));
connect (Capacity_spinBox, SIGNAL (editingFinished ()),
	SLOT (source_list_capacity_change ()));
horizontal_layout->addWidget (Capacity_spinBox);
//			Reset button.
Capacity_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Capacity_Reset_Button->setVisible (false);
Capacity_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Capacity_Reset_Button, SIGNAL (clicked ()),
	SLOT (source_list_capacity_reset ()));
horizontal_layout->addWidget (Capacity_Reset_Button);

//		Padding
horizontal_layout->addStretch (100);
grid_layout->addLayout (horizontal_layout,
	row, 0);

//	Row 1:
++row;
//		Source List.
Source_List_Widget = new QListWidget (this);
Source_List_Widget->setMinimumSize (500, 250);
Source_List_Widget->setAlternatingRowColors (true);
Source_List_Widget->setHorizontalScrollMode
	(QAbstractItemView::ScrollPerPixel);
Source_List_Widget->setVerticalScrollMode
	(QAbstractItemView::ScrollPerItem);
Source_List_Widget->setTextElideMode
	(Qt::ElideNone);
Source_List_Widget->setDragEnabled (true);
Source_List_Widget->viewport ()->setAcceptDrops (true);
Source_List_Widget->setDragDropMode
	(QAbstractItemView::InternalMove);
Source_List_Widget->setDropIndicatorShown (true);
Source_List_Widget->setSelectionBehavior
	(QAbstractItemView::SelectItems);
Source_List_Widget->setSelectionMode
	(QAbstractItemView::ExtendedSelection);
Source_List_Widget->setEditTriggers
	(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
connect (Source_List_Widget, SIGNAL (itemSelectionChanged ()),
	SLOT (source_list_selection_changed ()));
connect (Source_List_Widget, SIGNAL (itemChanged (QListWidgetItem*)),
	SLOT (source_list_changed (QListWidgetItem*)));

//		List reset button (LR corner of Source List).
Source_List_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Source_List_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Source_List_Reset_Button, SIGNAL (clicked ()),
	SLOT (source_list_reset ()));
Source_List_Widget->setVerticalScrollBarPolicy
	(Qt::ScrollBarAlwaysOn);
Source_List_Widget->setHorizontalScrollBarPolicy
	(Qt::ScrollBarAlwaysOn);
Source_List_Widget->setCornerWidget (Source_List_Reset_Button);
Source_List_Reset_Button->setVisible (false);
grid_layout->addWidget (Source_List_Widget,
	row, 0);
grid_layout->setRowStretch
	(row, 100);

//		Edit button.
QVBoxLayout
	*vertical_layout = new QVBoxLayout ();
Edit_Button = new QPushButton (tr ("Edit"), this);
Edit_Button->setEnabled (false);
connect (Edit_Button, SIGNAL (clicked ()),
	SLOT (source_list_edit_item ()));
vertical_layout->addWidget (Edit_Button);

//		Remove button.
Remove_Button = new QPushButton (tr ("Remove"), this);
Remove_Button->setEnabled (false);
connect (Remove_Button, SIGNAL (clicked ()),
	SLOT (source_list_remove_items ()));
vertical_layout->addWidget (Remove_Button);

//		Spacing
vertical_layout->addStretch (100);
grid_layout->addLayout (vertical_layout,
	row, 1);

//	Defaults/Apply buttons.
++row;
QDialogButtonBox
	*buttons = new QDialogButtonBox (Qt::Horizontal, this);
Defaults_Button = buttons->addButton
	(tr ("Defaults"), QDialogButtonBox::ResetRole);
Defaults_Button->setIcon (*Defaults_Button_Icon);
connect (Defaults_Button, SIGNAL (clicked ()), SLOT (defaults ()));
QAction
	*action = new QAction (tr ("Defaults"), this);
action->setShortcut (tr ("Ctrl+Shift+D"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
addAction (action);
connect (action, SIGNAL (triggered ()),
		 Defaults_Button, SLOT (click ()));
Apply_Button    = buttons->addButton (QDialogButtonBox::Apply);
if (Apply_Button_Icon)
	Apply_Button->setIcon (*Apply_Button_Icon);
connect (Apply_Button, SIGNAL (clicked ()), SLOT (apply ()));
action = new QAction (tr ("Apply"), this);
action->setShortcut (tr ("Ctrl+Shift+A"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
connect (action, SIGNAL (triggered ()),
		 Apply_Button, SLOT (click ()));
addAction (action);
grid_layout->addWidget (buttons,
	row, 0, 1, -1);

/*	Initialize values.

	This is done after the GUI has been constructed to allow for
	changes to the initial settings as a result of list resizing
	or capacity changes.
*/
source_list_capacity (capacity);
source_list (list);

//	Update settings with any changed values.
apply ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SOURCES))
clog << "<<< Sources_Section" << endl;
#endif
}


void
Sources_Section::source_list_capacity
	(
	int		capacity
	)
{
if (capacity < 1)
	capacity = 1;
else
if (capacity > Default_Source_List_Capacity_Max)
	{
	capacity = Default_Source_List_Capacity_Max;
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The source list capacity has been limited to the maximimum of ")
		+ QString::number (Default_Source_List_Capacity_Max) + '.');
	}

int
	entries = Source_List_Widget->count ();
if (entries > capacity)
	{
	if (QMessageBox::question ((isVisible () ? this : NULL), Title,
			tr ("The source list capacity of ")
			+ QString::number (capacity)
			+ tr (" is less than the ")
			+ QString::number (entries)
			+ tr (" entries in the sources list.\n\n")
			+ tr ("Increase the capacity?\n")
			+ tr ("If No, excess entries will be removed from the list."),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes)
		== QMessageBox::Yes)
		capacity = entries;
	else
		{
		while (entries > capacity)
			delete Source_List_Widget->takeItem (--entries);
		Entries->setNum (Source_List_Widget->count ());
		}
	}

if (capacity != Capacity_spinBox->value ())
	Capacity_spinBox->setValue (capacity);

reset_modifier_buttons ();
}


void
Sources_Section::source_list_capacity_change ()
{source_list_capacity (Capacity_spinBox->value ());}


void
Sources_Section::source_list_capacity_reset ()
{source_list_capacity (Capacity);}


#ifndef DOXYGEN_PROCESSING
namespace
{
bool
operator==
	(
	const QListWidget&	list,
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
		if (list.item (count)->text () != strings.at (count))
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
	const QListWidget&	list
	)
{return list == strings;}


bool
operator!=
	(
	const QListWidget&	list,
	const QStringList&	strings
	)
{return ! (list == strings);}


bool
operator!=
	(
	const QStringList&	strings,
	const QListWidget&	list
	)
{return ! (list == strings);}

}	//	local namespace.
#endif
	

void
Sources_Section::source_list
	(
	const QStringList&	list
	)
{
bool
	changed = false;
if (*Source_List != list)
	{
	changed = true;
	*Source_List = list;

	//	Remove any duplicates.
	int
		here,
		there;
	for (here = 0;
		 here < (Source_List->count () - 1);
		 here++)
		{
		there = Source_List->count ();
		while (--there > here)
			if (Source_List->at (there) == Source_List->at (here))
				Source_List->removeAt (there);
		}

	/*	Keep the list size under the capacity.

		N.B. Excess entries are silently removed.
	*/
	here = Capacity_spinBox->value ();
	while (Source_List->count () > here)
		Source_List->removeLast ();
	}
	
if (*Source_List != *Source_List_Widget)
	{
	Source_List_Widget->clear ();
	for (int
			index = 0;
			index < Source_List->count ();
		  ++index)
		{
		Source_List_Widget->addItem (Source_List->at (index));
		Source_List_Widget->item (index)->setFlags
			(Source_List_Widget->item (index)->flags () |
				Qt::ItemIsEditable);
		}
	//	Make sure the viewport is refreshed.
	Source_List_Widget->viewport ()->update ();

	//	Reset the number of entries.
	Entries->setNum (Source_List_Widget->count ());
	}

reset_modifier_buttons ();

if (changed)
	{
	QSettings
		settings;
	settings.setValue (SOURCE_LIST_KEY, *Source_List);
	//	>>> SIGNAL <<<
	emit source_list_changed (*Source_List);
	}
}


void
Sources_Section::source_list_selection_changed ()
{
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << ">>> Sources_Section::source_list_selection_changed" << endl;
#endif
int
	count = Source_List_Widget->selectedItems ().count ();
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << "    items selected = " << count << endl
	 << "       current row = " << Source_List_Widget->currentRow () << endl;
#endif
Edit_Button->setEnabled (count == 1);
Remove_Button->setEnabled (count != 0);
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << "<<< Sources_Section::source_list_selection_changed" << endl;
#endif
}


void
Sources_Section::source_list_changed
	(
	QListWidgetItem*
	)
{reset_modifier_buttons ();}



void
Sources_Section::source_list_edit_item ()
{
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << ">>> Sources_Section::source_list_edit_item" << endl;
#endif
QListWidgetItem
	*item = Source_List_Widget->currentItem ();
if (item)
	{
	#if ((DEBUG_SECTION) & DEBUG_SOURCES)
	clog << "    visualItemRect = "
			<< Source_List_Widget->visualItemRect (item) << endl
		 << "     viewport rect = "
		 	<< Source_List_Widget->viewport ()->rect () << endl;
	#endif
	if (Source_List_Widget->visualItemRect (item).bottom ()
		> Source_List_Widget->viewport ()->rect ().height ())
		Source_List_Widget->scrollToItem (item);
	Source_List_Widget->editItem (item);
	}
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << "<<< Sources_Section::source_list_edit_item" << endl;
#endif
}


void
Sources_Section::source_list_remove_items ()
{
#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << ">>> Sources_Section::source_list_remove_items" << endl;
#endif
QList<QListWidgetItem*>
	selected_items (Source_List_Widget->selectedItems ());
int
	count = selected_items.count ();
if (count)
	{
	int
		entries = Source_List_Widget->count ();
	QListWidgetItem
		*top_item = NULL;
	if (count != entries)
		{
		//	Find the item at the top of the viewport.
		#if ((DEBUG_SECTION) & DEBUG_SOURCES)
		clog << "     viewport rect = "
			 	<< Source_List_Widget->viewport ()->rect () << endl
			 << "    searching for top item in viewport -" << endl;
		#endif
		int
			top_row;
		for (top_row = 0;
			 top_row < entries;
			 top_row++)
			{
			top_item = Source_List_Widget->item (top_row);
			#if ((DEBUG_SECTION) & DEBUG_SOURCES)
			clog << "    " << top_row << ": visualItemRect = "
					<< Source_List_Widget->visualItemRect (top_item) << endl;
			#endif
			if (Source_List_Widget->visualItemRect (top_item).top () >= 0)
				break;
			}
			
		if (selected_items.contains (top_item))
			{
			//	Find the next unselected item.
			#if ((DEBUG_SECTION) & DEBUG_SOURCES)
			clog << "    searching for next unselected item -" << endl;
			#endif
			int
				row = top_row;
			while (++row < entries)
				{
				#if ((DEBUG_SECTION) & DEBUG_SOURCES)
				clog << "    " << row << endl;
				#endif
				top_item = Source_List_Widget->item (row);
				if (! selected_items.contains (top_item))
					break;
				}
			if (row == entries)
				{
				//	Find the previous unselected item.
				#if ((DEBUG_SECTION) & DEBUG_SOURCES)
				clog << "    searching for previous unselected item -" << endl;
				#endif
				row = top_row;
				while (--row >= 0)
					{
					#if ((DEBUG_SECTION) & DEBUG_SOURCES)
					clog << "    " << row << endl;
					#endif
					top_item = Source_List_Widget->item (row);
					if (! selected_items.contains (top_item))
						break;
					}
				}
			}
		}

	//	Remove the selected items.
	while (count--)
		{
		#if ((DEBUG_SECTION) & DEBUG_SOURCES)
		clog << "    items removed -" << endl
			 << "    " << Source_List_Widget->row (selected_items.at (count))
				<< ": " << selected_items.at (count)->text () << endl;
		#endif
		delete Source_List_Widget->takeItem
			(Source_List_Widget->row (selected_items.at (count)));
		}
	Entries->setNum (Source_List_Widget->count ());
	
	if (top_item)
		//	Scroll the top item to the top of the viewport.
		Source_List_Widget->scrollToItem
			(top_item, QAbstractItemView::PositionAtTop);

	reset_modifier_buttons ();
	}

#if ((DEBUG_SECTION) & DEBUG_SOURCES)
clog << "<<< Sources_Section::source_list_remove_items" << endl;
#endif
}


void
Sources_Section::source_list_reset ()
{
int
	capacity = Capacity_spinBox->value (),
	entries = Source_List->count ();
if (entries > capacity)
	{
	if (QMessageBox::question ((isVisible () ? this : NULL), Title,
			tr ("The source list capacity of ")
			+ QString::number (capacity)
			+ tr (" is less than the ")
			+ QString::number (entries)
			+ tr (" entries in the sources list.\n\n")
			+ tr ("Increase the capacity?\n")
			+ tr ("If No, excess entries will be removed from the list."),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes)
		== QMessageBox::Yes)
		source_list_capacity (entries);
	else
		//	Remove excess entries.
		while (entries-- > capacity)
			Source_List->takeLast ();
	}

//	Update the source list.
source_list (*Source_List);
}


void
Sources_Section::reset ()
{
source_list_capacity_reset ();
source_list_reset ();
}


void
Sources_Section::defaults ()
{source_list_capacity (Default_Source_List_Capacity);}


void
Sources_Section::reset_modifier_buttons ()
{
bool
	capacity_changed = (Capacity != Capacity_spinBox->value ()),
	list_changed = (*Source_List != *Source_List_Widget);
Capacity_Reset_Button->setVisible (capacity_changed);
Source_List_Reset_Button->setVisible (list_changed);
Apply_Button->setEnabled (list_changed || capacity_changed);

int
	count = Source_List_Widget->selectedItems ().count ();
Edit_Button->setEnabled (count == 1);
Remove_Button->setEnabled (count != 0);

Defaults_Button->setEnabled
	(Default_Source_List_Capacity != Capacity_spinBox->value ());
}


void
Sources_Section::apply ()
{
QSettings
	settings;
if (Capacity != Capacity_spinBox->value ())
	{
	Capacity = Capacity_spinBox->value ();
	settings.setValue (SOURCE_LIST_CAPACITY_KEY, Capacity);
	//	>>> SIGNAL <<<
	emit source_list_capacity_changed (Capacity);
	}
if (*Source_List != *Source_List_Widget)
	{
	Source_List->clear ();
	for (int
			index = 0;
			index < Source_List_Widget->count ();
			index++)
		Source_List->append (Source_List_Widget->item (index)->text ());
	settings.setValue (SOURCE_LIST_KEY, *Source_List);
	//	>>> SIGNAL <<<
	emit source_list_changed (*Source_List);
	}
reset_modifier_buttons ();
}


bool
Sources_Section::has_changed () const
{
return
	Capacity != Capacity_spinBox->value () ||
	*Source_List != *Source_List_Widget;
}

/*=*****************************************************************************
	Rendering_Section
*/
const char
	*Rendering_Section::INITIAL_SCALE_KEY			= "Initial_Scale";
#ifndef DEFAULT_INITIAL_SCALE
#define DEFAULT_INITIAL_SCALE \
	Preferences_Dialog::INITIAL_SCALE_AUTO_FIT
#endif
double
	Rendering_Section::Default_Initial_Scale
		= DEFAULT_INITIAL_SCALE;

const char
	*Rendering_Section::MIN_SCALE_KEY				= "Min_Scale";
#ifndef DEFAULT_MIN_SCALE
#define DEFAULT_MIN_SCALE					0.01
#endif
double
	Rendering_Section::Default_Min_Scale
		= DEFAULT_MIN_SCALE;
#ifndef MIN_SCALING
#define MIN_SCALING							0.001
#endif

const char
	*Rendering_Section::MAX_SCALE_KEY				= "Max_Scale";
#ifndef DEFAULT_MAX_SCALE
#define DEFAULT_MAX_SCALE					10.0
#endif
double
	Rendering_Section::Default_Max_Scale
		= DEFAULT_MAX_SCALE;
#ifndef MAX_SCALING
#define MAX_SCALING							100.0
#endif

const char
	*Rendering_Section::SCALING_MINOR_INCREMENT_KEY	= "Scaling_Minor_Increment";
#ifndef DEFAULT_SCALING_MINOR_INCREMENT
#define DEFAULT_SCALING_MINOR_INCREMENT		0.01
#endif
double
	Rendering_Section::Default_Scaling_Minor_Increment
		= DEFAULT_SCALING_MINOR_INCREMENT;

const char
	*Rendering_Section::SCALING_MAJOR_INCREMENT_KEY	= "Scaling_Major_Increment";
#ifndef DEFAULT_SCALING_MAJOR_INCREMENT
#define DEFAULT_SCALING_MAJOR_INCREMENT		0.1
#endif
double
	Rendering_Section::Default_Scaling_Major_Increment
		= DEFAULT_SCALING_MAJOR_INCREMENT;
#ifndef MAX_SCALING_INCREMENT
#define MAX_SCALING_INCREMENT				1.0
#endif


const char
	*Rendering_Section::CONTRAST_STRETCH_UPPER_KEY	= "Contrast_Stretch_Upper";
#ifndef DEFAULT_CONTRAST_STRETCH_UPPER
#define DEFAULT_CONTRAST_STRETCH_UPPER	{0.01, 0.01, 0.01}
#endif
double
	Rendering_Section::Default_Contrast_Stretch_Upper[3]
		= DEFAULT_CONTRAST_STRETCH_UPPER;

const char
	*Rendering_Section::CONTRAST_STRETCH_LOWER_KEY	= "Contrast_Stretch_Lower";
#ifndef DEFAULT_CONTRAST_STRETCH_LOWER
#define DEFAULT_CONTRAST_STRETCH_LOWER	{0.10, 0.10, 0.10}
#endif
double
	Rendering_Section::Default_Contrast_Stretch_Lower[3]
		= DEFAULT_CONTRAST_STRETCH_LOWER;


const char
	*Rendering_Section::BACKGROUND_COLOR_KEY		= "Background_Color";
	
const char
	*Rendering_Section::LINE_COLOR_KEY 				= "Line_Color";

#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)				"" #__VA_ARGS__ ""
#define AS_STRING(...)					STRINGIFIED(__VA_ARGS__)
#endif

#ifndef DEFAULT_BACKGROUND_COLOR
#define DEFAULT_BACKGROUND_COLOR		black
#endif

#define _DEFAULT_BACKGROUND_COLOR_		AS_STRING(DEFAULT_BACKGROUND_COLOR)
QRgb
	Rendering_Section::Default_Background_Color
		= QColor (_DEFAULT_BACKGROUND_COLOR_).rgba ();

#ifndef DEFAULT_LINE_COLOR
#define DEFAULT_LINE_COLOR				"#ff0000"
#endif

const char
	*Rendering_Section::TILE_SIZE_KEY				= "Tile_Size";
#ifndef	MIN_TILE_SIZE
#define MIN_TILE_SIZE						256
#endif
#ifndef DEFAULT_TILE_SIZE
#define DEFAULT_TILE_SIZE					(MIN_TILE_SIZE << 2)
#endif
int
	Rendering_Section::Default_Tile_Size
		= DEFAULT_TILE_SIZE;
#ifndef MAX_TILE_SIZE
#define MAX_TILE_SIZE						8192
#endif

const char
	*Rendering_Section::RENDERING_INCREMENT_LINES_KEY
		= "Rendering_Increment_Lines";

#ifndef DEFAULT_RENDERING_INCREMENT_LINES
#define DEFAULT_RENDERING_INCREMENT_LINES	100
#endif
int
	Rendering_Section::Default_Rendering_Increment_Lines
		= DEFAULT_RENDERING_INCREMENT_LINES;


Rendering_Section::Rendering_Section
	(
	QWidget*	parent
	)
	:	QWidget (parent)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << ">>> Rendering_Section" << endl;
#endif
if (parent)
	Title = parent->windowTitle ();
else
	Title = tr ("Preferences");
Title += tr (": Rendering");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    Title = \"" << Title << '"' << endl;
#endif

//	Initialize values.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    settings -" << endl;
#endif
QSettings
	settings;
bool
	OK;

/*------------------------------------------------------------------------------
	Scaling
*/
double
	initial_scaling =
	Initial_Scale = settings.value (INITIAL_SCALE_KEY,
		Default_Initial_Scale).toDouble (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << INITIAL_SCALE_KEY << " = " << Initial_Scale << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + INITIAL_SCALE_KEY + " \""
		+ settings.value (INITIAL_SCALE_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Initial_Scale)
		+ tr (" (fit to viewport) is being used."));
	initial_scaling = Default_Initial_Scale;
	}
else
if (! settings.contains (INITIAL_SCALE_KEY))
	settings.setValue (INITIAL_SCALE_KEY, Default_Initial_Scale);

double
	min_scaling =
	Min_Scale = settings.value (MIN_SCALE_KEY,
		Default_Min_Scale).toDouble (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << MIN_SCALE_KEY << " = " << Min_Scale << endl;
#endif
if (! OK ||
	Min_Scale <= 0)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + MIN_SCALE_KEY + " \""
		+ settings.value (MIN_SCALE_KEY).toString ()
		+ tr ("\" value is invalid - a positive number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Min_Scale)
		+ tr (" is being used."));
	min_scaling = Default_Min_Scale;
	}
else
if (! settings.contains (MIN_SCALE_KEY))
	settings.setValue (MIN_SCALE_KEY, Default_Min_Scale);

double
	max_scaling =
	Max_Scale = settings.value (MAX_SCALE_KEY,
		Default_Max_Scale).toDouble (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << MAX_SCALE_KEY << " = " << Max_Scale << endl;
#endif
if (! OK ||
	Max_Scale <= 0)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + MAX_SCALE_KEY + " \""
		+ settings.value (MAX_SCALE_KEY).toString ()
		+ tr ("\" value is invalid - a positive number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Max_Scale)
		+ tr (" is being used."));
	max_scaling = Default_Max_Scale;
	}
else
if (! settings.contains (MAX_SCALE_KEY))
	settings.setValue (MAX_SCALE_KEY, Default_Max_Scale);

if (max_scaling < min_scaling)
	max_scaling = min_scaling;

double
	minor_increment =
	Scaling_Minor_Increment = settings.value (SCALING_MINOR_INCREMENT_KEY,
		Default_Scaling_Minor_Increment).toDouble (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << SCALING_MINOR_INCREMENT_KEY << " = "
		<< Scaling_Minor_Increment << endl;
#endif
if (! OK ||
	Scaling_Minor_Increment <= 0)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + SCALING_MINOR_INCREMENT_KEY + " \""
		+ settings.value (MAX_SCALE_KEY).toString ()
		+ tr ("\" value is invalid - a positive number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Scaling_Minor_Increment)
		+ tr (" is being used."));
	minor_increment = Default_Scaling_Minor_Increment;
	}
else
if (! settings.contains (SCALING_MINOR_INCREMENT_KEY))
	settings.setValue (SCALING_MINOR_INCREMENT_KEY,
		Default_Scaling_Minor_Increment);

double
	major_increment =
	Scaling_Major_Increment = settings.value (SCALING_MAJOR_INCREMENT_KEY,
		Default_Scaling_Major_Increment).toDouble (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << SCALING_MAJOR_INCREMENT_KEY << " = "
		<< Scaling_Major_Increment << endl;
#endif
if (! OK ||
	Scaling_Major_Increment <= 0)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + SCALING_MAJOR_INCREMENT_KEY + " \""
		+ settings.value (SCALING_MAJOR_INCREMENT_KEY).toString ()
		+ tr ("\" value is invalid - a positive number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Scaling_Major_Increment)
		+ tr (" is being used."));
	major_increment = Default_Scaling_Major_Increment;
	}
else
if (! settings.contains (SCALING_MAJOR_INCREMENT_KEY))
	settings.setValue (SCALING_MAJOR_INCREMENT_KEY,
		Default_Scaling_Major_Increment);

/*------------------------------------------------------------------------------
	Contrast Stretch
*/
double
	percent;
QList<QVariant>
	values;
if (settings.contains (CONTRAST_STRETCH_UPPER_KEY))
	values = settings.value (CONTRAST_STRETCH_UPPER_KEY).toList ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	Default_Contrast_Stretch_Upper[band] =
		round_to (Default_Contrast_Stretch_Upper[band], PERCENT_DECIMAL_PLACES);
	if (band < values.count ())
		{
		percent = values[band].toDouble (&OK);
		if (! OK ||
			percent <   0.0 ||
			percent > 100.0)
			{
			QMessageBox::warning ((isVisible () ? this : NULL), Title,
				tr ("The ") + CONTRAST_STRETCH_UPPER_KEY + '['
				+ QString::number (band) + "] \""
				+ values[band].toString ()
				+ tr ("\" value is invalid - "
					"a number in the range 0.0 - 100.0 is required.\n\n")
				+ tr ("This default value of ")
				+ QString::number (Default_Contrast_Stretch_Upper[band])
				+ tr (" is being used."));
			percent = Default_Contrast_Stretch_Upper[band];
			}
		}
	else
		percent = Default_Contrast_Stretch_Upper[band];
	Contrast_Stretch_Upper[band] = round_to (percent, PERCENT_DECIMAL_PLACES);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << CONTRAST_STRETCH_UPPER_KEY << '[' << band << "] = "
			<< Contrast_Stretch_Upper[band] << endl;
	#endif
	}

if (settings.contains (CONTRAST_STRETCH_LOWER_KEY))
	values = settings.value (CONTRAST_STRETCH_LOWER_KEY).toList ();
else
	values.clear ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	Default_Contrast_Stretch_Lower[band] =
		round_to (Default_Contrast_Stretch_Lower[band], PERCENT_DECIMAL_PLACES);
	if ((Default_Contrast_Stretch_Lower[band]
			+ Default_Contrast_Stretch_Upper[band]) > 100.0)
		 Default_Contrast_Stretch_Lower[band] =
		 	100.0 - Default_Contrast_Stretch_Upper[band];

	if (band < values.count ())
		{
		percent = values[band].toDouble (&OK);
		if (! OK ||
			percent <   0.0 ||
			percent > 100.0)
			{
			QMessageBox::warning ((isVisible () ? this : NULL), Title,
				tr ("The ") + CONTRAST_STRETCH_LOWER_KEY + '['
				+ QString::number (band) + "] \""
				+ values[band].toString ()
				+ tr ("\" value is invalid - "
					"a number in the range 0.0 - 100.0 is required.\n\n")
				+ tr ("This default value of ")
				+ QString::number (Default_Contrast_Stretch_Lower[band])
				+ tr (" is being used."));
			percent = Default_Contrast_Stretch_Lower[band];
			}
		}
	else
		percent = Default_Contrast_Stretch_Lower[band];
	Contrast_Stretch_Lower[band] = round_to (percent, PERCENT_DECIMAL_PLACES);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << CONTRAST_STRETCH_LOWER_KEY << '[' << band << "] = "
			<< Contrast_Stretch_Lower[band] << endl;
	#endif

	if ((Contrast_Stretch_Lower[band] + Contrast_Stretch_Upper[band]) > 100.0)
		 Contrast_Stretch_Lower[band] = 100.0 - Contrast_Stretch_Upper[band];
	}

/*------------------------------------------------------------------------------
	Miscellaneous
*/
#ifdef Q_WS_X11
QColor::setAllowX11ColorNames (true);
#endif
Default_Background_Color_Text = color_text (Default_Background_Color);
Background_Color = Default_Background_Color; // VALGRIND
QString
	color =
	Background_Color_Text = settings.value (BACKGROUND_COLOR_KEY,
		Default_Background_Color_Text).toString ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << BACKGROUND_COLOR_KEY << " = "
		<< Background_Color_Text << endl;
#endif
if (! settings.contains (BACKGROUND_COLOR_KEY))
	settings.setValue (BACKGROUND_COLOR_KEY, Default_Background_Color_Text);

Default_Line_Color_Text = DEFAULT_LINE_COLOR;
Line_Color_Text = settings.value (LINE_COLOR_KEY, Default_Line_Color_Text).toString().toLower();
if(! settings.contains(LINE_COLOR_KEY))
	settings.setValue(LINE_COLOR_KEY, Default_Line_Color_Text);

int
	tile_extent =
	Tile_Size = settings.value (TILE_SIZE_KEY,
		Default_Tile_Size).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << TILE_SIZE_KEY << " = " << Tile_Size << endl;
#endif
if (! OK ||
	Tile_Size < MIN_TILE_SIZE ||
	Tile_Size > MAX_TILE_SIZE)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + TILE_SIZE_KEY + " \""
		+ settings.value (TILE_SIZE_KEY).toString ()
		+ tr ("\" value is invalid;\n")
		+ tr ("A value in the range ")
		+ QString::number (MIN_TILE_SIZE) + " - "
		+ QString::number (MAX_TILE_SIZE)
		+ tr (" is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Tile_Size)
		+ tr (" is being used."));
	tile_extent = Default_Tile_Size;
	}
else
if (! settings.contains (TILE_SIZE_KEY))
	settings.setValue (TILE_SIZE_KEY, Default_Tile_Size);

int
	increment_lines =
	Rendering_Increment_Lines = settings.value (RENDERING_INCREMENT_LINES_KEY,
		Default_Rendering_Increment_Lines).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << RENDERING_INCREMENT_LINES_KEY << " = "
		<< Rendering_Increment_Lines << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + RENDERING_INCREMENT_LINES_KEY + " \""
		+ settings.value (RENDERING_INCREMENT_LINES_KEY).toString ()
		+ tr ("\" value is invalid - a positive number is required.\n\n")
		+ tr ("The default value of ") + ' '
		+ QString::number (Default_Rendering_Increment_Lines)
		+ tr (" is being used."));
	increment_lines = Default_Rendering_Increment_Lines;
	}
else
if (! settings.contains (RENDERING_INCREMENT_LINES_KEY))
	settings.setValue (RENDERING_INCREMENT_LINES_KEY,
		Default_Rendering_Increment_Lines);

/*==============================================================================
	Layout controls
*/
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    layout controls -" << endl;
#endif
QGridLayout
	*grid_layout = new QGridLayout (this);
grid_layout->setHorizontalSpacing (HORIZONTAL_SPACING);
Drawn_Line
	*line;
QLabel
	*label;
int
	reset_button_width,
	row = -1,
	col = 0;

//	Scaling:

//		Heading line.
++row;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, col, 1, -1, Qt::AlignBottom);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": heading line" << endl;
#endif
//		Heading label.
++row;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << " heading line" << endl;
#endif
grid_layout->addWidget (new QLabel (tr ("<b>Scaling</b>")),
	row, col, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Scaling label" << endl;
#endif

//		Initial scaling.
++row;
//			Label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Initial:")),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Initial label" << endl;
#endif
//			Value.
++col;
Initial_Scale_doubleSpinBox = new QDoubleSpinBox;
Initial_Scale_doubleSpinBox->setAlignment (Qt::AlignRight);
Initial_Scale_doubleSpinBox->setAccelerated (true);
Initial_Scale_doubleSpinBox->setDecimals (3);
Initial_Scale_doubleSpinBox->setMinimum (0.0);
Initial_Scale_doubleSpinBox->setSpecialValueText (tr ("auto fit"));
Initial_Scale_doubleSpinBox->setMaximum (MAX_SCALING);
Initial_Scale_doubleSpinBox->setKeyboardTracking (true);
Initial_Scale_doubleSpinBox->setValue (Initial_Scale);
Initial_Scale_doubleSpinBox->setSingleStep (0.01);
Initial_Scale_doubleSpinBox->setToolTip (tr
	("Initial scaling factor on image load; zero for auto fit to viewport"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Initial_Scale_doubleSpinBox);
#endif
connect (Initial_Scale_doubleSpinBox, SIGNAL (valueChanged (double)),
	SLOT (changing ()));
connect (Initial_Scale_doubleSpinBox, SIGNAL (editingFinished ()),
	SLOT (initial_scale_change ()));
grid_layout->addWidget (Initial_Scale_doubleSpinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Initial_Scale_doubleSpinBox" << endl;
#endif
//			Reset button.
++col;
Initial_Scale_Reset_Button = new Icon_Button (*Reset_Button_Icon);
Initial_Scale_Reset_Button->setVisible (false);
Initial_Scale_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Initial_Scale_Reset_Button, SIGNAL (clicked ()),
	SLOT (initial_scale_reset ()));
grid_layout->addWidget (Initial_Scale_Reset_Button,
	row, col, Qt::AlignLeft);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Initial_Scale_Reset_Button" << endl;
#endif
reset_button_width = Initial_Scale_Reset_Button->iconSize ().width ();
grid_layout->setColumnMinimumWidth (col, reset_button_width);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    reset buttons icon size = "
		<< Initial_Scale_Reset_Button->iconSize () << endl;
#endif

//		Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ": Spacing" << endl;
#endif

//		Minimum/Minor scaling.
++row;
//			Range label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Range Mi&nimum:")),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Range Minimum label" << endl;
#endif
//			Range value.
++col;
Min_Scale_doubleSpinBox = new QDoubleSpinBox;
Min_Scale_doubleSpinBox->setAlignment (Qt::AlignRight);
Min_Scale_doubleSpinBox->setAccelerated (true);
Min_Scale_doubleSpinBox->setDecimals (3);
Min_Scale_doubleSpinBox->setMinimum (MIN_SCALING);
Min_Scale_doubleSpinBox->setMaximum (MAX_SCALING);
Min_Scale_doubleSpinBox->setKeyboardTracking (true);
Min_Scale_doubleSpinBox->setValue (Min_Scale);
Min_Scale_doubleSpinBox->setSingleStep (0.01);
Min_Scale_doubleSpinBox->setToolTip (tr ("Minimum image scaling factor"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Min_Scale_doubleSpinBox);
#endif
connect (Min_Scale_doubleSpinBox, SIGNAL (valueChanged (double)),
	SLOT (changing ()));
connect (Min_Scale_doubleSpinBox, SIGNAL (editingFinished ()),
	SLOT (min_scale_change ()));
grid_layout->addWidget (Min_Scale_doubleSpinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Min_Scale_doubleSpinBox" << endl;
#endif
//			Reset button.
++col;
Min_Scale_Reset_Button = new Icon_Button (*Reset_Button_Icon);
Min_Scale_Reset_Button->setVisible (false);
Min_Scale_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Min_Scale_Reset_Button, SIGNAL (clicked ()),
	SLOT (min_scale_reset ()));
grid_layout->addWidget (Min_Scale_Reset_Button,
	row, col, Qt::AlignLeft);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Min_Scale_Reset_Button" << endl;
#endif
grid_layout->setColumnMinimumWidth (col, reset_button_width);

//			Padding.
++col;
grid_layout->setColumnMinimumWidth (col, HORIZONTAL_SPACING);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Padding" << endl;
#endif

//			Increment label.
++col;
grid_layout->addWidget (label = new QLabel (tr ("M&inor Increment:")),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Range Minimum label" << endl;
#endif
//			Increment value.
++col;
Scaling_Minor_Increment_doubleSpinBox = new QDoubleSpinBox;
Scaling_Minor_Increment_doubleSpinBox->setAlignment (Qt::AlignRight);
Scaling_Minor_Increment_doubleSpinBox->setAccelerated (true);
Scaling_Minor_Increment_doubleSpinBox->setDecimals (3);
Scaling_Minor_Increment_doubleSpinBox->setMinimum (MIN_SCALING);
Scaling_Minor_Increment_doubleSpinBox->setMaximum (MAX_SCALING_INCREMENT);
Scaling_Minor_Increment_doubleSpinBox->setValue (Scaling_Minor_Increment);
Scaling_Minor_Increment_doubleSpinBox->setKeyboardTracking (true);
Scaling_Minor_Increment_doubleSpinBox->setSingleStep (0.01);
Scaling_Minor_Increment_doubleSpinBox->setToolTip
	(tr ("Scaling factor small increment"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Scaling_Minor_Increment_doubleSpinBox);
#endif
connect (Scaling_Minor_Increment_doubleSpinBox, SIGNAL (valueChanged (double)),
	SLOT (changing ()));
connect (Scaling_Minor_Increment_doubleSpinBox, SIGNAL (editingFinished ()),
	SLOT (scaling_minor_increment_change ()));
grid_layout->addWidget (Scaling_Minor_Increment_doubleSpinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Scaling_Minor_Increment_doubleSpinBox" << endl;
#endif
//			Reset button.
++col;
Scaling_Minor_Increment_Reset_Button = new Icon_Button (*Reset_Button_Icon);
Scaling_Minor_Increment_Reset_Button->setVisible (false);
Scaling_Minor_Increment_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Scaling_Minor_Increment_Reset_Button, SIGNAL (clicked ()),
	SLOT (scaling_minor_increment_reset ()));
grid_layout->addWidget (Scaling_Minor_Increment_Reset_Button, row, col);
grid_layout->setColumnMinimumWidth (col, reset_button_width);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Scaling_Minor_Increment_Reset_Button" << endl;
#endif

//			Padding.
++col;
grid_layout->setColumnStretch (col, 100);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Padding" << endl;
#endif

//		Maximum/Major
++row;
//			Range label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Range Ma&ximum:")),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Range Maximum label" << endl;
#endif
//			Range value.
++col;
Max_Scale_doubleSpinBox = new QDoubleSpinBox;
Max_Scale_doubleSpinBox->setAlignment (Qt::AlignRight);
Max_Scale_doubleSpinBox->setAccelerated (true);
Max_Scale_doubleSpinBox->setDecimals (3);
Max_Scale_doubleSpinBox->setMinimum (MIN_SCALING);
Max_Scale_doubleSpinBox->setMaximum (MAX_SCALING);
Max_Scale_doubleSpinBox->setValue (Max_Scale);
Max_Scale_doubleSpinBox->setKeyboardTracking (true);
Max_Scale_doubleSpinBox->setToolTip (tr ("Maximum image scaling factor"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Max_Scale_doubleSpinBox);
#endif
connect (Max_Scale_doubleSpinBox, SIGNAL (valueChanged (double)),
	SLOT (changing ()));
connect (Max_Scale_doubleSpinBox, SIGNAL (editingFinished ()),
	SLOT (max_scale_change ()));
grid_layout->addWidget (Max_Scale_doubleSpinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Max_Scale_doubleSpinBox" << endl;
#endif
//			Reset button.
++col;
Max_Scale_Reset_Button = new Icon_Button (*Reset_Button_Icon);
Max_Scale_Reset_Button->setVisible (false);
Max_Scale_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Max_Scale_Reset_Button, SIGNAL (clicked ()),
	SLOT (max_scale_reset ()));
grid_layout->addWidget (Max_Scale_Reset_Button, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Max_Scale_Reset_Button" << endl;
#endif

//			Increment label.
col += 2;
grid_layout->addWidget (label = new QLabel (tr ("M&ajor Increment:")),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Major Increment label" << endl;
#endif
//			Increment value.
++col;
Scaling_Major_Increment_doubleSpinBox = new QDoubleSpinBox;
Scaling_Major_Increment_doubleSpinBox->setAlignment (Qt::AlignRight);
Scaling_Major_Increment_doubleSpinBox->setAccelerated (true);
Scaling_Major_Increment_doubleSpinBox->setDecimals (3);
Scaling_Major_Increment_doubleSpinBox->setMinimum (MIN_SCALING);
Scaling_Major_Increment_doubleSpinBox->setMaximum (MAX_SCALING_INCREMENT);
Scaling_Major_Increment_doubleSpinBox->setValue (Scaling_Major_Increment);
Scaling_Major_Increment_doubleSpinBox->setSingleStep (0.1);
Scaling_Major_Increment_doubleSpinBox->setKeyboardTracking (true);
Scaling_Major_Increment_doubleSpinBox->setToolTip
	(tr ("Scaling factor large increment"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Scaling_Major_Increment_doubleSpinBox);
#endif
connect (Scaling_Major_Increment_doubleSpinBox, SIGNAL (valueChanged (double)),
	SLOT (changing ()));
connect (Scaling_Major_Increment_doubleSpinBox, SIGNAL (editingFinished ()),
	SLOT (scaling_major_increment_change ()));
grid_layout->addWidget (Scaling_Major_Increment_doubleSpinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Scaling_Major_Increment_doubleSpinBox" << endl;
#endif
//			Reset button.
++col;
Scaling_Major_Increment_Reset_Button = new Icon_Button (*Reset_Button_Icon);
Scaling_Major_Increment_Reset_Button->setVisible (false);
Scaling_Major_Increment_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Scaling_Major_Increment_Reset_Button, SIGNAL (clicked ()),
	SLOT (scaling_major_increment_reset ()));
grid_layout->addWidget (Scaling_Major_Increment_Reset_Button, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Scaling_Major_Increment_Reset_Button" << endl;
#endif


//	Contrast Stretch:

//		Heading line.
++row;
col = 0;
line = new Drawn_Line (Heading_Line_Weight);
line->alignment (Qt::AlignBottom);
grid_layout->addWidget (line,
	row, col, 1, -1, Qt::AlignBottom);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": heading line" << endl;
#endif
//		Heading label.
++row;
grid_layout->addWidget (new QLabel (tr ("<b>Default Contrast Stretch</b>")),
	row, col, 1, -1, Qt::AlignLeft | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Default Contrast Stretch label" << endl;
#endif

//		Upper.
++row;
col = -1;
QString
	name;
name = tr ("Upper - ");
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	//		Label.
	++col;
	name += tr (DISPLAY_BAND_NAMES[band]);
	name += tr (" Band:");
	grid_layout->addWidget (label = new QLabel (name, this),
		row, col, Qt::AlignRight | Qt::AlignVCenter);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col << ": " << name << " label" << endl;
	#endif
	//		Value.
	++col;
	Contrast_Stretch_Upper_doubleSpinBox[band] = new QDoubleSpinBox;
	Contrast_Stretch_Upper_doubleSpinBox[band]->setAlignment (Qt::AlignRight);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setAccelerated (true);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setDecimals
		(PERCENT_DECIMAL_PLACES);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setMinimum (0.0);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setMaximum (100.0);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setKeyboardTracking (true);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setValue
		(Contrast_Stretch_Upper[band]);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setSingleStep (0.01);
	Contrast_Stretch_Upper_doubleSpinBox[band]->setSuffix ("%");
	Contrast_Stretch_Upper_doubleSpinBox[band]->setToolTip
		(tr ("Saturation Upper Bound Percent"));
	#ifndef QT_NO_SHORTCUT
	label->setBuddy (Contrast_Stretch_Upper_doubleSpinBox[band]);
	#endif
	connect (Contrast_Stretch_Upper_doubleSpinBox[band],
		SIGNAL (valueChanged (double)),
		SLOT (changing ()));
	connect (Contrast_Stretch_Upper_doubleSpinBox[band],
		SIGNAL (editingFinished ()),
		SLOT (contrast_stretch_change ()));
	grid_layout->addWidget (Contrast_Stretch_Upper_doubleSpinBox[band],
		row, col);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col
		<< ": Contrast_Stretch_Upper_doubleSpinBox[" << band << ']' << endl;
	#endif
	//		Reset button.
	++col;
	Contrast_Stretch_Upper_Reset_Button[band] =
		new Icon_Button (*Reset_Button_Icon);
	Contrast_Stretch_Upper_Reset_Button[band]->setVisible (false);
	Contrast_Stretch_Upper_Reset_Button[band]->setFocusPolicy (Qt::NoFocus);
	connect (Contrast_Stretch_Upper_Reset_Button[band],
		SIGNAL (clicked ()),
		SLOT (contrast_stretch_reset ()));
	grid_layout->addWidget (Contrast_Stretch_Upper_Reset_Button[band],
		row, col, Qt::AlignLeft);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col
		<< ": Contrast_Stretch_Upper_Reset_Button[" << band << ']' << endl;
	#endif
	grid_layout->setColumnMinimumWidth (col, reset_button_width);

	if (band != 2)
		//		Padding.
		++col;

	name.clear ();
	}
//		Lower.
++row;
col = -1;
name = tr ("Lower - ");
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	//		Label.
	++col;
	name += tr (DISPLAY_BAND_NAMES[band]);
	name += tr (" Band:");
	grid_layout->addWidget (label = new QLabel (name, this),
		row, col, Qt::AlignRight | Qt::AlignVCenter);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col << ": " << name << " label" << endl;
	#endif
	//		Value.
	++col;
	Contrast_Stretch_Lower_doubleSpinBox[band] = new QDoubleSpinBox;
	Contrast_Stretch_Lower_doubleSpinBox[band]->setAlignment (Qt::AlignRight);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setAccelerated (true);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setDecimals
		(PERCENT_DECIMAL_PLACES);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setMinimum (0.0);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setMaximum (100.0);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setKeyboardTracking (true);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setValue
		(Contrast_Stretch_Lower[band]);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setSingleStep (0.01);
	Contrast_Stretch_Lower_doubleSpinBox[band]->setSuffix ("%");
	Contrast_Stretch_Lower_doubleSpinBox[band]->setToolTip
		(tr ("Saturation Lower Bound Percent"));
	#ifndef QT_NO_SHORTCUT
	label->setBuddy (Contrast_Stretch_Lower_doubleSpinBox[band]);
	#endif
	connect (Contrast_Stretch_Lower_doubleSpinBox[band],
		SIGNAL (valueChanged (double)),
		SLOT (changing ()));
	connect (Contrast_Stretch_Lower_doubleSpinBox[band],
		SIGNAL (editingFinished ()),
		SLOT (contrast_stretch_change ()));
	grid_layout->addWidget (Contrast_Stretch_Lower_doubleSpinBox[band],
		row, col);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col
		<< ": Contrast_Stretch_Lower_doubleSpinBox[" << band << ']' << endl;
	#endif
	//		Reset button.
	++col;
	Contrast_Stretch_Lower_Reset_Button[band] =
		new Icon_Button (*Reset_Button_Icon);
	Contrast_Stretch_Lower_Reset_Button[band]->setVisible (false);
	Contrast_Stretch_Lower_Reset_Button[band]->setFocusPolicy (Qt::NoFocus);
	connect (Contrast_Stretch_Lower_Reset_Button[band],
		SIGNAL (clicked ()),
		SLOT (contrast_stretch_reset ()));
	grid_layout->addWidget (Contrast_Stretch_Lower_Reset_Button[band],
		row, col, Qt::AlignLeft);
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
	clog << "    " << row << ',' << col
		<< ": Contrast_Stretch_Lower_Reset_Button[" << band << ']' << endl;
	#endif

	if (band != 2)
		//		Padding.
		++col;

	name.clear ();
	}


//	Miscellaneous:

//		Separator line.
++row;
col = 0;
grid_layout->addWidget (new Drawn_Line (Heading_Line_Weight),
	row, col, 1, -1);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": heading line" << endl;
#endif

//		Background Color.
++row;
//			Label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Background &Color:"), this),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Background Color label" << endl;
#endif
//			Value.
++col;
Background_Color_lineEdit = new QLineEdit (color_text (Background_Color), this);
Background_Color_lineEdit->setToolTip
	(tr ("Background color to use in the image display"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Background_Color_lineEdit);
#endif
connect (Background_Color_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (background_color (const QString&)));
connect (Background_Color_lineEdit, SIGNAL (editingFinished ()),
	SLOT (background_color_changed ()));
grid_layout->addWidget (Background_Color_lineEdit,
	row, col, 1, 2);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << "/2: Background_Color_lineEdit" << endl;
#endif
//			Select and reset buttons.
col += 3;	//	includes padding column
//			Select button.
QHBoxLayout
	*horizontal_layout = new QHBoxLayout;
horizontal_layout->setSpacing (HORIZONTAL_SPACING);
Background_Color_Select_Button = new QPushButton (tr ("Select ..."));
connect (Background_Color_Select_Button, SIGNAL (clicked ()),
	SLOT (select_background_color ()));
horizontal_layout->addWidget (Background_Color_Select_Button);
//			Reset button.
Background_Color_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Background_Color_Reset_Button->setVisible (false);
Background_Color_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Background_Color_Reset_Button, SIGNAL (clicked ()),
	SLOT (background_color_reset ()));
horizontal_layout->addWidget (Background_Color_Reset_Button);
//			Padding.
horizontal_layout->addStretch (100);
grid_layout->addLayout (horizontal_layout,
	row, col, 1, -1);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col
		<< "/-1: Background_Color_Select_Button, Background_Color_Reset_Button"
			" horizontal layout" << endl;
#endif

//		Line Color Selection
++row;
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Line &Color:"), this),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
//		Value
++col;
Line_Color_lineEdit = new QLineEdit(Line_Color_Text, this);
Line_Color_lineEdit->setToolTip (tr ("Line color used by the Distance Line tool"));
label->setBuddy(Line_Color_lineEdit);
connect(Line_Color_lineEdit, SIGNAL(textEdited (const QString&)), SLOT (line_color(const QString&)));
connect(Line_Color_lineEdit, SIGNAL(editingFinished ()), SLOT (line_color_changed ()));
grid_layout->addWidget(Line_Color_lineEdit, row, col, 1, 2);
col += 3;

horizontal_layout = new QHBoxLayout;
horizontal_layout->setSpacing (HORIZONTAL_SPACING);
Line_Color_Select_Button = new QPushButton (tr ("Select ..."));
connect(Line_Color_Select_Button, SIGNAL(clicked ()), SLOT(select_line_color()));
horizontal_layout->addWidget(Line_Color_Select_Button);

Line_Color_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Line_Color_Reset_Button->setVisible (false);
Line_Color_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Line_Color_Reset_Button, SIGNAL (clicked()), SLOT(line_color_reset()));
horizontal_layout->addWidget (Line_Color_Reset_Button);
horizontal_layout->addStretch(100);
grid_layout->addLayout (horizontal_layout, row, col, 1, -1);
	

//		Tile Size.
++row;
//			Label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("&Tile Size:"), this),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Tile Size label" << endl;
#endif
//			Value.
++col;
Tile_Size_spinBox = new QSpinBox (this);
Tile_Size_spinBox->setAlignment (Qt::AlignRight);
Tile_Size_spinBox->setAccelerated (true);
Tile_Size_spinBox->setMinimum (MIN_TILE_SIZE);
Tile_Size_spinBox->setMaximum (MAX_TILE_SIZE);
Tile_Size_spinBox->setValue (Tile_Size);
Tile_Size_spinBox->setKeyboardTracking (true);
Tile_Size_spinBox->setToolTip
	(tr ("Maximum size, in pixels, of an image display tile"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Tile_Size_spinBox);
#endif
connect (Tile_Size_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
connect (Tile_Size_spinBox, SIGNAL (editingFinished ()),
	SLOT (tile_size_change ()));
grid_layout->addWidget (Tile_Size_spinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Tile_Size_spinBox" << endl;
#endif
//			Reset button.
++col;
Tile_Size_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Tile_Size_Reset_Button->setVisible (false);
Tile_Size_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Tile_Size_Reset_Button, SIGNAL (clicked ()),
	SLOT (tile_size_reset ()));
grid_layout->addWidget (Tile_Size_Reset_Button,
	row, col, Qt::AlignLeft);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Tile_Size_Reset_Button" << endl;
#endif

//		Rendering Increment Lines.
++row;
//			Label.
col = 0;
grid_layout->addWidget (label = new QLabel (tr ("Incremental &Lines:"), this),
	row, col, Qt::AlignRight | Qt::AlignVCenter);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": Incremental Lines label" << endl;
#endif
//			Value.
++col;
Rendering_Increment_Lines_spinBox = new QSpinBox (this);
Rendering_Increment_Lines_spinBox->setAlignment (Qt::AlignRight);
Rendering_Increment_Lines_spinBox->setAccelerated (true);
Rendering_Increment_Lines_spinBox->setMinimum (0);
Rendering_Increment_Lines_spinBox->setSpecialValueText ("auto");
Rendering_Increment_Lines_spinBox->setMaximum (MAX_TILE_SIZE);
Rendering_Increment_Lines_spinBox->setValue (Rendering_Increment_Lines);
Rendering_Increment_Lines_spinBox->setKeyboardTracking (true);
Rendering_Increment_Lines_spinBox->setToolTip
	(tr ("Suggested number of image lines to render in a display increment"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Rendering_Increment_Lines_spinBox);
#endif
connect (Rendering_Increment_Lines_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
connect (Rendering_Increment_Lines_spinBox, SIGNAL (editingFinished ()),
	SLOT (rendering_increment_lines_change ()));
grid_layout->addWidget (Rendering_Increment_Lines_spinBox, row, col);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col
		<< ": Rendering_Increment_Lines_spinBox" << endl;
#endif
//			Reset button.
++col;
Rendering_Increment_Lines_Reset_Button =
	new Icon_Button (*Reset_Button_Icon, this);
Rendering_Increment_Lines_Reset_Button->setVisible (false);
Rendering_Increment_Lines_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Rendering_Increment_Lines_Reset_Button, SIGNAL (clicked ()),
	SLOT (rendering_increment_lines_reset ()));
grid_layout->addWidget (Rendering_Increment_Lines_Reset_Button,
	row, col, Qt::AlignLeft);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col
		<< ": Rendering_Increment_Lines_Reset_Button" << endl;
#endif

//		Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);
grid_layout->setRowStretch (row, 100);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ": Spacing" << endl;
#endif

//		Defaults/Apply buttons.
++row;
col = 0;
QDialogButtonBox
	*buttons = new QDialogButtonBox (Qt::Horizontal, this);
Defaults_Button = buttons->addButton
	(tr ("Defaults"), QDialogButtonBox::ResetRole);
Defaults_Button->setIcon (*Defaults_Button_Icon);
connect (Defaults_Button, SIGNAL (clicked ()), SLOT (defaults ()));
QAction
	*action = new QAction (tr ("Defaults"), this);
action->setShortcut (tr ("Ctrl+Shift+D"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
addAction (action);
connect (action, SIGNAL (triggered ()),
		 Defaults_Button, SLOT (click ()));

Apply_Button = buttons->addButton
	(tr ("Apply"), QDialogButtonBox::ApplyRole);
if (Apply_Button_Icon)
	Apply_Button->setIcon (*Apply_Button_Icon);
connect (Apply_Button, SIGNAL (clicked ()), SLOT (apply ()));
action = new QAction (tr ("Apply"), this);
action->setShortcut (tr ("Ctrl+Shift+A"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
connect (action, SIGNAL (triggered ()),
		 Apply_Button, SLOT (click ()));
addAction (action);
grid_layout->addWidget (buttons,
	row, col, 1, -1);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "    " << row << ',' << col << ": QDialogButtonBox" << endl;
#endif

//	Initialize the GUI widget values.
initial_scale (initial_scaling);
min_scale (min_scaling);
max_scale (max_scaling);
scaling_minor_increment (minor_increment);
scaling_major_increment (major_increment);
tile_size (tile_extent);
rendering_increment_lines (increment_lines);
Background_Color_Text_Pending = Default_Background_Color_Text;
if (! (OK = background_color_is_valid (color)) &&
	color != Default_Background_Color_Text)
	OK = background_color_is_valid (Default_Background_Color_Text);
if (! OK)
	background_color_is_valid (Default_Background_Color_Text = "transparent");
Background_Color = color_value (Background_Color_Text);

Line_Color_Text_Pending = Default_Line_Color_Text;
if (!(OK = line_color_is_valid (Line_Color_Text)) && Line_Color_Text != Default_Line_Color_Text)
	OK = line_color_is_valid (Default_Line_Color_Text);
if (!OK)
	line_color_is_valid(Default_Line_Color_Text = "transparent");
//	Update settings with any changed values.
apply ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDERING))
clog << "<<< Rendering_Section" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Scaling
*/
void
Rendering_Section::initial_scale
	(
	double	scaling
	)
{

if (Initial_Scale_doubleSpinBox->value () != scaling)
	Initial_Scale_doubleSpinBox->setValue (scaling);

Initial_Scale_Reset_Button->setVisible (scaling != Initial_Scale);
reset_modifier_buttons ();
}


void
Rendering_Section::initial_scale_change ()
{initial_scale (Initial_Scale_doubleSpinBox->value ());}


void
Rendering_Section::initial_scale_reset ()
{initial_scale (Initial_Scale);}


void
Rendering_Section::min_scale
	(
	double	scaling
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::min_scale: " << scaling << endl
	 << "    current value = " << Min_Scale << endl;
#endif
if (scaling < MIN_SCALING ||
	scaling > MAX_SCALING)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The minimum scaling of ")
		+ QString::number (scaling)
		+ tr (" was clipped to the ")
		+ QString::number (MIN_SCALING) + " - "
		+ QString::number (MAX_SCALING)
		+ tr (" limits."));
	scaling = qBound (MIN_SCALING, scaling, MAX_SCALING);
	}

if (Min_Scale_doubleSpinBox->value () != scaling)
	Min_Scale_doubleSpinBox->setValue (scaling);

if (Max_Scale_doubleSpinBox->value () < scaling)
	max_scale (scaling);

Min_Scale_Reset_Button->setVisible (scaling != Min_Scale);

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::min_scale" << endl;
#endif
}


void
Rendering_Section::min_scale_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::min_scale_change" << endl;
#endif
min_scale (Min_Scale_doubleSpinBox->value ());
}


void
Rendering_Section::min_scale_reset ()
{min_scale (Min_Scale);}


void
Rendering_Section::max_scale
	(
	double	scaling
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::max_scale: " << scaling << endl
	 << "    current value = " << Max_Scale << endl;
#endif
if (scaling < MIN_SCALING ||
	scaling > MAX_SCALING)
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The maximum scaling of ")
		+ QString::number (scaling)
		+ tr (" was clipped to the ")
		+ QString::number (MIN_SCALING) + " - "
		+ QString::number (MAX_SCALING)
		+ tr (" limits."));
	scaling = qBound (MIN_SCALING, scaling, MAX_SCALING);
	}

if (Max_Scale_doubleSpinBox->value () != scaling)
	Max_Scale_doubleSpinBox->setValue (scaling);

if (Min_Scale_doubleSpinBox->value () > scaling)
	min_scale (scaling);

Max_Scale_Reset_Button->setVisible (scaling != Max_Scale);

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::max_scale" << endl;
#endif
}


void
Rendering_Section::max_scale_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::max_scale_change" << endl;
#endif
max_scale (Max_Scale_doubleSpinBox->value ());
}


void
Rendering_Section::max_scale_reset ()
{max_scale (Max_Scale);}


void
Rendering_Section::scaling_minor_increment
	(
	double	increment
	)
{
if (increment < MIN_SCALING ||
	increment > MAX_SCALING_INCREMENT)
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The minor scaling increment of ")
		+ QString::number (increment)
		+ tr (" was clipped to the ")
		+ QString::number (MIN_SCALING) + " - "
		+ QString::number (MAX_SCALING_INCREMENT)
		+ tr (" limits."));
	increment = qBound (MIN_SCALING, increment, MAX_SCALING_INCREMENT);
	}

if (Scaling_Minor_Increment_doubleSpinBox->value () != increment)
	Scaling_Minor_Increment_doubleSpinBox->setValue (increment);

Scaling_Minor_Increment_Reset_Button->setVisible
	(increment != Scaling_Minor_Increment);

reset_modifier_buttons ();
}


void
Rendering_Section::scaling_minor_increment_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::scaling_minor_increment_change" << endl;
#endif
scaling_minor_increment (Scaling_Minor_Increment_doubleSpinBox->value ());
}


void
Rendering_Section::scaling_minor_increment_reset ()
{scaling_minor_increment (Scaling_Minor_Increment);}


void
Rendering_Section::scaling_major_increment
	(
	double	increment
	)
{
if (increment < MIN_SCALING ||
	increment > MAX_SCALING_INCREMENT)
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The major scaling increment of ")
		+ QString::number (increment)
		+ tr (" was clipped to the ")
		+ QString::number (MIN_SCALING) + " - "
		+ QString::number (MAX_SCALING_INCREMENT)
		+ tr (" limits."));
	increment = qBound (MIN_SCALING, increment, MAX_SCALING_INCREMENT);
	}

if (Scaling_Major_Increment_doubleSpinBox->value () != increment)
	Scaling_Major_Increment_doubleSpinBox->setValue (increment);

Scaling_Major_Increment_Reset_Button->setVisible
	(increment != Scaling_Major_Increment);

reset_modifier_buttons ();
}


void
Rendering_Section::scaling_major_increment_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::scaling_major_increment_change" << endl;
#endif
scaling_major_increment (Scaling_Major_Increment_doubleSpinBox->value ());
}


void
Rendering_Section::scaling_major_increment_reset ()
{scaling_major_increment (Scaling_Major_Increment);}

/*------------------------------------------------------------------------------
	Contrast Stretch
*/
double
Rendering_Section::contrast_stretch_upper
	(
	int		band
	) const
{
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message << Preferences_Dialog::ID << endl
			<< "Can't get band " << band << " constrast stretch upper value.";
	throw invalid_argument (message.str ());
	}
return Contrast_Stretch_Upper[band];
}


double
Rendering_Section::contrast_stretch_lower
	(
	int		band
	) const
{
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message << Preferences_Dialog::ID << endl
			<< "Can't get band " << band << " constrast stretch lower value.";
	throw invalid_argument (message.str ());
	}
return Contrast_Stretch_Lower[band];
}


void
Rendering_Section::contrast_stretch_upper
	(
	double	percent,
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::contrast_stretch_upper: "
	 	<< percent << " percent, band " << band << endl;
#endif
bool
	enabled = Contrast_Stretch_Upper_doubleSpinBox[band]->blockSignals (true);
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "    set value" << endl;
#endif
Contrast_Stretch_Upper_doubleSpinBox[band]->setValue (percent);
Contrast_Stretch_Upper_doubleSpinBox[band]->blockSignals (enabled);

Contrast_Stretch_Upper_Reset_Button[band]->setVisible
	(percent != Contrast_Stretch_Upper[band]);
reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::contrast_stretch_upper" << endl;
#endif
}


void
Rendering_Section::contrast_stretch_lower
	(
	double	percent,
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::contrast_stretch_lower: "
	 	<< percent << " percent, band " << band << endl;
#endif
bool
	enabled = Contrast_Stretch_Lower_doubleSpinBox[band]->blockSignals (true);
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "    set value" << endl;
#endif
Contrast_Stretch_Lower_doubleSpinBox[band]->setValue (percent);
Contrast_Stretch_Lower_doubleSpinBox[band]->blockSignals (enabled);

Contrast_Stretch_Lower_Reset_Button[band]->setVisible
	(percent != Contrast_Stretch_Lower[band]);
reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::contrast_stretch_lower" << endl;
#endif
}


void
Rendering_Section::contrast_stretch_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::contrast_stretch_change" << endl;
#endif
QObject
	*source = sender ();
int
	band;
if (source)
	{
	band = 3;
	while (band--)
		{
		if (source == Contrast_Stretch_Upper_doubleSpinBox[band])
			{
			#if ((DEBUG_SECTION) & DEBUG_RENDERING)
			clog << "    source is Contrast_Stretch_Upper_doubleSpinBox["
					<< band << ']' << endl;
			#endif
			if ((Contrast_Stretch_Upper_doubleSpinBox[band]->value () +
				 Contrast_Stretch_Lower_doubleSpinBox[band]->value ()) > 100.0)
				contrast_stretch_lower (100.0
					- Contrast_Stretch_Upper_doubleSpinBox[band]->value (),
					band);
			source = NULL;
			break;
			}
		}
	}
if (source)
	{
	band = 3;
	while (band--)
		{
		if (source == Contrast_Stretch_Lower_doubleSpinBox[band])
			{
			#if ((DEBUG_SECTION) & DEBUG_RENDERING)
			clog << "    source is Contrast_Stretch_Lower_doubleSpinBox["
					<< band << ']' << endl;
			#endif
			if ((Contrast_Stretch_Lower_doubleSpinBox[band]->value () +
				 Contrast_Stretch_Upper_doubleSpinBox[band]->value ()) > 100.0)
				contrast_stretch_upper (100.0
					- Contrast_Stretch_Lower_doubleSpinBox[band]->value (),
					band);
			break;
			}
		}
	}
reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::contrast_stretch_change" << endl;
#endif
}


void
Rendering_Section::contrast_stretch_reset ()
{
QObject
	*source = sender ();
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (! source ||
		source == Contrast_Stretch_Upper_Reset_Button[band])
		{
		contrast_stretch_upper (Contrast_Stretch_Upper[band], band);
		if (! source)
			break;
		}
	if (! source ||
		source == Contrast_Stretch_Lower_Reset_Button[band])
		{
		contrast_stretch_lower (Contrast_Stretch_Lower[band], band);
		if (! source)
			break;
		}
	}
}

/*------------------------------------------------------------------------------
	Miscellaneous
*/
void
Rendering_Section::tile_size
	(
	int		size
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::tile_size: " << size << endl;
#endif
if (size < MIN_TILE_SIZE ||
	size > MAX_TILE_SIZE)
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The tile size of ")
		+ QString::number (size)
		+ tr (" was clipped to the ")
		+ QString::number (MIN_TILE_SIZE) + " - "
		+ QString::number (MAX_TILE_SIZE)
		+ tr (" limits."));
	size = qBound (MIN_TILE_SIZE, size, MAX_TILE_SIZE);
	}

if (Tile_Size_spinBox->value () != size)
	Tile_Size_spinBox->setValue (size);

Tile_Size_Reset_Button->setVisible (size != Tile_Size);

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::tile_size" << endl;
#endif
}


void
Rendering_Section::tile_size_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::tile_size_change" << endl;
#endif
tile_size (Tile_Size_spinBox->value ());
}


void
Rendering_Section::tile_size_reset ()
{tile_size (Tile_Size);}


void
Rendering_Section::rendering_increment_lines
	(
	int		lines
	)
{
if (lines < 0 ||
	lines > MAX_TILE_SIZE)
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		tr ("The number of rendring increment lines of ")
		+ QString::number (lines)
		+ tr (" was clipped to the 0 - ")
		+ QString::number (MAX_TILE_SIZE)
		+ tr (" limits."));
	lines = qBound (0, lines, MAX_TILE_SIZE);
	}

if (Rendering_Increment_Lines_spinBox->value () != lines)
	Rendering_Increment_Lines_spinBox->setValue (lines);

Rendering_Increment_Lines_Reset_Button->setVisible
	(lines != Rendering_Increment_Lines);

reset_modifier_buttons ();
}


void
Rendering_Section::rendering_increment_lines_change ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::rendering_increment_lines_change" << endl;
#endif
rendering_increment_lines (Rendering_Increment_Lines_spinBox->value ());
}


void
Rendering_Section::rendering_increment_lines_reset ()
{rendering_increment_lines (Rendering_Increment_Lines);}


void
Rendering_Section::background_color
	(
	const QString&	text
	)
{
if (Background_Color_lineEdit->text () != text)
	Background_Color_lineEdit->setText (text);
Background_Color_Reset_Button->setVisible (text != Background_Color_Text);
reset_modifier_buttons ();
}

void Rendering_Section::line_color (const QString& text) {
	if(Line_Color_lineEdit->text () != text)
		Line_Color_lineEdit->setText(text);
	Line_Color_Reset_Button->setVisible (text != Line_Color_Text);
	reset_modifier_buttons ();
}


namespace
{
void
no_warning_messages (QtMsgType, const QMessageLogContext &, const QString &) {}
}


bool
Rendering_Section::background_color_is_valid
	(
	const QString&	color_spec
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::background_color_is_valid: "
		<< color_spec << endl;
#endif
bool
	accepted = true;
QString
	text (color_spec);
if (text.toLower () == "none")
	text = "transparent";

//	Prevent gratuitous warning message from QColor about invalid color name.
qInstallMessageHandler (no_warning_messages);
QColor
	color (text);
qInstallMessageHandler (0);

if (color.isValid ())
	//	Use the hex triplet color name.
	Background_Color_Text_Pending = color_text (color.rgba ());
else
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		QString ("\"") + Background_Color_lineEdit->text ()
		+ tr ("\" is an invalid background color.\n\n")
		+ tr ("The color must be an RGB hex value triplet "
		      "beginning with the '#' character, "
		      "or a recognized color name such as \"black\"."));
	accepted = false;
	//	Restore the pending color.
	}
background_color (Background_Color_Text_Pending);

#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::background_color_is_valid: "
		<< accepted << endl;
#endif
return accepted;
}

bool Rendering_Section::line_color_is_valid (const QString& color_spec) {
	bool accepted = true;
	QString text (color_spec);
	//don't want a transparent line
	if (text.toLower () == "transparent")
		text = "";

	//	Prevent gratuitous warning message from QColor about invalid color name.
	qInstallMessageHandler (no_warning_messages);
	QColor color (text);
	qInstallMessageHandler (0);

	if (color.isValid ())
		//	Use the hex triplet color name.
		Line_Color_Text_Pending = color_text (color.rgba ());
	else
	{
		QMessageBox::information ((isVisible () ? this : NULL), Title,
			QString ("\"") + Line_Color_lineEdit->text ()
			+ tr ("\" is an invalid line color.\n\n")
			+ tr ("The color must be an RGB hex value triplet "
		      "beginning with the '#' character, "
		      "or a recognized color name such as \"black\"."));
		accepted = false;
		//	Restore the pending color.
	}
	line_color (Line_Color_Text_Pending);

	return accepted;
}


void
Rendering_Section::background_color_changed ()
{
//	Avoid redundant editingFinished signals.
static bool
	OK = true;
if (OK &&
	Background_Color_lineEdit->text () != Background_Color_Text_Pending &&
	! Defaults_Button->hasFocus ())
	{
	#if ((DEBUG_SECTION) & DEBUG_RENDERING)
	clog << ">-< Rendering_Section::background_color_changed" << endl;
	#endif
	OK = false;
	background_color_is_valid (Background_Color_lineEdit->text ());
	OK = true;
	}
}

void Rendering_Section::line_color_changed () {
	static bool OK = true;
	if(OK && Line_Color_lineEdit->text() != Line_Color_Text_Pending && !Defaults_Button->hasFocus()) {
		OK = false;
		line_color_is_valid (Line_Color_lineEdit->text());
		OK = true;
	}
}


void
Rendering_Section::select_background_color ()
{
QColor
	color (QColorDialog::getColor
		(color_value (Background_Color_lineEdit->text ()),
		(isVisible () ? this : NULL),
		Title + " - " + tr ("Background Color")));
if (color.isValid ())
	background_color_is_valid (color_text (color.rgb ()));
}

void Rendering_Section::select_line_color () {
	QColor color (QColorDialog::getColor (color_value (Line_Color_lineEdit->text ()),
		(isVisible () ? this : NULL), Title + " - " + tr ("Line Color")));
if (color.isValid ())
	line_color_is_valid (color_text (color.rgb()));
}


void
Rendering_Section::background_color_reset ()
{background_color_is_valid (Background_Color_Text);}

void Rendering_Section::line_color_reset () {
	line_color_is_valid (Line_Color_Text);
}

/*------------------------------------------------------------------------------
	Utilities
*/
void
Rendering_Section::reset ()
{
initial_scale_reset ();
min_scale_reset ();
max_scale_reset ();
scaling_minor_increment_reset ();
scaling_major_increment_reset ();

contrast_stretch_reset ();

tile_size_reset ();
rendering_increment_lines_reset ();
background_color_reset ();
}


void
Rendering_Section::defaults ()
{
initial_scale (Default_Initial_Scale);
min_scale (Default_Min_Scale);
max_scale (Default_Max_Scale);
scaling_minor_increment (Default_Scaling_Minor_Increment);
scaling_major_increment (Default_Scaling_Major_Increment);

for (int
		band = 0;
		band < 3;
	  ++band)
	{
	contrast_stretch_upper (Default_Contrast_Stretch_Upper[band], band);
	contrast_stretch_lower (Default_Contrast_Stretch_Lower[band], band);
	}

tile_size (Default_Tile_Size);
rendering_increment_lines (Default_Rendering_Increment_Lines);
background_color (Default_Background_Color_Text);
line_color(Default_Line_Color_Text);
}


void
Rendering_Section::reset_modifier_buttons ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::reset_modifier_buttons" << endl;
#endif
reset_defaults_button ();
Apply_Button->setEnabled (has_changed ());
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::reset_modifier_buttons" << endl;
#endif
}


void
Rendering_Section::reset_defaults_button ()
{
bool
	enabled = false;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (Contrast_Stretch_Upper_doubleSpinBox[band]->value ()
		!= Default_Contrast_Stretch_Upper[band] ||
		Contrast_Stretch_Lower_doubleSpinBox[band]->value ()
		!= Default_Contrast_Stretch_Lower[band])
		{
		enabled = true;
		break;
		}
	}

Defaults_Button->setEnabled
	(
	enabled ||
	Initial_Scale_doubleSpinBox->value ()
		!= Default_Initial_Scale				||
	Min_Scale_doubleSpinBox->value ()
		!= Default_Min_Scale					||
	Max_Scale_doubleSpinBox->value ()
		!= Default_Max_Scale					||
	Scaling_Minor_Increment_doubleSpinBox->value ()
		!= Default_Scaling_Minor_Increment		||
	Scaling_Major_Increment_doubleSpinBox->value ()
		!= Default_Scaling_Major_Increment		||
	Rendering_Increment_Lines_spinBox->value ()
		!= Default_Rendering_Increment_Lines	||
	Tile_Size_spinBox->value ()
		!= Default_Tile_Size					||
	Background_Color_lineEdit->text ()
		!= Default_Background_Color_Text		||
	Line_Color_lineEdit->text()
		!= Default_Line_Color_Text
	);
}


void
Rendering_Section::changing ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::changing" << endl;
#endif
QObject
	*source = sender ();
bool
	change,
	changed = false;

Initial_Scale_Reset_Button->setVisible (change =
	(Initial_Scale_doubleSpinBox->value ()
		!= Initial_Scale));
changed |= change;
Min_Scale_Reset_Button->setVisible (change =
	(Min_Scale_doubleSpinBox->value ()
		!= Min_Scale));
changed |= change;
Max_Scale_Reset_Button->setVisible (change =
	(Max_Scale_doubleSpinBox->value ()
		!= Max_Scale));
changed |= change;
Scaling_Minor_Increment_Reset_Button->setVisible (change =
	(Scaling_Minor_Increment_doubleSpinBox->value ()
		!= Scaling_Minor_Increment));
changed |= change;
Scaling_Major_Increment_Reset_Button->setVisible (change =
	(Scaling_Major_Increment_doubleSpinBox->value ()
		!= Scaling_Major_Increment));
changed |= change;

for (int
		band = 0;
		band < 3;
	  ++band)
	{
	#if ((DEBUG_SECTION) & DEBUG_RENDERING)
	if (source == Contrast_Stretch_Upper_doubleSpinBox[band])
		clog << "    source is Contrast_Stretch_Upper_doubleSpinBox[" << band
				<< ']' << endl;
	#endif
	Contrast_Stretch_Upper_Reset_Button[band]->setVisible (change =
		(Contrast_Stretch_Upper_doubleSpinBox[band]->value ()
			!= Contrast_Stretch_Upper[band]));
	changed |= change;
	if (source == Contrast_Stretch_Upper_doubleSpinBox[band] &&
		(Contrast_Stretch_Upper_doubleSpinBox[band]->value () +
		 Contrast_Stretch_Lower_doubleSpinBox[band]->value ()) > 100.0)
		contrast_stretch_lower
			(100.0 - Contrast_Stretch_Upper_doubleSpinBox[band]->value (),
				band);

	#if ((DEBUG_SECTION) & DEBUG_RENDERING)
	if (source == Contrast_Stretch_Lower_doubleSpinBox[band])
		clog << "    source is Contrast_Stretch_Lower_doubleSpinBox[" << band
				<< ']' << endl;
	#endif
	Contrast_Stretch_Lower_Reset_Button[band]->setVisible (change =
		(Contrast_Stretch_Lower_doubleSpinBox[band]->value ()
			!= Contrast_Stretch_Lower[band]));
	changed |= change;
	if (source == Contrast_Stretch_Lower_doubleSpinBox[band] &&
		(Contrast_Stretch_Lower_doubleSpinBox[band]->value () +
		 Contrast_Stretch_Upper_doubleSpinBox[band]->value ()) > 100.0)
		contrast_stretch_upper
			(100.0 - Contrast_Stretch_Lower_doubleSpinBox[band]->value (),
				band);
	}

Tile_Size_Reset_Button->setVisible (change =
	(Tile_Size_spinBox->value ()
		!= Tile_Size));
changed |= change;
Rendering_Increment_Lines_Reset_Button->setVisible (change =
	(Rendering_Increment_Lines_spinBox->value ()
		!= Rendering_Increment_Lines));
changed |= change;
//	Text fields manage their Reset_Buttons.
changed |= (Background_Color_lineEdit->text ()
		!= Background_Color_Text);

reset_defaults_button ();
Apply_Button->setEnabled (changed);
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::changing" << endl;
#endif
}


bool
Rendering_Section::has_changed () const
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">-< Rendering_Section::has_changed" << endl
	 << "    Background_Color_lineEdit->text = "
	 	<< Background_Color_lineEdit->text () << endl
	 << "              Background_Color_Text = "
	 	<< Background_Color_Text << endl;
#endif
bool
	changed = false;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (Contrast_Stretch_Upper_doubleSpinBox[band]->value ()
		!= Contrast_Stretch_Upper[band] ||
		Contrast_Stretch_Lower_doubleSpinBox[band]->value ()
		!= Contrast_Stretch_Lower[band])
		{
		changed = true;
		break;
		}
	}

return
	changed ||
	Initial_Scale_doubleSpinBox->value ()
		!= Initial_Scale				||
	Min_Scale_doubleSpinBox->value ()
		!= Min_Scale					||
	Max_Scale_doubleSpinBox->value ()
		!= Max_Scale					||
	Scaling_Minor_Increment_doubleSpinBox->value ()
		!= Scaling_Minor_Increment		||
	Scaling_Major_Increment_doubleSpinBox->value ()
		!= Scaling_Major_Increment		||
	Rendering_Increment_Lines_spinBox->value ()
		!= Rendering_Increment_Lines	||
	Tile_Size_spinBox->value ()
		!= Tile_Size					||
	Background_Color_lineEdit->text ()
		!= Background_Color_Text		||
	Line_Color_lineEdit->text()
		!= Line_Color_Text;
}


void
Rendering_Section::apply ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << ">>> Rendering_Section::apply" << endl;
#endif
QSettings
	settings;

if (Initial_Scale != Initial_Scale_doubleSpinBox->value ())
	{
	Initial_Scale = Initial_Scale_doubleSpinBox->value ();
	Initial_Scale_Reset_Button->setVisible (false);
	settings.setValue (INITIAL_SCALE_KEY, Initial_Scale);
	}
if (Min_Scale != Min_Scale_doubleSpinBox->value ())
	{
	Min_Scale = Min_Scale_doubleSpinBox->value ();
	Min_Scale_Reset_Button->setVisible (false);
	settings.setValue (MIN_SCALE_KEY, Min_Scale);
	//	>>> SIGNAL <<<
	emit min_scale_changed (Min_Scale);
	}
if (Max_Scale != Max_Scale_doubleSpinBox->value ())
	{
	Max_Scale = Max_Scale_doubleSpinBox->value ();
	Max_Scale_Reset_Button->setVisible (false);
	settings.setValue (MAX_SCALE_KEY, Max_Scale);
	//	>>> SIGNAL <<<
	emit max_scale_changed (Max_Scale);
	}
if (Scaling_Minor_Increment != Scaling_Minor_Increment_doubleSpinBox->value ())
	{
	Scaling_Minor_Increment = Scaling_Minor_Increment_doubleSpinBox->value ();
	Scaling_Minor_Increment_Reset_Button->setVisible (false);
	settings.setValue
		(SCALING_MINOR_INCREMENT_KEY, Scaling_Minor_Increment);
	//	>>> SIGNAL <<<
	emit scaling_minor_increment_changed (Scaling_Minor_Increment);
	}
if (Scaling_Major_Increment != Scaling_Major_Increment_doubleSpinBox->value ())
	{
	Scaling_Major_Increment = Scaling_Major_Increment_doubleSpinBox->value ();
	Scaling_Major_Increment_Reset_Button->setVisible (false);
	settings.setValue
		(SCALING_MAJOR_INCREMENT_KEY, Scaling_Major_Increment);
	//	>>> SIGNAL <<<
	emit scaling_major_increment_changed (Scaling_Major_Increment);
	}

QList<QVariant>
	values;
int
	band;
band = 3;
while (band--)
	{
	if (Contrast_Stretch_Upper[band] !=
		Contrast_Stretch_Upper_doubleSpinBox[band]->value ())
		{
		Contrast_Stretch_Upper[band] =
			Contrast_Stretch_Upper_doubleSpinBox[band]->value ();
		Contrast_Stretch_Upper_Reset_Button[band]->setVisible (false);
		//	>>> SIGNAL <<<
		emit contrast_stretch_upper_changed
			(Contrast_Stretch_Upper[band], band);
		}
	values.append (QVariant (Contrast_Stretch_Upper[band]));
	}
settings.setValue (CONTRAST_STRETCH_UPPER_KEY, values);
values.clear ();
band = 3;
while (band--)
	{
	if (Contrast_Stretch_Lower[band] !=
		Contrast_Stretch_Lower_doubleSpinBox[band]->value ())
		{
		Contrast_Stretch_Lower[band] =
			Contrast_Stretch_Lower_doubleSpinBox[band]->value ();
		Contrast_Stretch_Lower_Reset_Button[band]->setVisible (false);
		//	>>> SIGNAL <<<
		emit contrast_stretch_lower_changed
			(Contrast_Stretch_Lower[band], band);
		}
	values.append (QVariant (Contrast_Stretch_Lower[band]));
	}
settings.setValue (CONTRAST_STRETCH_LOWER_KEY, values);

QString
	text = Background_Color_lineEdit->text ();
if (Background_Color_Text != text &&
	background_color_is_valid (text))
	{
	Background_Color_Text = text;
	Background_Color = color_value (text);
	Background_Color_Reset_Button->setVisible (false);
	#if ((DEBUG_SECTION) & DEBUG_RENDERING)
	clog << "    Background_Color_Text_Pending = "
		 	<< Background_Color_Text_Pending << endl
		 << "            Background_Color_Text = "
			<< Background_Color_Text << endl
		 << "                 Background_Color = "
		 	<< color_text (Background_Color) << endl;
	#endif
	settings.setValue (BACKGROUND_COLOR_KEY, Background_Color_Text);
	//	>>> SIGNAL <<<
	emit background_color_changed (Background_Color);
	}
text = Line_Color_lineEdit->text();
if(Line_Color_Text != text && line_color_is_valid(text)) {
	Line_Color_Text = text;
	Line_Color_Reset_Button->setVisible(false);
	settings.setValue(LINE_COLOR_KEY, Line_Color_Text);
	emit line_color_changed (QColor(text));
}
if (Tile_Size != Tile_Size_spinBox->value ())
	{
	Tile_Size = Tile_Size_spinBox->value ();
	Tile_Size_Reset_Button->setVisible (false);
	settings.setValue (TILE_SIZE_KEY, Tile_Size);
	//	>>> SIGNAL <<<
	emit tile_size_changed (Tile_Size);
	}
if (Rendering_Increment_Lines != Rendering_Increment_Lines_spinBox->value ())
	{
	Rendering_Increment_Lines = Rendering_Increment_Lines_spinBox->value ();
	Rendering_Increment_Lines_Reset_Button->setVisible (false);
	settings.setValue
		(RENDERING_INCREMENT_LINES_KEY, Rendering_Increment_Lines);
	//	>>> SIGNAL <<<
	emit rendering_increment_lines_changed (Rendering_Increment_Lines);
	}

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_RENDERING)
clog << "<<< Rendering_Section::apply" << endl;
#endif
}


QString
Rendering_Section::color_text
	(
	QRgb	color
	)
{return Preferences_Dialog::color_text (color);}


QRgb
Rendering_Section::color_value
	(
	const QString&	text
	)
{return Preferences_Dialog::color_value (text);}

/*=*****************************************************************************
	JPIP_Section

const char
	*JPIP_Section::HTTP_TO_JPIP_HOSTNAME_KEY	= "HTTP_to_JPIP_Hostname";

#ifndef DEFAULT_HTTP_TO_JPIP_HOSTNAME
#define DEFAULT_HTTP_TO_JPIP_HOSTNAME		hirise-jpip.lpl.arizona.edu
#endif
#define _DEFAULT_HTTP_TO_JPIP_HOSTNAME_	AS_STRING(DEFAULT_HTTP_TO_JPIP_HOSTNAME)
QString
	JPIP_Section::Default_HTTP_to_JPIP_Hostname
		= _DEFAULT_HTTP_TO_JPIP_HOSTNAME_;
*/
const char
	*JPIP_Section::JPIP_SERVER_PORT_KEY			= "JPIP_Server_Port";

#ifndef DEFAULT_JPIP_SERVER_PORT
#define DEFAULT_JPIP_SERVER_PORT			8064
#endif
int
	JPIP_Section::Default_JPIP_Server_Port
		= DEFAULT_JPIP_SERVER_PORT;
#ifndef DEFAULT_JPIP_SERVER_PORT_MIN
#define DEFAULT_JPIP_SERVER_PORT_MIN		0
#endif
#ifndef DEFAULT_JPIP_SERVER_PORT_MAX
#define DEFAULT_JPIP_SERVER_PORT_MAX		65535
#endif

const char
	*JPIP_Section::JPIP_PROXY_KEY				= "JPIP_Proxy";

#ifndef DEFAULT_JPIP_PROXY
#define _DEFAULT_JPIP_PROXY_			""
#else
#define _DEFAULT_JPIP_PROXY_			AS_STRING(DEFAULT_JPIP_PROXY)
#endif
QString
	JPIP_Section::Default_JPIP_Proxy
		= _DEFAULT_JPIP_PROXY_;

const char
	*JPIP_Section::JPIP_CACHE_DIRECTORY_KEY		= "JPIP_Cache_Directory";

#ifndef DEFAULT_JPIP_CACHE_DIRECTORY
#define _DEFAULT_JPIP_CACHE_DIRECTORY_	""
#else
#define _DEFAULT_JPIP_CACHE_DIRECTORY_	AS_STRING(DEFAULT_JPIP_CACHE_DIRECTORY)
#endif
QString
	JPIP_Section::Default_JPIP_Cache_Directory
		= _DEFAULT_JPIP_CACHE_DIRECTORY_;

const char
	*JPIP_Section::JPIP_REQUEST_TIMEOUT_KEY		= "JPIP_Request_Timeout";

#ifndef JPIP_REQUEST_TIMEOUT_MIN
#define JPIP_REQUEST_TIMEOUT_MIN			2
#endif
#ifndef DEFAULT_JPIP_REQUEST_TIMEOUT
#define DEFAULT_JPIP_REQUEST_TIMEOUT		30
#endif
int
	JPIP_Section::Default_JPIP_Request_Timeout
		= DEFAULT_JPIP_REQUEST_TIMEOUT;
#ifndef DEFAULT_JPIP_REQUEST_TIMEOUT_MAX
#define DEFAULT_JPIP_REQUEST_TIMEOUT_MAX	3600
#endif

const char
	*JPIP_Section::MAX_SOURCE_IMAGE_AREA_MB_KEY	= "Max_Source_Image_Area_MB";

#ifndef DEFAULT_MAX_SOURCE_IMAGE_AREA_MB
#define DEFAULT_MAX_SOURCE_IMAGE_AREA_MB	2
#endif
int
	JPIP_Section::Default_Max_Source_Image_Area_MB
		= DEFAULT_MAX_SOURCE_IMAGE_AREA_MB;
#define MAX_IMAGE_AREA_MB					64


#ifndef DOXYGEN_PROCESSING
namespace
{
bool
possible_abbreviated_home_path
	(
	QString&	name
	)
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> possible_abbreviated_home_path: " << name << endl;
#endif
bool
	changed = false;
if (name[0] == '~' &&
	name[1] == QDir::separator ())
	{
	QString
		home_path = QDir::homePath ();
	if (home_path != QDir::rootPath ())
		{
		name.replace (0, 1, home_path);
		changed = true;
		}
	}
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "<<< possible_abbreviated_home_path: "
		<< changed << " - " << name << endl;
#endif
return changed;
}
}	//	local namespace.
#endif


JPIP_Section::JPIP_Section
	(
	QWidget*	parent
	)
	:	QWidget (parent),
		File_Selection_Dialog (NULL)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << ">>> JPIP_Section" << endl;
#endif
if (parent)
	Title = parent->windowTitle ();
else
	Title = tr ("Preferences");
Title += tr (": JPIP");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    Title = \"" << Title << '"' << endl;
#endif

//	Initialize values.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    settings -" << endl;
#endif
QSettings
	settings;
bool
	OK;
/*
QString
	hostname =
	HTTP_to_JPIP_Hostname = settings.value (HTTP_TO_JPIP_HOSTNAME_KEY,
		Default_HTTP_to_JPIP_Hostname).toString ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << HTTP_TO_JPIP_HOSTNAME_KEY << " = \""
		<< HTTP_to_JPIP_Hostname << '"' << endl;
#endif
if (! settings.contains (HTTP_TO_JPIP_HOSTNAME_KEY))
	settings.setValue (HTTP_TO_JPIP_HOSTNAME_KEY,
		Default_HTTP_to_JPIP_Hostname);
*/
int
	port =
	Port = settings.value (JPIP_SERVER_PORT_KEY,
		Default_JPIP_Server_Port).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << JPIP_SERVER_PORT_KEY << " = " << Port << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + JPIP_SERVER_PORT_KEY + " \""
		+ settings.value (JPIP_SERVER_PORT_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_JPIP_Server_Port)
		+ tr (" is being used."));
	port = Default_JPIP_Server_Port;
	}
else
if (! settings.contains (JPIP_SERVER_PORT_KEY))
	settings.setValue (JPIP_SERVER_PORT_KEY, Default_JPIP_Server_Port);

QString
	proxy =
	Proxy = settings.value (JPIP_PROXY_KEY,
		Default_JPIP_Proxy).toString ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << JPIP_PROXY_KEY << " = \"" << Proxy << '"' << endl;
#endif
if (! settings.contains (JPIP_PROXY_KEY))
	settings.setValue (JPIP_PROXY_KEY, Default_JPIP_Proxy);

possible_abbreviated_home_path (Default_JPIP_Cache_Directory);
QString
	cache_directory =
	Cache_Directory = settings.value (JPIP_CACHE_DIRECTORY_KEY,
		Default_JPIP_Cache_Directory).toString ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << JPIP_CACHE_DIRECTORY_KEY << " = \""
		<< Cache_Directory << '"' << endl;
#endif
if (! settings.contains (JPIP_CACHE_DIRECTORY_KEY))
	settings.setValue (JPIP_CACHE_DIRECTORY_KEY, Default_JPIP_Cache_Directory);

int
	seconds =
	Request_Timeout = settings.value (JPIP_REQUEST_TIMEOUT_KEY,
		Default_JPIP_Request_Timeout).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << JPIP_REQUEST_TIMEOUT_KEY << " = " << Request_Timeout << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + JPIP_REQUEST_TIMEOUT_KEY + " \""
		+ settings.value (JPIP_REQUEST_TIMEOUT_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_JPIP_Request_Timeout)
		+ tr (" is being used."));
	seconds = Default_JPIP_Request_Timeout;
	}
else
if (! settings.contains (JPIP_REQUEST_TIMEOUT_KEY))
	settings.setValue (JPIP_REQUEST_TIMEOUT_KEY, Default_JPIP_Request_Timeout);

int
	image_area =
	Max_Source_Image_Area_MB = settings.value (MAX_SOURCE_IMAGE_AREA_MB_KEY,
		Default_Max_Source_Image_Area_MB).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    " << MAX_SOURCE_IMAGE_AREA_MB_KEY << " = "
		<< Max_Source_Image_Area_MB << endl;
#endif
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + MAX_SOURCE_IMAGE_AREA_MB_KEY + " \""
		+ settings.value (MAX_SOURCE_IMAGE_AREA_MB_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Max_Source_Image_Area_MB)
		+ tr (" is being used."));
	image_area = Default_Max_Source_Image_Area_MB;
	}
else
if (! settings.contains (MAX_SOURCE_IMAGE_AREA_MB_KEY))
	settings.setValue (MAX_SOURCE_IMAGE_AREA_MB_KEY,
		Default_Max_Source_Image_Area_MB);

//	Layout controls.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "    layout controls -" << endl;
#endif
QGridLayout
	*grid_layout = new QGridLayout (this);
grid_layout->setHorizontalSpacing (HORIZONTAL_SPACING);
QLabel
	*label;
int
	row,
	col;

//	HTTP-to-JPIP URL Hostname.
row = 0;
//		Label.
col = 0;
/*
label = new QLabel (tr ("&HTTP-to-JPIP URL Hostname:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
HTTP_to_JPIP_Hostname_lineEdit = new QLineEdit (HTTP_to_JPIP_Hostname, this);
HTTP_to_JPIP_Hostname_lineEdit->setToolTip
	(tr ("Hostname to convert an HTTP URL for a JP2 file to the JPIP protocol"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (HTTP_to_JPIP_Hostname_lineEdit);
#endif
connect (HTTP_to_JPIP_Hostname_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (HTTP_to_JPIP_hostname (const QString&)));
connect (HTTP_to_JPIP_Hostname_lineEdit, SIGNAL (editingFinished ()),
	SLOT (HTTP_to_JPIP_hostname_changed ()));
grid_layout->addWidget (HTTP_to_JPIP_Hostname_lineEdit, row, col, 1, 5);
//		Reset button.
col += 5;
HTTP_to_JPIP_Hostname_Reset_Button =
	new Icon_Button (*Reset_Button_Icon, this);
HTTP_to_JPIP_Hostname_Reset_Button->setVisible (false);
HTTP_to_JPIP_Hostname_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (HTTP_to_JPIP_Hostname_Reset_Button, SIGNAL (clicked ()),
	SLOT (HTTP_to_JPIP_hostname_reset ()));
grid_layout->addWidget (HTTP_to_JPIP_Hostname_Reset_Button,
	row, col, Qt::AlignLeft);
grid_layout->setColumnMinimumWidth
	(col, HTTP_to_JPIP_Hostname_Reset_Button->iconSize ().width ());
*/
//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Server Port.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Default Server &Port:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Port_spinBox = new QSpinBox (this);
Port_spinBox->setAlignment (Qt::AlignRight);
Port_spinBox->setAccelerated (true);
Port_spinBox->setMinimum (DEFAULT_JPIP_SERVER_PORT_MIN);
Port_spinBox->setSpecialValueText ("none");
Port_spinBox->setMaximum (DEFAULT_JPIP_SERVER_PORT_MAX);
Port_spinBox->setValue (Port);
Port_spinBox->setKeyboardTracking (true);
Port_spinBox->setToolTip
	(tr ("Server port to use when none specified"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Port_spinBox);
#endif
connect (Port_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
grid_layout->addWidget (Port_spinBox, row, col);
//		Reset button.
++col;
Port_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Port_Reset_Button->setVisible (false);
Port_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Port_Reset_Button, SIGNAL (clicked ()),
	SLOT (JPIP_server_port_reset ()));
grid_layout->addWidget (Port_Reset_Button, row, col, Qt::AlignLeft);

//	Spacing
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Proxy.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Pro&xy URL:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Proxy_lineEdit = new QLineEdit (Proxy, this);
Proxy_lineEdit->setToolTip
	(tr ("Proxy URL for JPIP server access"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Proxy_lineEdit);
#endif
connect (Proxy_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (JPIP_proxy (const QString&)));
connect (Proxy_lineEdit, SIGNAL (editingFinished ()),
	SLOT (JPIP_proxy_changed ()));
grid_layout->addWidget (Proxy_lineEdit, row, col, 1, 5);
//		Reset button.
col += 5;
Proxy_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Proxy_Reset_Button->setVisible (false);
Proxy_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Proxy_Reset_Button, SIGNAL (clicked ()),
	SLOT (JPIP_proxy_reset ()));
grid_layout->addWidget (Proxy_Reset_Button,
	row, col, Qt::AlignLeft);

//	Spacing
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Cache Directory.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("&Cache Directory:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Cache_Directory_lineEdit = new QLineEdit (Cache_Directory, this);
Cache_Directory_lineEdit->setToolTip
	(tr ("JPIP codestream data bin cache directory"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Cache_Directory_lineEdit);
#endif
connect (Cache_Directory_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (JPIP_cache_directory (const QString&)));
connect (Cache_Directory_lineEdit, SIGNAL (editingFinished ()),
	SLOT (JPIP_cache_directory_changed ()));
grid_layout->addWidget (Cache_Directory_lineEdit, row, col, 1, 4);
//		Select button.
col += 4;
Cache_Directory_Select_Button = new QPushButton (tr ("Select ..."), this);
connect (Cache_Directory_Select_Button, SIGNAL (clicked ()),
	SLOT (select_JPIP_cache_directory ()));
grid_layout->addWidget (Cache_Directory_Select_Button, row, col);
//		Reset button.
++col;
Cache_Directory_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Cache_Directory_Reset_Button->setVisible (false);
Cache_Directory_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Cache_Directory_Reset_Button, SIGNAL (clicked ()),
	SLOT (JPIP_cache_directory_reset ()));
grid_layout->addWidget (Cache_Directory_Reset_Button,
	row, col, Qt::AlignLeft);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Wait Seconds
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Maximum &Wait Time:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Request_Timeout_spinBox = new QSpinBox (this);
Request_Timeout_spinBox->setAlignment (Qt::AlignRight);
Request_Timeout_spinBox->setAccelerated (true);
Request_Timeout_spinBox->setMinimum (JPIP_REQUEST_TIMEOUT_MIN);
Request_Timeout_spinBox->setMaximum (DEFAULT_JPIP_REQUEST_TIMEOUT_MAX);
Request_Timeout_spinBox->setValue (Request_Timeout);
Request_Timeout_spinBox->setKeyboardTracking (true);
Request_Timeout_spinBox->setToolTip
	(tr ("Maximum time to wait for a server response"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Request_Timeout_spinBox);
#endif
connect (Request_Timeout_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
grid_layout->addWidget (Request_Timeout_spinBox, row, col);
//		units.
++col;
label = new QLabel (tr ("seconds"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Reset button.
++col;
Request_Timeout_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Request_Timeout_Reset_Button->setVisible (false);
Request_Timeout_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Request_Timeout_Reset_Button, SIGNAL (clicked ()),
	SLOT (JPIP_request_timeout_reset ()));
grid_layout->addWidget (Request_Timeout_Reset_Button, row, col, Qt::AlignLeft);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Max Source Image Area.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Maximum Source Image &Area:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Max_Source_Image_Area_MB_spinBox = new QSpinBox (this);
Max_Source_Image_Area_MB_spinBox->setAlignment (Qt::AlignRight);
Max_Source_Image_Area_MB_spinBox->setAccelerated (true);
Max_Source_Image_Area_MB_spinBox->setMinimum (0);
Max_Source_Image_Area_MB_spinBox->setMaximum (MAX_IMAGE_AREA_MB);
Max_Source_Image_Area_MB_spinBox->setValue (Max_Source_Image_Area_MB);
Max_Source_Image_Area_MB_spinBox->setKeyboardTracking (true);
Max_Source_Image_Area_MB_spinBox->setToolTip
	(tr ("Maximum time to wait for a server response"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Max_Source_Image_Area_MB_spinBox);
#endif
connect (Max_Source_Image_Area_MB_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
grid_layout->addWidget (Max_Source_Image_Area_MB_spinBox, row, col);
//		units.
++col;
label = new QLabel (tr ("M pixels"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Reset button.
++col;
Max_Source_Image_Area_MB_Reset_Button =
	new Icon_Button (*Reset_Button_Icon, this);
Max_Source_Image_Area_MB_Reset_Button->setVisible (false);
Max_Source_Image_Area_MB_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Max_Source_Image_Area_MB_Reset_Button, SIGNAL (clicked ()),
	SLOT (max_source_image_area_MB_reset ()));
grid_layout->addWidget (Max_Source_Image_Area_MB_Reset_Button,
	row, col, Qt::AlignLeft);
//		Padding.
++col;
grid_layout->setColumnStretch (col, 100);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);
grid_layout->setRowStretch (row, 100);

//	Defaults/Apply buttons.
++row;
QDialogButtonBox
	*buttons = new QDialogButtonBox (Qt::Horizontal, this);
Defaults_Button = buttons->addButton
	(tr ("Defaults"), QDialogButtonBox::ResetRole);
Defaults_Button->setIcon (*Defaults_Button_Icon);
connect (Defaults_Button, SIGNAL (clicked ()), SLOT (defaults ()));
QAction
	*action = new QAction (tr ("Defaults"), this);
action->setShortcut (tr ("Ctrl+Shift+D"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
addAction (action);
connect (action, SIGNAL (triggered ()),
		 Defaults_Button, SLOT (click ()));

Apply_Button = buttons->addButton
	(tr ("Apply"), QDialogButtonBox::ApplyRole);
if (Apply_Button_Icon)
	Apply_Button->setIcon (*Apply_Button_Icon);
connect (Apply_Button, SIGNAL (clicked ()), SLOT (apply ()));
action = new QAction (tr ("Apply"), this);
action->setShortcut (tr ("Ctrl+Shift+A"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
connect (action, SIGNAL (triggered ()),
		 Apply_Button, SLOT (click ()));
addAction (action);
grid_layout->addWidget (buttons,
	row, 0, 1, -1);
/*
//	Initialize the GUI widget values.
HTTP_to_JPIP_Hostname_Pending = Default_HTTP_to_JPIP_Hostname;
if (! (OK = HTTP_to_JPIP_hostname_is_valid (hostname)) &&
	hostname != Default_HTTP_to_JPIP_Hostname)
	OK = HTTP_to_JPIP_hostname_is_valid (Default_HTTP_to_JPIP_Hostname);
if (! OK &&
	! Default_HTTP_to_JPIP_Hostname.isEmpty ())
	HTTP_to_JPIP_hostname_is_valid ("");
*/
JPIP_server_port (port);

Proxy_Pending = Default_JPIP_Proxy;
if (! (OK = JPIP_proxy_is_valid (proxy)) &&
	proxy != Default_JPIP_Proxy)
	OK = JPIP_proxy_is_valid (Default_JPIP_Proxy);
if (! OK)
	JPIP_proxy_is_valid (Default_JPIP_Proxy = "");

Cache_Directory_Pending = Default_JPIP_Cache_Directory;
if (! (OK = JPIP_cache_directory_is_valid (cache_directory)) &&
	cache_directory != Default_JPIP_Cache_Directory)
	OK = JPIP_cache_directory_is_valid (Default_JPIP_Cache_Directory);
if (! OK)
	JPIP_cache_directory_is_valid (Default_JPIP_Cache_Directory = "");

JPIP_request_timeout (seconds);
max_source_image_area_MB (image_area);

//	Update settings with any changed values.
apply ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_JPIP))
clog << "<<< JPIP_Section" << endl;
#endif
}

/*
void
JPIP_Section::HTTP_to_JPIP_hostname
	(
	const QString&	text
	)
{
if (HTTP_to_JPIP_Hostname_lineEdit->text () != text)
	HTTP_to_JPIP_Hostname_lineEdit->setText (text);
HTTP_to_JPIP_Hostname_Reset_Button->setVisible (text != HTTP_to_JPIP_Hostname);
reset_modifier_buttons ();
}


bool
JPIP_Section::HTTP_to_JPIP_hostname_is_valid
	(
	const QString&	hostname
	)
{
bool
	accepted = true;

if (hostname.isEmpty () ||
	QHostInfo::fromName (hostname).error () == QHostInfo::NoError ||
	QMessageBox::question ((isVisible () ? this : NULL), Title,
		tr ("A DNS lookup of the \"") + hostname
		+ tr ("\" HTTP-to-JPIP hostname failed.\n\n")
		+ tr ("The host may be temporarily unavailable -\n")
		+ tr ("for example, a VPN connection may be required.\n\n")
		+ tr ("Use the hostname anyway?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No)
	== QMessageBox::Yes)
	HTTP_to_JPIP_Hostname_Pending = hostname;
else
	accepted = false;

HTTP_to_JPIP_hostname (HTTP_to_JPIP_Hostname_Pending);

return accepted;
}


void
JPIP_Section::HTTP_to_JPIP_hostname_changed ()
{
//	Avoid redundant editingFinished signals.
static bool
	OK = true;
if (OK &&
	HTTP_to_JPIP_Hostname_lineEdit->text () != HTTP_to_JPIP_Hostname_Pending &&
	! Defaults_Button->hasFocus ())
	{
	OK = false;
	HTTP_to_JPIP_hostname_is_valid (HTTP_to_JPIP_Hostname_lineEdit->text ());
	OK = true;
	}
}


void
JPIP_Section::HTTP_to_JPIP_hostname_reset ()
{HTTP_to_JPIP_hostname_is_valid (HTTP_to_JPIP_Hostname);}
*/

void
JPIP_Section::JPIP_server_port
	(
	int		port
	)
{
if (Port_spinBox->value () != port)
	Port_spinBox->setValue (port);
Port_Reset_Button->setVisible (port != Default_JPIP_Server_Port);
reset_modifier_buttons ();
}


void
JPIP_Section::JPIP_server_port_reset ()
{JPIP_server_port (Port);}


void
JPIP_Section::JPIP_proxy
	(
	const QString&	text
	)
{
if (Proxy_lineEdit->text () != text)
	Proxy_lineEdit->setText (text);
Proxy_Reset_Button->setVisible (text != Proxy);
reset_modifier_buttons ();
}


bool
JPIP_Section::JPIP_proxy_is_valid
	(
	const QString&	proxy
	)
{
bool
	accepted = true;
if (proxy.isEmpty ())
	Proxy_Pending = proxy;
else
	{
	QUrl
		URL (QUrl::fromUserInput (proxy));
	if (URL.isValid () &&
		URL.scheme ().toLower () == "http" &&
		! URL.host ().isEmpty ())
		{
		if (QHostInfo::fromName (URL.host ()).error () == QHostInfo::NoError ||
			QMessageBox::question ((isVisible () ? this : NULL), Title,
				tr ("A lookup of the \"") + URL.toString ()
				+ tr ("\" JPIP proxy hostname failed.\n\n")
				+ tr ("Use the proxy anyway?"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No)
			== QMessageBox::Yes)
				Proxy_Pending = URL.toString ();
		else
			accepted = false;
		}
	else
		{
		QMessageBox::information ((isVisible () ? this : NULL), Title,
			QString ("JPIP proxy \"") + proxy + "\" "
			+ tr ("is not a valid proxy URL."));
		accepted = false;
		}
	}

JPIP_proxy (Proxy_Pending);

return accepted;
}


void
JPIP_Section::JPIP_proxy_changed ()
{
//	Avoid redundant editingFinished signals.
static bool
	OK = true;
if (OK &&
	Proxy_lineEdit->text () != Proxy_Pending &&
	! Defaults_Button->hasFocus ())
	{
	OK = false;
	JPIP_proxy_is_valid (Proxy_lineEdit->text ());
	OK = true;
	}
}


void
JPIP_Section::JPIP_proxy_reset ()
{JPIP_proxy_is_valid (Proxy);}


void
JPIP_Section::JPIP_cache_directory
	(
	const QString&	text
	)
{
if (Cache_Directory_lineEdit->text () != text)
	Cache_Directory_lineEdit->setText (text);
Cache_Directory_Reset_Button->setVisible (text != Cache_Directory);
reset_modifier_buttons ();
}


bool
JPIP_Section::JPIP_cache_directory_is_valid
	(
	const QString&	cache_directory
	)
{
bool
	accepted = true;
QString
	pathname (cache_directory);
possible_abbreviated_home_path (pathname);

if (! pathname.isEmpty ())
	{
	QFileInfo
		file (pathname);
	pathname = file.absoluteFilePath ();
	accepted = false;
	if (! file.exists ())
		{
		if (QMessageBox::question ((isVisible () ? this : NULL), Title,
				tr ("The JPIP cache directory \"") + pathname
				+ tr ("\" does not exist.\n\n")
				+ tr ("Create the directory?\n"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::Yes)
			== QMessageBox::Yes)
			{
			QDir 
				directory;
			if (directory.mkpath (pathname))
				accepted = true;
			else
				QMessageBox::warning ((isVisible () ? this : NULL), Title,
					tr ("The JPIP cache directory at \"") + pathname
					+ tr ("\" could not be created."));
			}
		}
	else
	if (! file.isDir ())
		QMessageBox::warning ((isVisible () ? this : NULL), Title,
			QString ("\"") + pathname + tr ("\" is not a directory."));
	else
	if (! file.isReadable () ||
		! file.isWritable ())
		QMessageBox::warning ((isVisible () ? this : NULL), Title,
			tr ("The JPIP cache directory \"") + pathname
			+ tr ("\" is not accessible for reading or writing."));
	else
		accepted = true;
	}
if (accepted)
	Cache_Directory_Pending = pathname;

JPIP_cache_directory (Cache_Directory_Pending);

return accepted;
}


void
JPIP_Section::JPIP_cache_directory_changed ()
{
//	Avoid redundant editingFinished signals.
static bool
	OK = true;
if (OK &&
	Cache_Directory_lineEdit->text () != Cache_Directory_Pending &&
	! Defaults_Button->hasFocus ())
	{
	OK = false;
	JPIP_cache_directory_is_valid (Cache_Directory_lineEdit->text ());
	OK = true;
	}
}


void
JPIP_Section::select_JPIP_cache_directory ()
{
if (! File_Selection_Dialog)
	{
	File_Selection_Dialog = new QFileDialog ((isVisible () ? this : NULL),
		Title + " - " + tr ("Cache Directory"),
		QDir::homePath ());
	File_Selection_Dialog->setAcceptMode (QFileDialog::AcceptOpen);
	File_Selection_Dialog->setFileMode (QFileDialog::Directory);
	File_Selection_Dialog->setFilter
		(QDir::Dirs |
		 QDir::Hidden |
		 QDir::NoDotAndDotDot);
	File_Selection_Dialog->setOptions
		(QFileDialog::DontUseNativeDialog |
		 QFileDialog::ShowDirsOnly |
		 QFileDialog::DontResolveSymlinks);
	}

if (File_Selection_Dialog->exec ())
	{
	QString
		pathname (File_Selection_Dialog->selectedFiles ().value (0));
	if (! pathname.isEmpty ())
		JPIP_cache_directory_is_valid (pathname);
	}
}


void
JPIP_Section::JPIP_cache_directory_reset ()
{JPIP_cache_directory_is_valid (Cache_Directory);}


void
JPIP_Section::JPIP_request_timeout
	(
	int		seconds
	)
{
if (seconds < 0)
	seconds = 0;

if (Request_Timeout_spinBox->value () != seconds)
	Request_Timeout_spinBox->setValue (seconds);
Request_Timeout_Reset_Button->setVisible (seconds != Request_Timeout);
reset_modifier_buttons ();
}


void
JPIP_Section::JPIP_request_timeout_reset ()
{JPIP_request_timeout (Request_Timeout);}


void
JPIP_Section::max_source_image_area_MB
	(
	int		area
	)
{
if (Max_Source_Image_Area_MB_spinBox->value () != area)
	Max_Source_Image_Area_MB_spinBox->setValue (area);
Max_Source_Image_Area_MB_Reset_Button->setVisible
	(area != Max_Source_Image_Area_MB);
reset_modifier_buttons ();
}


void
JPIP_Section::max_source_image_area_MB_reset ()
{max_source_image_area_MB (Max_Source_Image_Area_MB);}


void
JPIP_Section::reset ()
{
//HTTP_to_JPIP_hostname_reset ();
JPIP_server_port_reset ();
JPIP_proxy_reset ();
JPIP_cache_directory_reset ();
JPIP_request_timeout_reset ();
max_source_image_area_MB_reset ();
}


void
JPIP_Section::defaults ()
{
//HTTP_to_JPIP_hostname (Default_HTTP_to_JPIP_Hostname);
JPIP_server_port (Default_JPIP_Server_Port);
JPIP_proxy (Default_JPIP_Proxy);
JPIP_cache_directory (Default_JPIP_Cache_Directory);
JPIP_request_timeout (Default_JPIP_Request_Timeout);
max_source_image_area_MB (Default_Max_Source_Image_Area_MB);
}


void
JPIP_Section::reset_modifier_buttons ()
{
reset_defaults_button ();
Apply_Button->setEnabled (has_changed ());
}


void
JPIP_Section::reset_defaults_button ()
{
Defaults_Button->setEnabled
	(
//HTTP_to_JPIP_Hostname_lineEdit->text ()
//		!= Default_HTTP_to_JPIP_Hostname	||
	Port_spinBox->value ()
		!= Default_JPIP_Server_Port			||
	Proxy_lineEdit->text ()
		!= Default_JPIP_Proxy				||
	Cache_Directory_lineEdit->text ()
		!= Default_JPIP_Cache_Directory		||
	Request_Timeout_spinBox->value ()
		!= Default_JPIP_Request_Timeout				||
	Max_Source_Image_Area_MB_spinBox->value ()
		!= Default_Max_Source_Image_Area_MB
	);
}


void
JPIP_Section::changing ()
{
bool
	change,
	changed = false;

//	Text fields manage their Reset_Buttons.
//changed |= (HTTP_to_JPIP_Hostname_lineEdit->text ()
//		!= HTTP_to_JPIP_Hostname);
Port_Reset_Button->setVisible (change =
	(Port_spinBox->value ()
		!= Port));
changed |= change;
changed |= (Proxy_lineEdit->text ()
		!= Proxy);
changed |= (Cache_Directory_lineEdit->text ()
		!= Cache_Directory);
Request_Timeout_Reset_Button->setVisible (change =
	(Request_Timeout_spinBox->value ()
		!= Request_Timeout));
changed |= change;
Max_Source_Image_Area_MB_Reset_Button->setVisible (change =
	(Max_Source_Image_Area_MB_spinBox->value ()
		!= Max_Source_Image_Area_MB));
changed |= change;

reset_defaults_button ();
Apply_Button->setEnabled (changed);
}


bool
JPIP_Section::has_changed () const
{
return
//	HTTP_to_JPIP_Hostname_lineEdit->text ()
//		!= HTTP_to_JPIP_Hostname			||
	Port_spinBox->value ()
		!= Port								||
	Proxy_lineEdit->text ()
		!= Proxy							||
	Cache_Directory_lineEdit->text ()
		!= Cache_Directory					||
	Request_Timeout_spinBox->value ()
		!= Request_Timeout						||
	Max_Source_Image_Area_MB_spinBox->value ()
		!= Max_Source_Image_Area_MB;
}


void
JPIP_Section::apply ()
{
QSettings
	settings;
QString
	text;

/*text = HTTP_to_JPIP_Hostname_lineEdit->text ();
if (HTTP_to_JPIP_Hostname != text)
	{
	HTTP_to_JPIP_Hostname = text;
	HTTP_to_JPIP_Hostname_Reset_Button->setVisible (false);
	settings.setValue (HTTP_TO_JPIP_HOSTNAME_KEY, HTTP_to_JPIP_Hostname);
	//	>>> SIGNAL <<<
	emit HTTP_to_JPIP_hostname_changed (HTTP_to_JPIP_Hostname);
	}
*/
if (Port != Port_spinBox->value ())
	{
	Port = Port_spinBox->value ();
	Port_Reset_Button->setVisible (false);
	settings.setValue (JPIP_SERVER_PORT_KEY, Port);
	//	>>> SIGNAL <<<
	emit JPIP_server_port_changed (Port);
	}
text = Proxy_lineEdit->text ();
if (Proxy != text &&
	JPIP_proxy_is_valid (text))
	{
	Proxy = text;
	Proxy_Reset_Button->setVisible (false);
	settings.setValue (JPIP_PROXY_KEY, Proxy);
	//	>>> SIGNAL <<<
	emit JPIP_proxy_changed (Proxy);
	}
text = Cache_Directory_lineEdit->text ();
if (Cache_Directory != text &&
	JPIP_cache_directory_is_valid (text))
	{
	Cache_Directory = text;
	Cache_Directory_Reset_Button->setVisible (false);
	settings.setValue (JPIP_CACHE_DIRECTORY_KEY, Cache_Directory);
	//	>>> SIGNAL <<<
	emit JPIP_cache_directory_changed (Cache_Directory);
	}
if (Request_Timeout != Request_Timeout_spinBox->value ())
	{
	Request_Timeout = Request_Timeout_spinBox->value ();
	Request_Timeout_Reset_Button->setVisible (false);
	settings.setValue (JPIP_REQUEST_TIMEOUT_KEY, Request_Timeout);
	//	>>> SIGNAL <<<
	emit JPIP_request_timeout_changed (Request_Timeout);
	}
if (Max_Source_Image_Area_MB != Max_Source_Image_Area_MB_spinBox->value ())
	{
	Max_Source_Image_Area_MB = Max_Source_Image_Area_MB_spinBox->value ();
	Max_Source_Image_Area_MB_Reset_Button->setVisible (false);
	settings.setValue
		(MAX_SOURCE_IMAGE_AREA_MB_KEY, Max_Source_Image_Area_MB);
	//	>>> SIGNAL <<<
	emit max_source_image_area_MB_changed (Max_Source_Image_Area_MB);
	}

reset_modifier_buttons ();
}

/*=*****************************************************************************
	Graphs_Section
*/
const char
	*Graphs_Section::SELECTION_SENSITIVITY_KEY = "Graphs_Selection_Sensitivity";

#ifndef DEFAULT_SELECTION_SENSITIVITY
#define DEFAULT_SELECTION_SENSITIVITY		5
#endif
int
	Graphs_Section::Default_Selection_Sensitivity
		= DEFAULT_SELECTION_SENSITIVITY;

const char
	*Graphs_Section::CANVAS_COLOR_KEY	= "Graphs_Canvas_Color";

QRgb
	Graphs_Section::Default_Canvas_Color;


Graphs_Section::Graphs_Section
	(
	QWidget*	parent
	)
	:	QWidget (parent)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << ">>> Graphs_Section" << endl;
#endif
if (parent)
	Title = parent->windowTitle ();
else
	Title = tr ("Preferences");
Title += tr (": Graphs");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    Title = \"" << Title << '"' << endl;
#endif

//	Initialize values.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    settings -" << endl;
#endif
QSettings
	settings;
bool
	OK;

int
	sensitivity =
	Selection_Sensitivity = settings.value (SELECTION_SENSITIVITY_KEY,
		Default_Selection_Sensitivity).toInt (&OK);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    " << SELECTION_SENSITIVITY_KEY
		<< " = " << Selection_Sensitivity << endl;
#endif
if (sensitivity < 0)
	sensitivity =
	Selection_Sensitivity = 0;
if (! OK)
	{
	QMessageBox::warning ((isVisible () ? this : NULL), Title,
		tr ("The ") + SELECTION_SENSITIVITY_KEY + " \""
		+ settings.value (SELECTION_SENSITIVITY_KEY).toString ()
		+ tr ("\" value is invalid - a number is required.\n\n")
		+ tr ("The default value of ")
		+ QString::number (Default_Selection_Sensitivity)
		+ tr (" is being used."));
	sensitivity = Default_Selection_Sensitivity;
	}
else
if (! settings.contains (SELECTION_SENSITIVITY_KEY))
	settings.setValue (SELECTION_SENSITIVITY_KEY,
		Default_Selection_Sensitivity);

#ifdef Q_WS_X11
QColor::setAllowX11ColorNames (true);
#endif
//	Initialize the default value from the HiView_Config setting.
Default_Canvas_Color = HiView_Config::Default_Graph_Canvas_Color;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    Default_Canvas_Color = "
		<< hex << Default_Canvas_Color << dec << endl;
#endif
Default_Canvas_Color_Text = color_text (Default_Canvas_Color);
QString
	color =
	Canvas_Color_Text = settings.value (CANVAS_COLOR_KEY,
		Default_Canvas_Color_Text).toString ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    " << CANVAS_COLOR_KEY << " = "
		<< Canvas_Color_Text << endl;
#endif
if (! settings.contains (CANVAS_COLOR_KEY))
	{
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
	clog << "    initial " << CANVAS_COLOR_KEY << " setting = "
			<< Default_Canvas_Color_Text << endl;
	#endif
	settings.setValue (CANVAS_COLOR_KEY, Default_Canvas_Color_Text);
	}

//	Layout controls.
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "    layout controls -" << endl;
#endif
QGridLayout
	*grid_layout = new QGridLayout (this);
grid_layout->setHorizontalSpacing (HORIZONTAL_SPACING);

QLabel
	*label;
int
	row = -1,
	col;

//	Selection Sensitivity.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Selection Sensiti&vity:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Selection_Sensitivity_spinBox = new QSpinBox (this);
Selection_Sensitivity_spinBox->setAlignment (Qt::AlignRight);
Selection_Sensitivity_spinBox->setAccelerated (true);
Selection_Sensitivity_spinBox->setMinimum (0);
Selection_Sensitivity_spinBox->setMaximum (100);
Selection_Sensitivity_spinBox->setValue (Selection_Sensitivity);
Selection_Sensitivity_spinBox->setKeyboardTracking (true);
Selection_Sensitivity_spinBox->setToolTip
	(tr ("Sensitivity (roughly distance) to selecting a graph item"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Selection_Sensitivity_spinBox);
#endif
connect (Selection_Sensitivity_spinBox, SIGNAL (valueChanged (int)),
	SLOT (changing ()));
connect (Selection_Sensitivity_spinBox, SIGNAL (editingFinished ()),
	SLOT (selection_sensitivity_change ()));
grid_layout->addWidget (Selection_Sensitivity_spinBox, row, col);
//		Reset button.
++col;
Selection_Sensitivity_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Selection_Sensitivity_Reset_Button->setVisible (false);
Selection_Sensitivity_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Selection_Sensitivity_Reset_Button, SIGNAL (clicked ()),
	SLOT (selection_sensitivity_reset ()));
grid_layout->addWidget (Selection_Sensitivity_Reset_Button,
	row, col, Qt::AlignLeft);

//	Spacing
++row;
grid_layout->setRowMinimumHeight (row, 10);

//	Canvas Color.
++row;
//		Label.
col = 0;
label = new QLabel (tr ("Canvas &Color:"), this);
label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);
grid_layout->addWidget (label, row, col);
//		Value.
++col;
Canvas_Color = Default_Canvas_Color; // VALGRIND

Canvas_Color_lineEdit = new QLineEdit (color_text (Canvas_Color), this);
Canvas_Color_lineEdit->setMaximumWidth (90);
Canvas_Color_lineEdit->setToolTip
	(tr ("Color of graph canvas"));
#ifndef QT_NO_SHORTCUT
label->setBuddy (Canvas_Color_lineEdit);
#endif
connect (Canvas_Color_lineEdit, SIGNAL (textEdited (const QString&)),
	SLOT (canvas_color (const QString&)));
connect (Canvas_Color_lineEdit, SIGNAL (editingFinished ()),
	SLOT (canvas_color_changed ()));
grid_layout->addWidget (Canvas_Color_lineEdit, row, col, 1, 2);
//		Select button.
col += 2;
Canvas_Color_Select_Button = new QPushButton (tr ("Select ..."), this);
connect (Canvas_Color_Select_Button, SIGNAL (clicked ()),
	SLOT (select_canvas_color ()));
grid_layout->addWidget (Canvas_Color_Select_Button, row, col);
//		Reset button.
++col;
Canvas_Color_Reset_Button = new Icon_Button (*Reset_Button_Icon, this);
Canvas_Color_Reset_Button->setVisible (false);
Canvas_Color_Reset_Button->setFocusPolicy (Qt::NoFocus);
connect (Canvas_Color_Reset_Button, SIGNAL (clicked ()),
	SLOT (canvas_color_reset ()));
grid_layout->addWidget (Canvas_Color_Reset_Button, row, col, Qt::AlignLeft);
//		Padding.
++col;
grid_layout->setColumnStretch (col, 100);

//	Spacing.
++row;
grid_layout->setRowMinimumHeight (row, 10);
grid_layout->setRowStretch (row, 100);


//	Defaults/Apply buttons.
++row;
QDialogButtonBox
	*buttons = new QDialogButtonBox (Qt::Horizontal, this);
Defaults_Button = buttons->addButton
	(tr ("Defaults"), QDialogButtonBox::ResetRole);
Defaults_Button->setIcon (*Defaults_Button_Icon);
connect (Defaults_Button, SIGNAL (clicked ()), SLOT (defaults ()));
QAction
	*action = new QAction (tr ("Defaults"), this);
action->setShortcut (tr ("Ctrl+Shift+D"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
addAction (action);
connect (action, SIGNAL (triggered ()),
		 Defaults_Button, SLOT (click ()));

Apply_Button = buttons->addButton
	(tr ("Apply"), QDialogButtonBox::ApplyRole);
if (Apply_Button_Icon)
	Apply_Button->setIcon (*Apply_Button_Icon);
connect (Apply_Button, SIGNAL (clicked ()), SLOT (apply ()));
action = new QAction (tr ("Apply"), this);
action->setShortcut (tr ("Ctrl+Shift+A"));
action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
connect (action, SIGNAL (triggered ()),
		 Apply_Button, SLOT (click ()));
addAction (action);
grid_layout->addWidget (buttons,
	row, 0, 1, -1);

//	Initialize the GUI widget values.
selection_sensitivity (sensitivity);
Canvas_Color_Text_Pending = Default_Canvas_Color_Text;
if (! (OK = canvas_color_is_valid (color)) &&
	color != Default_Canvas_Color_Text)
	OK = canvas_color_is_valid (Default_Canvas_Color_Text);
if (! OK)
	canvas_color_is_valid (Default_Canvas_Color_Text = "transparent");
Canvas_Color = color_value (Canvas_Color_Text);

//	Update settings with any changed values.
apply ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_GRAPHS))
clog << "<<< Graphs_Section" << endl;
#endif
}


void
Graphs_Section::selection_sensitivity
	(
	int		size
	)
{
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << ">>> Graphs_Section::selection_sensitivity: " << size << endl;
#endif
if (size < 0)
	size = 0;

if (Selection_Sensitivity_spinBox->value () != size)
	Selection_Sensitivity_spinBox->setValue (size);

Selection_Sensitivity_Reset_Button->setVisible (size != Selection_Sensitivity);

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << "<<< Graphs_Section::selection_sensitivity" << endl;
#endif
}


void
Graphs_Section::selection_sensitivity_change ()
{
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << ">-< Rendering_Section::selection_sensitivity_change" << endl;
#endif
selection_sensitivity (Selection_Sensitivity_spinBox->value ());
}


void
Graphs_Section::selection_sensitivity_reset ()
{selection_sensitivity (Selection_Sensitivity);}


void
Graphs_Section::canvas_color
	(
	const QString&	text
	)
{
if (Canvas_Color_lineEdit->text () != text)
	Canvas_Color_lineEdit->setText (text);
Canvas_Color_Reset_Button->setVisible (text != Canvas_Color_Text);
reset_modifier_buttons ();
}


bool
Graphs_Section::canvas_color_is_valid
	(
	const QString&	color_spec
	)
{
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << ">>> Graphs_Section::canvas_color_is_valid: "
		<< color_spec << endl;
#endif
bool
	accepted = true;
QString
	text (color_spec);
if (text.toLower () == "none")
	text = "transparent";

//	Prevent gratuitous warning message from QColor about invalid color name.
qInstallMessageHandler (no_warning_messages);
QColor
	color (text);
qInstallMessageHandler (0);

if (color.isValid ())
	//	Use the hex triplet color name.
	Canvas_Color_Text_Pending = color_text (color.rgba ());
else
	{
	QMessageBox::information ((isVisible () ? this : NULL), Title,
		QString ("\"") + Canvas_Color_lineEdit->text ()
		+ tr ("\" is an invalid canvas color.\n\n")
		+ tr ("The color must be an RGB hex value triplet "
		      "beginning with the '#' character, "
		      "or a recognized color name such as \"black\"."));
	accepted = false;
	//	Restore the pending color.
	}
canvas_color (Canvas_Color_Text_Pending);

#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << "<<< Graphs_Section::canvas_color_is_valid: " << accepted << endl;
#endif
return accepted;
}


void
Graphs_Section::canvas_color_changed ()
{
//	Avoid redundant editingFinished signals.
static bool
	OK = true;
if (OK &&
	Canvas_Color_lineEdit->text () != Canvas_Color_Text_Pending &&
	! Defaults_Button->hasFocus ())
	{
	#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
	clog << ">-< Graphs_Section::canvas_color_changed" << endl;
	#endif
	OK = false;
	canvas_color_is_valid (Canvas_Color_lineEdit->text ());
	OK = true;
	}
}


void
Graphs_Section::select_canvas_color ()
{
QColor
	color (QColorDialog::getColor
		(color_value (Canvas_Color_lineEdit->text ()),
		(isVisible () ? this : NULL),
		Title + " - " + tr ("Canvas Color")));
if (color.isValid ())
	canvas_color_is_valid (color_text (color.rgb ()));
}


void
Graphs_Section::canvas_color_reset ()
{canvas_color_is_valid (Canvas_Color_Text);}


QString
Graphs_Section::color_text
	(
	QRgb	color
	)
{return Preferences_Dialog::color_text (color);}


QRgb
Graphs_Section::color_value
	(
	const QString&	text
	)
{return Preferences_Dialog::color_value (text);}


void
Graphs_Section::reset ()
{
selection_sensitivity_reset ();
canvas_color_reset ();
}


void
Graphs_Section::defaults ()
{
selection_sensitivity (Default_Selection_Sensitivity);
canvas_color (Default_Canvas_Color_Text);
}


void
Graphs_Section::reset_modifier_buttons ()
{
reset_defaults_button ();
Apply_Button->setEnabled (has_changed ());
}


void
Graphs_Section::reset_defaults_button ()
{
Defaults_Button->setEnabled
	(
	Selection_Sensitivity_spinBox->value ()
		!= Default_Selection_Sensitivity					||
	Canvas_Color_lineEdit->text ()
		!= Default_Canvas_Color_Text
	);
}


void
Graphs_Section::changing ()
{
bool
	changed = false;
Selection_Sensitivity_Reset_Button->setVisible (changed =
	(Selection_Sensitivity_spinBox->value ()
		!= Selection_Sensitivity));
//	Text fields manage their Reset_Buttons.
changed |= (Canvas_Color_lineEdit->text ()
		!= Canvas_Color_Text);

reset_defaults_button ();
Apply_Button->setEnabled (changed);
}


bool
Graphs_Section::has_changed () const
{
return
	Selection_Sensitivity_spinBox->value ()
		!= Selection_Sensitivity					||
	Canvas_Color_lineEdit->text ()
		!= Canvas_Color_Text;
}


void
Graphs_Section::apply ()
{
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << ">>> Graphs_Section::apply" << endl;
#endif
QSettings
	settings;

if (Selection_Sensitivity != Selection_Sensitivity_spinBox->value ())
	{
	Selection_Sensitivity = Selection_Sensitivity_spinBox->value ();
	Selection_Sensitivity_Reset_Button->setVisible (false);
	settings.setValue (SELECTION_SENSITIVITY_KEY, Selection_Sensitivity);
	//	>>> SIGNAL <<<
	emit selection_sensitivity_changed (Selection_Sensitivity);
	}
QString
	text = Canvas_Color_lineEdit->text ();
if (Canvas_Color_Text != text &&
	canvas_color_is_valid (text))
	{
	Canvas_Color_Text = text;
	Canvas_Color = color_value (text);
	Canvas_Color_Reset_Button->setVisible (false);
	#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
	clog << "    Canvas_Color_Text_Pending = "
		 	<< Canvas_Color_Text_Pending << endl
		 << "            Canvas_Color_Text = "
			<< Canvas_Color_Text << endl
		 << "                 Canvas_Color = "
		 	<< color_text (Canvas_Color) << endl;
	#endif
	settings.setValue (CANVAS_COLOR_KEY, Canvas_Color_Text);
	//	>>> SIGNAL <<<
	emit canvas_color_changed (Canvas_Color);
	}

reset_modifier_buttons ();
#if ((DEBUG_SECTION) & DEBUG_GRAPHS)
clog << "<<< Graphs_Section::apply" << endl;
#endif
}

/******************************************************************
 *Scripts Section
 ******************************************************************/
 
/*=================================================================
  = Constants
  =================================================================*/
const char* Scripts_Section::CURRENT_SCRIPT_KEY = "Scripts_Current_Script";
const char* Scripts_Section::SHOW_SCRIPT_KEY = "Scripts_Show_Script";

/*=================================================================
  = Defaults
  =================================================================*/
#ifndef DEFUALT_SHOW_SCRIPT
#define DEFAULT_SHOW_SCRIPT		true
#endif

/*=================================================================
  = Constructors
  =================================================================*/
  
Scripts_Section::Scripts_Section (QWidget* parent) {
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SCRIPTS))
		clog << ">>> Graphs_Section" << endl;
	#endif
	if (parent)
		Title = parent->windowTitle ();
	else
		Title = tr ("Preferences");
	Title += tr (": Graphs");
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_SCRIPTS))
		clog << "    Title = \"" << Title << '"' << endl;
	#endif
	
	//Restore Settings
	QSettings settings;
	
	if(settings.contains(CURRENT_SCRIPT_KEY))
		Script = settings.value(CURRENT_SCRIPT_KEY, "").toString();
	else 
		settings.setValue(CURRENT_SCRIPT_KEY, Script = "");
	
	if(settings.contains(SHOW_SCRIPT_KEY))
		Show_Script = settings.value(SHOW_SCRIPT_KEY).toBool();
	else 
		settings.setValue(SHOW_SCRIPT_KEY, Show_Script = DEFAULT_SHOW_SCRIPT);
		
	//Layout Variables	
	QGridLayout *grid_layout = new QGridLayout (this);
	int row = 0;
	
	QLabel *label = new QLabel(tr ("Enter Script:"));
	label->setAlignment(Qt::AlignVCenter);
	grid_layout->addWidget(label, row, 0);
	
	label = new QLabel(tr ("Variables:"));
	label->setAlignment(Qt::AlignVCenter);
	grid_layout->addWidget(label, row, 1);
	++row;
	
	Script_TextEdit = new QTextEdit(tr ("Script"));
	Script_TextEdit->setText(Script);
	Script_TextEdit->setMinimumSize(500, 250);
	connect(Script_TextEdit, SIGNAL(textChanged ()), SLOT(script_edited()));
	grid_layout->addWidget(Script_TextEdit,row, 0);
	
	Variables_ListWidget = new QListWidget (this);
	Variables_ListWidget->setAlternatingRowColors (true);
	Variables_ListWidget->setVerticalScrollMode (QAbstractItemView::ScrollPerItem);
	Variables_ListWidget->setTextElideMode (Qt::ElideNone);
	Variables_ListWidget->setFixedWidth(Variables_ListWidget->sizeHint().width());
	connect(Variables_ListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(add_variable(QListWidgetItem *)));
	grid_layout->addWidget(Variables_ListWidget);
	++row;
	
	grid_layout->setRowMinimumHeight(0, 20);
	Show_Script_CheckBox = new QCheckBox(tr ("Show Script Panel"));
	Show_Script_CheckBox->setChecked(Show_Script);
	Show_Script_CheckBox->setChecked (Show_Script);
	connect(Show_Script_CheckBox, SIGNAL (toggled (bool)), SLOT(show_script(bool)));
	grid_layout->addWidget(Show_Script_CheckBox, row, 0); 
	++row;
	
	//Default and Apply Buttons
	QDialogButtonBox *buttons = new QDialogButtonBox (Qt::Horizontal, this);
	Defaults_Button = buttons->addButton(tr ("Defaults"), QDialogButtonBox::ResetRole);
	Defaults_Button->setIcon (*Defaults_Button_Icon);
	connect(Defaults_Button, SIGNAL(clicked()), SLOT(defaults()));
	
/*	DEFAULTS QACTION*/
	QAction *action = new QAction(tr ("Defaults"), this);
	action->setShortcut(tr ("Ctrl+Shift+D"));
	action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
	addAction(action);
	connect(action, SIGNAL(triggered()), Defaults_Button, SLOT(click ()));
	Defaults_Button->setEnabled(reset_defaults_button());
	
	Apply_Button = buttons->addButton(QDialogButtonBox::Apply);
	if(Apply_Button_Icon)
		Apply_Button->setIcon(*Apply_Button_Icon);
	connect(Apply_Button, SIGNAL(clicked ()), SLOT(apply()));

/*	APPLY QACTION*/
	action = new QAction (tr ("Apply"), this);
	action->setShortcut(tr ("Ctrl+Shift+A"));
	action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	addAction(action);
	connect(action, SIGNAL (triggered()), Apply_Button, SLOT(click ()));
	addAction (action);
	Apply_Button->setEnabled(false);
	
	grid_layout->addWidget (buttons, row, 0, 1, -1);
	
	
}

/*=================================================================
  = Slots
  =================================================================*/
void Scripts_Section::apply() {
	QSettings settings;
	
	QString script = Script_TextEdit->toPlainText();
	if(script != Script) {
		Script = script;
		settings.setValue(CURRENT_SCRIPT_KEY, Script);
		emit script_changed(Script);
	}
	
	if(Show_Script_CheckBox->isChecked() != Show_Script) {
		Show_Script = Show_Script_CheckBox->isChecked();
		settings.setValue(SHOW_SCRIPT_KEY, Show_Script);
		emit show_script_changed(Show_Script);
	}
	
	Apply_Button->setEnabled(false);
	Defaults_Button->setEnabled(reset_defaults_button());
}

void Scripts_Section::defaults() {
	Show_Script_CheckBox->setChecked(DEFAULT_SHOW_SCRIPT);
	Script_TextEdit->clear();
}

void Scripts_Section::variables_updated(QStringList &pds_variables) {
	Variables_ListWidget->clear();
	Variables_ListWidget->addItem("x_px");
	Variables_ListWidget->addItem("y_px");
	Variables_ListWidget->addItem("red");
	Variables_ListWidget->addItem("green");
	Variables_ListWidget->addItem("blue");
	Variables_ListWidget->addItem("scale");
 	Variables_ListWidget->addItem("region_area_px");
 	Variables_ListWidget->addItem("region_width_px");
 	Variables_ListWidget->addItem("region_height_px");
 	Variables_ListWidget->addItem("region_area_m");
 	Variables_ListWidget->addItem("region_width_m");
 	Variables_ListWidget->addItem("region_height_m");
 	Variables_ListWidget->addItem("distance_length_px");
 	Variables_ListWidget->addItem("distance_length_m");
	Variables_ListWidget->addItems(pds_variables);
	
}

void Scripts_Section::show_script(bool enabled) {
	if (Show_Script_CheckBox->isChecked () != enabled)
		Show_Script_CheckBox->setChecked (enabled);
	else
		reset_modifier_buttons ();
}

void Scripts_Section::script_edited() {
	reset_modifier_buttons();
}

void Scripts_Section::add_variable(QListWidgetItem *variable) {
	Script_TextEdit->insertPlainText(variable->text());
}

/*=================================================================
  = Helpers
  =================================================================*/
  
bool Scripts_Section::reset_defaults_button() {
	return (Script_TextEdit->toPlainText() != "") || (Show_Script_CheckBox->isChecked() != DEFAULT_SHOW_SCRIPT);
}

bool Scripts_Section::has_changed() {
	return (Script_TextEdit->toPlainText() != Script) || (Show_Script_CheckBox->isChecked() != Show_Script);
}
  
void Scripts_Section::reset_modifier_buttons() {
	Defaults_Button->setEnabled(reset_defaults_button());
	Apply_Button->setEnabled(has_changed());
}

}	//	namespace HiRISE
}	//	namespace UA
