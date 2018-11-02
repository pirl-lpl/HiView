/*	Image_Viewer
 
 HiROC CVS ID: $Id: Image_Viewer.hh,v 1.93 2014/05/27 17:23:09 guym Exp $
 
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

#ifndef HiView_Image_Viewer_hh
#define HiView_Image_Viewer_hh

#include	<QFrame>

#include	"Tiled_Image_Display.hh"
#include    "Projection.hh"

//	Forward references.
class QScrollBar;
class QSlider;
class QLabel;
class QAction;
class QMenu;
class QContextMenuEvent;
class QErrorMessage;
class QCursor;
template<typename T> class QVector;
template<typename T> class QList;

namespace idaeim {
    namespace PVL {
        class Aggregate;
    }}


namespace UA
{
    namespace HiRISE
    {
        //	Forward references.
        class Plastic_Image;
        
        /**	An <i>Image_Viewer</i> provides a QWidget (in a QFrame) for the
         interactive viewing of image files.
         
         An Image_Viewer may be used to view any QImage, as well very large
         JP2 image data sources or any image data source that implements the
         Plastic_Image abstract class. The Image_Viewer is optimized for
         interactive viewing of very large image data sources that may be
         selectively rendered by use of a Tiled_Image_Display component. The
         Tiled_Image_Display dynamically manages a grid of Plastic_Image tile
         objects that cover the image viewport as well as image tiles
         surrounding the viewport that are asynchronously pre-rendered in
         anticipation of local region scrolling. Scrolling and scaling of the
         image that appears in the viewport is provided. In addition, the
         image data may be band mapped to select which image data band will
         appear as the red, green or blue image display band; the source image
         may have an arbitrary number of data bands. The image data may also
         be pixel data mapped for contrast control or any lookup table based
         pixel data mapping; source image data of up to 16 bits per pixel
         sample (each band) is supported.
         
         The image that is viewed may be provided as a QImage object or any
         Plastic_Image implementation (a Plastic_Image is a QImage
         subclass). The image may also be provided as a Shared_Image pointer
         in which case the reference image for the Tiled_Image_Display will be
         shared with the provider rather than copied. The image source may
         also be specified by name. If the name refers to a JP2 file, either
         as a local pathname or a JPIP server URL, the appropriate JP2_Image
         implementation of a Plastic_Image is constructed; otherwise a plain
         Plastic_QImage implementation is constructed. A JP2_Image takes full
         advantage of the selective rendering capabilities of the JPEG2000
         codestream. A Plastic_QImage will always be displayed as a single
         tile encompassing the entire image, but will be able to take
         advantage of the band and pixel data mapping capabilities of a
         Plastic_Image.
         
         @author		Bradford Castalia, UA/HiROC
         @version	$Revision: 1.93 $
         */
        class Image_Viewer
        :	public QFrame
        {
            //	Qt Object declaration.
            Q_OBJECT
            
        public:
            /*==============================================================================
             Types:
             */
            typedef Tiled_Image_Display::Shared_Image	Shared_Image;
            typedef Tiled_Image_Display::Data_Map		Data_Map;
            typedef Tiled_Image_Display::Histogram		Histogram;
            
            /*==============================================================================
             Constants
             */
            //!	Class identification name with source code version and date.
            static const char* const
            ID;
            
            
            //!	Default image display viewport size (default: 512,316).
            static const QSize
            DEFAULT_IMAGE_DISPLAY_SIZE;
            
            //!	The {@link rendering_status(int) rendering status} values.
            enum
            {
                NOT_RENDERING
                = Tiled_Image_Display::NOT_RENDERING,
                RENDERING_BACKGROUND
                = Tiled_Image_Display::RENDERING_BACKGROUND,
                RENDERING_VISIBLE_TILES
                = Tiled_Image_Display::RENDERING_VISIBLE_TILES,
                LOADING_IMAGE
                = Tiled_Image_Display::LOADING_IMAGE,
                RENDERING_CANCELED
                = Tiled_Image_Display::RENDERING_CANCELED
            };
            
            //!	{@link state_change(int) state change} bit flags.
            enum
            {
                NO_STATE_CHANGE
                = Tiled_Image_Display::NO_STATE_CHANGE,
                IMAGE_LOAD_STATE
                = Tiled_Image_Display::IMAGE_LOAD_STATE,
                DISPLAY_SIZE_STATE
                = Tiled_Image_Display::DISPLAY_SIZE_STATE,
                TILE_SIZE_STATE
                = Tiled_Image_Display::TILE_SIZE_STATE,
                IMAGE_MOVE_STATE
                = Tiled_Image_Display::IMAGE_MOVE_STATE,
                IMAGE_SCALE_STATE
                = Tiled_Image_Display::IMAGE_SCALE_STATE,
                BAND_MAPPING_STATE
                = Tiled_Image_Display::BAND_MAPPING_STATE,
                DATA_MAPPING_STATE
                = Tiled_Image_Display::DATA_MAPPING_STATE,
                STATE_TYPE_MASK
                = Tiled_Image_Display::STATE_TYPE_MASK,
                
                RENDERING_VISIBLE_TILES_STATE
                = Tiled_Image_Display::RENDERING_VISIBLE_TILES_STATE,
                RENDERING_BACKGROUND_STATE
                = Tiled_Image_Display::RENDERING_BACKGROUND_STATE,
                RENDERING_CANCELED_STATE
                = Tiled_Image_Display::RENDERING_CANCELED_STATE,
                RENDERING_COMPLETED_STATE
                = Tiled_Image_Display::RENDERING_COMPLETED_STATE,
                RENDERING_VISIBLE_TILES_COMPLETED_STATE
                = Tiled_Image_Display::RENDERING_VISIBLE_TILES_COMPLETED_STATE,
                COMPLETED_WITHOUT_RENDERING_STATE
                = Tiled_Image_Display::COMPLETED_WITHOUT_RENDERING_STATE,
                STATE_QUALIFIER_MASK
                = Tiled_Image_Display::STATE_QUALIFIER_MASK
            };
            
            //!	The image scrolling and scaling {@link control_mode() control modes}.
            enum
            {
                NO_CONTROL_MODE			= 0,
                SHIFT_MODE				= (1 << 0),
                SCALE_MODE				= (1 << 1),
                SCALE_MODE_PENDING		= (1 << 2),
                CONTROL_MODE			= (SHIFT_MODE | SCALE_MODE)
            };
            
            /*==============================================================================
             Class data members
             */
            //!	Default sliding scale action.
            static bool
            Default_Scaling_Immediate;
            
            /*==============================================================================
             Constructors
             */
            /**	Construct an empty Image_Viewer.
             
             @param	parent	A pointer to the parent QWidget for this widget.
             May be NULL.
             */
            Image_Viewer (QWidget* parent = NULL);
            
            virtual ~Image_Viewer ();
            
            /*==============================================================================
             Accessors
             */
            /**	Load an image into the Image_Viewer.
             
             <b>N.B.</b>: The image is not loaded immediately: Image loading is
             done asynchronously (unless the application has been built to do
             synchronous rendering, of course). When the image load complete
             signal has been received from the the {@link image_display() image
             display} object the Image_Viewer will update its layout and then emit
             the {@link image_loaded(bool)} signal.
             
             <b>N.B.</b>: The image display object is expected to emit an {@link
             image_scaled(const QSizeF&, int)} signal that will be passed on by
             the Image_Viewer.
             
             @param	source_name	A QString naming the source of the image to load.
             This may be a pathname to a local file or a URL to a remote server
             source.
             @param	display_size	A QSize specifying the desired display size of
             the image to which it will be scaled when loaded. If the size is
             empty the current size of the display viewport will be used.
             @return	true if the image was registered for loading; false if the image
             load request was rejected.
             @see	image(const QString&, const QSizeF&)
             */
            bool image (const QString& source_name,
                        const QSize& display_size);
            
            /**	Load an image into the Image_Viewer.
             
             <b>N.B.</b>: The image is not loaded immediately: Image loading is
             done asynchronously (unless the application has been built to do
             synchronous rendering, of course). When the image load complete
             signal has been received from the the {@link image_display() image
             display} object the Image_Viewer will update its layout and then emit
             the {@link image_loaded(bool)} signal.
             
             <b>N.B.</b>: The image display object is expected to emit an {@link
             image_scaled(const QSizeF&, int)} signal that will be passed on by
             the Image_Viewer.
             
             @param	source_name	A QString naming the source of the image to load.
             This may be a pathname to a local file or a URL to a remote server
             source.
             @param	scaling	A QSizeF that specifies how the image is to be scaled
             when first loaded. Positive values are absolute scaling factors.
             Negative values are desired image dimensions. An empty size
             indicates that the image should be scaled to fit in the current
             display viewport.
             @return	true if the image was registered for loading; false if the image
             load request was rejected.
             @see	image(const QString&, const QSize&)
             */
            bool image (const QString& source_name,
                        const QSizeF& scaling = QSizeF ());
            
            bool image (const Shared_Image& source_image,
                        const QSize& display_size);
            bool image (const Shared_Image& source_image,
                        const QSizeF& scaling = QSizeF ());
            bool image (const QImage& source_image,
                        const QSize& display_size);
            bool image (const QImage& source_image,
                        const QSizeF& scaling = QSizeF ());
            
            inline Shared_Image image () const
            {return Image_Display->image ();}
            
            inline static void default_source_image_rendering (bool enabled)
            {Tiled_Image_Display::default_source_image_rendering (enabled);}
            inline static bool default_source_image_rendering ()
            {return Tiled_Image_Display::default_source_image_rendering ();}
            inline void source_image_rendering (bool enabled)
            {Image_Display->source_image_rendering (enabled);}
            inline bool source_image_rendering ()
            {return Image_Display->source_image_rendering ();}
            
            inline int max_source_image_area () const
            {return (int)(Image_Display->max_source_image_area () >> 20);}
            inline static void default_max_source_image_area (int area)
            {Tiled_Image_Display::default_max_source_image_area
                ((unsigned long)area << 20);}
            inline static int long default_max_source_image_area ()
            {return (int)(Tiled_Image_Display::default_max_source_image_area () >> 20);}
            
            inline QString image_name () const
            {return Source_Name;}
            Image_Viewer& image_name (const QString& name)
            {Source_Name = name; return *this;}
            
            /*=============================================================================
             World Information
             */
            
            void projection(Projection *projector);
            
            /*------------------------------------------------------------------------------
             Image metadata
             */
            inline idaeim::PVL::Aggregate* image_metadata () const
            {return Image_Display->image_metadata ();}
            
            /*------------------------------------------------------------------------------
             Image geometry
             */
            inline QSize image_size () const
            {return Image_Display->image_size ();}
            inline int image_width () const
            {return Image_Display->image_width ();}
            inline int image_height () const
            {return Image_Display->image_height ();}
            inline QSize scaled_image_size () const
            {return Image_Display->scaled_image_size ();}
            inline int image_bands () const
            {return Image_Display->image_bands ();}
            inline unsigned int* band_map () const
            {return Image_Display->band_map ();}
            inline int image_data_precision () const
            {return Image_Display->image_data_precision ();}
            
            /*------------------------------------------------------------------------------
             Image origin and region
             */
            /**	Get the image coordinate at the {@link image_display{} image display}
             viewport origin.
             
             @param	band	The image band for which to get the display origin.
             @return	A QPointF coordinate in unscaled image space corresponding
             to the display origin (0,0; the top-left corner). <b>N.B.</b>:
             Floating point image coordinate values are provided for accuracy
             when accuracy is needed (e.g. scaling). The values should be
             truncated (excess partial pixels clipped off) if integer values
             are needed, not rounded to the nearest integer.
             @see displayed_image_region(int, bool)
             */
            inline QPointF displayed_image_origin (int band = 0) const
            {return Image_Display->displayed_image_origin (band);}
            
            /**	Get the region of the image, in image space, that is contained within
             the image display viewport.
             
             Only that portion of the image that is being displayed is returned.
             The {@link image_display() image display} viewport may extend beyond
             the boundaries of the displayed image.
             
             @param	band	The image band for which to obtain the region of the
             image appearing in the display.
             @return	A QRectF describing the region of the image that appears in
             the display viewport. <b>N.B.</b>: The image display region is
             provided as floating point values for accuracy in other contexts
             (e.g. scaled image dimensions). The dimensions of the region
             rectangle should be truncated if integer values are needed,  not
             rounded the the nearest integer.
             @see	image_display_region(int)
             */
            inline QRectF displayed_image_region (int band = 0) const
            {return Image_Display->displayed_image_region (band);}
            
            /**	Get the region of the image display viewport, in display space, that
             contains the displayed image region.
             
             <b>N.B.</b>: The image display viewport only includes the {@link
             image_display() image display} area; it does not include any GUI
             widgets managed by the Image_Viewer, such as scrollbars.
             
             The origin of the display space (0,0) is at the top-left corner of
             the display viewport and the coordinate values increase to the right
             and down.
             
             The returned rectangle will always have its top-left coordinate at
             the display origin. The returned rectangle will be different from the
             display viewport rectangle only when the right and/or bottom edge of
             the image appears in the display.
             
             @param	band	The image band for which the display region is desired.
             @return	A QRect specifying the region of the display viewport, in
             display space, that contains a region of the selected image band.
             @see	displayed_image_region(int)
             */
            inline QRect image_display_region (int band = 0) const
            {return Image_Display->image_display_region (band);}
            
            /*------------------------------------------------------------------------------
             Coordinate mapping
             */
            /**	Map an image display viewport coordinate to its image coordinate.
             
             The display viewport coordinate is relative to the {@link
             image_display() image display}, not this Image_Viewer. The display
             viewport coordinate can be obtained for an Image_Viewer coordinate
             using:
             
             <code>
             QPoint image_display_coordinate
             = Image_Viewer.image_display ()->mapFromGlobal
             (Image_Viewer.mapToGlobal (image_viewer_coordinate));
             </code>
             
             @param	coordinate	A QPoint coordinate in the image display viewport.
             @param	band	The image band for which the image position is
             requested.
             @return	A QPointF coordinate in image space corresponding the image
             display viewport coordinate. <b>N.B.</b>: The returned image
             coordinate may be located outside the {@link image_size() image
             bounds} if the specified display coordinate falls on an area of
             the display viewport that is not within the {@link
             displayed_image_region(int) displayed image region} for the band.
             @see	image_display_region(int)
             @see	map_image_to_display(const QPointF&, int) const
             */
            inline QPointF map_display_to_image
            (const QPoint& coordinate, int band = 0) const
            {return Image_Display->map_display_to_image (coordinate, band);}
            
            /**	Map an image display viewport rectangle to its source image region.
             
             This is a convenience overload method that maps the defining corner
             coordinates of the specified image display region to their source
             image coordinates.
             
             @param	display_region	A QRect in image display viewport coordinates.
             @param	band	The image band for which the image position is
             requested.
             @return	A QRectF in source image space cooresponding to the image
             display viewport coordinate. <b>N.B.</b>: The returned image
             coordinate may be located outside the {@link image_size() image
             bounds} if the specified display coordinate falls on an area of
             the display viewport that is not within the {@link
             displayed_image_region(int) displayed image region} for the band.
             @see	map_display_to_image(const QPoint&, int) const
             */
            inline QRectF map_display_to_image
            (const QRect& display_region, int band = 0) const
            {return Image_Display->map_display_to_image (display_region, band);}
            
            /**	Map an image coordinate to its display viewport coordinate.
             
             @param	coordinate	A QPointF coordinate in image space.
             @param	band	The image band to use as a location and scaling
             reference.
             @return	A QPoint coordinate relative to the image display viewport
             corresponding the source image coordinate. <b>N.B.</b>: The
             coordinate may lie outside the bounds of the display viewport.
             @see	map_display_to_image(const QPoint&, int) const
             */
            inline QPoint map_image_to_display
            (const QPointF& coordinate, int band = 0) const
            {return Image_Display->map_image_to_display (coordinate, band);}
            
            /**	Map an image coordinate to its display viewport coordinate.
             
             This is a convenience overload method that maps the defining corner
             coordinates of the specified image region to their image display
             coordinates.
             
             @param	image_region	A QRectF in source image coordinates.
             @param	band	The image band to use as a location and scaling
             reference.
             @return	A QRect relative to the image display viewport corresponding
             the source image region. <b>N.B.</b>: The rectangle may lie
             outside the bounds of the display viewport.
             @see	map_image_to_display(const QPointF&, int) const
             */
            inline QRect map_image_to_display
            (const QRectF& image_region, int band = 0) const
            {return Image_Display->map_image_to_display (image_region, band);}
            
            /*------------------------------------------------------------------------------
             Pixel values
             */
            inline Plastic_Image::Pixel_Datum image_pixel_datum
            (unsigned int x, unsigned int y, unsigned int band) const
            {return Image_Display->image_pixel_datum (x, y, band);}
            inline Plastic_Image::Triplet image_pixel (const QPoint& coordinate) const
            {return Image_Display->image_pixel (coordinate);}
            inline QRgb display_value (const QPoint& coordinate) const
            {return Image_Display->display_value (coordinate);}
            inline Plastic_Image::Triplet display_pixel (const QPoint coordinate) const
            {return Image_Display->display_pixel (coordinate);}
            
            /*------------------------------------------------------------------------------
             Scaling
             */
            /**	Get the image scaling factors.
             
             @param	band	The band for which image scaling factors are to be
             obtained.
             @return	A QSizeF containing the horizontal and vertical scaling
             factors for the specified image band.
             */
            inline QSizeF image_scaling (int band = 0) const
            {return Image_Display->image_scaling (band);}
            
            /**	Change the current {@link image_scaling(int) image scaling} by adding
             a scaling factor increment.
             
             If the menu position has been set by a menu action handler before
             invoking this method, then the image center (invariant) position will
             correspond to the position of the menu in the display (i.e. the
             position of the cursor when the menu was selected). Otherwise the
             center position will correspond to the current position of the image
             cursor. However, if the display position of the menu or cursor is not
             within the {@link image_display() image display} then the center
             position will be at the origin (upper left corner) of the display.
             After the {@link scale_image(const QSizeF&, const QPoint& int) image
             scaling} has been done, if the scaling center cursor position was
             within the image display and the center position is no longer under
             the cursor position the cursor is moved to the new position over the
             image center position or the closest display edge point to that image
             position.
             
             @param	scaling_factors	A QSizeF containing the horizontal and
             vertical incremental scaling factors to add to the current
             {@link image_scaling(int) image scaling}.
             @return	true if the image scale changed; false otherwise.
             */
            bool scale_by (const QSizeF& scaling_factors);
            
            bool display_fit_to_image () const;
            
            inline static double min_scale ()
            {return Tiled_Image_Display::min_scale ();}
            inline static double max_scale ()
            {return Tiled_Image_Display::max_scale ();}
            
            inline static double scaling_minor_increment ()
            {return Scaling_Minor_Increment;}
            inline static double scaling_major_increment ()
            {return Scaling_Major_Increment;}
            
            inline static bool default_scaling_immediate ()
            {return Default_Scaling_Immediate;}
            inline static void default_scaling_immediate (bool enabled)
            {Default_Scaling_Immediate = enabled;}
            
            /**	Get the list of menu actions for image scaling.
             
             Four menu actions are provided:
             <dl>
             <dt>Scale Up
             <dd>Scales the image up (increases the scaling factor). The scaling
             factor is increased to the next larger value that results in pixel
             multiples; i.e. pixel replication for scale factors greater than
             1.0, or pixel sub-sampling for scale factors less than 1.0. For
             example, scaling up from a current scale factor of 1 (normal
             scaling) results in a scale factor of 2; scaling up from 0.4 results
             in a scale factor of 0.5, and scaling up from 0.5 results in a scale
             factor of 1.
             <dt>Scale Down
             <dd>Scales the image down (descreases the scaling factor). The scaling
             factor is descresed to the next smaller value that results in pixel
             multiples; i.e. pixel replication for scale factors greater than
             1.0, or pixel sub-sampling for scale factors less than 1.0.
             <dt>Normal Size
             <dd>Sets the scale factor to 1.
             <dt>Fit Image to Window
             <dd>Sets the scale factor such the the image will fit entirely within
             the current display region.
             </dl>
             
             Actions will be automatically disabled when they can not be used. For
             example, Normal size will be disable if the current scale factor is 1;
             Scale Up or Scale Down will be disabled if the corresponding scaling
             factor limit has been reached.
             
             @return	A QList of pointers to the scaling menu actions.
             */
            QList<QAction*> scale_menu_actions () const;
            
            QAction* copy_coordinates_action() const;
            
            /*------------------------------------------------------------------------------
             Histograms
             */
            inline bool source_data_histograms (QVector<Histogram*> histograms,
                                                const QRect& image_region) const
            {return Image_Display
                ->source_data_histograms (histograms, image_region);}
            inline bool display_data_histograms (QVector<Histogram*> histograms,
                                                 const QRect& display_region) const
            {return Image_Display
                ->display_data_histograms (histograms, display_region);}
            
            //------------------------------------------------------------------------------
            
            inline Data_Map** data_maps () const
            {return Image_Display->data_maps ();}
            
            /**	Get the pending state change state of the image display.
             
             When the pending state change has been completed the {@link
             state_change(int) state change} signal will be emitted.
             
             @return A state change code with bit flags indicating the pending
             change to the {@link image_display() image display}.
             */
            inline int pending_state_change () const
            {return Image_Display->pending_state_change ();}
            
            /**	Get the current Image_Viewer control mode state.
             
             User interface controls are provided to move and scale the image
             with various device (keyboard and mouse) events. The same device
             event - e.g. pressing the up arrow key or moving the mouse scroller -
             will produce different image modification actions depending on the
             current control mode. Which control mode is in effect is determined
             by {@link keyPressEvent(QKeyEvent*) key events}.
             
             The possible control mode states are:
             
             <dl>
             <dt>{@link #NO_CONTROL_MODE}
             <dd>No image control mode is in effect.
             
             <dt>{@link #SHIFT_MODE}
             <dd>Device events will move (shift) the image.
             
             <dt>{@link #SCALE_MODE}
             <dd>Device events will scale the image.
             
             <dt>{@link #SCALE_MODE_PENDING}
             <dd>A key that is part of a key combination for entering SCALE_MODE has
             been pressed. If the the other part of combination is pressed the
             SCALE_MODE_PENDING mode will become SCALE_MODE.
             </dl>
             
             @return	An Image_Viewer control mode code.
             */
            inline int control_mode () const
            {return Control_Mode;}
            
            static QString control_mode_description (int control_mode);
            
            /**	Set an alternative default image cursor.
             
             The Image_Viewer sets various cursor images for the image display
             to refect the current image {@link control_mode() control mode}:
             
             - NO_CONTROL_MODE - Reticule_Cursor; the default image position cursor.
             - SHIFT_MODE - Shift_Cursor; image shifting (pan) mode is set.
             - SCALE_MODE - Scale_Cursor; image scaling (zoom) mode is set.
             
             This method may be used to override the default position cursor. The
             specified cursor is remembered and, if NO_CONTROL_MODE is in effect
             and the cursor is non-NULL, is set as the image display cursor. When
             the control mode changes out of NO_CONTROL_MODE the corresponding
             cursor is always set - this is because shift and scale image controls
             are presumed to always take precedence over external display changes
             such as drawing an image region selection overlay. When the mode
             reverts to NO_CONTROL_MODE the current default cursor is restored.
             
             @param	cursor	A pointer to a QCursor to be used as the default
             image cursor. If NULL the Reticule_Cursor is used.
             */
            void default_cursor (QCursor* cursor = NULL);
            
            /**	Get the preferred size of the display.
             
             The preferred size of the display is the {@link scaled_image_size()
             scaled size of the image content}, plus the scaling slider width,
             vertical scrollbar width and horizontal scrollbar height if these
             widgets are visible, and the width the enclosing frame. If this
             Image_Viewer contains no image content the {@link
             #DEFAULT_IMAGE_DISPLAY_SIZE default display size} will be used.
             
             @return	A QSize containing the preferred size of the display.
             */
            virtual QSize sizeHint () const;
            
            /**	Get the size of the display viewport.
             
             The size of the display viewport is the current {@link
             image_display_size() image display size} plus the width of the
             vertical scrollbar if it is visible and the height of the horizontal
             scrollbar if it is visible. However, if the image display is empty
             (as would occur during startup) the {@link DEFAULT_IMAGE_DISPLAY_SIZE
             default display size} is used.
             
             <b>N.B.</b>: The viewport size excludes the width the scaling slider
             if it is visible (it will always be visible unless the {@link
             scrollbars(bool) scrollbars have been disabled}).
             
             @return	A QSize containing the size of the display viewport with no
             scrollbars visible.
             @see	image_display_size()
             */
            QSize viewport_size () const;
            
            /**	Get the size of the image display area.
             
             The image display area is that part of the {@link
             viewport_size() display viewport} excluding the scaling
             slider or any scrollbars, if visible.
             
             @return A QSize containing the current size of the image display area.
             @see	viewport_size() const
             */
            QSize image_display_size () const;
            
            inline static int rendering_increment_lines ()
            {return Tiled_Image_Display::rendering_increment_lines ();}
            
            inline static QRgb background_color ()
            {return Tiled_Image_Display::background_color ();}
            
            inline QSize tile_size () const
            {return Image_Display->tile_display_size ();}
            
            /**	Get the Tiled_Image_Display that manages the image display.
             
             @return	A pointer to the Tiled_Image_Display that is managing the
             image display.
             */
            inline Tiled_Image_Display* image_display () const
            {return Image_Display;}
            
            inline QSlider* scale_slider () const
            {return Sliding_Scale;}
            
            /*------------------------------------------------------------------------------
             JP2 specific settings.
             */
            static int JPIP_request_timeout ();
            static QString JPIP_proxy ();
            static QString JPIP_cache_directory ();
            
            /*==============================================================================
             Manipulators
             */
            /**	Enable or disable the display of the scrollbars and scaling slider.
             
             When the scrollbars are enabled they will be visible if the
             corresponding dimension of the scaled image exceeds the display
             viewport dimension; otherwise the associated scrollbar will not be
             displayed; i.e. scrollbars are displayed as needed. The scaling
             slider is always displayed if scrollbars display is enabled.
             
             <b>N.B.</b>: The scaling slider is enabled or disabled along with the
             scrollbars.
             
             @param	enable	true if the scrollbars and scaling slider are to be
             displayed; false otherwise.
             @see	scrollbars()
             */
            void scrollbars (bool enable);
            
            /**	Test if display of the scrollbars and scaling sliding is enabled.
             
             <b>N.B.</b>: One or both of the scrollbars may not be currently
             displayed, even if scrollbars display is enabled, if the
             corresponding scaled image dimension fits within the display
             viewport.
             
             @return	true if the display of the scrollbars and scaling slider are
             enabled; false otherwise.
             @see	scrollbars(bool)
             */
            bool scrollbars () const
            {return Scrollbars_Enabled;}
            
            /*==============================================================================
             Utilities
             */
            /**	Get the rendering status of the image.
             
             Image tiles for the display grid are queued for rendering in high-to-low
             priority order. A high priority image tile is visible in the display
             viewport; a low priority tile is in the display grid margin outside the
             viewport or is for the background fill image.
             
             @return	A rendering status value indicating the priority of the
             display tile image being actively rendered or the first tile in
             the queue for rendering. This will be either {@link
             #RENDERING_VISIBLE_TILES} or {@link #RENDERING_BACKGROUND}. If no
             tile is being rendered or is queued {@link #NOT_RENDERING} is
             returned.
             */
            inline int rendering_status () const
            {return Image_Display->rendering_status ();}
            
            /**	Provide a brief description of a {@link rendering_status(int)
             rendering status} value;
             
             @param	status	A rendering status value.
             @return	A QString that very briefly describes the status value.
             @see	Tiled_Image_Display::rendering_status_description(int)
             */
            inline static QString rendering_status_description (int status)
            {return Tiled_Image_Display::rendering_status_description (status);}
            
            /**	Provide a brief description of a {@link state_change(int) state
             change} code.
             
             @param	state	A state change code.
             @return	A QString that very briefly describes the state change
             indicated by the bit flags of the code value.
             @see	Tiled_Image_Display::state_change_description(int)
             */
            inline static QString state_change_description (int state)
            {return Tiled_Image_Display::state_change_description (state);}
            
            static QErrorMessage* error_message ()
            {return Error_Message;}
            
            //	Ownership of the QErrorMesage is NOT transferred.
            static void error_message (QErrorMessage* dialog);
            
            /*==============================================================================
             Qt signals
             */
        signals:
            
            /**	Signals the result of an image load request.
             
             This signal is emitted from the {@link loaded(bool) image loaded} slot
             <b>after</b> the image display reset has been completed.
             
             @param	successful	true if the image load completed successfully;
             false otherwise.
             @see	image(const QString&, const QSizeF&)
             @see	image(const QString&, const QSize&)
             @see	image(const Shared_Image&, const QSizeF&)
             @see	image(const Shared_Image&, const QSize&)
             @see	image(const QImage&, const QSizeF&)
             @see	image(const QImage&, const QSize&)
             @see	Image_Renderer::image_loaded(bool)
             */
            void image_loaded (bool successful);
            
            /**	Signals the position of the image cursor.
             
             This signal is emitted from the {@link cursor_moved(const QPoint&,
             const QPoint&) cursor moved slot} to progagate the corresponding
             signal from the {@link image_display() image display}.
             
             @param	display_position	A QPoint specifying the cursor position
             in display image coordinates, which is the same as the position
             from the mouse move event.
             @param	image_position	A QPoint specifying the cursor position
             in source image coordinates. This will be -1,-1 if the cursor
             is not within the {@link displayed_image_region(int) displayed
             image region} of the reference band.
             */
            void image_cursor_moved
            (const QPoint& display_position, const QPoint& image_position);
            
            /**	Signals the image pixel values at the cursor position.
             
             This signal is emitted from the {@link cursor_moved(const QPoint&,
             const QPoint&) cursor moved slot} that handles the corresponding
             signal from the {@link image_display() image display}.
             
             @param	display_pixel	A Plastic_Image::Triplet containing the
             {@link display_pixel(const QPoint&) display image pixel value} at
             the cursor position. This will be the {@link
             Plastic_Image::UNDEFINED_PIXEL_VALUE} if the cursor is not
             located within the displayed image region.
             @param	image_pixel	A Plastic_Image::Triplet containing the {@link
             image_pixel(const QPoint&) source image pixel value} at the
             cursor position. This will be the {@link
             Plastic_Image::UNDEFINED_PIXEL_VALUE} if the cursor is not
             located within the displayed image region.
             */
            void image_pixel_value
            (const Plastic_Image::Triplet& display_pixel,
             const Plastic_Image::Triplet& image_pixel);
            
            /** Image location change signal progagation.
             
             @param	origin	A QPoint specifying the position of the image, in
             source image coordinates, at the display viewport origin.
             @param	band	The image band that moved. This will be -1 if all
             image bands moved.
             @see	Iiled_Image_Display::image_moved(const QPoint&, int)
             */
            void image_moved (const QPoint& image_position, int band);
            
            /**	Displayed image region size change signal progagation.
             
             @param	region_size	A QSize specifying the size of the {@link
             displayed_image_region() displayed image region}.
             @see	Iiled_Image_Display::displayed_image_region_resized(const QSize&)
             */
            void displayed_image_region_resized (const QSize& region_size);
            
            /**	Image display viewport size change signal progagation.
             
             @param	viewport_size	A QSize specifying the new display viewport
             size.
             @see	Iiled_Image_Display::display_viewport_resized(const QSize&)
             */
            void display_viewport_resized (const QSize& viewport_size);
            
            /**	Image scaling change signal progagation.
             
             @param	scaling	A QSizeF with the horizontal and vertical scaling
             factors that have been applied to the displayed image.
             @param	band	The image band that was scaled. This will be -1
             if all image bands were scaled.
             @see	Iiled_Image_Display::image_scaled(const QSizeF&, int)
             */
            void image_scaled (const QSizeF& scaling, int band);
            
            /**	Tile image rendering status signal progagation.
             
             @param	status	A rendering status value.
             @see	Iiled_Image_Display::rendering_status(int)
             */
            void rendering_status (int status);
            
            /**	Tile image rendering progress status notice message signal progagation.
             
             @param	message	A QString forwarded from the rendering progress status
             notice message.
             @see	Iiled_Image_Display::rendering_status_notice(const QString&)
             */
            void rendering_status_notice (const QString& message);
            
            /**	 Image display state change signal propagation.
             
             @see	Tiled_Image_Display::state_change(int)
             */
            void state_change (int state);
            
            /*==============================================================================
             Qt slots
             */
            public slots:
            
            bool move_image (const QPoint& origin, int band = -1);
            
            /**	Shift the image in the display by image pixel units.
             
             @param	offset	A QSize specifying the amount to shift the image.
             <b>N.B.</b>: The image can only be moved by whole pixel units.
             Positive offsets move the image origin relative to the display
             to the right or down.
             @param	band	The image band to be shifted. If negative all
             image bands will be shifted.
             @return	true if the image moved; false otherwise.
             @see	move_image(const QPoint&, int)
             @see	shift_display(const QPoint&, int)
             */
            bool shift_image (const QSize& offsets, int band = -1);
            
            /**	Shift the image in the display by display pixel units.
             
             <b>N.B.</b>: Shifting the display relative to the image involves
             {@link shift_image(const QPoint&, int) shifting the image} in the
             display viewport by the number of image pixel units in a display
             pixel. However, when the image scaling is greater than 1.0 a display
             pixel is smaller than an image pixel. This could result in the image
             moving less than the specifed number of display units, or not at all,
             depending on the {@link image_scaling(int) image scaling}. Therefor
             the amount of the shift is rounded up to the next image unit
             multiple.
             
             @param	offset	A QSize specifying the amount to shift the image
             in display units. Positive offsets move the display relative
             to the image right or down; i.e. the image origin is moved
             left or up.
             @param	band	The image band to be shifted. If negative all
             image bands will be shifted.
             @return	true if the image moved; false otherwise.
             @see	move_image(const QPoint&, int)
             @see	shift_image(const QPoint&, int)
             */
            bool shift_display (const QSize& offsets, int band = -1);
            
            bool scale_image
            (const QSizeF& scaling, const QPoint& center = QPoint (), int band = -1);
            
            /**	Scale the image by adding a scaling factor to the current image scaling.
             
             If the specified factor value is zero a scaling factor will be
             calculated that will produce the next larger pixel multiple image
             scaling. Pixel multiple image scaling values are positive integer
             values greater than or equal to 1, or the inverse of positive integer
             values for scaling values less than 1.
             
             If the specified factor value is negative a scaling factor will be
             calculated such that the size of the displayed image region will
             increase by no more than one display pixel in width and/or height.
             
             A specified factor value greater than zero will be used as given
             
             The image is {@link scale_by(const QSizeF) scaled by} the resulting
             incremental scaling factor.
             
             @param	factor	The incremental scaling factor to be added to the
             current {@link image_scaling(int) image scaling}. A value that is
             not greater than zero will result in a special value being
             calculated.
             @return	true if the image scale changed; false otherwise.
             */
            bool scale_up (double factor = 0.0);
            
            /**	Scale the image by subtracting a scaling factor to the current image
             scaling.
             
             If the specified factor value is zero a scaling factor will be
             calculated that will produce the next smaller pixel multiple image
             scaling. Pixel multiple image scaling values are positive integer
             values greater than or equal to 1, or the inverse of positive integer
             values for scaling values less than 1.
             
             If the specified factor value is negative a scaling factor will be
             calculated such that the size of the displayed image region will
             decrease by no more than one display pixel in width and/or height.
             
             A specified factor value greater than zero will be used as given
             
             The image is {@link scale_by(const QSizeF) scaled by} the resulting
             incremental scaling factor.
             
             @param	factor	The incremental scaling factor to be subtracted from
             the current {@link image_scaling(int) image scaling}. A value
             that is not greater than zero will result in a special value
             being calculated.
             @return	true if the image scale changed; false otherwise.
             */
            bool scale_down (double factor = 0.0);
            
            bool actual_size ();
            bool fit_image_to_window ();
            bool fit_to_width ();
            bool fit_to_height ();
            bool copy_coordinates();
            void min_scale (double scale_factor);
            void max_scale (double scale_factor);
            void scaling_minor_increment (double increment);
            void scaling_major_increment (double increment);
            
            bool map_bands (const unsigned int* band_map);
            
            /**	Map the source image data to the display image data.
             
             The Data_Maps to be used for mapping the source image data to the
             display image data may be obtained as pointers to the reference image
             {@link data_maps() data maps}. If different data maps are specified
             their contents will be copied into the reference image data maps
             before the data mapping is applied.
             
             @param	maps	An array of three Data_Map pointers. If NULL
             the reference image data maps are used.
             @return	true if the data mapping changed and tile image updates
             have been (or will be) applied.
             */
            bool map_data (Data_Map** maps = NULL);
            
            /**	Set the maximum image area to use when loading a JP2 source image.
             
             When a JP2 source image is loaded it may be exceedingly large,
             possibly too large to fit into memory. So the image will be rendered
             into a limited area. A scaling factor for the image will be
             determined by fitting the image size to the maximum image area and
             limiting the scaling factor to be less than or equal to 1.0.
             
             The closer the image scaling factor is to 1.0 the better the source
             image will appear when being used to fill the display viewport until
             the corresponding image tiles are rendered and become available for
             display. However, the smaller the source image is the faster it will
             load and the less system memory it will use.
             
             @param	area	The maximum image area, in mega-pixels, to be allowed
             when loading a JP2 image.
             */
            void max_source_image_area (int area);
            
            /**	Set the suggested rendering increment.
             
             The specified number of rendering increment lines is a suggestion to
             the Plastic_Image implementation of how many lines of the display
             image (not the source data image} to render before providing a
             notification to all registered rendering monitors. If the value is
             zero the underlying source data rendering implementation may still
             incrementally render the pixel data (this is particularly true for
             JP2 rendering), but rendering notifications will only be sent at the
             completion of rendering. Having nofications sent often increases the
             opportunities for the implmentation to notice that rendering has been
             canceled and thus prevent a highly interactive application from
             having to wait on cancellation longer than might be desired.
             
             <b>N.B.</b>: The rendering increment is a <b>suggestion</b> to the
             implementation, which may render more or less lines - but always
             whole lines - for each rendering increment.
             
             @param	rendering_increment	The number of display image lines to
             incrementally render that will be suggested to the rendering
             implementation.
             @see	rendering_increment_lines()
             */
            void rendering_increment_lines (int rendering_increment);
            
            void background_color (QRgb color);
            
            /**	Set the size, in display space, of an image rendering tile.
             
             The specified size becomes the default for all new images that
             are displayed.
             
             @param	size	The display size of an image rendering tile.
             <b>N.B.</b>: A size less than the minimum of 256 will be
             increased to the minimum.
             */
            void tile_size (int size);
            void tile_size (const QSize& size);
            
            void cancel_rendering ();
            
            /*------------------------------------------------------------------------------
             JP2 specific settings.
             */
            void JPIP_request_timeout (int seconds);
            void JPIP_proxy (const QString& proxy);
            void JPIP_cache_directory (const QString& pathname);
            
            
            private slots:
            
            /**	Handles the {@link Tiled_Image_Display::image_loaded(bool)} signal.
             
             @param	successful	true if the an image was successfully loaded;
             false otherwise.
             */
            void loaded (bool successful);
            
            /**	Handles the {@link
             Tiled_Image_Display::image_cursor_moved(const QPoint&, const QPoint&)}
             signal.
             
             @param	display_position	A QPoint specifying the cursor position
             in display image coordinates, which is the same as the position
             from the mouse move event.
             @param	image_position	A QPoint specifying the cursor position
             in source image coordinates. This will be -1,-1 if the cursor
             is not within the {@link displayed_image_region(int) displayed
             image region} of the reference band.
             */
            void cursor_moved
            (const QPoint& display_position, const QPoint& image_position);
            
            void scrollbar_value_changed ();
            
            void sliding_scale_value (int value);
            void sliding_scale_value (double value);
            void sliding_scale_value_changed (int value);
            
            /*==============================================================================
             Event Handlers
             */
        protected:
            
            virtual void resizeEvent (QResizeEvent* event);
            virtual void contextMenuEvent (QContextMenuEvent* event);
            
            virtual void mousePressEvent (QMouseEvent* event);
            virtual void mouseMoveEvent (QMouseEvent* event);
            virtual void mouseReleaseEvent (QMouseEvent* event);
            
            virtual void wheelEvent (QWheelEvent* event);
            
            virtual void keyPressEvent (QKeyEvent* event);
            virtual void keyReleaseEvent (QKeyEvent* event);
            
            /*==============================================================================
             Helpers
             */
        private:
            
            void layout_display (QSize display_size);
            
            void adjust_scrollbars_range ();
            
            void update_sliding_scale ();
            int scale_to_slider (double value);
            double slider_to_scale (int value);
            
            void create_menus ();
            void update_actions ();
            void update_scaling_actions ();
            void update_window_fit_actions ();
            void update_copy_action();
            
            void change_cursor ();
            
            /*==============================================================================
             Data
             */
        private:
            
            QString
            Source_Name;
            
            Tiled_Image_Display
            *Image_Display;
            
            QScrollBar
            *Horizontal_Scrollbar,
            *Vertical_Scrollbar;
            static int
            Horizontal_Scrollbar_Height,
            Vertical_Scrollbar_Width;
            bool
            Scrollbars_Enabled;
            
            QWidget
            *LRC_Widget;
            
            static double
            Scaling_Minor_Increment,
            Scaling_Major_Increment;
            QSlider
            *Sliding_Scale;
            static int
            Sliding_Scale_Width;
            QLabel
            *Sliding_Scale_Value;
            
            QMenu
            *View_Menu;
            QAction
            *Scale_Up_Action,
            *Scale_Down_Action,
            *Normal_Size_Action,
            *Fit_to_Window_Action,
            *Fit_to_Width_Action,
            *Fit_to_Height_Action,
            *Copy_Action;
            QPoint
            Menu_Position;
            
            int
            Control_Mode,
            Times_Copied;
            QPoint
            Mouse_Drag_Image_Position;
            QCursor
            *Default_Cursor;
            
            Projection* Projector;
            
            /*
             Flag to block unnecessary and undesirable image updates as a result
             of primary image updates. For example, scaling is followed by an
             update of the viewer layout which includes changing scrollbar
             positions which generate signals that would ordinarily result in
             moving the image accordingly; but the image has already been moved by
             the primary image scaling operation so the redundant two additional
             image move operations are unnecessary and undesireably expensive
             (tiles updated and rendering queued, etc.) to allow them to occur.
             Setting this flag before the layout update - and resetting it
             afterwards, of course - avoids the wasted time and expense.
             */
            bool
            Block_Image_Updates;
            
            
            
            static QErrorMessage
            *Error_Message;
        };
        
        
    }	//	namespace HiRISE
}	//	namespace UA
#endif
