/*	Preferences_Dialog

HiROC CVS ID: $Id: Preferences_Dialog.hh,v 1.34 2016/01/07 22:13:14 guym Exp $

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

#ifndef HiView_Preferences_Dialog_hh
#define HiView_Preferences_Dialog_hh

#include	<QWidget>
#include	<QDialog>
#include 	<QString>
#include	<QComboBox>
#include	<QTextEdit>

class QTabWidget;
class QCheckBox;
class QRadioButton;
class QLabel;
class QStringList;
class QListWidget;
class QListWidgetItem;
class QSpinBox;
class QPushButton;
class QDoubleSpinBox;
class QLineEdit;
class QIcon;
class QVariant;
class QFileDialog;
class QEvent;
class QCloseEvent;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Icon_Button;
class Help_Docs;

/*=*****************************************************************************
	General_Section
*/
class General_Section
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
static const char
	*RESTORE_LAYOUT_KEY;
//!	Obsolete.
static const char
	*RESTORE_WINDOW_POSITIONS_KEY;

static const char
	*RESTORE_LAST_SOURCE_KEY;

static const char
	*BAND_NUMBERS_INDEXED_KEY;

static const char
	*GET_PDS_LABEL_KEY;

static const char
	*DOCUMENTATION_LOCATION_KEY;
	
static const char
	*RESTORE_LONGITUDE_FORMAT_KEY;

static const char
	*RESTORE_LONGITUDE_DIRECTION_KEY;
	
static const char
	*RESTORE_LATITUDE_FORMAT_KEY;

/*==============================================================================
	Defaults
*/
static bool
	Default_Restore_Layout;

static bool
	Default_Restore_Last_Source;

static bool
	Default_Band_Numbers_Indexed;

static bool
	Default_Get_PDS_Metadata;
	
static int
	Default_Coordinate_Format;	

static const char
	*Default_Documentation_Search_Locations[];
static QStringList
	Documentation_Search_Locations;
static const QString
	Default_Documentation_Filename;
	
/*==============================================================================
	Constructor
*/
General_Section (QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
inline bool restore_layout () const
	{return Restore_Layout;}

inline bool restore_last_source () const
	{return Restore_Last_Source;}

inline bool band_numbers_indexed () const
	{return Band_Numbers_Indexed;}

inline bool get_PDS_metadata () const
	{return Get_PDS_Metadata;}

inline QString documentation_location () const
	{return Documentation_Location;}

inline static const QString& default_documentation_filename ()
	{return Default_Documentation_Filename;}

bool has_changed () const;

/*==============================================================================
	Signals
*/
signals:

void band_numbers_indexed_changed (bool indexed);

void documentation_location_changed (const QString& location);

void longitude_direction_changed (int direction);

void longitude_units_changed (int units);

void latitude_units_changed (int units);

/*==============================================================================
	Slots:
*/
public slots:

void reset ();
void defaults ();
void apply ();

private slots:

void restore_layout (bool enabled);
void restore_last_source (bool enabled);

void band_numbers_indexed (bool indexed);

void longitude_format(int format);
void longitude_direction(int direction);
void latitude_format(int format);

void get_PDS_metadata (bool enabled);

void documentation_location (const QString& location);
void documentation_location_changed ();
bool find_documentation_location ();
void documentation_location_reset ();

/*==============================================================================
	Helpers
*/
private:

bool documentation_location_is_valid (const QString& location);

void reset_modifier_buttons ();
void reset_defaults_button ();

/*==============================================================================
	Data
*/
private:

QString
	Title;

bool
	Restore_Layout;
QCheckBox
	*Restore_Layout_CheckBox;

bool
	Restore_Last_Source;
QCheckBox
	*Restore_Last_Source_CheckBox;

bool
	Band_Numbers_Indexed;
QRadioButton
	*Band_Numbers_Indexed_Button;

bool Get_PDS_Metadata;
QCheckBox
	*Get_PDS_Metadata_CheckBox;

QString
	Documentation_Location,
	Documentation_Location_Pending;
QLineEdit
	*Documentation_Location_lineEdit;
Icon_Button
	*Documentation_Location_Reset_Button;

QPushButton
	*Defaults_Button,
	*Apply_Button;

QComboBox
	*Longitude_Units_ComboBox,
	*Longitude_Direction_ComboBox,
	*Latitude_Units_ComboBox;

int
	Longitude_Units,
	Longitude_Direction,
	Latitude_Units;
};

/*=*****************************************************************************
	Sources_Section
*/
class Sources_Section
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
static const char
	*SOURCE_LIST_CAPACITY_KEY;
static const char
	*SOURCE_LIST_KEY;

/*==============================================================================
	Defaults
*/
static int
	Default_Source_List_Capacity;
static int
	Default_Source_List_Capacity_Max;

/*==============================================================================
	Constructor
*/
Sources_Section (QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
inline int source_list_capacity () const
	{return Capacity;}

inline const QStringList& source_list () const
	{return *Source_List;}

/**	Set the source list.

	Any duplicate entries in the list are removed; the first entry of any
	duplicates remains.

	N.B.: If the number of entries in the list is greater than the
	capacity setting (which may be different than the {@link
	source_list_capacity() current capacity}) excess entries are silently
	removed from the end of the list.

	The specified list becomes the {@link source_list() current source
	list}.  If the new list is different than the contents of the source
	list widget, the widget is updated as is the Entries value.

	If the current source list has changed it is saved to the
	configuration settings and {@link source_list_changed(const
	QStringList&) is signaled}.

	The modifier buttons are always reset.

	@param	list	A QStringList reference that contains the new source
		list.
*/
void source_list (const QStringList& list);

bool has_changed () const;

/*==============================================================================
	Signals
*/
signals:

void source_list_capacity_changed (int capacity);
void source_list_changed (const QStringList& list);

/*==============================================================================
	Slots:
*/
public slots:

void reset ();
void defaults ();
void apply ();

private slots:

void source_list_capacity (int capacity);
void source_list_capacity_change ();
void source_list_capacity_reset ();
void source_list_reset ();
void source_list_selection_changed ();
void source_list_changed (QListWidgetItem* item = NULL);
void source_list_edit_item ();
void source_list_remove_items ();

//	Handles value changing from Capacity_spinBox.
void reset_modifier_buttons ();

/*==============================================================================
	Data
*/
private:

QString
	Title;

int
	Capacity;
QSpinBox
	*Capacity_spinBox;
Icon_Button
	*Capacity_Reset_Button;
QLabel
	*Entries;

QStringList
	*Source_List;
QListWidget
	*Source_List_Widget;
Icon_Button
	*Source_List_Reset_Button;
QPushButton
	*Edit_Button,
	*Remove_Button;

QPushButton
	*Defaults_Button,
	*Apply_Button;
};

/*=*****************************************************************************
	Rendering_Section
*/
class Rendering_Section
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
static const char
	*INITIAL_SCALE_KEY;
static const char
	*MAX_SCALE_KEY;
static const char
	*MIN_SCALE_KEY;
static const char
	*SCALING_MAJOR_INCREMENT_KEY;
static const char
	*SCALING_MINOR_INCREMENT_KEY;

static const char
	*CONTRAST_STRETCH_UPPER_KEY;
static const char
	*CONTRAST_STRETCH_LOWER_KEY;

static const char
	*TILE_SIZE_KEY;
static const char
	*RENDERING_INCREMENT_LINES_KEY;
static const char
	*BACKGROUND_COLOR_KEY;
	
static const char
	*LINE_COLOR_KEY;

/*==============================================================================
	Defaults
*/
static double
	Default_Initial_Scale;
static double
	Default_Min_Scale;
static double
	Default_Max_Scale;
static double
	Default_Scaling_Major_Increment;
static double
	Default_Scaling_Minor_Increment;

static double
	Default_Contrast_Stretch_Upper[3];
static double
	Default_Contrast_Stretch_Lower[3];

static int
	Default_Tile_Size;
static int
	Default_Rendering_Increment_Lines;
static QRgb
	Default_Background_Color;

/*==============================================================================
	Constructor
*/
Rendering_Section (QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
inline double initial_scale () const
	{return Initial_Scale;}

inline double min_scale () const
	{return Min_Scale;}

inline double max_scale () const
	{return Max_Scale;}

inline double scaling_minor_increment () const
	{return Scaling_Minor_Increment;}

inline double scaling_major_increment () const
	{return Scaling_Major_Increment;}


double contrast_stretch_upper (int band) const;
double contrast_stretch_lower (int band) const;


inline int tile_size () const
	{return Tile_Size;}

inline int rendering_increment_lines () const
	{return Rendering_Increment_Lines;}

inline QRgb background_color () const
	{return Background_Color;}

bool has_changed () const;

/*==============================================================================
	Signals
*/
signals:

void min_scale_changed (double scaling);
void max_scale_changed (double scaling);
void scaling_minor_increment_changed (double increment);
void scaling_major_increment_changed (double increment);

void contrast_stretch_upper_changed (double percent, int band);
void contrast_stretch_lower_changed (double percent, int band);

void background_color_changed (QRgb color);
void line_color_changed (const QColor & color);
void tile_size_changed (int size);
void rendering_increment_lines_changed (int lines);

/*==============================================================================
	Slots:
*/
public slots:

void reset ();
void defaults ();
void apply ();

private slots:

void initial_scale_change ();
void initial_scale_reset ();
void min_scale_change ();
void min_scale_reset ();
void max_scale_change ();
void max_scale_reset ();
void scaling_minor_increment_change ();
void scaling_minor_increment_reset ();
void scaling_major_increment_change ();
void scaling_major_increment_reset ();

void contrast_stretch_change ();
void contrast_stretch_reset ();

void background_color (const QString& text);
void background_color_changed ();
void select_background_color ();
void background_color_reset ();
void line_color (const QString & text);
void line_color_changed();
void select_line_color ();
void line_color_reset ();
void tile_size_change ();
void tile_size_reset ();
void rendering_increment_lines_change ();
void rendering_increment_lines_reset ();

void changing ();

/*==============================================================================
	Helpers
*/
private:

void initial_scale (double scaling);
void min_scale (double scaling);
void max_scale (double scaling);
void scaling_minor_increment (double increment);
void scaling_major_increment (double increment);

void contrast_stretch_upper (double percent, int band);
void contrast_stretch_lower (double percent, int band);

bool background_color_is_valid (const QString& color);
bool line_color_is_valid (const QString& color_spec);
static QString color_text (QRgb color);
static QRgb color_value (const QString& text);
void tile_size (int size);
void rendering_increment_lines (int lines);

void reset_modifier_buttons ();
void reset_defaults_button ();

/*==============================================================================
	Data
*/
private:

QString
	Title;

double
	Initial_Scale,
	Min_Scale,
	Max_Scale,
	Scaling_Minor_Increment,
	Scaling_Major_Increment,
	Contrast_Stretch_Upper[3],
	Contrast_Stretch_Lower[3];

QDoubleSpinBox
	*Initial_Scale_doubleSpinBox,
	*Min_Scale_doubleSpinBox,
	*Max_Scale_doubleSpinBox,
	*Scaling_Minor_Increment_doubleSpinBox,
	*Scaling_Major_Increment_doubleSpinBox,
	*Contrast_Stretch_Upper_doubleSpinBox[3],
	*Contrast_Stretch_Lower_doubleSpinBox[3];
Icon_Button
	*Initial_Scale_Reset_Button,
	*Min_Scale_Reset_Button,
	*Max_Scale_Reset_Button,
	*Scaling_Minor_Increment_Reset_Button,
	*Scaling_Major_Increment_Reset_Button,
	*Contrast_Stretch_Upper_Reset_Button[3],
	*Contrast_Stretch_Lower_Reset_Button[3];

int
	Tile_Size;
QSpinBox
	*Tile_Size_spinBox;
Icon_Button
	*Tile_Size_Reset_Button;

int
	Rendering_Increment_Lines;
QSpinBox
	*Rendering_Increment_Lines_spinBox;
Icon_Button
	*Rendering_Increment_Lines_Reset_Button;

QRgb
	Background_Color;
QString
	Background_Color_Text,
	Background_Color_Text_Pending,
	Default_Background_Color_Text;
QLineEdit
	*Background_Color_lineEdit;
Icon_Button
	*Background_Color_Reset_Button;
QPushButton
	*Background_Color_Select_Button;
	
QString
	Line_Color_Text,
	Line_Color_Text_Pending,
	Default_Line_Color_Text;
QLineEdit
	*Line_Color_lineEdit;
Icon_Button
	*Line_Color_Reset_Button;
QPushButton
	*Line_Color_Select_Button;

QPushButton
	*Defaults_Button,
	*Apply_Button;
};

/*=*****************************************************************************
	JPIP_Section
*/
class JPIP_Section
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//static const char
//	*HTTP_TO_JPIP_HOSTNAME_KEY;
static const char
	*JPIP_SERVER_PORT_KEY;
static const char
	*JPIP_PROXY_KEY;
static const char
	*JPIP_CACHE_DIRECTORY_KEY;
static const char
	*JPIP_REQUEST_TIMEOUT_KEY;
static const char
	*MAX_SOURCE_IMAGE_AREA_MB_KEY;

/*==============================================================================
	Defaults
*/
//static QString
//	Default_HTTP_to_JPIP_Hostname;
static int
	Default_JPIP_Server_Port;
static QString
	Default_JPIP_Proxy;
static QString
	Default_JPIP_Cache_Directory;
static int
	Default_JPIP_Request_Timeout;
static int
	Default_Max_Source_Image_Area_MB;

/*==============================================================================
	Constructor
*/
JPIP_Section (QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
//inline QString HTTP_to_JPIP_hostname () const
//	{return HTTP_to_JPIP_Hostname;}

inline int JPIP_server_port () const
	{return Port;}

inline QString JPIP_proxy () const
	{return Proxy;}

inline QString JPIP_cache_directory () const
	{return Cache_Directory;}

inline int JPIP_request_timeout () const
	{return Request_Timeout;}

inline int max_source_image_area_MB () const
	{return Max_Source_Image_Area_MB;}

bool has_changed () const;

/*==============================================================================
	Signals
*/
signals:

//void HTTP_to_JPIP_hostname_changed (const QString& hostname);
void JPIP_server_port_changed (int port);
void JPIP_proxy_changed (const QString& proxy);
void JPIP_cache_directory_changed (const QString& cache_directory);
void JPIP_request_timeout_changed (int seconds);
void max_source_image_area_MB_changed (int area);

/*==============================================================================
	Slots:
*/
public slots:

void reset ();
void defaults ();
void apply ();

private slots:

//void HTTP_to_JPIP_hostname (const QString& text);
//void HTTP_to_JPIP_hostname_changed ();
//void HTTP_to_JPIP_hostname_reset ();
void JPIP_server_port (int port);
void JPIP_server_port_reset ();
void JPIP_proxy (const QString& text);
void JPIP_proxy_changed ();
void JPIP_proxy_reset ();
void JPIP_cache_directory (const QString& text);
void JPIP_cache_directory_changed ();
void select_JPIP_cache_directory ();
void JPIP_cache_directory_reset ();
void JPIP_request_timeout (int seconds);
void JPIP_request_timeout_reset ();
void max_source_image_area_MB (int area);
void max_source_image_area_MB_reset ();

void changing ();

/*==============================================================================
	Helpers
*/
private:

//bool HTTP_to_JPIP_hostname_is_valid (const QString& hostname);
bool JPIP_proxy_is_valid (const QString& proxy);
bool JPIP_cache_directory_is_valid (const QString& cache_directory);

void reset_modifier_buttons ();
void reset_defaults_button ();

/*==============================================================================
	Data
*/
private:

QString
	Title;

//QString
//	HTTP_to_JPIP_Hostname,
//	HTTP_to_JPIP_Hostname_Pending;
//QLineEdit
//	*HTTP_to_JPIP_Hostname_lineEdit;
//Icon_Button
//	*HTTP_to_JPIP_Hostname_Reset_Button;

int
	Port;
QSpinBox
	*Port_spinBox;
Icon_Button
	*Port_Reset_Button;

QString
	Proxy,
	Proxy_Pending;
QLineEdit
	*Proxy_lineEdit;
Icon_Button
	*Proxy_Reset_Button;

QString
	Cache_Directory,
	Cache_Directory_Pending;
QLineEdit
	*Cache_Directory_lineEdit;
Icon_Button
	*Cache_Directory_Reset_Button;
QPushButton
	*Cache_Directory_Select_Button;

int
	Request_Timeout;
QSpinBox
	*Request_Timeout_spinBox;
Icon_Button
	*Request_Timeout_Reset_Button;

int
	Max_Source_Image_Area_MB;
QSpinBox
	*Max_Source_Image_Area_MB_spinBox;
Icon_Button
	*Max_Source_Image_Area_MB_Reset_Button;

QPushButton
	*Defaults_Button,
	*Apply_Button;

QFileDialog
	*File_Selection_Dialog;
};

/*=*****************************************************************************
	Graphs_Section
*/
class Graphs_Section
:	public QWidget
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
static const char
	*SELECTION_SENSITIVITY_KEY;
static const char
	*CANVAS_COLOR_KEY;

/*==============================================================================
	Defaults
*/
static int
	Default_Selection_Sensitivity;
static QRgb
	Default_Canvas_Color;

/*==============================================================================
	Constructor
*/
Graphs_Section (QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
inline int selection_sensitivity () const
	{return Selection_Sensitivity;}

inline QRgb canvas_color () const
	{return Canvas_Color;}

bool has_changed () const;

/*==============================================================================
	Signals
*/
signals:

void selection_sensitivity_changed (int sensitivity);
void canvas_color_changed (QRgb color);

/*==============================================================================
	Slots:
*/
public slots:

void reset ();
void defaults ();
void apply ();

private slots:

void selection_sensitivity (int size);
void selection_sensitivity_change ();
void selection_sensitivity_reset ();
void canvas_color (const QString& text);
void canvas_color_changed ();
void select_canvas_color ();
void canvas_color_reset ();

void changing ();

/*==============================================================================
	Helpers
*/
private:

bool canvas_color_is_valid (const QString& color);
static QString color_text (QRgb color);
static QRgb color_value (const QString& text);

void reset_modifier_buttons ();
void reset_defaults_button ();

/*==============================================================================
	Data
*/
private:

QString
	Title;

int
	Selection_Sensitivity;
QSpinBox
	*Selection_Sensitivity_spinBox;
Icon_Button
	*Selection_Sensitivity_Reset_Button;

QRgb
	Canvas_Color;
QString
	Canvas_Color_Text,
	Canvas_Color_Text_Pending,
	Default_Canvas_Color_Text;
QLineEdit
	*Canvas_Color_lineEdit;
Icon_Button
	*Canvas_Color_Reset_Button;
QPushButton
	*Canvas_Color_Select_Button;

QPushButton
	*Defaults_Button,
	*Apply_Button;
};

/*******************************************************************************
	Scripts Section
*/
class Scripts_Section
:	public QWidget
{
//	Qt Object Declaration
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
static const char* CURRENT_SCRIPT_KEY;
static const char* SHOW_SCRIPT_KEY;

/*==============================================================================
	Defaults
*/
//static bool Default_Show_Script;

/*==============================================================================
	Constructor
*/
Scripts_Section(QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/

/*==============================================================================
	Signals
*/
signals:
	void script_changed(const QString& Script);
	void show_script_changed(bool show_script);
/*==============================================================================
	Slots
*/
public slots:
	void apply();
	void defaults();
	void variables_updated(QStringList& pds_variables);
private slots:
	void show_script(bool enabled);
	void script_edited();
	void add_variable(QListWidgetItem *variable);

/*==============================================================================
	Helpers
*/
void reset_modifier_buttons();
bool reset_defaults_button();
bool has_changed();
/*==============================================================================
	Data
*/
private:
	QString Title;
	QString Script;
	QStringList Variables;
	
	QTextEdit* Script_TextEdit;
	QListWidget* Variables_ListWidget;
	QCheckBox* Show_Script_CheckBox;
	
	bool Show_Script;
	
	QPushButton *Defaults_Button;
	QPushButton *Apply_Button;
};

/*******************************************************************************
	Preferences_Dialog
*/
class Preferences_Dialog
:	public QDialog
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

static const char
	*LAYOUT_GEOMETRY_SECTION;

static const double
	INITIAL_SCALE_AUTO_FIT;

/*==============================================================================
	Statics
*/
static Help_Docs
	*Docs_Helper;

/*==============================================================================
	Constructor
*/
Preferences_Dialog (QWidget* parent = NULL);

~Preferences_Dialog ();

/*==============================================================================
	Accessors
*/
inline void show_tooltips (bool enabled)
	{Show_Tooltips = enabled;}

inline bool show_tooltips () const
	{return Show_Tooltips;}

inline static Help_Docs* help_docs ()
	{return Docs_Helper;}

bool has_changed () const;

void apply ();

void reset ();

//	General:

inline bool restore_layout () const
	{return General->restore_layout ();}

inline bool restore_last_source () const
	{return General->restore_last_source ();}

inline bool band_numbers_indexed () const
	{return General->band_numbers_indexed ();}

inline bool get_PDS_metadata () const
	{return General->get_PDS_metadata ();}

inline QString documentation_location () const
	{return General->documentation_location ();}

inline const QString& default_documentation_filename ()
	{return General->default_documentation_filename ();}

//	Sources:
inline int source_list_capacity () const
	{return Sources->source_list_capacity ();}

inline const QStringList& source_list () const
	{return Sources->source_list ();}

inline void source_list (const QStringList& list)
	{Sources->source_list (list);}

//	Rendering:

inline double initial_scale () const
	{return Rendering->initial_scale ();}

inline double min_scale () const
	{return Rendering->min_scale ();}

inline double max_scale () const
	{return Rendering->max_scale ();}

inline double scaling_minor_increment () const
	{return Rendering->scaling_minor_increment ();}

inline double scaling_major_increment () const
	{return Rendering->scaling_major_increment ();}

inline double contrast_stretch_upper (int band) const
	{return Rendering->contrast_stretch_upper (band);}

inline double contrast_stretch_lower (int band) const
	{return Rendering->contrast_stretch_lower (band);}

inline QRgb background_color () const
	{return Rendering->background_color ();}

inline int tile_size () const
	{return Rendering->tile_size ();}

inline int rendering_increment_lines () const
	{return Rendering->rendering_increment_lines ();}

//	JPIP:

//inline QString HTTP_to_JPIP_hostname () const
//{return JPIP->HTTP_to_JPIP_hostname ();}

inline int JPIP_server_port () const
	{return JPIP->JPIP_server_port ();}

inline QString JPIP_proxy () const
	{return JPIP->JPIP_proxy ();}

inline QString JPIP_cache_directory () const
	{return JPIP->JPIP_cache_directory ();}

inline int JPIP_request_timeout () const
	{return JPIP->JPIP_request_timeout ();}

inline int max_source_image_area_MB () const
	{return JPIP->max_source_image_area_MB ();}

//	Graphs

inline int selection_sensitivity () const
	{return Graphs->selection_sensitivity ();}

inline QRgb canvas_color () const
	{return Graphs->canvas_color ();}

/*==============================================================================
	Utilities
*/
static void save (const QString& key, const QVariant& value);

static QString color_text (QRgb color);
static QRgb color_value (const QString& text);

/*==============================================================================
	Signals
*/
signals:

void band_numbers_indexed_changed (bool indexed);
void documentation_location_changed (const QString& location);
void longitude_direction_changed (int direction);
void longitude_units_changed (int units);
void latitude_units_changed (int units);

void source_list_capacity_changed (int capacity);
void source_list_changed (const QStringList& list);

void min_scale_changed (double scaling);
void max_scale_changed (double scaling);
void scaling_minor_increment_changed (double increment);
void scaling_major_increment_changed (double increment);
void contrast_stretch_upper_changed (double percent, int band);
void contrast_stretch_lower_changed (double percent, int band);
void background_color_changed (QRgb color);
void line_color_changed (const QColor & color);
void tile_size_changed (int size);
void rendering_increment_lines_changed (int lines);

//void HTTP_to_JPIP_hostname_changed (const QString& proxy);
void JPIP_server_port_changed (int port);
void JPIP_proxy_changed (const QString& proxy);
void JPIP_cache_directory_changed (const QString& cache_directory);
void JPIP_request_timeout_changed (int seconds);
void max_source_image_area_MB_changed (int area);

void selection_sensitivity_changed (int sensitivity);
void canvas_color_changed (QRgb color);

void script_changed(const QString& Script);
void show_script_changed(bool show_script);
void variables_updated(QStringList& variables);

/*==============================================================================
	Event Handlers
*/
protected:

bool eventFilter (QObject* object, QEvent* event);

virtual void closeEvent (QCloseEvent* event);

/*==============================================================================
	Data
*/
private:

General_Section
	*General;
Sources_Section
	*Sources;
Rendering_Section
	*Rendering;
JPIP_Section
	*JPIP;
Graphs_Section
	*Graphs;
Scripts_Section
	*Scripts;

bool
	Show_Tooltips;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
