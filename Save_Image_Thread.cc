/*	Save_Image_Thread

HiROC CVS ID: $Id: Save_Image_Thread.cc,v 1.8 2012/06/15 01:16:07 castalia Exp $

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

#include	"Save_Image_Thread.hh"

#include	"Plastic_Image.hh"
#include	"Activity_Indicator.hh"
#include	"HiView_Utilities.hh"

#include	<QDialog>
#include	<QHBoxLayout>
#include	<QLabel>
#include	<QMessageBox>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_SLOTS				(1 << 1)
#define DEBUG_SIGNALS			(1 << 2)
#define	DEBUG_RUN				(1 << 3)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Save_Image_Thread::ID =
		"UA::HiRISE::Save_Image_Thread ($Revision: 1.8 $ $Date: 2012/06/15 01:16:07 $)";

/*==============================================================================
	Constructors
*/
Save_Image_Thread::Save_Image_Thread
	(
	QObject*	parent
	)
	:	QThread (parent),
		Dialog (NULL),
		Message (NULL),
		Saved (false)
{
setObjectName ("Save_Image_Thread");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Save_Image_Thread" << endl;
#endif
connect (this,
	SIGNAL (finished ()),
	SLOT (thread_finished ()));
/*connect (this,
	SIGNAL (terminated ()),
	SLOT (thread_terminated ()));*/
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Save_Image_Thread" << endl;
#endif
}


Save_Image_Thread::~Save_Image_Thread ()
{
cancel ();

if (Dialog)
	delete Dialog;
}

/*==============================================================================
	Accessors
*/
Save_Image_Thread::Save_Sequence
Save_Image_Thread::save_sequence () const
{
QMutexLocker
	sequence_lock (&Sequence_Lock);
return Sequence;
}


bool
Save_Image_Thread::image_saved () const
{
QMutexLocker
	sequence_lock (&Sequence_Lock);
return Saved;
}


QString
Save_Image_Thread::failure_message () const
{
QMutexLocker
	sequence_lock (&Sequence_Lock);
return Failure_Message;
}


void
Save_Image_Thread::invalidate ()
{
QMutexLocker
	sequence_lock (&Sequence_Lock);
Image = NULL;
Pathname.clear ();
Format.clear ();
}

/*==============================================================================
	Run the thread
*/
void
Save_Image_Thread::run ()
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << ">>> Save_Image_Thread::run" << endl));
#endif
Sequence_Lock.lock ();
Saved = false;
Failure_Message.clear ();

if (Image &&
	! Pathname.isEmpty () &&
	! Format.isEmpty ())
	{
	setTerminationEnabled (true);

	Sequence = SAVE_IMAGE_RENDER;
	//	Copy so the user can change them while the thread is running.
	Plastic_Image
		*plastic_image = Image;
	QString
		pathname (Pathname),
		format (Format),
		message;
	#if ((DEBUG_SECTION) & DEBUG_RUN)
	LOCKED_LOGGING ((
	clog << "    Save_Image_Thread::run: SAVE_IMAGE_RENDER" << endl));
	#endif
	//	Release the sequence lock while rendering the image.
	Sequence_Lock.unlock ();

	//	Slight pause to enable widgets to be shown.
	msleep (1000);

	bool
		saved = false,
		rendered = false;
	try {rendered = plastic_image->update ();}
	catch (Plastic_Image::Render_Exception& except)
		{message = QString::fromStdString (except.message ());}
	catch (...)
		{message = tr ("Unexpected exception!");}

	Sequence_Lock.lock ();
	if (Sequence == SAVE_IMAGE_DONE)
		{
		//	User canceled.
		if (message.isEmpty ())
			message = tr ("Image save canceled during rendering.");
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		LOCKED_LOGGING ((
		clog << "    " << message << endl));
		#endif
		rendered = false;
		}

	if (rendered)
		{
		Sequence = SAVE_IMAGE_WRITE;
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		LOCKED_LOGGING ((
		clog << "    Save_Image_Thread::run: SAVE_IMAGE_WRITE" << endl));
		#endif
		//	Release the sequence lock while writing the image.
		Sequence_Lock.unlock ();
		if (! (saved = plastic_image->save (pathname, format.toLatin1 ())))
			message = tr ("Writing the image file did not succeed.");

		Sequence_Lock.lock ();
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		LOCKED_LOGGING ((
		clog << "    Save_Image_Thread::run: SAVE_IMAGE_DONE" << endl));
		#endif
		}
	else
		{
		if (message.isEmpty ())
			message = tr ("Image rendering did not complete.");
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		LOCKED_LOGGING ((
		clog << "    " << message << endl));
		#endif
		}

	//	Still locked.
	Sequence = SAVE_IMAGE_DONE;
	Saved = saved;
	if (! message.isEmpty ())
		Failure_Message = message;
	}
else
	{
	if (! Image)
		Failure_Message = tr ("No image to save has been specified.");
	if (Pathname.isEmpty ())
		{
		if (! Failure_Message.isEmpty ())
			Failure_Message += '\n';
		Failure_Message = tr ("No save file pathname has been specified.");
		}
	if (Format.isEmpty ())
		{
		if (! Failure_Message.isEmpty ())
			Failure_Message += '\n';
		Failure_Message += tr ("No image file format has been specified.");
		}
	#if ((DEBUG_SECTION) & DEBUG_RUN)
	LOCKED_LOGGING ((
	clog << "    " << Failure_Message << endl));
	#endif
	}

Sequence_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "<<< Save_Image_Thread::run" << endl));
#endif
}


bool
Save_Image_Thread::image
	(
	Plastic_Image*		plastic_image,
	const QString&		pathname,
	const QString&		format
	)
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
clog << ">>> Save_Image_Thread::image:" << endl
	 << "    plastic_";
if (plastic_image)
	clog << *plastic_image << endl;
else
	clog << "image NULL" << endl;
clog << "    pathname = " << pathname << endl
	 << "    format = " << format << endl;
#endif
QMutexLocker
	sequence_lock (&Sequence_Lock);
if (plastic_image)
	Image = plastic_image;
if (! pathname.isEmpty ())
	Pathname = pathname;
if (! format.isEmpty ())
	Format = format;

#if ((DEBUG_SECTION) & DEBUG_RUN)
clog << "    current save info -" << endl
	 << "    plastic_";
if (Image)
	clog << *Image << endl;
else
	clog << "image NULL" << endl;
clog << "    Pathname = " << Pathname << endl
	 << "    Format = " << Format << endl;
#endif
bool
	ready =
		(Image &&
		 ! Pathname.isEmpty () &&
		 ! Format.isEmpty ());
#if ((DEBUG_SECTION) & DEBUG_RUN)
clog << "<<< Save_Image_Thread::image: " << boolalpha << ready << endl;
#endif
return ready;
}


bool
Save_Image_Thread::save_image
	(
	Plastic_Image*		plastic_image,
	const QString&		pathname,
	const QString&		format
	)
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
clog << ">>> Save_Image_Thread::save_image:" << endl;
#endif
bool
	started = false;

//	Update the image save information.
if (image (plastic_image, pathname, format) &&
	! isRunning ())
	{
	if (! Dialog)
		{
		#if ((DEBUG_SECTION) & DEBUG_RUN)
		clog << "    Dialog construction ..." << endl;
		#endif
		Dialog = new QDialog;
		Dialog->setWindowTitle (tr ("Saving Image"));
		Dialog->setModal (false);
		Dialog->setMinimumSize (100, 50);
		Dialog_Layout = new QHBoxLayout (Dialog);
		Dialog_Layout->addWidget (Activity = new Activity_Indicator,
			0, Qt::AlignCenter);
		Dialog_Layout->addWidget (Format_Label = new QLabel,
			1, Qt::AlignLeft | Qt::AlignVCenter);
		Dialog_Layout->addWidget (Pathname_Label = new QLabel,
			100, Qt::AlignLeft | Qt::AlignVCenter);
		connect (Activity,
			SIGNAL (button_clicked (int)),
			 SLOT (cancel ()));
		connect (this,
			SIGNAL (canceled (bool)),
			SLOT (save_canceled (bool)));
		}
	Format_Label->setText (Format + tr (" file:"));

	QString
		pathname (wrapped_pathname (Pathname, 0, Dialog->font ()));
	Qt::Alignment
		alignment = Qt::AlignLeft | Qt::AlignVCenter;
	if (pathname.contains ('\n'))
		alignment = Qt::AlignLeft | Qt::AlignTop;
	Pathname_Label->setText (pathname);
	Dialog_Layout->setAlignment (Pathname_Label, alignment);

	Activity->start_delay (0);
	Dialog->show ();
	Activity->state (Activity_Indicator::STATE_1);

	//	Start the thread.
	#if ((DEBUG_SECTION) & DEBUG_RUN)
	clog << "    start ..." << endl;
	#endif
	start ();
	started = true;
	}
#if ((DEBUG_SECTION) & DEBUG_RUN)
clog << ">>> Save_Image_Thread::save_image:" << boolalpha << started << endl;
#endif
return started;
}

/*==============================================================================
	Qt slots
*/
bool
Save_Image_Thread::show_dialog
	(
	bool	enabled
	)
{
bool
	shown = false;
if (Dialog)
	{
	if (enabled)
		{
		if (save_sequence () != SAVE_IMAGE_DONE)
			{
			Dialog->show ();
			shown = true;
			}
		}
	else
		{
		if (Message &&
			Message->isVisible ())
			Message->hide ();
		Dialog->hide ();
		}
	}
return shown;
}

void
Save_Image_Thread::thread_finished ()
{
if (Dialog)
	{
	Activity->state (Activity_Indicator::STATE_OFF);
	if (Message &&
		Message->isVisible ())
		Message->hide ();

	Dialog->hide ();
	}
//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_RUN | DEBUG_SIGNALS))
LOCKED_LOGGING ((
clog << "^^^ Save_Image_Thread::thread_finished: "
		  "emit done " << boolalpha << Saved << endl));
#endif
emit done (Saved);
}


void
Save_Image_Thread::thread_terminated ()
{
if (Dialog &&
	Dialog->isVisible ())
	{
	Activity->state (Activity_Indicator::STATE_OFF);
	Dialog->hide ();
	}

Sequence_Lock.lock ();
Sequence = SAVE_IMAGE_DONE;
Sequence_Lock.unlock ();

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_RUN | DEBUG_SIGNALS))
LOCKED_LOGGING ((
clog << "^^^ Save_Image_Thread::thread_terminated: "
		  "emit done false " << endl));
#endif
emit done (false);
}


bool
Save_Image_Thread::cancel ()
{
#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << ">>> Save_Image_Thread::cancel" << endl));
#endif
bool
	completed = true;
if (isRunning ())
	{
	/*	Hold the Sequence_Lock
		which prevents the thread from advancing from rendering to writing
		and prevents the thread from hold the lock while terminated.
	*/
	Sequence_Lock.lock ();

	if (Sequence == SAVE_IMAGE_RENDER)
		{
		//	Cancel rendering in progress.
		Image->cancel_update ();

		//	Let the thread know it's done.
		Sequence = SAVE_IMAGE_DONE;
		}
	else
	if (Sequence == SAVE_IMAGE_WRITE)
		//	Can't cancel file writing.
		completed = false;

	Sequence_Lock.unlock ();
	}

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_RUN | DEBUG_SIGNALS))
LOCKED_LOGGING ((
clog << "^^^ Save_Image_Thread::cancel: "
		  "emit canceled " << boolalpha << completed << endl));
#endif
emit canceled (completed);

#if ((DEBUG_SECTION) & DEBUG_RUN)
LOCKED_LOGGING ((
clog << "<<< Save_Image_Thread::cancel" << boolalpha << completed << endl));
#endif
return completed;
}


void
Save_Image_Thread::save_canceled
	(
	bool	completed
	)
{
if (! completed &&
	Dialog)
	{
	if (! Message)
		{
		Message =
			new QMessageBox (QMessageBox::Information, tr ("Writing Image"),
				tr ("The image file is being written.\nPlease wait ...."),
				QMessageBox::Ok, Dialog);
		Message->setModal (false);
		}
	Message->show ();
	}
}


}	//	namespace UA::HiRISE
