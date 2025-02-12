/*	PDS_Metadata

HiROC CVS ID: $Id: PDS_Metadata.hh,v 1.14 2013/06/25 17:24:23 guym Exp $

Copyright (C) 2011-2012  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/

#ifndef PDS_Metadata_hh
#define PDS_Metadata_hh

//#include	"Network_Status.hh"

#include	<QThread>
#include	<QUrl>
#include	<QString>
//class QMutex;
class QNetworkAccessManager;
class QNetworkReply;

#include	"PVL/Parameter.hh"
namespace idaeim {
namespace PVL {
class Lister;
}}


namespace UA
{
namespace HiRISE
{
/**	A <i>PDS_Metadata</i> fetches PDS metadata from a URL source.

	The PDS_Metadata is a fully thread safe QThread subclass, and a subclass
	of Network_Status. When fetching an HTTP URL a thread is run that
	uses a QNetworkAccessManager to fetch the URL file content. The URL
	fetch may be done asynchronously or synchrously with a maximum wait
	time for it to complete, and a fetch in progress may be canceled. A
	local file URL may also be fetched; this is always done
	synchronously. A signal is emitted whenever a fetch has been
	completed.

	The results of a URL fetch is the PDS metadata parameters in the form
	of a PVL Aggregate.

	A set of utility functions are provided to find parameters in PDS
	metadata, and get and set parameter values.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.14 $
*/
class PDS_Metadata
:	public QObject/*,
	public Network_Status
   */
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


//!	Name of the PDS metadata parameters group.
static const char* const
	PDS_METADATA_GROUP;


//	Structure definition parameters:

//! PDS ID.
static const char* const
	PDS_ID_PARAMETER_NAME;

//!	Type of metadata records.
static const char* const
	RECORD_TYPE_PARAMETER_NAME;

//!	The RECORD_TYPE_PARAMETER_NAME used to define byte count based records.
static const char* const
	BYTE_RECORD_TYPE;

//!	The RECORD_TYPE_PARAMETER_NAME used to define fixed length records.
static const char* const
	FIXED_LENGTH_RECORD_TYPE;

//!	Number of records in the metadata label.
static const char* const
	LABEL_RECORDS_PARAMETER_NAME;

//!	Number of bytes per record.
static const char* const
	RECORD_BYTES_PARAMETER_NAME;

//!	Number of records in the file.
static const char* const
	FILE_RECORDS_PARAMETER_NAME;

//!	Data format.
static const char* const
	INTERCHANGE_FORMAT_PARAMETER_NAME;

//!	Name of the Image data block parameter group.
static const char* const
	IMAGE_DATA_BLOCK_NAME;

//!	Units name for storage measured in bytes.
static const char* const
	BYTES_UNITS;

/**	Record pointer parameter prefix.

	When this character is the first character of a parameter name it is
	marked as a record pointer parameter for a data block. An associated
	group of parameters having the same name, but without the initial
	marker character, is expected to be present that describes the
	content of the data block.
*/
static const char
	RECORD_POINTER_PARAMETER_MARKER;


//	Image map projection parameters:

//!	Name of the parameter group containing projection parameters.
static const char* const
	IMAGE_MAP_PROJECTION_GROUP_NAME;

//!	Type of projection.
static const char* const
	PROJECTION_TYPE_PARAMETER_NAME;
//!	The PROJECTION_TYPE_PARAMETER_NAME for equirectangular projection.
static const char* const
	EQUIRECTANGULAR_PROJECTION_NAME;
//!	The PROJECTION_TYPE_PARAMETER_NAME for polar stereographic projection.
static const char* const
	POLARSTEREOGRAPHIC_PROJECTION_NAME;

//!	Type of coordinate system.
static const char* const
	COORDINATE_SYSTEM_PARAMETER_NAME;
//!	Type of coordinate system (alternative name).
static const char* const
	LATITUDE_TYPE_PARAMETER_NAME;
//!	Planeto-centric (non-geographic) type of coordinate system.
static const char* const
	PLANETOCENTRIC_PROJECTION_NAME;
//!	Planeto-graphic (geographic) type of coordinate system.
static const char* const
	PLANETOGRAPHIC_PROJECTION_NAME;

//!	Horizontal offset of the projection origin from the image origin.
static const char* const
	HORIZONATAL_OFFSET_PARAMETER_NAME;
//!	Vertical offset of the projection origin from the image origin.
static const char* const
	VERTICAL_OFFSET_PARAMETER_NAME;

//!	Number of image lines.
static const char* const
	IMAGE_HEIGHT_PARAMETER_NAME;
//!	Number of pixels per image line.
static const char* const
	IMAGE_WIDTH_PARAMETER_NAME;

//!	Number of pixels per world degree.
static const char* const
	PIXELS_PER_DEGREE_PARAMETER_NAME;
//!	Real world size of a pixel (km by default, may be m).
static const char* const
	PIXEL_SIZE_PARAMETER_NAME;

//!	Planet's semi-major (equitorial) axis radius.
static const char* const
	EQUITORIAL_RADIUS_PARAMETER_NAME;
//!	Planet's semi-minor (polar) axis radius.
static const char* const
	POLAR_RADIUS_PARAMETER_NAME;

//!	Projection first standard parallel.
static const char* const
	FIRST_STANDARD_PARALLEL_PARAMETER_NAME;
//!	Projection second standard parallel.
static const char* const
	SECOND_STANDARD_PARALLEL_PARAMETER_NAME;

//!	Projection center longitude.
static const char* const
	CENTER_LONGITUDE_PARAMETER_NAME;
//!	Projection center latitude.
static const char* const
	CENTER_LATITUDE_PARAMETER_NAME;

//!	Image minimum latitude.
static const char* const
	MINIMUM_LATITUDE_PARAMETER_NAME;
//!	Image maximum latitude.
static const char* const
	MAXIMUM_LATITUDE_PARAMETER_NAME;
//!	Image minimum longitude.
static const char* const
	MINIMUM_LONGITUDE_PARAMETER_NAME;
//!	Image maximum longitude.
static const char* const
	MAXIMUM_LONGITUDE_PARAMETER_NAME;

//!	Positive longitude direction.
static const char* const
	POSITIVE_LONGITUDE_PARAMETER_NAME;
//!	POSITIVE_LONGITUDE_PARAMETER_NAME for easterly positive longitude.
static const char* const
	POSITIVE_LONGITUDE_EAST_NAME;
//!	POSITIVE_LONGITUDE_PARAMETER_NAME for westerly positive longitude.
static const char* const
	POSITIVE_LONGITUDE_WEST_NAME;

//!	Projection rotation.
static const char* const
	PROJECTION_ROTATION_PARAMETER_NAME;

//!	Not-applicable constant.
static const char* const
	NOT_APPLICABLE_CONSTANT_PARAMETER_NAME;

/*==============================================================================
	Constructor
*/
/**	Constructs a PDS_Metadata object.

	@param	parent	A QObject that owns the constructed object and will
		destroy it when the parent is destroyed.
*/
explicit PDS_Metadata (QObject* parent = NULL);

//!	Destroys the PDS_Metadata.
virtual ~PDS_Metadata ();

/*==============================================================================
	Accessors
*/
/**	Get the most recently {@link fetch(const QUrl&, bool) fetched}
	PDS metadata.

	@return	A pointer to an idaeim::PVL::Aggregate containing the PDS
		metadata parameters. This will be NULL if no metadata is available.
*/
idaeim::PVL::Aggregate* metadata () const;

/**	Test if a {@link fetch(const QUrl&, bool) URL fetch} is currently in
	progress.

	@return	true if a fetch is in progress; false otherwise.
*/
bool fetching () const;

/**	Reset this PDS_Metadata.

	The {@link metadata() metadata parameters}, if any, are deleted and the
	metadata is set to NULL. The network status values are also {@link
	Network_Status::reset() reset}.
*/
bool reset ();

/*==============================================================================
	Utilities
*/
/**	Selects the class of parameter to {@link
	find(const std::string&, bool, int, Parameter_Class_Selection) find} or
	remove(const std::string&, bool, int, Parameter_Class_Selection) remove}.
*/
enum Parameter_Class_Selection
	{
	AGGREGATE_PARAMETER	 = -1,
	ANY_PARAMETER        =  0,
	ASSIGNMENT_PARAMETER =  1
	};

//!	Flag values for case sensitivity.
enum
	{
	CASE_INSENSITIVE,
	CASE_SENSITIVE
	};

/**	Find a named Parameter within a Parameter Aggregate.

	The Parameter to be found may be specified by a simple name, a
	relative pathname or an absolute pathname. A simple name is the
	{@link idaeim::PVL::Parameter::name() name} of a Parameter itself. A
	relative pathname is a simple name preceeded by the pathname of an
	Aggregate Parameter that must contain the Parameter with the simple
	name. An {@link absolute_pathname(const std::string) absolute
	pathname} is a pathname that begins with a {@link
	idaeim::PVL::Parameter::path_delimiter() path delimiter} character
	('/' by default); i.e. it is a fully qualified pathname beginning at
	the {@link idaeim::PVL::Parameter::root root} of the Aggregate
	hierarchy, compared to a relative pathname that may begin at any
	Aggregate in the hierarchy.

	Since it is not necessary that the pathname to each Parameter be
	unique, the number of matching pathnames to skip before selecting a
	Parameter may need to be specified to get the desired Parameter.
	Also, a specific type of Parameter - an Aggregate of Parameters or an
	Assignment of Values - may be specified. When skipping parameters,
	only the specified type are counted.

	@param	parameters	The Aggregate within which to search.
	@param	pathname	The pathname for the parameter to be found.
	@param	case_sensitive	If true, the name search is case sensitive;
		case insensitive otherwise.
	@param	skip		The number of parameters that match the
		name to skip before selecting a matching parameter.
	@param	parameter_class		A PDS::Parameter_Class_Selection: If
		PDS_Data::AGGREGATE_PARAMETER, only Aggregate parameters will be
		selected; if PDS_Data::ASSIGNMENT_PARAMETER, only Assignment
		parameters will be selected; otherwise any type of parameter
		(PDS_Data::ANY_PARAMETER) is acceptable.
	@return	A pointer to the matching parameter, or NULL if it could not
		be found.
	@see	find_parameter(const idaeim::PVL::Aggregate&, const QString&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
find_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{
return parameters.find (pathname, case_sensitive, skip,
	static_cast<idaeim::PVL::Parameter::Type>(
		((parameter_class == ANY_PARAMETER) ?
			idaeim::PVL::Parameter::ASSIGNMENT |
			idaeim::PVL::Parameter::AGGREGATE :
		((parameter_class == ASSIGNMENT_PARAMETER) ?
		  	idaeim::PVL::Parameter::ASSIGNMENT :
			idaeim::PVL::Parameter::AGGREGATE))));
}

/**	Find a named Parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
find_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{return PDS_Metadata::find_parameter (parameters, std::string (pathname),
	case_sensitive, skip, parameter_class);}

/**	Find a named Parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
find_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{return PDS_Metadata::find_parameter (parameters, pathname.toStdString (),
	case_sensitive, skip, parameter_class);}

/**	Find a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
find_parameter
	(
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return metadata () ?
	find_parameter (*metadata (), pathname,
		case_sensitive, skip, parameter_class) :
	NULL;}

/**	Find a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
find_parameter
	(
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return metadata () ?
	find_parameter (*metadata (), std::string (pathname),
		case_sensitive, skip, parameter_class) :
	NULL;}

/**	Find a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a QString pathname.

	@see	find_parameter(const std::string&, bool, int,
		Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
find_parameter
	(
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return find_parameter (pathname.toStdString (),
	case_sensitive, skip, parameter_class);}


/**	Removes a named Parameter within a Parameter Aggregate.

	@param	parameters	The Aggregate within which to search.
	@param	pathname	The pathname for the parameter to be found.
	@param	case_sensitive	If true, the name search is case sensitive;
		case insensitive otherwise.
	@param	skip	The number of parameters that match the name to skip
		before selecting a matching parameter.
	@param	parameter_class		A Parameter_Class_Selection: If
		AGGREGATE_PARAMETER, only Aggregate parameters will be selected;
		if ASSIGNMENT_PARAMETER, only Assignment parameters will be
		selected; otherwise any type of parameter (ANY_PARAMETER) is
		acceptable.
	@return	A pointer to the parameter that was removed, or NULL if it
		could not be found.
	@see	remove_parameter(const idaeim::PVL::Aggregate&, const QString&,
		bool, int, Parameter_Class_Selection)
	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
remove_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{
idaeim::PVL::Parameter
	*parameter (PDS_Metadata::find_parameter
		(parameters, pathname, case_sensitive, skip, parameter_class));
if (parameter)
	parameter = parameter->parent ()->remove (parameter);
return parameter;
}

/**	Removes a named Parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	remove_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
remove_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{return PDS_Metadata::remove_parameter (parameters, std::string (pathname),
	case_sensitive, skip, parameter_class);}

/**	Removes a named Parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	remove_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static idaeim::PVL::Parameter*
remove_parameter
	(
	const idaeim::PVL::Aggregate&	parameters,
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	)
{return PDS_Metadata::remove_parameter (parameters, pathname.toStdString (),
	case_sensitive, skip, parameter_class);}

/**	Removes a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	remove_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
remove_parameter
	(
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return metadata () ?
	remove_parameter (*metadata (), pathname,
		case_sensitive, skip, parameter_class) :
	NULL;}

/**	Removes a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a char* pathname.

	@see	remove_parameter(const std::string&, bool, int,
		Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
remove_parameter
	(
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return remove_parameter (std::string (pathname),
	case_sensitive, skip, parameter_class);}

/**	Removes a named Parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a QString pathname.

	@see	remove_parameter(const std::string&, bool, int,
		Parameter_Class_Selection)
*/
inline idaeim::PVL::Parameter*
remove_parameter
	(
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0,
	Parameter_Class_Selection		parameter_class = ANY_PARAMETER
	) const
{return remove_parameter (pathname.toStdString (),
	case_sensitive, skip, parameter_class);}


/**	Get the numeric value of a named parameter within a Parameter Aggregate.

	@param	parameters	The Aggregate within which to search.
	@param	pathname	The pathname for the parameter to be found.
	@param	case_sensitive	If true, the name search is case sensitive;
		case insensitive otherwise.
	@param	skip	The number of parameters that match the pathname to
		skip before selecting the parameter to modify.
	@return	The value of the parameter as a double.
	@throws	idaeim::Invalid_Argument	If an assignment parameter can
		not be found at the pathname.
	@throws	idaeim::PVL::Invalid_Value	If the Value is not Numeric.
	@see	numeric_value(const idaeim::PVL::Aggregate&, const QString&,
		bool, int, Parameter_Class_Selection)
	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
static double
numeric_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	);

/**	Get the numeric value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static double
numeric_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return PDS_Metadata::numeric_value (parameters, std::string (pathname),
	case_sensitive, skip);}

/**	Get the numeric value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
inline static double
numeric_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return PDS_Metadata::numeric_value (parameters, pathname.toStdString (),
	case_sensitive, skip);}

/**	Get the numeric value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	numeric_value(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
double
numeric_value
	(
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	) const;

/**	Get the numeric value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a char* pathname.

	@see	numeric_value(const std::string&, bool, int,
		Parameter_Class_Selection)
*/
double
numeric_value
	(
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	) const
{return numeric_value (std::string (pathname), case_sensitive, skip);}

/**	Get the numeric value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a QString pathname.

	@see	numeric_value(const std::string&, bool, int,
		Parameter_Class_Selection)
*/
double
numeric_value
	(
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0
	) const
{return numeric_value (pathname.toStdString (), case_sensitive, skip);}

/**	Get the numeric value of a parameter.

	@param	parameter	The parameter from which to obtain a numeric value.
	@return	The value of the parameter as a double.
	@throws	idaeim::PVL::Invalid_Value	If the Value is not Numeric.
*/
static double numeric_value (const idaeim::PVL::Parameter& parameter);


/**	Get the string value of a named parameter within a Parameter Aggregate.

	@param	parameters	The Aggregate within which to search.
	@param	pathname	The pathname for the parameter to be found.
	@param	case_sensitive	If true, the name search is case sensitive;
		case insensitive otherwise.
	@param	skip	The number of parameters that match the pathname to
		skip before selecting the parameter to modify.
	@return	The string value of the parameter.
	@throws	idaeim::Invalid_Argument	If an assignment parameter can
		not be found at the pathname.
	@throws	idaeim::PVL::Invalid_Value	If the Value is not a String.
	@see	string_value(const idaeim::PVL::Aggregate&, const QString&,
		bool, int)
	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
static std::string
string_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	);

/**	Get the string value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	string_value(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int)
*/
inline static std::string
string_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return PDS_Metadata::string_value (parameters, std::string (pathname),
	case_sensitive, skip);}

/**	Get the string value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	string_value(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int)
*/
inline static QString
string_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return QString::fromStdString (PDS_Metadata::string_value
	(parameters, pathname.toStdString (), case_sensitive, skip));}

/**	Get the string value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	find_parameter(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int, Parameter_Class_Selection)
*/
std::string
string_value
	(
	const std::string&				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	) const;

/**	Get the string value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a char* pathname.

	@see	string_value(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int)
*/
inline std::string
string_value
	(
	const char* const				pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return string_value (std::string (pathname), case_sensitive, skip);}

/**	Get the string value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a QString pathname.

	@see	string_value(const idaeim::PVL::Aggregate&, const std::string&,
		bool, int)
*/
inline QString
string_value
	(
	const QString&					pathname,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return QString::fromStdString (string_value
	(pathname.toStdString (), case_sensitive, skip));}

/**	Get the string value of a parameter.

	@param	parameter	The parameter from which to obtain a string value.
	@return	The value of the parameter as a string.
*/
static std::string string_value (const idaeim::PVL::Parameter& parameter);

/**	Get the string value of a parameter.

	This is a convenience function that overloads the string_value function,
	but returns a QString value.

	@param	parameter	The parameter from which to obtain a string value.
	@return	The value of the parameter as a string.
	@see	string_value(const idaeim::PVL::Parameter&)
*/
inline static QString QString_value (const idaeim::PVL::Parameter& parameter)
{return QString::fromStdString (PDS_Metadata::string_value (parameter));}

/**	Set the value of a named parameter within a Parameter Aggregate.

	Only Assignment Parameters with a Value that is not an Array will be
	modified. However, the search for a matching name does not make this
	distinction.

	@param	parameters	The Aggregate within which to search.
	@param	pathname	The pathname for the parameter to be found.
	@param	value	The value to assign to the parameter.
	@param	case_sensitive	If true, the name search is case sensitive;
		case insensitive otherwise.
	@param	skip	The number of parameters that match the pathname to
		skip before selecting the parameter to modify.
	@return	true if the parameter was found; false otherwise.
	@see	find_parameter(const std::string&, bool, int,
				Parameter_Class_Selection)
*/
template <typename T>
bool
parameter_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	)
{
idaeim::PVL::Parameter
	*parameter (parameters.find
		(pathname, case_sensitive, skip, idaeim::PVL::Parameter::ASSIGNMENT));
if (! parameter ||
	  parameter->value ().is_Array ())
	return false;
parameter->value () = value;
return true;
}

/**	Set the value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	parameter_value(const idaeim::PVL::Aggregate&, const std::string&,
		T&, bool, int)
*/
template <typename T>
bool
parameter_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const char* const				pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return PDS_Metadata::parameter_value (parameters, std::string (pathname),
	value, case_sensitive, skip);}

/**	Set the value of a named parameter within a Parameter Aggregate.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	parameter_value(const idaeim::PVL::Aggregate&, const std::string&,
		T&, bool, int)
*/
template <typename T>
bool
parameter_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const QString&					pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	)
{return PDS_Metadata::parameter_value (parameters, pathname.toStdString (),
	value, case_sensitive, skip);}

/**	Set the value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent function.

	@see	parameter_value(const idaeim::PVL::Aggregate&, const std::string&,
		T&, bool, int)
*/
template <typename T>
bool
parameter_value
	(
	const std::string&				pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	) const
{return metadata () ?
	parameter_value (*metadata (), pathname, value, case_sensitive, skip) :
	false;}

/**	Set the value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a char* pathname.

	@see	parameter_value(const std::string&, T&, bool, int)
*/
template <typename T>
bool
parameter_value
	(
	const char* const				pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	) const
{return parameter_value (std::string (pathname), value, case_sensitive, skip);}

/**	Set the value of a named parameter within the metadata parameters.

	This is a convenience method that overloads the equivalent method,
	but for use with a QString pathname.

	@see	parameter_value(const std::string&, T&, bool, int)
*/
template <typename T>
bool
parameter_value
	(
	const QString&					pathname,
	T&								value,
	bool							case_sensitive = false,
	int								skip = 0
	) const
{return parameter_value (pathname.toStdString (), value, case_sensitive, skip);}

/**	Provide an absolute pathname.

	If the pathname does not begin with a {@link
	idaeim::PVL::Parameter::path_delimiter() Parameter pathname delimiter}
	one is prepended. An empty pathname results in an empty string.

	@param	pathname	A Parameter pathname string.
	@return	The pathname in absolute form.
*/
static std::string absolute_pathname (const std::string& pathname);

/**	Provide an absolute pathname.

	This is a convenience function that overloads the equivalent function,
	but for use with a char* pathname.

	@see	absolute_pathname(const std::string&)
*/
inline static std::string absolute_pathname (const char* const pathname)
{return PDS_Metadata::absolute_pathname (std::string (pathname));}

/**	Provide an absolute pathname.

	This is a convenience function that overloads the equivalent function,
	but for use with a QString pathname.

	@see	absolute_pathname(const std::string&)
*/
inline static QString absolute_pathname (const QString& pathname)
{return QString::fromStdString
	(PDS_Metadata::absolute_pathname (pathname.toStdString ()));}

/**	Get a PVL Lister appropriately configured for PDS metadata generation.

	The Lister will be configured to use strict mode except: Begin and
	End name markers on Aggregates will not be included, assignment
	value alignment will be enabled, array indenting will be enabled,
	single line commenting style will be used, and the statement end
	delimiter will not be used.

	@return	A pointer to an idaeim::PVL::Lister that has been allocated
		on the heap. <b>N.B.</b>: Delete when no longer needed.
*/
static idaeim::PVL::Lister* metadata_lister ();

/*==============================================================================
	Thread run
*/
//!	Flag values for the {@link fetch(const QUrl&, bool) fetch} method.
enum
	{
	ASYNCHRONOUS,
	SYNCHRONOUS
	};

/**	Fetch a PDS metadata from a URL.

	If a URL fetch is {@link fetching() in progress}, or the specified
	URL is empty, nothing is done and false is returned immediately.

	When a check is started the network request state is {@link reset()
	reset} to unset values with the {@link request_status() request status}
	set to {@link #IN_PROGRESS}. Also existing {@link metadata() metadata
	parameters are deleted and the metadata is reset to NULL.

	The URL is checked to see if it is accessible. Only a URL with an
	HTTP or "file" protocol will be accepted for fetching. A URL without
	a protocol is taken to be a file URL. The URL must have a pathname.
	HTTP URLs must have a hostname; a hostname in a file URLs is ignored.
	Any other URLs are rejected and the {@link request_status() request
	status} value will be set to {@link #INVALID_URL}.

	For an HTTP URL a network get request will be made for the URL
	context which will be parsed as it is received to obtain PVL
	Parameter statements. Only the initial part of the URL content that
	contains PVL will be parsed; any addition content is ignored by
	cancelling the network request after PVL parsing completes.

	<b>N.B.</b>: A URL source will appear to be accessible when the
	server returns a {@link redirected_URL() redirected URL} where the
	ultimate source may or may not be accesible; it is the responsibility
	of the application to decide if a redirected URL is to be fetched.

	An HTTP URL fetch may be synchronous or asynchronous. A synchronous
	fetch will wait - the thread of the caller will be blocked - until
	the fetch completes. The wait for the network request to complete
	will timeout if the {@link wait_time() wait time} is exceeded, in
	which case the network request will be {@link cancel() canceled} and
	the result value will be set to {@link #SYNCHRONOUS_TIMEOUT}. Thread
	blocking can be avoided - this can be important, for example, when
	the fetch is being done from the thread running the main GUI event
	loop - by specifying an asynchronous check with a connection to the
	{@link fetched(idaeim::PVL::Aggregate*) fetched} signal to obtain the
	results of the fetch.

	If the URL specifies the file protocol, or does not specify any
	protocol, then the local filesystem is used as the source for the
	path portion of the URL. The pathname must exist and be a readable
	file for the metadata fetch to proceed by parsing the PVL content of the
	file.

	The {@link fetched(idaeim::PVL::Aggregate*) fetched} signal is always
	emitted when the fetch completesregardless of whether the fetch is
	synchronous or asynchronous, or if a timeout occurs.

	<b>>>> WARNING <<<</b> A filesystem pathname on MS/Windows that
	includes a drive specification - e.g. "D:" - will be interpreted as
	having a URL scheme specification that is the drive letter when a
	QUrl is constructed from such a pathname string. For this reason it
	is advisable that a {@link normalized_URL(const QString&) normalized}
	QUrl be used here.

	@param	URL	A QUrl that is to be used as the PDS metadata source.
	@param	synchronous	This argument is ignored unless the URL specifies
		an HTTP protocol. If synchronous is true the method will wait for
		the network fetch to complete or timeout before returning;
		otherwise the method will return without waiting for the network
		fetch to complete.
	@return	true if the fetch completed successfully and the source was
		found to be accessible; false otherwise. A successful fetch may
		result in a NULL {@link metadata() metadata} if no PVL was parsed from
		the source. <b>N.B.</b>: For an asynchronous HTTP URL fetch the
		return value is always false; connect to the {@link
		fetched(idaeim::PVL::Aggregate*) fetched} signal to get the
		results of the fetch, or test if the {@link request_status()
		request status} is {@link #ACCESSIBLE_URL} when {@link fetching()
		fetching} becomes false.
*/
void fetch (const QUrl& URL, bool synchronous = true);


/*==============================================================================
	Qt signals
*/
public:

signals:

/**	Signals the result of a {@link fetch(const QUrl&, bool) URL fetch}.

	The signal is emitted whenever a URL fetch completes.

	@param	metadata	A pointer to the metadata that was fetched. This
		will be NULL if an error occurred while fetching the metadata, if
		the fetch was cancelled, or if the beginning of the source
		content did not contain any PVL.
	@see request_status()
*/
void fetched (idaeim::PVL::Aggregate* metadata);
/*==============================================================================
	Qt slots
*/

private slots:

void fetched ();
    
/*==============================================================================
	Data
*/
private:


QNetworkReply* Network_Reply;
QUrl Requested_URL;


idaeim::PVL::Aggregate*
	Metadata;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
