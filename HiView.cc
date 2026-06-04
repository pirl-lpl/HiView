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

#include "HiView_Application.hh"
#include "HiView_Utilities.hh"
#include "HiView_Window.hh"
using namespace UA::HiRISE;

#include <QImageReader>
#include <QSizeF>
#include <QString>
#include <QStyleFactory>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
using std::ifstream;
#include <vector>
using std::vector;
#include <cstring>
using std::memmove;
using std::strcpy;
using std::strlen;

namespace
{

#ifdef DEBUG_SECTION
//	DEBUG_SECTION controls.

#define DEBUG_OFF 0
#define DEBUG_ALL -1
#define DEBUG_COMMAND_LINE (1 << 0)

#define DEBUG_DEFAULT DEBUG_ALL

#if (DEBUG_SECTION + 0) == 0
#undef DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
using std::clog;
#endif

#endif  //	DEBUG_SECTION

#define AS_STRING(string) #string

#ifndef APP_NAME
#define APP_NAME "HiView"
#endif

#ifndef APP_ORGNAME
#define APP_ORGNAME "University of Arizona Lunar and Planetary Lab"
#endif

#ifndef APP_VERSION
#define APP_VERSION ""
#endif

#ifndef APP_INFO
#define APP_INFO ""
#endif

constexpr const char* NAME = APP_NAME;
constexpr const char* ORGNAME = APP_ORGNAME;
constexpr const char* VERSION = APP_VERSION;
constexpr const char* INFO = APP_INFO;

//!	The runtime command name.
char* Command_Name;

//!	Exit status values.
const int SUCCESS = 0, BAD_SYNTAX = 1;

const char* TEXT_USAGE = R"(
Viewer for images obtained from local file or remote server sources.
)";

const char* TEXT_SOURCE = R"(
The image source name may be the pathname to a local image file
or a URL to a remote image file. The image source data may be in
various common formats - such as JPEG, PNG, etc. - as well as a
JP2 encapsulated JPEG2000 codestream. If a URL is specified for a
JP2 file the protocol is expected to be "jpip".

Default: An internal application image is displayed, or the
last viewed image is displayed if session restoration is enabled.
)";

const char* TEXT_STYLE = R"(
Sets the application GUI style. Platform dependent; use the -styles (plural)
option to get a list of available styles.

Default: The default style for the platform being used.
)";

const char* TEXT_SCALE = R"(
The initial scaling factor(s) will be applied when the specified
image source is displayed. If no image source name is specified
any scaling factors are ignored. If only one scaling factor is
specified it will apply to both horizontal and vertical scaling.

The scaling factors are decimal values relative to 1.0 for full
resolution display; 0.5 will display the image at half size, 2.5
will display the image at two and half times normal size, etc.

Default: 1.0.
)";

const char* TEXT_RESTORE = R"(
Do (not) restore the GUI layout geometry that was saved from the
last time HiView was used. This option overrides the corresponding
General Preferences settings.

If No_Restore is specified neither the GUI layout geometry nor the
last source viewed are restored; i.e. the preferences settings are
ignored in this case. If Restore is specified the Restore Geometry
preferences setting is ignored but the Restore Last Source setting
is used.

Default: The GUI layout geometry is restored if available and as
the General Preferences settings specify.
)";

const char* TEXT_HELP = R"(
Print this help description and exit.
)";

const char* TEXT_VERSION = R"(
List the application version identification and exit.

Default: No version identification.
)";

/*==============================================================================
    Usage
*/
/**	Print the application usage description and exit.

    The usage description is printed to the standard output stream.

    @param	exit_status	The exit status value,
    @param	list_descriptions	If true usage details will be included
   in the output. If false a brief usage description will be provided.
*/
void usage(int exit_status = BAD_SYNTAX, bool list_descriptions = false)
{
    cout << "Usage: " << Command_Name << " [options] [[-Image] <source name>]" << endl;
    if (list_descriptions) cout << TEXT_USAGE << endl;

    cout << "   [-Image] <source name>" << endl;
    if (list_descriptions) cout << TEXT_SOURCE << endl;

    cout << "   -SCale <horizontal>[,<vertical>]" << endl;
    if (list_descriptions) cout << TEXT_SCALE << endl;

    cout << "   -[No_]Restore" << endl;
    if (list_descriptions) cout << TEXT_RESTORE << endl;

    cout << "   -STYLE <style> | -STYLES" << endl;
    if (list_descriptions) cout << TEXT_STYLE << endl;

    cout << "   -Version" << endl;
    if (list_descriptions) cout << TEXT_VERSION << endl;

    cout << "   -Help" << endl;
    if (list_descriptions) cout << TEXT_HELP << endl;

    exit(exit_status);
}

}  // namespace

/*==============================================================================
    Main
*/
int main(int arg_count, char** arg_list)
{
    Command_Name = *arg_list;
#if defined(DEBUG_SECTION) && DEBUG_SECTION != 0
    clog << Command_Name << ": " << APP_NAME << endl << APP_VERSION << ' ' << APP_INFO << endl;
#endif
    // TODO this should not be necessary
    /*	Initialize application resources and Qt run-time environment. */
    Q_INIT_RESOURCE(HiView);  //	Can't use APP_NAME in this macro.

    //	Application object.
    HiView_Application const application(arg_count, arg_list);

    // QDesktopWidget* widget = application.desktop();
    // widget->installEventFilter(new NullEventFilter());

    //	Application identification.
    UA::HiRISE::HiView_Application::setApplicationName(APP_NAME);
    UA::HiRISE::HiView_Application::setOrganizationName(APP_ORGNAME);
    UA::HiRISE::HiView_Application::setApplicationVersion(APP_VERSION);
    UA::HiRISE::HiView_Application::setWindowIcon(QIcon(":/Images/HiView_Icon.png"));

    QString Source_Name;
    QSizeF Scaling;
    HiView_Window::Layout_Restoration Restore_Layout = HiView_Window::PREFERENCES_RESTORE_LAYOUT;

    for (int count = 1; count < arg_count; count++)
    {
#if ((DEBUG_SECTION) & DEBUG_COMMAND_LINE)
        clog << "    " << count << ": " << arg_list[count] << endl;
#endif
        if (arg_list[count][0] == '-')
        {
            switch (toupper(arg_list[count][1]))
            {
                case 'I':  //	-Image
                    if (++count == arg_count || arg_list[count][0] == '-')
                    {
                        cout << "Missing image source name." << endl << endl;
                        usage();
                    }

                Image_Input:
                    if (!Source_Name.isEmpty())
                    {
                        cout << "More than one image source specified:" << endl
                             << Source_Name << endl
                             << "and" << endl
                             << arg_list[count] << endl;
                        usage();
                    }
                    Source_Name = arg_list[count];
                    break;

                case 'S':
                    if (toupper(arg_list[count][2]) == 'C')
                    {
                        //	-SCale
                        if (++count == arg_count)
                        {
                            cout << "Missing scaling factor(s)." << endl;
                            usage();
                        }

                        char* last_character;
                        double scale = strtod(arg_list[count], &last_character);
                        if (*last_character && *last_character != ',')
                        {
                        Bad_Scale:
                            cout << "A scale factor was expected for the " << arg_list[count - 1]
                                 << " option but \"" << arg_list[count] << "\" found." << endl;
                            usage();
                        }
                        Scaling.rwidth() = scale;
                        if (*last_character && *(++last_character))
                        {
                            scale = strtod(last_character, &last_character);
                            if (*last_character) goto Bad_Scale;
                        }
                        Scaling.rheight() = scale;
                    }
                    else if (QString(arg_list[count]).compare("-STYLES", Qt::CaseInsensitive) == 0)
                    {
                        //	-STYLES
                        cout << "Supported styles -" << endl;
                        QStringList const styles(QStyleFactory::keys());
                        for (const auto& style : styles) cout << "  " << style << endl;
                        exit(SUCCESS);
                    }
                    else goto Unknown_Option;
                    break;

                case 'F':  // -Formats
                    foreach (const QByteArray& entry, QImageReader::supportedImageFormats())
                    {
                        cout << QString(entry) << ' ';
                    }
                    cout << endl;

                    if (!QImageReader::supportedImageFormats().contains("PDS"))
                    {
                        cout << "The PDS plugin was not found." << endl;
                    }
                    exit(SUCCESS);

                case 'N':  //	-No_Restore
                    Restore_Layout = HiView_Window::DO_NOT_RESTORE_LAYOUT;
                    break;

                case 'R':  //	-Restore
                    Restore_Layout = HiView_Window::RESTORE_LAYOUT;
                    break;

                case 'H':  //	-Help
                    usage(SUCCESS, true);

                case 'V':  //	-Version
                    cout << APP_NAME << " " << APP_VERSION << " " << endl
                         << "Build system: " << APP_INFO << endl;
                    exit(SUCCESS);

                default:
                Unknown_Option:
                    cout << "Unrecognized command line argument: " << arg_list[count] << endl;
                    usage();
            }
        }
        else goto Image_Input;
    }

    //	Use any Requested_Pathname from a HiView_Application FileOpen
    // event.
    if (Source_Name.isEmpty()) Source_Name = application.requestedPathname();

    if (HiView_Application::is_jpip_passthru_link(Source_Name))
    {
        QString const Requested_Link = HiView_Application::parse_jpip_passthru_link(Source_Name);
        if (!Requested_Link.isEmpty()) Source_Name = Requested_Link;
    }

    //	Construct the main application window and display it.
    try
    {
        HiView_Window const main_window(Source_Name, Scaling, Restore_Layout);
    }
    catch (...)
    {
        cout << "Unhandled exception" << endl;
    }

    //	Run the application event loop.
    return UA::HiRISE::HiView_Application::exec();
}
