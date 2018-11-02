/*	Save_Image_Thread

HiROC CVS ID: $Id: Save_Image_Thread.hh,v 1.6 2012/06/15 01:16:07 castalia Exp $

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

#ifndef HiView_Save_Image_Thread_hh
#define HiView_Save_Image_Thread_hh

#include	<QThread>
#include	<QMutex>
#include	<QString>

//	Forward renferences.
class QDialog;
class QHBoxLayout;
class QLabel;
class QMessageBox;

namespace UA
{
namespace HiRISE
{
//	Forward references.
class Plastic_Image;
class Activity_Indicator;

/**	A <i>Save_Image_Thread</i> is a QThread subclass that saves a
	Plastic_Image to a file.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.6 $
*/
class Save_Image_Thread
:	public QThread
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


//!	The {@link save_sequence() save sequence} values.
enum Save_Sequence
	{
	SAVE_IMAGE_DONE,
	SAVE_IMAGE_RENDER,
	SAVE_IMAGE_WRITE
	};

/*==============================================================================
	Constructors
*/
Save_Image_Thread (QObject* parent = NULL);

virtual ~Save_Image_Thread ();

/*==============================================================================
	Accessors
*/
/*	Specify the image and the file pathname and image file format to
	be used when saving.

	@param	plastic_image	A pointer to the Plastic_Image to be saved.
		If NULL, the current {@link image() image} will be used.
	@param	pathname	The pathname to the file where the image is to
		be saved. If empty the current {@link image_pathname() pathname}
		will be used.
	@param	format	The name of the image file format to use when saving
		the file. If empty the current {@link image_format() format} will
		be used.
	@return	true if the required Plastic_Image, image file pathname
		and format are all valid, whether they were set by the current
		method call or not. By calling this method with a single NULL
		argument one can test if the Save_Image_Thread has all the
		required information without affecting any information that has
		already been set. false will be returned if any required information
		is invalid.
	@see	invalidate()
	@see	save_image(Plastic_Image*, const QString&, const QString&)
*/
bool image (Plastic_Image* plastic_image,
	const QString& pathname = QString (), const QString& format = QString ());

inline Plastic_Image* image () const
	{return Image;}
inline QString image_pathname () const
	{return Pathname;}
inline QString image_format () const
	{return Format;}

/**	Invalidate any previous image save information.

	The {@link image() image} is set to NULL and the {@link image_pathname()
	pathname} and {@link image_format() format} are cleared.

	@see	image(Plastic_Image*, const QString&, const QString&)
*/
void invalidate ();

/**	Get the current sequence state for the save operation.

	The thread will be in one of three {@link #Save_Sequence states}:
<dl>
<dt>SAVE_IMAGE_DONE
	<dd>The thread is done with the save operation; or no save operation
		has yet been started. The thread may still be running while it is
		in this state.
<dt>SAVE_IMAGE_RENDER
	<dd>The {@link image() image} is being updated which causes all
		rendering operations pending for the image, if any, to be
		applied. The save operation may be successfully {@link cancel()
		canceled} during this sequence.
<dt>SAVE_IMAGE_WRITE
	<dd>If the image update rendering succeeded the image is written
		to the file at the current {@link image_pathname() pathname} using
		the {@link image_format() image file format}. The save operation
		may not be canceled during this sequence.
</dl>

	@return	A Save_Sequence value.
*/
Save_Sequence save_sequence () const;

/**	Get the completion status of the last image save operation.

	@return	true if the last image save operation completed successfully;
		false if no save operation has yet been started; no image,
		pathname or format was specified; the last save operation was
		canceled before image rendering updated completed, or the image
		rendering update failed; or writing the image file failed.
	@see	save_image(Plastic_Image*, const QString&, const QString&)
	@see	failure_message()
*/
bool image_saved () const;

QString failure_message () const;

/*==============================================================================
	Thread run
*/
/**	Display a save activity dialog and start the thread.

	This method provides a modeless save activity dialog with the name of
	image file format and pathname along with an activity inidicator that
	can be clicked to {@link cancel() cancel} the operation. After
	showing the dialog the thread is started to save the image that was
	specified in the constructor to the pathname with the image file
	format specified.

	<b>N.B.</b>: Using the thread's start method will also save the image
	to the pathname with the specified image file format, but in that
	case no activity dialog will be shown.

	This method returns once the thread has been started. If the thread
	is already running nothing is done.

	@param	plastic_image	A pointer to the Plastic_Image to be saved.
		If NULL, the current {@link image() image} will be used.
	@param	pathname	The pathname to the file where the image is to
		be saved. If empty the current {@link image_pathname() pathname}
		will be used.
	@param	format	The name of the image file format to use when saving
		the file. If empty the current {@link image_format() format} will
		be used.
	@return	true if the save image thread was started; false otherwise.
	@see	image(Plastic_Image*, const QString&, const QString&)
*/
bool save_image (Plastic_Image* plastic_image = NULL,
	const QString& pathname = QString (), const QString& format = QString ());

protected:

/**	Begin running the thread.

	The {@link saved() saved} flag is set to false. If no {@link image()
	image} has been specified or either the pathname or image file format
	are empty, nothing is done. Otherwise the image save operation is
	executed.

	The image save operation includes two {@link save_sequence()
	sequences}:
<dl>
<dt>SAVE_IMAGE_RENDER
The image is
	first updated which causes all rendering operations pending for
	the image to be applied. If this succeeds 
*/
virtual void run ();

/*==============================================================================
	Qt signals
*/
public:

signals:

/**	Signals that the image save operation is done.

	@param	completed	true if the image was {@link saved() saved};
		otherwise false.
*/
void done (bool completed);

/**	Signals the completion status of a {@link cancel() cancel} request.

	@param	completed	true if the saving operation was canceled or was
		not active; false if the image file is being written which can
		not be canceled.
*/
void canceled (bool completed);

/*==============================================================================
	Qt slots
*/
public slots:

bool show_dialog (bool enabled);

/**	Cancel the thread's image save operation.

	If the thread is not running nothing is done.

	If the thread is running and it is in the {@link #SAVE_IMAGE_RENDER}
	{@link save_sequence() save sequence} the image rendering is canceled.
	While the thread is in the {@link #SAVE_IMAGE_WRITE} sequence it can
	not be canceled.

	The {@link canceled(bool) canceled} signal is emitted with the
	completion status. If image saving was started by the {@link
	save_image(Plastic_Image*, const QString&, const QString&) save_image}
	method the activity dialog will be stopped and hidden when
	the thread is finished. If, however, the image file is being written
	an information dialog noting this is shown along with the activity
	dialog until the thread is finished (or the user dismisses the
	information dialog).

	@return true if the saving operation was canceled or was not active;
		false if the image file is being written which can not be canceled.
*/
bool cancel ();


private slots:

void thread_finished ();
void thread_terminated ();
void save_canceled (bool);

/*==============================================================================
	Data
*/
private:

Plastic_Image
	*Image;

QString
	Pathname,
	Format;

QDialog
	*Dialog;
QHBoxLayout
	*Dialog_Layout;
Activity_Indicator
	*Activity;
QLabel
	*Format_Label,
	*Pathname_Label;

QMessageBox
	*Message;

mutable QMutex
	Sequence_Lock;
Save_Sequence
	Sequence;

bool
	Saved;

QString
	Failure_Message;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
