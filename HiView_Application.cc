/*	HiView_Application

HiROC CVS ID: $Id: HiView_Application.cc,v 1.9 2018/11/05 18:40:31 guym Exp $

Copyright (C) 2011  Arizona Board of Regents on behalf of the
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

#include	"HiView_Utilities.hh"
using namespace UA::HiRISE;

#include	"HiView_Application.hh"
#include <QApplication>
#include	<QFile>
#include	<QFileOpenEvent>
#include <QResizeEvent>
#include	<QTextStream>
#include	<QUrl>
#include	<QStringRef>
#include	<QString>
#ifdef __APPLE__
#include <QPalette>
#include <QStyleFactory>
#endif

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SIGNALS			(1 << 1)
#define DEBUG_HELPERS			(1 << 2)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include        <QDebug>

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	HiView_Application::ID =
		"UA::HiRISE::HiView_Application ($Revision: 1.9 $ $Date: 2018/11/05 18:40:31 $)";

/*==============================================================================
	Constructor
*/
HiView_Application::HiView_Application
	(
	int&	argc,
	char**	argv
	)
	:	QApplication (argc, argv)
{
#ifdef __APPLE__
QApplication::setStyle(QStyleFactory::create("Fusion"));
QPalette p;
p = qApp->palette();
/*p.setColor(QPalette::Window, QColor(53,53,53));
p.setColor(QPalette::Button, QColor(53,53,53));
p.setColor(QPalette::Highlight, QColor(142,45,197));
p.setColor(QPalette::ButtonText, QColor(255,255,255));
p.setColor(QPalette::WindowText, QColor(255,255,255));
*/
setPalette(p);
#endif
setObjectName ("HiView_Application");
//QDesktopWidget* widget = this->desktop();
//widget->installEventFilter(new NullEventFilter());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< HiView_Application" << endl;
#endif
}


/*==============================================================================
	Utilities
*/
bool
HiView_Application::is_jpip_passthru_link
	(
	const QString& input
	)
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> jpip_passthru_link:" << input << endl;
#endif

if (!input.isEmpty() && ! HiView_Utilities::is_URL (input) &&
	input.endsWith (JPIP_PASSTHRU_LINK_EXT, Qt::CaseInsensitive)
	)
	{
	    #if ((DEBUG_SECTION) & DEBUG_HELPERS)
	    clog << "<<< jpip_passthru_link: true" << endl;
	    #endif
	    return true;
	}
	#if ((DEBUG_SECTION) & DEBUG_HELPERS)
	clog << "<<< jpip_passthru_link: false" << endl;
	#endif
	return false;
}

QString
HiView_Application::parse_jpip_passthru_link
	(
	const QString& input
	)
{
    QFile meta(input);
    if (meta.open(QFile::ReadOnly) )
        {
            QTextStream stream(&meta);

            return stream.readLine(MAX_JPIP_PASSTHRU_LINK_SIZE);
        }

    return QString ();
}

/*==============================================================================
	Qt events:
*/

#ifdef DEBUG_SECTION
bool HiView_Application::notify(QObject* receiver, QEvent* event)
{
     try
     {
         return QApplication::notify(receiver, event);
     }
     catch(std::exception& e)
     {
        qDebug() << "Exception thrown:" << e.what() << " on " << receiver->objectName() << " from event type " << event->type();
        receiver->dumpObjectInfo();
        exit(-1);
     }

     return true;
}
#endif

bool
HiView_Application::event
	(
	QEvent*	event
	)
{
if (event->type () == QEvent::FileOpen)
	{
    QFileOpenEvent
		*file_open_event = static_cast<QFileOpenEvent*>(event);
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
	clog << ">-< HiView_Application::event: QFileOpenEvent -" << endl
		 << "    file = \"" << file_open_event->file () << '"' << endl
		 << "     url = \"" << file_open_event->url ().toString () << '"'
		 	<< endl
		 << "^^^ HiView_Application::event: emit file_open_request "
			<< file_open_event->file () << endl;
	#endif

    if (!file_open_event->file().isEmpty())
        Requested_Pathname = file_open_event->file();
    else if (!file_open_event->url().isEmpty())
        Requested_Pathname = file_open_event->url().toString();
    else
        return false;

	if (is_jpip_passthru_link(Requested_Pathname))
	{
	    QString Requested_Link = parse_jpip_passthru_link(Requested_Pathname);
	    if (! Requested_Link.isEmpty() ) Requested_Pathname = Requested_Link;
	}

    emit file_open_request (Requested_Pathname);
    return true;
	}
return QApplication::event (event);
}


}	//	namespace UA::HiRISE
