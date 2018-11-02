#include "Image_Viewer.hh"
#include "Statistics_Tool.hh"
#include "Data_Mapper_Tool.hh"
using namespace UA::HiRISE;

#include "Mac_Voice_Adapter.hh"
#include "MacSpeechHandler.h"


/*==============================================================================
	Constants
*/
const char* const
	Mac_Voice_Adapter::ID =
		"Mac_Voice_Adapter ($Revision: 2.1 $ $Date: 2013/04/08 19:29:10 $)";


/*==============================================================================
	Constructors
*/
Mac_Voice_Adapter::Mac_Voice_Adapter
(
   UA::HiRISE::Image_Viewer* viewer,
   UA::HiRISE::Statistics_Tools* stattool,
   UA::HiRISE::Data_Mapper_Tool* mapper
)
 : Voice_Adapter(viewer, stattool, mapper)
{
    speechHandler = new MacSpeechHandler(this);
}

void Mac_Voice_Adapter::toggle(bool on)
{
    if (speechHandler) on ? speechHandler->listen() : speechHandler->suspend();
}
