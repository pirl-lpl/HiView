#include "Image_Viewer.hh"
#include "Statistics_Tools.hh"
#include "Data_Mapper_Tool.hh"
#include "Voice_Adapter.hh"
using namespace UA::HiRISE;

#include <cmath>
using namespace std;

#include <QtDebug>

/*==============================================================================
	Constants
*/
const char* const
	Voice_Adapter::ID =
		"Voice_Adapter ($Revision: 2.4 $ $Date: 2014/05/23 00:49:35 $)";

#define SHIFT_FRACTION 0.95

/*==============================================================================
	Constructors
*/
Voice_Adapter::Voice_Adapter
(
   UA::HiRISE::Image_Viewer* viewer,
   UA::HiRISE::Statistics_Tools* stattool,
   UA::HiRISE::Data_Mapper_Tool* mapper
)
 : viewer(viewer), stattool(stattool), mapper(mapper)
{
}

/*==============================================================================
	Callback Methods
*/
void Voice_Adapter::doEnhance()
{
   #ifndef QT_NO_DEBUG_OUTPUT
   qDebug() << "Received enhance command";
   #endif

   mapper->default_contrast_stretch();
}

void Voice_Adapter::doRestore()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received restore command";
    #endif

    mapper->restore_original_contrast_stretch();
}

void Voice_Adapter::doFullSize()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received full size command";
    #endif
    
    viewer->actual_size();
}

void Voice_Adapter::doFitImage()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received fit image command";
    #endif
    
    viewer->fit_image_to_window();
}

void Voice_Adapter::doZoomIn()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Zooming in from " << viewer->image_scaling();
    #endif
    
    //QSizeF image_scaling = viewer->image_scaling();
    
    viewer->scale_up(/*image_scaling.height()/2*/);
}

void Voice_Adapter::doZoomOut()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Zooming out from " << viewer->image_scaling();
    #endif
    
    //QSizeF image_scaling = viewer->image_scaling();
    
    viewer->scale_down(/*image_scaling.height()*1.5*/);
}

void Voice_Adapter::doPanUp()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan up command";
    #endif
    
    QSize size = viewer->image_display_size();
    
    if (size.height() < 1) return;
    
    size.setWidth(0);
    size.setHeight(std::ceil(size.height() * SHIFT_FRACTION));
    
    viewer->shift_image(size);
}

void Voice_Adapter::doPanDown()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan out command";
    #endif
    
    
    QSize size = viewer->image_display_size();
    
    if (size.height() < 1) return;
    
    size.setWidth(0);
    size.setHeight(std::ceil(-size.height() * SHIFT_FRACTION));
    
    viewer->shift_image(size);
}

void Voice_Adapter::doPanLeft()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan left command";
    #endif
    
    
    QSize size = viewer->image_display_size();
    
    if (size.width() < 1) return;
    
    size.setHeight(0);
    size.setWidth(std::ceil(size.width() * SHIFT_FRACTION));
    
    viewer->shift_image(size);
}

void Voice_Adapter::doPanRight()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan right command";
    #endif
    
    
    QSize size = viewer->image_display_size();
    
    if (size.width() < 1) return;
    
    size.setHeight(0);
    size.setWidth(std::ceil(-size.width() * SHIFT_FRACTION));
    
    viewer->shift_image(size);
}

