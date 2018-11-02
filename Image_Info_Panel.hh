/*	Image_Info_Panel

HiROC CVS ID: $Id: Image_Info_Panel.hh,v 1.14 2014/08/05 17:58:09 stephens Exp $

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

#ifndef HiView_Image_Info_Panel_hh
#define HiView_Image_Info_Panel_hh

#include	"Plastic_Image.hh"
#include	"PVL.hh"
#include    "Stats.hh"
#include	<QFrame>
#include	<QScriptEngine>
#include	<QStringList>

//	Forward references.
class QWidget;
class QLabel;
class QPoint;
class QSizeF;
class QComboBox;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Coordinate;
class Projection;

/**	The <i>Image_Info_Panel</i> provides brief image information -
	location, value and scale - on a dynamically updated single line
	display.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.14 $
*/
class Image_Info_Panel
:	public QFrame
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


/**	Units for longitude and latitude values.

	@see	longitude_units(Units)
	@see	latitude_units(Units)
*/
enum Units
	{
	DEGREES,
	HMS,
	RADIANS
	};

/**	Direction for longitude value.

	@see	longitude_direction(Direction)
*/
enum Direction
	{
	EAST,
	WEST
	};

/*==============================================================================
	Constructors
*/
explicit Image_Info_Panel (QWidget* parent = NULL);

virtual ~Image_Info_Panel ();

/*==============================================================================
	Manipulators
*/
void image_bands (int);
void use_avg_pixel_value(bool use);
void set_property(const char * name, unsigned long long data);
void set_property(const char * name, unsigned int data);
void set_property_qsreal(const char * name, qsreal data);
void set_property_f(const char * name, double data);
void add_exception(int exception);
void clear_exceptions();
void evaluate_script();
void update_region_stats();
void update_statistics(Stats *stats);
QList<bool> *parse_variable_names(QList<QString> *list);
void check_names();

/*==============================================================================
	Signals
*/
signals:
	void variables_updated(QStringList &variables);

/*==============================================================================
	Qt slots
*/
public slots:

void image_values (bool enabled);

void cursor_location
	(const QPoint& display_location, const QPoint& image_location);

void pixel_value
	(const Plastic_Image::Triplet& display_value,
	 const Plastic_Image::Triplet& image_value);

void image_scale (const QSizeF& scaling, int band = -1);


void projection (Projection* projection);

void set_metadata(idaeim::PVL::Aggregate *metadata);

void longitude_units (int units);
void latitude_units (int units);

void longitude_direction (int direction);

void location (const Coordinate& coordinate);

void script_changed(const QString& script);

void show_script_changed(bool show_script);

/*==============================================================================
	Helpers
*/
private:

QWidget* image_data_panel ();

QWidget* world_location_panel ();

QWidget* create_script_engine();

void longitude (double value);
void latitude (double value);
QString location_representation (double value, int units) const;
void array_to_string(idaeim::PVL::Array &array, QScriptValue &engine_array);
void get_properties(idaeim::PVL::Aggregate &metadata);

void preparse_script(QString &script);

void unset_properties();

void initialize_script_values();

/*==============================================================================
	Data
*/
private:

bool
	Image_Values,
	Use_Avg_Rgb;
int
	Image_Bands;
QPoint
	Display_Location,
	Image_Location;
Plastic_Image::Triplet
	Display_Value,
	Image_Value;
QSizeF
	Image_Scale;

QString
	Display_Data,
	Image_Data;
QLabel
	*Data_Source,
	*Location_X,
	*Annotation_X,
	*Location_Y,
	*Annotation_Y,
	*Pixel_Value[3],
	*Pixel_Label[3],
	*Scaling,
	*Script_Output,
	*Script_Value;

QSize
	Data_Source_Size;
int
	Latitude_Units,
	Longitude_Units,
	Longitude_Direction;
double
	Longitude_Location,
	Latitude_Location;
Projection
	*Projector;
QWidget
	*World_Location;
	
QWidget
	*Script_Panel;
QLabel
	*Projection_Name,
	*Longitude,
	*Latitude;
	
QScriptEngine *Engine;

QStringList Properties_List;

QScriptValue Global_Object;

bool
	Evaluate_X,
	Evaluate_Y,
	Evaluate_R,
	Evaluate_G,
	Evaluate_B,
	Evaluate_Scale;
	
QString Script;
bool Show_Script;
	
Stats *Statistics;
	
int
	degree_precision;
	
QList<int>
	Exception_List;
};

}	//	namespace HiRISE
}	//	namespace UA


#endif
