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

#pragma once

#include <QComboBox>
#include <QDialog>
#include <QString>
#include <QTextEdit>
#include <QWidget>

class QTabWidget;
class QCheckBox;
class QRadioButton;
class QLabel;
// class QStringList;
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

namespace UA::HiRISE
{
//	Forward references.
class Icon_Button;
class Help_Docs;

/*=*****************************************************************************
    General_Section
*/
class General_Section : public QWidget
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    static const char* RESTORE_LAYOUT_KEY;
    //!	Obsolete.
    static const char* RESTORE_WINDOW_POSITIONS_KEY;

    static const char* RESTORE_LAST_SOURCE_KEY;

    static const char* BAND_NUMBERS_INDEXED_KEY;

    static const char* GET_PDS_LABEL_KEY;

    static const char* DOCUMENTATION_LOCATION_KEY;

    static const char* RESTORE_LONGITUDE_FORMAT_KEY;

    static const char* RESTORE_LONGITUDE_DIRECTION_KEY;

    static const char* RESTORE_LATITUDE_FORMAT_KEY;

    /*==============================================================================
        Defaults
    */
    static bool Default_Restore_Layout;

    static bool Default_Restore_Last_Source;

    static bool Default_Band_Numbers_Indexed;

    static bool Default_Get_PDS_Metadata;

    static int Default_Coordinate_Format;

    static const char* Default_Documentation_Search_Locations[];
    static QStringList Documentation_Search_Locations;
    static const QString Default_Documentation_Filename;

    /*==============================================================================
        Constructor
    */
    General_Section(QWidget* parent = NULL);

    /*==============================================================================
        Accessors
    */
    bool restore_layout() const { return Restore_Layout; }

    bool restore_last_source() const { return Restore_Last_Source; }

    bool band_numbers_indexed() const { return Band_Numbers_Indexed; }

    bool get_PDS_metadata() const { return Get_PDS_Metadata; }

    QString documentation_location() const { return Documentation_Location; }

    static const QString& default_documentation_filename()
    { return Default_Documentation_Filename; }

    bool has_changed() const;

    /*==============================================================================
        Signals
    */

    Q_SIGNAL void band_numbers_indexed_changed(bool indexed);

    Q_SIGNAL void documentation_location_changed(const QString& location);

    Q_SIGNAL void longitude_direction_changed(int direction);

    Q_SIGNAL void longitude_units_changed(int units);

    Q_SIGNAL void latitude_units_changed(int units);

    /*==============================================================================
        Slots:
    */

    Q_SLOT void reset();
    Q_SLOT void defaults();
    Q_SLOT void apply();

 private:
    Q_SLOT void restore_layout(bool enabled);
    Q_SLOT void restore_last_source(bool enabled);

    Q_SLOT void band_numbers_indexed(bool indexed);

    Q_SLOT void longitude_format(int format);
    Q_SLOT void longitude_direction(int direction);
    Q_SLOT void latitude_format(int format);

    Q_SLOT void get_PDS_metadata(bool enabled);

    Q_SLOT void documentation_location(const QString& location);
    Q_SLOT void documentation_location_changed();
    Q_SLOT bool find_documentation_location();
    Q_SLOT void documentation_location_reset();

    /*==============================================================================
        Helpers
    */
    bool documentation_location_is_valid(const QString& location);

    void reset_modifier_buttons();
    void reset_defaults_button();

    /*==============================================================================
        Data
    */
    QString Title;

    bool Restore_Layout;
    QCheckBox* Restore_Layout_CheckBox;

    bool Restore_Last_Source;
    QCheckBox* Restore_Last_Source_CheckBox;

    bool Band_Numbers_Indexed;
    QRadioButton* Band_Numbers_Indexed_Button;

    bool Get_PDS_Metadata;
    QCheckBox* Get_PDS_Metadata_CheckBox;

    QString Documentation_Location, Documentation_Location_Pending;
    QLineEdit* Documentation_Location_lineEdit;
    Icon_Button* Documentation_Location_Reset_Button;

    QPushButton *Defaults_Button, *Apply_Button;

    QComboBox *Longitude_Units_ComboBox, *Longitude_Direction_ComboBox, *Latitude_Units_ComboBox;

    int Longitude_Units, Longitude_Direction, Latitude_Units;
};

/*=*****************************************************************************
    Sources_Section
*/
class Sources_Section : public QWidget
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    static const char* SOURCE_LIST_CAPACITY_KEY;
    static const char* SOURCE_LIST_KEY;

    /*==============================================================================
        Defaults
    */
    static int Default_Source_List_Capacity;
    static int Default_Source_List_Capacity_Max;

    /*==============================================================================
        Constructor
    */
    Sources_Section(QWidget* parent = NULL);

    /*==============================================================================
        Accessors
    */
    int source_list_capacity() const { return Capacity; }

    const QStringList& source_list() const { return *Source_List; }

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
    void source_list(const QStringList& list);

    bool has_changed() const;

    /*==============================================================================
        Signals
    */
    Q_SIGNAL void source_list_capacity_changed(int capacity);
    Q_SIGNAL void source_list_changed(const QStringList& list);

    /*==============================================================================
        Slots:
    */
    Q_SLOT void reset();
    Q_SLOT void defaults();
    Q_SLOT void apply();

 private:
    Q_SLOT void source_list_capacity(int capacity);
    Q_SLOT void source_list_capacity_change();
    Q_SLOT void source_list_capacity_reset();
    Q_SLOT void source_list_reset();
    Q_SLOT void source_list_selection_changed();
    Q_SLOT void source_list_changed(QListWidgetItem* item = NULL);
    Q_SLOT void source_list_edit_item();
    Q_SLOT void source_list_remove_items();

    //	Handles value changing from Capacity_spinBox.
    Q_SLOT void reset_modifier_buttons();

    /*==============================================================================
        Data
    */
    QString Title;

    int Capacity;
    QSpinBox* Capacity_spinBox;
    Icon_Button* Capacity_Reset_Button;
    QLabel* Entries;

    QStringList* Source_List;
    QListWidget* Source_List_Widget;
    Icon_Button* Source_List_Reset_Button;
    QPushButton *Edit_Button, *Remove_Button;

    QPushButton *Defaults_Button, *Apply_Button;
};

/*=*****************************************************************************
    Rendering_Section
*/
class Rendering_Section : public QWidget
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    static const char* INITIAL_SCALE_KEY;
    static const char* MAX_SCALE_KEY;
    static const char* MIN_SCALE_KEY;
    static const char* SCALING_MAJOR_INCREMENT_KEY;
    static const char* SCALING_MINOR_INCREMENT_KEY;

    static const char* CONTRAST_STRETCH_UPPER_KEY;
    static const char* CONTRAST_STRETCH_LOWER_KEY;

    static const char* TILE_SIZE_KEY;
    static const char* RENDERING_INCREMENT_LINES_KEY;
    static const char* BACKGROUND_COLOR_KEY;

    static const char* LINE_COLOR_KEY;

    /*==============================================================================
        Defaults
    */
    static double Default_Initial_Scale;
    static double Default_Min_Scale;
    static double Default_Max_Scale;
    static double Default_Scaling_Major_Increment;
    static double Default_Scaling_Minor_Increment;

    static double Default_Contrast_Stretch_Upper[3];
    static double Default_Contrast_Stretch_Lower[3];

    static int Default_Tile_Size;
    static int Default_Rendering_Increment_Lines;
    static QRgb Default_Background_Color;

    /*==============================================================================
        Constructor
    */
    Rendering_Section(QWidget* parent = nullptr);

    /*==============================================================================
        Accessors
    */
    double initial_scale() const { return Initial_Scale; }

    double min_scale() const { return Min_Scale; }

    double max_scale() const { return Max_Scale; }

    double scaling_minor_increment() const { return Scaling_Minor_Increment; }

    double scaling_major_increment() const { return Scaling_Major_Increment; }

    double contrast_stretch_upper(int band) const;
    double contrast_stretch_lower(int band) const;

    int tile_size() const { return Tile_Size; }

    int rendering_increment_lines() const { return Rendering_Increment_Lines; }

    QRgb background_color() const { return Background_Color; }

    bool has_changed() const;

    /*==============================================================================
        Signals
    */
    Q_SIGNAL void min_scale_changed(double scaling);
    Q_SIGNAL void max_scale_changed(double scaling);
    Q_SIGNAL void scaling_minor_increment_changed(double increment);
    Q_SIGNAL void scaling_major_increment_changed(double increment);

    Q_SIGNAL void contrast_stretch_upper_changed(double percent, int band);
    Q_SIGNAL void contrast_stretch_lower_changed(double percent, int band);

    Q_SIGNAL void background_color_changed(QRgb color);
    Q_SIGNAL void line_color_changed(const QColor& color);
    Q_SIGNAL void tile_size_changed(int size);
    Q_SIGNAL void rendering_increment_lines_changed(int lines);

    /*==============================================================================
        Slots:
    */

    Q_SLOT void reset();
    Q_SLOT void defaults();
    Q_SLOT void apply();

 private:
    Q_SLOT void initial_scale_change();
    Q_SLOT void initial_scale_reset();
    Q_SLOT void min_scale_change();
    Q_SLOT void min_scale_reset();
    Q_SLOT void max_scale_change();
    Q_SLOT void max_scale_reset();
    Q_SLOT void scaling_minor_increment_change();
    Q_SLOT void scaling_minor_increment_reset();
    Q_SLOT void scaling_major_increment_change();
    Q_SLOT void scaling_major_increment_reset();

    Q_SLOT void contrast_stretch_change();
    Q_SLOT void contrast_stretch_reset();

    Q_SLOT void background_color(const QString& text);
    Q_SLOT void background_color_changed();
    Q_SLOT void select_background_color();
    Q_SLOT void background_color_reset();
    Q_SLOT void line_color(const QString& text);
    Q_SLOT void line_color_changed();
    Q_SLOT void select_line_color();
    Q_SLOT void line_color_reset();
    Q_SLOT void tile_size_change();
    Q_SLOT void tile_size_reset();
    Q_SLOT void rendering_increment_lines_change();
    Q_SLOT void rendering_increment_lines_reset();

    Q_SLOT void changing();

    /*==============================================================================
        Helpers
    */

    void initial_scale(double scaling);
    void min_scale(double scaling);
    void max_scale(double scaling);
    void scaling_minor_increment(double increment);
    void scaling_major_increment(double increment);

    void contrast_stretch_upper(double percent, int band);
    void contrast_stretch_lower(double percent, int band);

    bool background_color_is_valid(const QString& color);
    bool line_color_is_valid(const QString& color_spec);
    static QString color_text(QRgb color);
    static QRgb color_value(const QString& text);
    void tile_size(int size);
    void rendering_increment_lines(int lines);

    void reset_modifier_buttons();
    void reset_defaults_button();

    /*==============================================================================
        Data
    */
    QString Title;

    double Initial_Scale, Min_Scale, Max_Scale, Scaling_Minor_Increment, Scaling_Major_Increment,
        Contrast_Stretch_Upper[3], Contrast_Stretch_Lower[3];

    QDoubleSpinBox *Initial_Scale_doubleSpinBox, *Min_Scale_doubleSpinBox, *Max_Scale_doubleSpinBox,
        *Scaling_Minor_Increment_doubleSpinBox, *Scaling_Major_Increment_doubleSpinBox,
        *Contrast_Stretch_Upper_doubleSpinBox[3], *Contrast_Stretch_Lower_doubleSpinBox[3];
    Icon_Button *Initial_Scale_Reset_Button, *Min_Scale_Reset_Button, *Max_Scale_Reset_Button,
        *Scaling_Minor_Increment_Reset_Button, *Scaling_Major_Increment_Reset_Button,
        *Contrast_Stretch_Upper_Reset_Button[3], *Contrast_Stretch_Lower_Reset_Button[3];

    int Tile_Size;
    QSpinBox* Tile_Size_spinBox;
    Icon_Button* Tile_Size_Reset_Button;

    int Rendering_Increment_Lines;
    QSpinBox* Rendering_Increment_Lines_spinBox;
    Icon_Button* Rendering_Increment_Lines_Reset_Button;

    QRgb Background_Color;
    QString Background_Color_Text, Background_Color_Text_Pending, Default_Background_Color_Text;
    QLineEdit* Background_Color_lineEdit;
    Icon_Button* Background_Color_Reset_Button;
    QPushButton* Background_Color_Select_Button;

    QString Line_Color_Text, Line_Color_Text_Pending, Default_Line_Color_Text;
    QLineEdit* Line_Color_lineEdit;
    Icon_Button* Line_Color_Reset_Button;
    QPushButton* Line_Color_Select_Button;

    QPushButton *Defaults_Button, *Apply_Button;

    bool m_updatingScale;
};

/*=*****************************************************************************
    JPIP_Section
*/
class JPIP_Section : public QWidget
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    static const char* HTTP_TO_JPIP_HOSTNAME_KEY;
    static const char* JPIP_TO_HTTP_HOSTNAME_KEY;
    static const char* JPIP_SERVER_PORT_KEY;
    static const char* JPIP_PROXY_KEY;
    static const char* JPIP_CACHE_DIRECTORY_KEY;
    static const char* JPIP_REQUEST_TIMEOUT_KEY;
    static const char* MAX_SOURCE_IMAGE_AREA_MB_KEY;

    /*==============================================================================
        Defaults
    */
    static QString Default_HTTP_to_JPIP_Hostname;
    static QString Default_JPIP_to_HTTP_Hostname;
    static int Default_JPIP_Server_Port;
    static QString Default_JPIP_Proxy;
    static QString Default_JPIP_Cache_Directory;
    static int Default_JPIP_Request_Timeout;
    static int Default_Max_Source_Image_Area_MB;

    /*==============================================================================
        Constructor
    */
    JPIP_Section(QWidget* parent = NULL);

    /*==============================================================================
        Accessors
    */
    QString HTTP_to_JPIP_hostname() const { return HTTP_to_JPIP_Hostname; }

    QString JPIP_to_HTTP_hostname() const { return JPIP_to_HTTP_Hostname; }

    int JPIP_server_port() const { return Port; }

    QString JPIP_proxy() const { return Proxy; }

    QString JPIP_cache_directory() const { return Cache_Directory; }

    int JPIP_request_timeout() const { return Request_Timeout; }

    int max_source_image_area_MB() const { return Max_Source_Image_Area_MB; }

    bool has_changed() const;

    /*==============================================================================
        Signals
    */

    Q_SIGNAL void HTTP_to_JPIP_hostname_changed(const QString& hostname);
    Q_SIGNAL void JPIP_to_HTTP_hostname_changed(const QString& hostname);
    Q_SIGNAL void JPIP_server_port_changed(int port);
    Q_SIGNAL void JPIP_proxy_changed(const QString& proxy);
    Q_SIGNAL void JPIP_cache_directory_changed(const QString& cache_directory);
    Q_SIGNAL void JPIP_request_timeout_changed(int seconds);
    Q_SIGNAL void max_source_image_area_MB_changed(int area);

    /*==============================================================================
        Slots:
    */
    Q_SLOT void reset();
    Q_SLOT void defaults();
    Q_SLOT void apply();

 private:
    Q_SLOT void HTTP_to_JPIP_hostname(const QString& text);
    Q_SLOT void HTTP_to_JPIP_hostname_changed();
    Q_SLOT void HTTP_to_JPIP_hostname_reset();
    Q_SLOT void JPIP_to_HTTP_hostname(const QString& text);
    Q_SLOT void JPIP_to_HTTP_hostname_changed();
    Q_SLOT void JPIP_to_HTTP_hostname_reset();
    Q_SLOT void JPIP_server_port(int port);
    Q_SLOT void JPIP_server_port_reset();
    Q_SLOT void JPIP_proxy(const QString& text);
    Q_SLOT void JPIP_proxy_changed();
    Q_SLOT void JPIP_proxy_reset();
    Q_SLOT void JPIP_cache_directory(const QString& text);
    Q_SLOT void JPIP_cache_directory_changed();
    Q_SLOT void select_JPIP_cache_directory();
    Q_SLOT void JPIP_cache_directory_reset();
    Q_SLOT void JPIP_request_timeout(int seconds);
    Q_SLOT void JPIP_request_timeout_reset();
    Q_SLOT void max_source_image_area_MB(int area);
    Q_SLOT void max_source_image_area_MB_reset();

    Q_SLOT void changing();

    /*==============================================================================
        Helpers
    */
    bool JPIP_to_HTTP_hostname_verify(const QString& hostname);
    bool HTTP_to_JPIP_hostname_verify(const QString& hostname);
    bool JPIP_proxy_is_valid(const QString& proxy);
    bool JPIP_cache_directory_is_valid(const QString& cache_directory);

    void reset_modifier_buttons();
    void reset_defaults_button();

    /*==============================================================================
        Data
    */

    QString Title;

    QString HTTP_to_JPIP_Hostname, HTTP_to_JPIP_Hostname_Pending;
    QLineEdit* HTTP_to_JPIP_Hostname_lineEdit;
    Icon_Button* HTTP_to_JPIP_Hostname_Reset_Button;

    QString JPIP_to_HTTP_Hostname, JPIP_to_HTTP_Hostname_Pending;
    QLineEdit* JPIP_to_HTTP_Hostname_lineEdit;
    Icon_Button* JPIP_to_HTTP_Hostname_Reset_Button;

    int Port;
    QSpinBox* Port_spinBox;
    Icon_Button* Port_Reset_Button;

    QString Proxy, Proxy_Pending;
    QLineEdit* Proxy_lineEdit;
    Icon_Button* Proxy_Reset_Button;

    QString Cache_Directory, Cache_Directory_Pending;
    QLineEdit* Cache_Directory_lineEdit;
    Icon_Button* Cache_Directory_Reset_Button;
    QPushButton* Cache_Directory_Select_Button;

    int Request_Timeout;
    QSpinBox* Request_Timeout_spinBox;
    Icon_Button* Request_Timeout_Reset_Button;

    int Max_Source_Image_Area_MB;
    QSpinBox* Max_Source_Image_Area_MB_spinBox;
    Icon_Button* Max_Source_Image_Area_MB_Reset_Button;

    QPushButton *Defaults_Button, *Apply_Button;

    QFileDialog* File_Selection_Dialog;
};

/*=*****************************************************************************
    Graphs_Section
*/
class Graphs_Section : public QWidget
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    static const char* SELECTION_SENSITIVITY_KEY;
    static const char* CANVAS_COLOR_KEY;

    /*==============================================================================
        Defaults
    */
    static int Default_Selection_Sensitivity;
    static QRgb Default_Canvas_Color;

    /*==============================================================================
        Constructor
    */
    Graphs_Section(QWidget* parent = nullptr);

    /*==============================================================================
        Accessors
    */
    int selection_sensitivity() const { return Selection_Sensitivity; }

    QRgb canvas_color() const { return Canvas_Color; }

    bool has_changed() const;

    /*==============================================================================
        Signals
    */

    Q_SIGNAL void selection_sensitivity_changed(int sensitivity);
    Q_SIGNAL void canvas_color_changed(QRgb color);

    /*==============================================================================
        Slots:
    */

    Q_SLOT void reset();
    Q_SLOT void defaults();
    Q_SLOT void apply();

 private:
    Q_SLOT void selection_sensitivity(int size);
    Q_SLOT void selection_sensitivity_change();
    Q_SLOT void selection_sensitivity_reset();
    Q_SLOT void canvas_color(const QString& text);
    Q_SLOT void canvas_color_changed();
    Q_SLOT void select_canvas_color();
    Q_SLOT void canvas_color_reset();

    Q_SLOT void changing();

    /*==============================================================================
        Helpers
    */
    bool canvas_color_is_valid(const QString& color);
    static QString color_text(QRgb color);
    static QRgb color_value(const QString& text);

    void reset_modifier_buttons();
    void reset_defaults_button();

    /*==============================================================================
        Data
    */
    QString Title;

    int Selection_Sensitivity;
    QSpinBox* Selection_Sensitivity_spinBox;
    Icon_Button* Selection_Sensitivity_Reset_Button;

    QRgb Canvas_Color;
    QString Canvas_Color_Text, Canvas_Color_Text_Pending, Default_Canvas_Color_Text;
    QLineEdit* Canvas_Color_lineEdit;
    Icon_Button* Canvas_Color_Reset_Button;
    QPushButton* Canvas_Color_Select_Button;

    QPushButton *Defaults_Button, *Apply_Button;
};

/*******************************************************************************
    Scripts Section
*/
class Scripts_Section : public QWidget
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
    // static bool Default_Show_Script;

    /*==============================================================================
        Constructor
    */
    Scripts_Section(QWidget* parent = nullptr);

    /*==============================================================================
        Accessors
    */

    /*==============================================================================
        Signals
    */
    Q_SIGNAL void script_changed(const QString& Script);
    Q_SIGNAL void show_script_changed(bool show_script);
    /*==============================================================================
        Slots
    */
    Q_SLOT void apply();
    Q_SLOT void defaults();
    Q_SLOT void variables_updated(QStringList& pds_variables);

 private:
    Q_SLOT void show_script(bool enabled);
    Q_SLOT void script_edited();
    Q_SLOT void add_variable(QListWidgetItem* variable);

    /*==============================================================================
        Helpers
    */
    void reset_modifier_buttons();
    bool reset_defaults_button();
    bool has_changed();
    /*==============================================================================
        Data
    */
    QString Title;
    QString Script;
    QStringList Variables;

    QTextEdit* Script_TextEdit;
    QListWidget* Variables_ListWidget;
    QCheckBox* Show_Script_CheckBox;

    bool Show_Script;

    QPushButton* Defaults_Button;
    QPushButton* Apply_Button;
};

/*******************************************************************************
    Preferences_Dialog
*/
class Preferences_Dialog : public QDialog
{
    //	Qt Object declaration.
    Q_OBJECT

 public:
    /*==============================================================================
        Constants
    */
    //!	Class identification name with source code version and date.
    static const char* const ID;

    static const char* LAYOUT_GEOMETRY_SECTION;

    static const double INITIAL_SCALE_AUTO_FIT;

    /*==============================================================================
        Statics
    */
    static Help_Docs* Docs_Helper;

    /*==============================================================================
        Constructor
    */
    Preferences_Dialog(QWidget* parent = nullptr);

    ~Preferences_Dialog();

    /*==============================================================================
        Accessors
    */
    void show_tooltips(bool enabled) { Show_Tooltips = enabled; }

    bool show_tooltips() const { return Show_Tooltips; }

    static Help_Docs* help_docs() { return Docs_Helper; }

    bool has_changed() const;

    void apply();

    void reset();

    //	General:

    bool restore_layout() const { return General->restore_layout(); }

    bool restore_last_source() const { return General->restore_last_source(); }

    bool band_numbers_indexed() const { return General->band_numbers_indexed(); }

    bool get_PDS_metadata() const { return General->get_PDS_metadata(); }

    QString documentation_location() const { return General->documentation_location(); }

    static const QString& default_documentation_filename()
    { return UA::HiRISE::General_Section::default_documentation_filename(); }

    //	Sources:
    int source_list_capacity() const { return Sources->source_list_capacity(); }

    const QStringList& source_list() const { return Sources->source_list(); }

    void source_list(const QStringList& list) { Sources->source_list(list); }

    //	Rendering:

    double initial_scale() const { return Rendering->initial_scale(); }

    double min_scale() const { return Rendering->min_scale(); }

    double max_scale() const { return Rendering->max_scale(); }

    double scaling_minor_increment() const { return Rendering->scaling_minor_increment(); }

    double scaling_major_increment() const { return Rendering->scaling_major_increment(); }

    double contrast_stretch_upper(int band) const
    { return Rendering->contrast_stretch_upper(band); }

    double contrast_stretch_lower(int band) const
    { return Rendering->contrast_stretch_lower(band); }

    QRgb background_color() const { return Rendering->background_color(); }

    int tile_size() const { return Rendering->tile_size(); }

    int rendering_increment_lines() const { return Rendering->rendering_increment_lines(); }

    //	JPIP:

    QString HTTP_to_JPIP_hostname() const { return JPIP->HTTP_to_JPIP_hostname(); }

    int JPIP_server_port() const { return JPIP->JPIP_server_port(); }

    QString JPIP_proxy() const { return JPIP->JPIP_proxy(); }

    QString JPIP_cache_directory() const { return JPIP->JPIP_cache_directory(); }

    int JPIP_request_timeout() const { return JPIP->JPIP_request_timeout(); }

    int max_source_image_area_MB() const { return JPIP->max_source_image_area_MB(); }

    //	Graphs

    int selection_sensitivity() const { return Graphs->selection_sensitivity(); }

    QRgb canvas_color() const { return Graphs->canvas_color(); }

    /*==============================================================================
        Utilities
    */
    static void save(const QString& key, const QVariant& value);

    static QString color_text(QRgb color);
    static QRgb color_value(const QString& text);

    /*==============================================================================
        Signals
    */

    Q_SIGNAL void band_numbers_indexed_changed(bool indexed);
    Q_SIGNAL void documentation_location_changed(const QString& location);
    Q_SIGNAL void longitude_direction_changed(int direction);
    Q_SIGNAL void longitude_units_changed(int units);
    Q_SIGNAL void latitude_units_changed(int units);

    Q_SIGNAL void source_list_capacity_changed(int capacity);
    Q_SIGNAL void source_list_changed(const QStringList& list);

    Q_SIGNAL void min_scale_changed(double scaling);
    Q_SIGNAL void max_scale_changed(double scaling);
    Q_SIGNAL void scaling_minor_increment_changed(double increment);
    Q_SIGNAL void scaling_major_increment_changed(double increment);
    Q_SIGNAL void contrast_stretch_upper_changed(double percent, int band);
    Q_SIGNAL void contrast_stretch_lower_changed(double percent, int band);
    Q_SIGNAL void background_color_changed(QRgb color);
    Q_SIGNAL void line_color_changed(const QColor& color);
    Q_SIGNAL void tile_size_changed(int size);
    Q_SIGNAL void rendering_increment_lines_changed(int lines);

    Q_SIGNAL void HTTP_to_JPIP_hostname_changed(const QString& proxy);
    Q_SIGNAL void JPIP_to_HTTP_hostname_changed(const QString& proxy);
    Q_SIGNAL void JPIP_server_port_changed(int port);
    Q_SIGNAL void JPIP_proxy_changed(const QString& proxy);
    Q_SIGNAL void JPIP_cache_directory_changed(const QString& cache_directory);
    Q_SIGNAL void JPIP_request_timeout_changed(int seconds);
    Q_SIGNAL void max_source_image_area_MB_changed(int area);

    Q_SIGNAL void selection_sensitivity_changed(int sensitivity);
    Q_SIGNAL void canvas_color_changed(QRgb color);

    Q_SIGNAL void script_changed(const QString& Script);
    Q_SIGNAL void show_script_changed(bool show_script);
    Q_SIGNAL void variables_updated(QStringList& variables);

    /*==============================================================================
        Event Handlers
    */
 protected:
    bool eventFilter(QObject* object, QEvent* event) override;

    void closeEvent(QCloseEvent* event) override;

    /*==============================================================================
        Data
    */
 private:
    General_Section* General;
    Sources_Section* Sources;
    Rendering_Section* Rendering;
    JPIP_Section* JPIP;
    Graphs_Section* Graphs;
    Scripts_Section* Scripts;

    bool Show_Tooltips;
};

}  // namespace UA::HiRISE
