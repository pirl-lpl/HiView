/*	HiView

HiROC CVS ID: $Id: HiView.cc,v 1.37 2014/05/27 17:13:51 guym Exp $

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

#include	"HiView_Application.hh"
#include	"HiView_Window.hh"
#include	"HiView_Utilities.hh"
using namespace UA::HiRISE;

#include	<QSizeF>
#include	<QString>
#include	<QStringRef>
#include	<QStyleFactory>
#include <QImageReader>
#include <QDesktopWidget>

#include	<string>
using std::string;
#include	<iostream>
using std::cout;
#include	<iomanip>
using std::endl;
#include	<fstream>
using std::ifstream;
#include	<vector>
using std::vector;
#include	<cstring>
using std::strlen;
using std::memmove;
using std::strcpy;


#if defined (DEBUG_SECTION)
//	DEBUG_SECTION controls.

#define DEBUG_OFF			0
#define DEBUG_ALL			-1
#define DEBUG_COMMAND_LINE	(1 << 0)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
using std::clog;
#endif

#endif	//	DEBUG_SECTION

/*==============================================================================
	Constants
*/
#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)		"" #__VA_ARGS__ ""
#define AS_STRING(...)			STRINGIFIED(__VA_ARGS__)
#endif


#ifndef APP_NAME
#define APP_NAME				HiView
#endif
#define APP_NAME_STRING			AS_STRING(APP_NAME)
//!	Application name.
extern const char* const
	APPLICATION_NAME			= APP_NAME_STRING;

#ifndef ORG_NAME
#define ORG_NAME				UA_HiRISE
#endif
#define ORG_NAME_STRING			AS_STRING(ORG_NAME)
//!	Organization name.
extern const char* const
	ORGANIZATION_NAME			= ORG_NAME_STRING;

#ifndef MODULE_VERSION
#define MODULE_VERSION_STRING	""
#define _VERSION_ " "
#else
#define MODULE_VERSION_STRING	AS_STRING(MODULE_VERSION)
#define _VERSION_ " v" MODULE_VERSION_STRING ", "
#endif
//!	Application version identification.
extern const char* const
	APPLICATION_VERSION			= MODULE_VERSION_STRING;

//!	Application identification with source code version and date.
extern const char* const
	APPLICATION_ID =
		APP_NAME_STRING
		_VERSION_ __DATE__
		" ($Revision: 1.37 $ $Date: 2014/05/27 17:13:51 $)";


#ifndef	MACHINE
#define MACHINE_STRING			""
#else
#define MACHINE_STRING			AS_STRING(MACHINE)
#endif
//!	Host machine identification (typically an architecture name).
extern const char* const
	HOST_MACHINE				= MACHINE_STRING;

#ifndef OS
#define HOST_OS_STRING			""
#else
#define HOST_OS_STRING			AS_STRING(OS)
#endif
//!	Host operating system identification.
extern const char* const
	HOST_OS						= HOST_OS_STRING;

//!	The runtime command name.
char
	*Command_Name;

//!	Exit status values.
const int
	SUCCESS						= 0,

	//	Command line syntax.
	BAD_SYNTAX					= 1;

/*==============================================================================
	Usage
*/
/**	Print the application usage description and exit.

	The usage description is printed to the standard output stream.

	@param	exit_status	The exit status value,
	@param	list_descriptions	If true usage details will be included in
		the output. If false a brief usage description will be provided.
*/
void
usage
	(
	int		exit_status = BAD_SYNTAX,
	bool	list_descriptions = false
	)
{
cout
	<< "Usage: " << Command_Name << " [options] [[-Image] <source name>]" << endl;
if (list_descriptions)
	cout
	<< endl
	<< "Viewer for images obtained from local file or remote server sources." << endl
	<< endl;

cout << "[-Image] <source name>" << endl;
if (list_descriptions)
	cout
	<< "    The image source name may be the pathname to a local image file" << endl
	<< "    or a URL to a remote image file. The image source data may be in" << endl
	<< "    various common formats - such as JPEG, PNG, etc. - as well as a" << endl
	<< "    JP2 encapsulated JPEG2000 codestream. If a URL is specified for a" << endl
	<< "    JP2 file the protocol is expected to be \"jpip\"." << endl
	<< endl
	<< "    Default: An internal application image is displayed, or the" << endl
	<< "    last viewed image is displayed if session restoration is enabled." << endl
	<< endl;

cout << "-SCale <horizontal>[,<vertical>]" << endl;
if (list_descriptions)
	cout
	<< "    The initial scaling factor(s) will be applied when the specified" << endl
	<< "    image source is displayed. If no image source name is specified" << endl
	<< "    any scaling factors are ignored. If only one scaling factor is" << endl
	<< "    specified it will apply to both horizontal and vertical scaling." << endl
	<< "    The scaling factors are decimal values relative to 1.0 for full" << endl
	<< "    resolution display; 0.5 will display the image at half size, 2.5" << endl
	<< "    will display the image at two and half times normal size, etc." << endl
	<< endl
	<< "    Default: 1.0" << endl
	<< endl;

cout << "-[No_]Restore" << endl;
if (list_descriptions)
	cout
	<< "    Do (not) restore the GUI layout geometry that was saved from the" << endl
	<< "    last time HiView was used. This option overrides the corresponding" << endl
	<< "    General Preferences settings." << endl
	<< endl
	<< "    If No_Restore is specified neither the GUI layout geometry nor the" << endl
	<< "    last source viewed are restored; i.e. the preferences settings are" << endl
	<< "    ignored in this case. If Restore is specified the Restore Geometry" << endl
	<< "    preferences setting is ignored but the Restore Last Source setting" << endl
	<< "    is used." << endl
	<< endl
	<< "    Default: The GUI layout geometry is restored if available and as" << endl
	<< "    the General Preferences settings specify." << endl
	<< endl;

cout << "-STYLE <style> | -STYLES" << endl;
if (list_descriptions)
	cout
	<< "    Sets the application GUI style. Possible values are \"platinum\"," << endl
	<< "    \"motif\" and \"windows\". Some platforms may support additional" << endl
	<< "    styles; use the -styles option to get a list of available styles." << endl
	<< endl
	<< "    Default: The default style for the platform being used." << endl
	<< endl;

cout << "-Version" << endl;
if (list_descriptions)
	cout
	<< "    List the application version identification and exit." << endl
	<< endl
	<< "    Default: No version identification." << endl
	<< endl;

cout << "-Help" << endl;
if (list_descriptions)
	cout
	<< "    Print this help description and exit." << endl
	<< endl;

exit (exit_status);
}

/*==============================================================================
	Main
*/
int
main
	(
	int		arg_count,
	char**	arg_list
	)
{
Command_Name = *arg_list;
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
clog << Command_Name << ": " << APPLICATION_ID << endl
	 << HOST_OS << ' ' << HOST_MACHINE << endl;
#endif

//	Initialize application resources and Qt run-time environment.
Q_INIT_RESOURCE (HiView);	//	Can't use APP_NAME in this macro.

//	Application object.
HiView_Application
	application (arg_count, arg_list);

//QDesktopWidget* widget = application.desktop();
//widget->installEventFilter(new NullEventFilter());

//	Application identification.
application.setApplicationName (APPLICATION_NAME);
application.setOrganizationName (ORGANIZATION_NAME);
application.setApplicationVersion (APPLICATION_VERSION);
application.setWindowIcon (QIcon (":/Images/HiView_Icon.png"));

QString
	Source_Name;
QSizeF
	Scaling;
HiView_Window::Layout_Restoration
	Restore_Layout = HiView_Window::PREFERENCES_RESTORE_LAYOUT;

for (int
		count = 1;
		count < arg_count;
		count++)
	{
	#if ((DEBUG_SECTION) & DEBUG_COMMAND_LINE)
	clog << "    " << count << ": " << arg_list[count] << endl;
	#endif
	if (arg_list[count][0] == '-')
		{
		switch (toupper (arg_list[count][1]))
			{
			case 'I':	//	-Image
				if (++count == arg_count ||
					arg_list[count][0] == '-')
					{
					cout << "Missing image source name." << endl
						 << endl;
					usage ();
					}

				Image_Input:
				if (! Source_Name.isEmpty ())
					{
					cout << "More than one image source specified:" << endl
						 << Source_Name << endl
						 << "and" << endl
						 << arg_list[count] << endl;
					usage ();
					}
				Source_Name = arg_list[count];
				break;

			case 'S':
				if (toupper (arg_list[count][2]) == 'C')
					{
					//	-SCale
					if (++count == arg_count)
						{
						cout << "Missing scaling factor(s)." << endl;
						usage ();
						}

					char
						*last_character;
					double
						scale = strtod (arg_list[count], &last_character);
					if (*last_character &&
						*last_character != ',')
						{
						Bad_Scale:
						cout << "A scale factor was expected for the "
								<< arg_list[count - 1] << " option but \""
								<< arg_list[count] << "\" found." << endl;
						usage ();
						}
					Scaling.rwidth () = scale;
					if (*last_character &&
						*(++last_character))
						{
						scale = strtod (last_character, &last_character);
						if (*last_character)
							goto Bad_Scale;
						}
					Scaling.rheight () = scale;
					}
				else
				if (QString (arg_list[count])
						.compare ("-STYLES", Qt::CaseInsensitive) == 0)
					{
					//	-STYLES
					cout << "Supported styles -" << endl;
					QStringList
						styles (QStyleFactory::keys ());
					for (int
							index = 0;
							index < styles.size ();
							index++)
						cout << "  " << styles.at (index) << endl;
					exit (SUCCESS);
					}
				else
					goto Unknown_Option;
				break;

			case 'F': // -Formats
			    foreach (const QByteArray& entry, QImageReader::supportedImageFormats ())
			    {
			        cout << QString (entry) << ' ';
			    }
			    cout << endl;

			    if (! QImageReader::supportedImageFormats ().contains ("PDS"))
			    {
			        cout << "The PDS plugin was not found." << endl;
			    }
	        		exit (SUCCESS);

			case 'N':	//	-No_Restore
				Restore_Layout = HiView_Window::DO_NOT_RESTORE_LAYOUT;
				break;

			case 'R':	//	-Restore
				Restore_Layout = HiView_Window::RESTORE_LAYOUT;
				break;

			case 'H':	//	-Help
				usage (SUCCESS, true);

			case 'V':	//	-Version
				cout << APPLICATION_ID << endl
					 << "Build system: "
					 	<< HOST_OS << ' ' << HOST_MACHINE << endl;
				exit (SUCCESS);

			default:
				Unknown_Option:
				cout << "Unrecognized command line argument: "
						<< arg_list[count] << endl;
				usage ();
			}
		}
	else
		goto Image_Input;
	}

if (Source_Name.isEmpty ())
	//	Use any Requested_Pathname from a HiView_Application FileOpen event.
	Source_Name = application.Requested_Pathname;

if (HiView_Application::is_jpip_passthru_link(Source_Name))
{
   QString Requested_Link = HiView_Application::parse_jpip_passthru_link(Source_Name);
   if (! Requested_Link.isEmpty() ) Source_Name = Requested_Link;
}

//	Construct the main application window and display it.
HiView_Window
	main_window (Source_Name, Scaling, Restore_Layout);

//	Run the application event loop.
return application.exec ();
}
