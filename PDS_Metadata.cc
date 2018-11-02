/*	PDS_Metadata

HiROC CVS ID: $Id: PDS_Metadata.cc,v 1.18 2018/03/06 19:06:15 guym Exp $

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

#include	"PDS_Metadata.hh"
#include "Qstream.hh"

#include	"PVL.hh"
using namespace idaeim::PVL;

#include	<QNetworkAccessManager>
#include	<QNetworkReply>
#include	<QUrl>
#include	<QFileInfo>
#include	<QDir>
#include <QBuffer>
//#include	<QMutex>
//#include	<QMutexLocker>

#include	<string>
using std::string;

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
#define DEBUG_UTILITIES			(1 << 1)
#define DEBUG_FETCH				(1 << 2)
#define	DEBUG_RUN				(1 << 3)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	"HiView_Utilities.hh"
#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
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
	PDS_Metadata::ID =
		"UA::HiRISE::PDS_Metadata ($Revision: 1.18 $ $Date: 2018/03/06 19:06:15 $)";


#ifndef PDS_METADATA_GROUP_NAME
#define PDS_METADATA_GROUP_NAME			"PDS"
#endif
const char* const
	PDS_Metadata::PDS_METADATA_GROUP	= PDS_METADATA_GROUP_NAME;

const char
	* const PDS_Metadata::PDS_ID_PARAMETER_NAME
		= "PDS_VERSION_ID",
	* const PDS_Metadata::RECORD_TYPE_PARAMETER_NAME
		= "RECORD_TYPE",
	* const PDS_Metadata::BYTE_RECORD_TYPE
			= "UNDEFINED",
	* const PDS_Metadata::FIXED_LENGTH_RECORD_TYPE
			= "FIXED_LENGTH",
	* const PDS_Metadata::LABEL_RECORDS_PARAMETER_NAME
		= "LABEL_RECORDS",
	* const PDS_Metadata::RECORD_BYTES_PARAMETER_NAME
		= "RECORD_BYTES",
	* const PDS_Metadata::FILE_RECORDS_PARAMETER_NAME
		= "FILE_RECORDS",
	* const PDS_Metadata::INTERCHANGE_FORMAT_PARAMETER_NAME
		= "INTERCHANGE_FORMAT",
	* const PDS_Metadata::IMAGE_DATA_BLOCK_NAME
		= "IMAGE",
	* const PDS_Metadata::BYTES_UNITS
		= "BYTES";

const char
	PDS_Metadata::RECORD_POINTER_PARAMETER_MARKER
		= '^';


const char
	* const PDS_Metadata::IMAGE_MAP_PROJECTION_GROUP_NAME
		= "IMAGE_MAP_PROJECTION",
	* const PDS_Metadata::PROJECTION_TYPE_PARAMETER_NAME
		= "MAP_PROJECTION_TYPE",
	* const PDS_Metadata::EQUIRECTANGULAR_PROJECTION_NAME
			= "EQUIRECTANGULAR",
	* const PDS_Metadata::POLARSTEREOGRAPHIC_PROJECTION_NAME
			= "POLARSTEREOGRAPHIC",

	* const PDS_Metadata::COORDINATE_SYSTEM_PARAMETER_NAME
		= "COORDINATE_SYSTEM_NAME",
	* const PDS_Metadata::LATITUDE_TYPE_PARAMETER_NAME
		= "PROJECTION_LATITUDE_TYPE",
	* const PDS_Metadata::PLANETOCENTRIC_PROJECTION_NAME
			= "PLANETOCENTRIC",
	* const PDS_Metadata::PLANETOGRAPHIC_PROJECTION_NAME
			= "PLANETOGRAPHIC",
	* const PDS_Metadata::HORIZONATAL_OFFSET_PARAMETER_NAME
		= "SAMPLE_PROJECTION_OFFSET",
	* const PDS_Metadata::VERTICAL_OFFSET_PARAMETER_NAME
		= "LINE_PROJECTION_OFFSET",
	* const PDS_Metadata::IMAGE_HEIGHT_PARAMETER_NAME
		= "LINE_LAST_PIXEL",
	* const PDS_Metadata::IMAGE_WIDTH_PARAMETER_NAME
		= "SAMPLE_LAST_PIXEL",
	* const PDS_Metadata::PIXELS_PER_DEGREE_PARAMETER_NAME
		= "MAP_RESOLUTION",
	* const PDS_Metadata::PIXEL_SIZE_PARAMETER_NAME
		= "MAP_SCALE",
	* const PDS_Metadata::EQUITORIAL_RADIUS_PARAMETER_NAME
		= "A_AXIS_RADIUS",
	* const PDS_Metadata::POLAR_RADIUS_PARAMETER_NAME
		= "C_AXIS_RADIUS",
	* const PDS_Metadata::FIRST_STANDARD_PARALLEL_PARAMETER_NAME
		= "FIRST_STANDARD_PARALLEL",
	* const PDS_Metadata::SECOND_STANDARD_PARALLEL_PARAMETER_NAME
		= "SECOND_STANDARD_PARALLEL",
	* const PDS_Metadata::MINIMUM_LATITUDE_PARAMETER_NAME
		= "MINIMUM_LATITUDE",
	* const PDS_Metadata::CENTER_LATITUDE_PARAMETER_NAME
		= "CENTER_LATITUDE",
	* const PDS_Metadata::MAXIMUM_LATITUDE_PARAMETER_NAME
		= "MAXIMUM_LATITUDE",
	* const PDS_Metadata::MINIMUM_LONGITUDE_PARAMETER_NAME
		= "WESTERNMOST_LONGITUDE",
	* const PDS_Metadata::CENTER_LONGITUDE_PARAMETER_NAME
		= "CENTER_LONGITUDE",
	* const PDS_Metadata::MAXIMUM_LONGITUDE_PARAMETER_NAME
		= "EASTERNMOST_LONGITUDE",
	* const PDS_Metadata::POSITIVE_LONGITUDE_PARAMETER_NAME
		= "POSITIVE_LONGITUDE_DIRECTION",
	* const PDS_Metadata::POSITIVE_LONGITUDE_EAST_NAME
			= "EAST",
	* const PDS_Metadata::POSITIVE_LONGITUDE_WEST_NAME
			= "WEST",
	* const PDS_Metadata::PROJECTION_ROTATION_PARAMETER_NAME
		= "MAP_PROJECTION_ROTATION",
	* const PDS_Metadata::NOT_APPLICABLE_CONSTANT_PARAMETER_NAME
		= "NOT_APPLICABLE_CONSTANT";

/*==============================================================================
	Constructor
*/
PDS_Metadata::PDS_Metadata
	(
	QObject*	parent
	)
	:	QObject (parent),
		//Network_Status (),
		Metadata (NULL)
{
setObjectName ("PDS_Metadata");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< PDS_Metadata" << endl;
#endif
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
clog << boolalpha;
#endif
}


PDS_Metadata::~PDS_Metadata ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~PDS_Metadata" << endl;
#endif
/*cancel ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
bool
	wait_completed =
#endif
wait (Default_Wait_Time);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    wait completed - " << wait_completed << endl
	 << "    Metadata @ " << (void*)Metadata << endl;
#endif
*/
if (Metadata)
	delete Metadata;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~PDS_Metadata" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
idaeim::PVL::Aggregate*
PDS_Metadata::metadata () const
{
/*QMutexLocker
	lock (Status_Lock);*/
return Metadata;
}


bool
PDS_Metadata::fetching () const
{return false;}
//{return isRunning () || request_status () == IN_PROGRESS;}


bool
PDS_Metadata::reset ()
{
/*QMutexLocker
	lock (Status_Lock);
bool
	was_reset = Network_Status::reset ();
if (was_reset &&
	Metadata)
	{
	delete Metadata;
	Metadata = NULL;
	}
return was_reset;*/
if (Metadata)
{
    delete Metadata;
}
return true;
}

/*==============================================================================
	Fetch the Metadata
*/
void
PDS_Metadata::fetch
	(
	const QUrl&	URL,
	bool
#if ((DEBUG_SECTION) & DEBUG_FETCH)
		synchronous
#endif
	)
{
    
#if ((DEBUG_SECTION) & DEBUG_FETCH)
clog << ">>> PDS_Metadata::fetch: " << URL.toString () << endl
	 << "     URL scheme = " << URL.scheme () << endl
	 << "       URL path = " << URL.path () << endl
	 << "    synchronous = " << synchronous << endl;
#endif
/*int
	result = IN_PROGRESS;*/
if (/* isRunning () &&*/  URL.isEmpty ()) return;

//{
	//Status_Lock->lock ();
/*
	if (Request_Status == IN_PROGRESS)
		{
		#if ((DEBUG_SECTION) & DEBUG_FETCH)
		clog << "    Request_Status IN_PROGRESS!" << endl
			 << "<<< PDS_Metadata::fetch: false" << endl;
		#endif
		//Status_Lock->unlock ();
		return false;
		}
      */
	//	Reset the fetch state.
	if (Metadata)
	{
		delete Metadata;
		Metadata = NULL;
	}
	
	Requested_URL = URL;
   /*
	Redirected_URL.clear ();
	Request_Status = result;
	HTTP_Status = NO_STATUS;
	HTTP_Status_Description.clear ();
   */
	//Status_Lock->unlock ();

	if ((URL.scheme ().compare ("HTTP", Qt::CaseInsensitive) == 0 ||
        URL.scheme ().compare ("HTTPS", Qt::CaseInsensitive) == 0) &&
		! URL.host ().isEmpty () &&
		! URL.path ().isEmpty ())
	{
		//	HTTP server fetch.
		#if ((DEBUG_SECTION) & DEBUG_FETCH)
		clog << "    PDS_Metadata::fetch: starting network get" << endl;
		#endif
		/*
		start ();

		if (synchronous)
		{
			#if ((DEBUG_SECTION) & DEBUG_CHECK)
			LOCKED_LOGGING ((
			clog << "    PDS_Metadata::fetch: waiting for thread to finish"
					<< endl));
			#endif
			wait ();	//	Wait for the thread to finish;
			result = Request_Status;
			synchronous = false;	//	No signal from here.
		}
		*/
	/*
		The QNetworkAccessManager must be constructed on the same thread
		where it is used with its QNetworkReply.
	*/
	QNetworkAccessManager* Network_Access_Manager = new QNetworkAccessManager(this);
		
		Network_Reply = Network_Access_Manager->get (QNetworkRequest (Requested_URL));
		QObject::connect
		(
		    Network_Reply,
		    SIGNAL(finished()),
		    this,
		    SLOT(fetched())
		);
	}
	else
	if ((URL.scheme ().compare ("FILE", Qt::CaseInsensitive) == 0 ||
		 URL.scheme ().isEmpty ()) &&
		! URL.path ().isEmpty ())
	{
		//	Local filesystem fetch.
		//synchronous = true;
		#if ((DEBUG_SECTION) & DEBUG_FETCH)
		clog << "    filesystem fetch for " << URL.path () << endl;
		#endif
		QFile
			file (QDir::toNativeSeparators (URL.path ()));
		if (file.exists ())
		{
			if (file.open (QIODevice::ReadOnly))
				{
				//result = ACCESSIBLE_URL;
				Qistream
					qistream (&file);
				Parser
					parser (qistream);
				try {
					Metadata = new Aggregate (parser, Parser::CONTAINER_NAME);
					#if ((DEBUG_SECTION) & DEBUG_FETCH)
					clog << "    Metadata @ " << (void*)Metadata << " -" << endl
						 << *Metadata;
					#endif
		//Request_Status = NO_STATUS;
							emit fetched (Metadata);

					}
				catch (idaeim::Exception except)
					{
					#if ((DEBUG_SECTION) & DEBUG_FETCH)
					clog << "!!! Failed to parse file source -" << endl
						 << except.message () << endl;
					#endif
					}
				file.close ();
				}
			/*else
				result = URL_ACCESS_DENIED;*/
		}
	/*	else
			result = URL_NOT_FOUND;*/
	/*}
	else
	{
		synchronous = true;
		result = INVALID_URL;
	}*/
	/*
	if (synchronous)
	{
		//	Set the request status.
		request_status (result);

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & DEBUG_FETCH)
		LOCKED_LOGGING ((
		clog << "    PDS_Metadata::fetch: emit fetched Metadata @ "
				<< (void*)Metadata << endl));
		#endif
		emit fetched (Metadata);
	}
	*/
}

}

/*
void
PDS_Metadata::run ()
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << ">>> PDS_Metadata::run" << endl
	 << "    Requested_URL: " << Requested_URL.toString () << endl));
#endif
Status_Lock->lock ();
if (! Network_Access_Manager)
	/
		The QNetworkAccessManager must be constructed on the same thread
		where it is used with its QNetworkReply.
	/
	Network_Access_Manager = new QNetworkAccessManager;
Network_Reply = Network_Access_Manager->get (QNetworkRequest (Requested_URL));
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "    constructing Qistream on QNetworkReply" << endl
	 << "      with " << Wait_Time << "ms wait time" << endl));
#endif
Qistream
	qistream (Network_Reply);
Qstreambuf
	*stream_buffer = dynamic_cast<Qstreambuf*>(qistream.rdbuf ());
stream_buffer->wait_time (Wait_Time);
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "    constructing Parser on Qistream" << endl));
#endif
Parser
	parser (qistream);

//	Assemble the Metadata Aggregate.
Status_Lock->unlock ();	//	Unlock access during data fetch and parse.
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "    constructing Aggregate on Parser" << endl));
#endif
Aggregate
	*metadata (NULL);
try {metadata = new Aggregate (parser, Parser::CONTAINER_NAME);}
catch (idaeim::Exception except)
	{
	#if ((DEBUG_SECTION) & DEBUG_RUN)
	clog << "!!! Failed to parse source -" << endl
		 << except.message () << endl;
	#endif
	}
Status_Lock->lock ();

//	Reset the status values.
reset_state (*Network_Reply);
if (stream_buffer->timeout ())
	{
	Network_Reply->abort ();
	request_status (SYNCHRONOUS_TIMEOUT);
	}

delete Network_Reply;
Network_Reply = NULL;

if (metadata)
	{
	if (request_status () != ACCESSIBLE_URL ||
		! redirected_URL ().isEmpty ())
		{
		//	Inaccessible URL or server redirection. Invalid metadata.
		delete metadata;
		metadata = NULL;
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		LOCKED_LOGGING ((
		clog << "    invalid metadata: "
				<< ((request_status () != ACCESSIBLE_URL) ?
					status_description (request_status ()) :
					(QString ("redirected URL - ")
						+= redirected_URL ().toString ()))
				<< endl));
		#endif
		}
	}
Metadata = metadata;
Status_Lock->unlock ();
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "    PDS_Metadata::run: emit fetched Metadata @ "
		<< (void*)Metadata << endl));
#endif
//	>>> SIGNAL <<<
emit fetched (Metadata);

#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "<<< PDS_Metadata::run" << endl));
#endif
}
*/

void PDS_Metadata::fetched()
{
    if (! Network_Reply) return;
    
    if (Network_Reply->error() != QNetworkReply::NoError)
    {
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!! PDS_Metadata::fetched got error" <<  Network_Reply->errorString() << endl;
	#endif 
	return;
    }
    
	QUrl predirectUrl =
	         Network_Reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

	
	if (predirectUrl == Network_Reply->request().url())
	{
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!! PDS_Metadata::fetched infinite loop detected" <<  endl;
	#endif  
		Network_Reply->deleteLater();
		return;
	}
 
	/* If the URL is not empty, we're being redirected. */
	if ( ! predirectUrl.isEmpty()) {
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!! PDS_Metadata::fetched redirected to " << predirectUrl.toString() << endl;
	#endif 

		/* We'll do another request to the redirection url. */
		Network_Reply->deleteLater();
		//Request_Status = NO_STATUS;
		fetch(predirectUrl, true);
		
		return;
	}    
    //QString* qstr = new QString(Network_Reply->readAll());
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "PDS_Metadata::fetched" << endl;
	//clog << qstr->toStdString() << endl;
	#endif        
/*	
   QBuffer* buffer = new QBuffer();
   QByteArray array = Network_Reply->readAll();

	buffer->setBuffer(&array);
				Qistream
					qistream (buffer);   
					Parser parser(qistream);
*/

    QString* qstr = new QString(Network_Reply->readAll());
    Parser parser(qstr->toStdString());
    
Aggregate
	*metadata (NULL);
try {metadata = new Aggregate (parser, Parser::CONTAINER_NAME);}
catch (idaeim::Exception except)
	{
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!!PDS_Metadata::fetched Failed to parse source -" << endl
		 << except.message () << endl;
	#endif
	} 
catch (std::exception except)
{
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!!PDS_Metadata::fetched Failed to parse source -" << endl
		 << except.what () << endl;
	#endif
}
catch (...)
{
	#if ((DEBUG_SECTION) & DEBUG_FETCH)
	clog << "!!!PDS_Metadata::fetched Failed to parse source!" << endl;
	#endif
}

   delete qstr;
	//Request_Status = NO_STATUS;
	Network_Reply->deleteLater();
Metadata = metadata;	
emit fetched (Metadata);	
}

/*==============================================================================
	Utilities
*/
double
PDS_Metadata::numeric_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive,
	int								skip
	)
{
Parameter
	*parameter (parameters.find
		(pathname, case_sensitive, skip, idaeim::PVL::Parameter::ASSIGNMENT));
if (! parameter)
	{
	ostringstream
		message;
	message 
		<< PDS_Metadata::ID << endl
		<< "Can't find the numeric value assignment parameter " /*<< pathname*/;
	throw idaeim::Invalid_Argument (message.str (), ID);
	}
return PDS_Metadata::numeric_value (*parameter);
}


double
PDS_Metadata::numeric_value
	(
	const std::string&				pathname,
	bool							case_sensitive,
	int								skip
	) const
{
Aggregate
	*parameters (metadata ());
if (! parameters)
	{
	ostringstream
		message;
	message 
		<< ID << endl
		<< "No metadata for numeric value parameter " /*<< pathname*/;
	throw idaeim::Invalid_Argument (message.str (), ID);
	}
return numeric_value (*parameters, pathname, case_sensitive, skip);
}


double
PDS_Metadata::numeric_value
	(
	const idaeim::PVL::Parameter&	parameter
	)
{
#if ((DEBUG_SECTION) & DEBUG_UTILITIES)
clog << ">>> PDS_Metadata::numeric_value: " << parameter;
#endif
if (! parameter.value ().is_Numeric ())
	{
	#if ((DEBUG_SECTION) & DEBUG_UTILITIES)
	clog << "    value = " << parameter.value () << endl;
	#endif
	ostringstream
		message;
	message
		<< PDS_Metadata::ID << endl
		<< "Numeric value expected for parameter "
		/* << parameter.pathname () << endl
		<< "but " << parameter.value ().type_name () << " value found."*/;
	throw idaeim::PVL::Invalid_Value (message.str (), -1, PDS_Metadata::ID);
	}
#if ((DEBUG_SECTION) & DEBUG_UTILITIES)
clog << "<<< PDS_Metadata::numeric_value: "
		<< static_cast<double>(parameter.value ()) << endl;
#endif
return static_cast<double>(parameter.value ());
}


std::string
PDS_Metadata::string_value
	(
	const idaeim::PVL::Aggregate&	parameters,
	const std::string&				pathname,
	bool							case_sensitive,
	int								skip
	)
{
Parameter
	*parameter (parameters.find
		(pathname, case_sensitive, skip, idaeim::PVL::Parameter::ASSIGNMENT));
if (! parameter)
	{
	ostringstream
		message;
	message
		<< PDS_Metadata::ID << endl
		<< "Can't find the string value assignment parameter " /*<< pathname*/;
	throw idaeim::Invalid_Argument (message.str (), ID);
	}
return PDS_Metadata::string_value (*parameter);
}


std::string
PDS_Metadata::string_value
	(
	const std::string&				pathname,
	bool							case_sensitive,
	int								skip
	) const
{
Aggregate
	*parameters (metadata ());
if (! parameters)
	{
	ostringstream
		message;
	message 
		<< ID << endl
		<< "No metadata for string value parameter " /*<< pathname*/;
	throw idaeim::Invalid_Argument (message.str (), ID);
	}
return string_value (*parameters, pathname, case_sensitive, skip);
}


std::string
PDS_Metadata::string_value
	(
	const idaeim::PVL::Parameter&	parameter
	)
{
if (parameter.is_Aggregate ())
	{
	ostringstream
		message;
	message
		<< PDS_Metadata::ID << endl
		<< "A string value can not be obtained for parameter "
		/* < parameter.pathname () << endl
		<< "because it an Aggregate (" << parameter.type_name () << ")."*/;
	throw idaeim::PVL::Invalid_Value (message.str (), -1, PDS_Metadata::ID);
	}
if (parameter.value ().is_Array ())
	{
	ostringstream
		message;
	message
		<< PDS_Metadata::ID << endl
		<< "A string value can not be obtained for parameter "
		/*<< parameter.pathname () << endl
		<< "because it has an Array (" << parameter.value ().type_name ()
			<< ") value."*/;
	throw idaeim::PVL::Invalid_Value (message.str (), -1, PDS_Metadata::ID);
	}
return static_cast<std::string>(parameter.value ()).c_str ();
}


std::string
PDS_Metadata::absolute_pathname
	(
	const std::string&	pathname
	)
{
char
	delimiter = Parameter::path_delimiter ();
if (pathname.empty () ||
	pathname[0] == delimiter)
	return pathname;
std::string
	absolute_path (1, delimiter);
return absolute_path += pathname;
}


idaeim::PVL::Lister*
PDS_Metadata::metadata_lister ()
{
Lister
	*lister (new Lister);
(*lister)
	.strict					(true)			
	.begin_aggregates		(false)
	.uppercase_aggregates	(true)
	.name_end_aggregates	(true)
	.assign_align			(true)
	.array_indenting		(false)
	.use_statement_delimiter(false)
	.single_line_comments	(true);
return lister;
}


}	//	namespace HiRISE
}	//	namespace UA
