/* Tile_Renderer

HiROC CVS ID: $Id: Tile_Renderer.cc,v 1.58 2013/05/22 23:27:23 guym Exp $

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

#include "Tile_Renderer.hh"

#include "Dynamic_Image.hh"
#include "Dynamic_QImage.hh"
#include "JP2_Image.hh"
#include "HiView_Utilities.hh"

// UA::HiRISE::JP2_Reader.
#include "JP2.hh"
#include "JP2_Reader.hh"
#include "JP2_Exception.hh"

using UA::HiRISE::JP2, UA::HiRISE::JP2_Reader, UA::HiRISE::JP2_Exception;

#include <QFileInfo>

#include <cmath>
#include <stdexcept>
using std::exception;
#include <cassert>


namespace UA::HiRISE
{

const int
Tile_Renderer::DO_NOT_WAIT = 0,
Tile_Renderer::WAIT_UNTIL_DONE = (1 << 0),
Tile_Renderer::DELETE_WHEN_DONE = (1 << 1),
Tile_Renderer::FORCE_CANCEL = (1 << 2);

const QPoint
Tile_Renderer::LOW_PRIORITY_RENDERING;

const bool
Tile_Renderer::CANCELABLE = true;


/*------------------------------------------------------------------------------
 Defaults
*/
#ifndef DEFAULT_MIN_SOURCE_IMAGE_AREA
#define DEFAULT_MIN_SOURCE_IMAGE_AREA (1 << 20)
#endif
#ifndef DEFAULT_MAX_SOURCE_IMAGE_AREA
#define DEFAULT_MAX_SOURCE_IMAGE_AREA (2 << 20)
#endif
unsigned long
Tile_Renderer::Default_Min_Source_Image_Area
= DEFAULT_MIN_SOURCE_IMAGE_AREA,
Tile_Renderer::Default_Max_Source_Image_Area
= DEFAULT_MAX_SOURCE_IMAGE_AREA;

#ifndef DEFAULT_WAIT_SECONDS
#define DEFAULT_WAIT_SECONDS 20
#endif
int
Tile_Renderer::Wait_Seconds = DEFAULT_WAIT_SECONDS;

class Tile_Rendering_Monitor : public Dynamic_Image::Rendering_Monitor
{
    private:
    Tile_Renderer* Owner;

    public:
    explicit Tile_Rendering_Monitor(Tile_Renderer* owner = NULL)
    {
        Owner = owner;
    }

    bool notification(Dynamic_Image&, Dynamic_Image::Rendering_Monitor::Status status,
                      const QString& message, const QRect& tile_region)
    {
        if (!Owner) return false;

        // >>> SIGNAL <<<
        Owner->send_status_notice(message);

        if (!Owner->Active_Tile->Tile_Region.isEmpty() && // Visible in display?
            (status & Dynamic_Image::Rendering_Monitor::RENDERED_DATA_MASK))
        {
            // >>> SIGNAL <<<
            Owner->send_rendered(Owner->Active_Tile->Tile_Coordinate, tile_region);
            return true;
        }
    }
}; // Tile_Renderer_Rendering_Monitor

// Signal emitter proxies for the rendering_montitor:
void Tile_Renderer::send_status_notice(const QString& message)
{
    // >>> SIGNAL <<<
    emit status_notice(message);
}

void Tile_Renderer::send_rendered(const QPoint& tile_coordinate, const QRect& tile_region)
{
    // >>> SIGNAL <<<
    emit rendered(tile_coordinate, tile_region);
}

/*==============================================================================
 Constructors
*/
Tile_Renderer::Tile_Renderer
(
    QObject* parent
)
    :
    Suspended(true),
    Runnable(false),
    Finish(false),
    Immediate_Mode(false),
    Source_Image(new Dynamic_QImage()),
    Max_Source_Image_Area(Default_Max_Source_Image_Area),
    Reference_Image(Source_Image->clone(QSize(0, 0))),
    Active_Tile(NULL),
    Cancel(false),
    Image_Rendering_Monitor(new Tile_Rendering_Monitor(this))
{
    setObjectName("Tile_Renderer");
}


Tile_Renderer::~Tile_Renderer()
{
    /* >>> WARNING <<< This Tile_Renderer is destroyed AFTER any derived
    classes. This creates complications for the order in which finish
    operations are done. It is assumed that derived class destructors
    will call this base class finish method. After the base class is
    destroyed it may not be possible to call the virtual finish method,
    and the local finish method will call the virtual start_rendering
    method. So, to avoid this snafu the finish mehtod is only called if
    it has not yet been called (Finish is not set). This has the
    additional benefit of avoiding a potential redundant finish.
    */
    if (!Finish) finish();

    if (Reference_Image)
    {
        delete Reference_Image;
    }

    if (Image_Rendering_Monitor)
    {
        delete Image_Rendering_Monitor;
    }
}

/*==============================================================================
 Accessors
*/
const Tile_Renderer::Shared_Image Tile_Renderer::source_image() const
{
    return Source_Image;
}

Dynamic_Image* Tile_Renderer::reference_image() const
{
    return Reference_Image;
}

void Tile_Renderer::immediate_mode(bool enable)
{
    Immediate_Mode = enable;
}

bool Tile_Renderer::immediate_mode() const
{
    return Immediate_Mode;
}

/*==============================================================================
 Queue management
*/
void Tile_Renderer::queue(Dynamic_Image* image, const QPoint& tile_coordinate,
                          const QRect& tile_region, bool cancelable)
{
    if (!image) return;

    Image_Tile* image_tile = new Image_Tile(image, tile_coordinate, tile_region, cancelable);

    qDebug() << "Queuing tile " << tile_region << " from " << image->source_name();

    int index = Render_Queue.find(image_tile->Image);
    if (index < 0)
    {
        add_tile(image_tile);
    }
    else
    {
        Render_Queue.replace(index, image_tile);
        /*
        // The image is already in the queue.
        if (image_tile->status() == Render_Queue[index]->status() &&
            image_tile->area() == Render_Queue[index]->area())
        {
            // No priority change; replace the current entry.

            // Replace the queue entry with the new tile.
            Render_Queue.replace(index, image_tile);
        }
        else
        {
            // Change of priority; remove current entry and add new tile.
            // Delete the existing tile.
            Render_Queue.remove(index);
            add_tile(image_tile);
        }*/
    }
}


bool Tile_Renderer::is_queued(Dynamic_Image* image) const
{
    return image ? Render_Queue.find(image) >= 0 : Render_Queue.size() != 0;
}

int Tile_Renderer::rendering_status() const
{
    return (Active_Tile) ? Active_Tile->status() : Render_Queue.priorityStatus();
}

bool Tile_Renderer::is_rendering(Dynamic_Image* image) const
{
    return (image && Active_Tile) ? Active_Tile->Image == image : Active_Tile != NULL;
}

/*
bool Tile_Renderer::will_delete(Dynamic_Image* image) const
{
    return Delete_Queue.find(image) >= 0;
}*/

bool Tile_Renderer::cancel(Dynamic_Image* image, int cancel_options)
{
    if (!image) return false;

    bool canceled = true;

    if (Active_Tile && Active_Tile->Image == image)
    {
        // Cancel rendering of the active tile.
        canceled = abort(cancel_options);
    }

    Render_Queue.remove(image);

    return canceled;
}

bool Tile_Renderer::cancel(int cancel_options)
{
    return clear(cancel_options);
}

bool Tile_Renderer::reset(int cancel_options)
{
    stop_rendering();

    return clear(cancel_options);
}

void Tile_Renderer::add_tile(Image_Tile* image_tile)
{
    // >>> CAUTION: The Queue_Lock is expected to be locked.

    // Safety check for the image in the Delete_Queue.
    int index = Render_Queue.find(image_tile->Image);
    if (index >= 0)
    {
        // Retain the Delete_Image_When_Done if set in either tile.
        image_tile->Delete_Image_When_Done |= Render_Queue.getTile(index)->Delete_Image_When_Done;
        // Remove the tile from the Delete_Queue.
        //Delete_Queue.remove(index);
    }

    // Safety check for the image in the Active_Tile.
    if (Active_Tile && Active_Tile->Image == image_tile->Image)
    {
        image_tile->Delete_Image_When_Done |= Active_Tile->Delete_Image_When_Done;
        // Image will be in the Render_Queue.
        Active_Tile->Delete_Image_When_Done = false;
    }

    if (Render_Queue.isEmpty() ||
        image_tile->is_low_priority() ||
        image_tile->Tile_Region.isEmpty())
    {
        // Empty queue or low priority tile.
        Render_Queue.enqueue(image_tile);
    }
    else
    {
        Render_Queue.enqueue(image_tile, true);
    }

    if (runnable())
    {
        // Start rendering.

        start_rendering();
    }

}


bool Tile_Renderer::clear(int cancel_options)
{
    // >>> CAUTION: The Queue_Lock is expected to be locked.

    bool done = abort(cancel_options);
    /*
        int index = Render_Queue.size();
        while (index--)
        {
            if (Render_Queue[index]->Cancelable ||
                (cancel_options & FORCE_CANCEL))
            {
                if (cancel_options & DELETE_WHEN_DONE)
                    // Mark the tile image for deletion when the tile is destroyed.
                    Render_Queue[index]->Delete_Image_When_Done = true;


                delete_tile(Render_Queue.takeAt(index));
            }
        }


        // Clear the Delete_Queue.
        delete_tiles();
    */
    if (!done &&
        (cancel_options & WAIT_UNTIL_DONE))
    {

        /* >>> SIGNAL <<<
        emit error ("Image rendering did not complete when canceled!");
           */
    }

    return done;
}


bool Tile_Renderer::abort(int cancel_options)
{
    // >>> CAUTION: The Queue_Lock is expected to be locked.
    bool done = true;
    if (Active_Tile)
    {
        // Raise the canceled flag for rendered tile disposition.
        Cancel = true;

        // Notify the image rendering machinery to cancel operations.
        Active_Tile->Image->cancel_update();

        if (cancel_options & DELETE_WHEN_DONE)
        {
            // Mark the tile image for deletion when the tile is destroyed.
            Active_Tile->Delete_Image_When_Done = true;
        }

        if (cancel_options & WAIT_UNTIL_DONE)
        {
            // Wait for any rendering to complete.
            done = false;
        }
    }

    return done;
}

bool Tile_Renderer::delete_image(Dynamic_Image* image)
{
    if (!image) return false;
    /*
        bool deleted = false;
        int index = Render_Queue.find(image);
        if (index >= 0)
        {

            Render_Queue[index]->Delete_Image_When_Done = true;
            delete_tile(Render_Queue.takeAt(index));
            deleted = true;
        }

        if (Active_Tile &&
            Active_Tile->Image == image)
        {

            // Delete when done if not in the Render_Queue.
            Active_Tile->Delete_Image_When_Done = !deleted;
            // The Active_Tile is put on the Delete_Queue when rendering is done.
            deleted = true;
        }
        if (!deleted)
        {

            delete_tile(new Image_Tile(image, QPoint(), true, true));
        }*/

        // TODO(guym) handle in future
    Render_Queue.remove(image);
    return true;
}

/*
void Tile_Renderer::delete_tile(Image_Tile* image_tile)
{
    // >>> CAUTION: The Queue_Lock is expected to be locked.

    // Safety check for the image in the Render_Queue.
    int index = Render_Queue.find(image_tile->Image);
    if (index >= 0 && image_tile->Delete_Image_When_Done)
    {
        image_tile->Delete_Image_When_Done = false;
    }

    index = Delete_Queue.size();
    while (index--)
    {
        if (Delete_Queue[index]->Image == image_tile->Image)
        {
            // The tile Image is already in the Delete_Queue.

            if (Delete_Queue[index] != image_tile)
            {
                // Different tiles.

                if (image_tile->Delete_Image_When_Done)
                {
                    Delete_Queue[index]->Delete_Image_When_Done = true;
                    image_tile->Delete_Image_When_Done = false;
                }

                delete image_tile;
            }

            return;
        }
    }
    // Queue for deletion of the tile and its image.

    Delete_Queue.enqueue(image_tile);
}


void
Tile_Renderer::delete_tiles()
{
    Delete_Queue.clear();
}
*/

void
Tile_Renderer::clean_up()
{
    // TODO(guym) ensure rendered tiles are deleted
    //delete_tiles();
}

/*==============================================================================
 Rendering
*/
void Tile_Renderer::run()
{
    Runnable = true;
    Suspended = false;
    Finish = false;

    // Start the rendering loop. This will not return until finish is called.
    render();
}

void Tile_Renderer::run_rendering()
{
    Finish = false;

    /* In order to prevent a deadlock in the render method due to an
    attempt to lock the Queue_Lock which may have already been locked
    in the calling, synchronous, context, the Queue_Lock needs to be
    unlocked here, if locked, before render is entered and relocked
    afterwards so the caller's expectation of needing to unlock it
    can be met.
    */

    render();
}


bool Tile_Renderer::is_ready()
{
    if (Suspended || Finish)
    {
        // >>> SIGNAL <<<
        emit status(NOT_RENDERING);
    }

    return (Runnable && !Finish);
}

void Tile_Renderer::render()
{
    int tile_status;
    bool canceled;

    qDebug() << "Rendering tiles 1.. " << Render_Queue.size();


    while (true)
    {
        if (!is_ready()) break;

        // Check for source image loading.
        load_image();

        if (Finish) break;

        if (Render_Queue.isEmpty())
        {
            suspend_rendering();
            continue;
        }

        qDebug() << "Rendering tile 1 of " << Render_Queue.size();

        /* Acquire the Active_Tile from the front of the Render_Queue.

        A non-NULL Active_Tile is used as a flag that an image tile has
        been acquired for rendering. When rendering of the Active_Tile
        is complete it will be deleted and the Active_Tile reset to NULL.
        */
        Active_Tile = Render_Queue.dequeue();
        tile_status = Active_Tile->status();
        canceled = Cancel;

        // Rendering begins .......................................................


        // Register the Image_Rendering_Monitor with the tile's image.
        Active_Tile->Image->add_rendering_monitor(Image_Rendering_Monitor);

        /* Release the Queue_Lock during rendering.

        >>> WARNING <<< The Queue_Lock must be released AFTER the
        Rendering_Lock is acquired to avoid a race condition in abort.
        */


        if (!canceled)
        {
            // >>> SIGNAL <<<
            emit status(tile_status);

            // Render the tile.

            try
            {

                Active_Tile->Image->update();
            }
            catch (Dynamic_Image::Render_Exception& except)
            {
                // >>> SIGNAL<<<
                emit error(QString::fromStdString(except.message()));
                reset(DO_NOT_WAIT);
                canceled = true;
            }

        }

        // Remove the Image_Rendering_Monitor from the image.
        Active_Tile->Image->remove_rendering_monitor(Image_Rendering_Monitor);

        // Rendering done .........................................................

        /* Reacquire the Queue_Lock.

        >>> WARNING << The Queue_Lock must be acquired AFTER the
        Rendering_Lock is released to avoid a deadlock condition in
        abort.

        N.B.: The Active_Tile may continue to be used under the
        presumption that it will only be deleted, once acquired, here.
        */


        // Tile disposition:

        // The Cancel flag may have been set while the Queue_Lock was unlocked.
        canceled |= Cancel;

        if (canceled)
        {
            // Clear any lingering rendering cancellation status.
            Active_Tile->Image->cancel_update(false);
            Cancel = false;

            tile_status |= RENDERING_CANCELED;

            // >>> SIGNAL <<<
            emit status(tile_status);
        }
        else
        {
            if (tile_status == RENDERING_HIGH_PRIORITY || !Active_Tile->Cancelable)
            {
                if (Immediate_Mode)
                {
                    // >>> SIGNAL <<<
                    emit rendered(Active_Tile->Tile_Coordinate);
                }
                else
                {
                    if (Render_Queue.isEmpty() ||
                        Render_Queue.priorityStatus() == Image_Tile::LOW_PRIORITY)
                    {
                        // >>> SIGNAL <<<
                        emit rendered(QPoint());
                    }
                }
            }
        }
        // Dispose of the Active_Tile.

        delete Active_Tile;
        Active_Tile = NULL;

        QThread::yieldCurrentThread();
    }
}


void
Tile_Renderer::start_rendering()
{
    Suspended = false;
    Runnable = true;

    start();
    /*
        // Start the thread running (it is not already running).
    QThread thread;
    moveToThread(&thread);
    thread.start();
    */
}

bool
Tile_Renderer::runnable() const
{
    return Runnable;
}


bool Tile_Renderer::suspend_rendering(bool wait)
{
    return true;
}


bool
Tile_Renderer::suspended() const
{
    return Suspended;
}


bool Tile_Renderer::stop_rendering(bool wait)
{
    // Suspend the rendering loop.
    bool done = suspend_rendering(wait);

    Runnable = false;

    return done;
}


bool Tile_Renderer::finish(int cancel_options)
{
    // Flag the run loop to finish.
    Finish = true;

    // Cancel all rendering; this will also clear the Delete_Queue.
    reset(cancel_options | FORCE_CANCEL);

    return true;
}

/*==============================================================================
 Source image
*/
bool Tile_Renderer::image(const QString& source_name)
{
    bool registered = false;
    if (!source_name.isEmpty() && Source_Name != source_name)
    {
        Source_Name = source_name;
        Image_Source = NULL;
        registered = true;
    }

    return registered;
}


bool Tile_Renderer::image(const Shared_Image& source_image)
{
    bool registered = false;
    if (source_image && Source_Image != source_image)
    {
        Image_Source = source_image;
        Source_Name.clear();
        registered = true;
    }

    return registered;
}

//------------------------------------------------------------------------------

void Tile_Renderer::load_image()
{
    if (!Image_Source && Source_Name.isEmpty())
    {
        return;
    }
    // >>> SIGNAL <<<
    emit status(LOADING_IMAGE);

    // Lock Queue_Lock during image loading.
    bool loaded = false;
    /*
    >>> WARNING <<< A Shared_Image must be used for the loaded image
    pointer because the source may be a Shared_Image and it is important
    that the Source_Image in this case remain connected to the shared
    source. Using a Dynamic_Image pointer here to convey the shared
    Image_Source to the Source_Image would be distinct from the original
    shared Image_Source (i.e. it would have the same pointer value but a
    different counter), which will result in premature destruction of the
    image object.
    */
    Shared_Image source_image;

    if (Image_Source)
    {
        source_image = Image_Source;
    }
    else if (!Source_Name.isEmpty())
    {
        source_image = load_image(Source_Name);
    }

    // Clear the source image load request.
    Image_Source = NULL;
    Source_Name.clear();

    qDebug() << "Loading " << source_image;

    if (source_image)
    {
        // Clone the Reference_Image from the source image; nothing shared.
        Dynamic_Image* reference_image = clone_image(source_image);
        if (reference_image)
        {
            // No Reference_Image disposition here.
            // Reset the Reference_Image data mapping.
            Reference_Image = reference_image;
            Reference_Image->auto_update(false);
            Reference_Image->source_band_map_reset();
            Reference_Image->source_data_map_reset();
            Reference_Image->source_transform_reset();

            // Transfer the new source_image to the Source_Image.

            Source_Image = source_image;
            loaded = true;
        }
        else
        {
            delete source_image;
        }
    }

    // Emit signals without Queue_Lock locked.

    // >>> SIGNAL <<<
    emit image_loaded(loaded);
}

Dynamic_Image* Tile_Renderer::load_image(const QString& source_name)
{
    Dynamic_Image* dynamic_image = NULL;

    if (source_name.isEmpty()) return dynamic_image;

    if (JP2_Image::is_JP2_file(source_name) || HiView_Utilities::is_URL(source_name))
    {
        return load_JP2_image(source_name);
    }

    const char* report = NULL;
    QFileInfo file(source_name);
    if (!file.exists())
    {
        report = "The file could not be found.";
    }
    else if (file.isDir())
    {
        report = "The file is a directory.";
    }
    else if (!file.isReadable())
    {
        report = "The file is not readable.";
    }
    else if (file.size() == 0)
    {
        report = "The file is empty.";
    }
    else
    {
        QImage* source_image = new QImage(source_name);

        if (source_image->isNull())
        {
            delete source_image;
            report = "An image could not be obtained from the file.";
        }
        else
        {
            dynamic_image = new Dynamic_QImage(source_image);
            dynamic_image->source_name(source_name);
        }
    }

    if (report)
    {
        QString error_message("Failed to load an image from");
        error_message += " \"";
        error_message += source_name;
        error_message += "\".\n";
        error_message += tr(report);

        // >>> SIGNAL <<<
        emit error(error_message.replace("\n", "<br>"));
    }

    return dynamic_image;
}


JP2_Image* Tile_Renderer::load_JP2_image(const QString& source_name)
{
    QString report;
    JP2_Reader* JP2_reader = NULL;

    try { JP2_reader = JP2::reader(source_name.toStdString()); }
    catch (JP2_Exception& except)
    {
        report = except.what();
    }
    catch (std::exception& except)
    {
        report = except.what();
    }
    catch (...)
    {
        report = "Unknown exception!";
    }
    if (!report.isEmpty())
    {
    Error_Report:
        QStringList lines = report.split("\n", Qt::SkipEmptyParts);
        QString
            error_message(lines.size() > 1 ? lines.at(lines.size() - 1) : report);
        error_message += '\n';
        error_message += tr("Failed to load a JP2 image from");
        error_message += " \"";
        error_message += source_name;
        error_message += '\n';
        error_message += report;
        error_message += "\".\n";

        // >>> SIGNAL <<<
        emit error(error_message.replace("\n", "<br>"));

        if (JP2_reader)
        {
            delete JP2_reader;
        }

        return NULL;
    }

    // Size the image appropriately for a source image.
    QSize
        source_size(JP2_reader->image_width(), JP2_reader->image_height());
    double
        scale = scale_to_area(source_size, max_source_image_area());

    if (scale < .95) source_size *= scale;

    // Construct the JP2_Image. Ownership of the JP2_Reader is transferred.
    JP2_Image
        * JP2_image = NULL;
    try { JP2_image = new JP2_Image(JP2_reader, source_size); }
    catch (JP2_Exception& except)
    {
        report = except.what();
    }
    catch (std::exception& except)
    {
        report = except.what();
    }
    catch (...)
    {
        report = "Unknown exception!";
    }
    if (!report.isEmpty())
        goto Error_Report;

    if (scale < .95)
        // Set the image scaling to fit.
        JP2_image->source_scale(scale);

    // Apply the source name.
    JP2_image->source_name(source_name);

    return JP2_image;
}

/*==============================================================================
 Cloned image
*/
Dynamic_Image* Tile_Renderer::image_clone(const QSize& size, Dynamic_Image::Mapping_Type shared_mappings)
{
    /* >>> WARNING <<<

    The Reference_Image must not be changed when clone_image is called
    (possible race condition), nor the object deleted while the cloning
    is in progress.
    */
    Dynamic_Image* cloned_image = clone_image(Reference_Image, size, shared_mappings);

    return cloned_image;
}

Dynamic_Image* Tile_Renderer::clone_image(Dynamic_Image* source_image, const QSize& image_size,
                                          Dynamic_Image::Mapping_Type shared_mappings)
{
    Dynamic_Image* cloned_image = NULL;
    if (!source_image) return cloned_image;

    QString error_message;

    try { cloned_image = source_image->clone(image_size, shared_mappings); }
    catch (JP2_Exception& except)
    {
        error_message = except.what();
    }
    catch (std::exception& except)
    {
        error_message = except.what();
    }
    catch (...)
    {
        error_message = "Unknown exception!";
    }
    if (!error_message.isEmpty())
    {
        QString message("Failed to clone image\n");

        if (source_image->source_name().isEmpty())
            message += ".\n";
        else
            message +=
            QString(" \"") + source_image->source_name() + "\".\n";
        error_message = message + error_message;

        // >>> SIGNAL <<<
        emit error(error_message.replace("\n", "<br>"));
    }

    return cloned_image;
}

/*==============================================================================
 Utilities
*/
double Tile_Renderer::scale_to_area(const QSize& size, unsigned long  area)
{
    double
        scaling = sqrt(static_cast<double>(area) / (static_cast<double>(size.width()) * size.height()));
    QSize
        scaled_size(size * scaling);
    unsigned long long
        amount = (unsigned long long)scaled_size.rwidth() * scaled_size.rheight();
    if (amount > area)
    {
        /* The scaling factor is too large, due to rounding.

        Reduce the scaled area by the smallest dimension that will bring
        the area under the limit. Then recalculate the scaling based on
        the adjusted dimension.
        */
        amount -= area;
        if (scaled_size.rwidth() < scaled_size.rheight())
        {
            /* If reducing the scaled area by a row width is sufficient
            to bring the area under the limit, then decrement the
            column height. Otherwise decrement the row width to reduce
            the scaled area by a column height.
            */
            if ((unsigned long long)scaled_size.rwidth() >= amount)
            {
                // Reduce the area by a row width.
                scaled_size.rheight()--;
                scaling = static_cast<double>(scaled_size.rheight()) / size.height();
            }
            else
            {
                // Reduce the area by a column height.
                scaled_size.rwidth()--;
                scaling = static_cast<double>(scaled_size.rwidth()) / size.width();
            }
        }
        else
        {
            if ((unsigned long long)scaled_size.rheight() >= amount)
            {
                scaled_size.rwidth()--;
                scaling = static_cast<double>(scaled_size.rwidth()) / size.width();
            }
            else
            {
                scaled_size.rheight()--;
                scaling = static_cast<double>(scaled_size.rheight()) / size.height();
            }
        }
    }
    return scaling;
}


QString Tile_Renderer::status_description(int condition)
{
    QString description;

    switch (condition & ~RENDERING_CANCELED)
    {
        case NOT_RENDERING:
            description = tr("Not Rendering"); break;
        case RENDERING_LOW_PRIORITY:
            description = tr("Rendering Low Priority"); break;
        case RENDERING_HIGH_PRIORITY:
            description = tr("Rendering High Priority"); break;
        case LOADING_IMAGE:
            description = tr("Loading Image"); break;
        default:
            description = tr("Unknown Status %1").arg(condition);
    }
    if (condition & RENDERING_CANCELED)
        description += tr(" Canceled");

    return description;
}

QString Tile_Renderer::cancel_options_descriptions(int cancel_options)
{
    QString descriptions((cancel_options & WAIT_UNTIL_DONE) ?
                         "WAIT_UNTIL_DONE" : "DO_NOT_WAIT");
    if (cancel_options & DELETE_WHEN_DONE)
        descriptions += ", DELETE_WHEN_DONE";
    if (cancel_options & FORCE_CANCEL)
        descriptions += ", FORCE_CANCEL";
    return descriptions;
}

} // namespace UA::HiRISE
