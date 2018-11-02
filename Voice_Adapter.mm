#include "HiView_Window.hh"
#include "Image_Viewer.hh"
#include "Statistics_Tool.hh"
#include "Data_Mapper_Tool.hh"
#include "Voice_Adapter.hh"
using namespace UA::HiRISE;

#include <cmath>
using namespace std;

#include <QObject>

/*==============================================================================
	Constants
*/
const char* const
	Voice_Adapter::ID =
		"Voice_Adapter ($Revision: 2.1 $ $Date: 2013/04/08 19:29:11 $)";

#define SHIFT_FRACTION 0.85

/*==============================================================================
	Constructors
*/
Voice_Adapter::Voice_Adapter
(
   UA::HiRISE::HiView_Window &window,
   UA::HiRISE::Image_Viewer &viewer,
   UA::HiRISE::Statistics_Tools &stattool,
   UA::HiRISE::Data_Mapper_Tool &mapper
)
 : viewer(viewer), window(window), stattool(stattool), mapper(mapper)
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

   mapper.default_contrast_stretch();
}

void Voice_Adapter::doRestore()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received restore command";
    #endif

}

void Voice_Adapter::doZoomIn()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received zoom in command";
    #endif
    
    viewer.scale_up(2);
}

void Voice_Adapter::doZoomOut()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received zoom out command";
    #endif
    
    viewer.scale_down(2);
}

void Voice_Adapter::doPanUp()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan up command";
    #endif
    
    QSize size = viewer.image_display_size();
    
    if (size.height() < 1) return;
    
    size.setWidth(0);
    size.setHeight(std::ceil(size.height() * SHIFT_FRACTION));
    
    viewer.shift_image(size);
}

void Voice_Adapter::doPanDown()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan out command";
    #endif
    
    
    QSize size = viewer.image_display_size();
    
    if (size.height() < 1) return;
    
    size.setWidth(0);
    size.setHeight(std::ceil(-size.height() * SHIFT_FRACTION));
    
    viewer.shift_image(size);
}

void Voice_Adapter::doPanLeft()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan left command";
    #endif
    
    
    QSize size = viewer.image_display_size();
    
    if (size.width() < 1) return;
    
    size.setHeight(0);
    size.setWidth(std::ceil(-size.width() * SHIFT_FRACTION));
    
    viewer.shift_image(size);
}

void Voice_Adapter::doPanRight()
{
    #ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan right command";
    #endif
    
    
    QSize size = viewer.image_display_size();
    
    if (size.width() < 1) return;
    
    size.setHeight(0);
    size.setWidth(std::ceil(size.width() * SHIFT_FRACTION));
    
    viewer.shift_image(size);
}

