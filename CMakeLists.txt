cmake_minimum_required(VERSION 3.17)

# Windows build
# set DEBUG_SECTION=DEBUG_ALL
# cmake --fresh -S ..\HiView -B HiView -DCMAKE_PREFIX_PATH=%HOMEPATH%
# -DQwt_DIR=%HOMEPATH%\Qwt-6.3.0 -DKAKADU_DIR=%HOMEPATH%\hiview3\v8_4-00307L
# cmake --build HiView --config Release
# devenv /debugexe HiView\HiView.exe

# MacOS build
# export DEBUG_SECTION=DEBUG_ALL
# cmake -S ../c++/HiView -B HiView -DCMAKE_BUILD_TYPE=DEBUG
# -DKAKADU_DIR=$HOME/build/v8_4-00307L
# cmake --build HiView
# lldb HiView/HiView
#
project(HiView)

set(PROJECT_VERSION "3.0.0-beta0")

cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
cmake_host_system_information(RESULT OS_RELEASE QUERY OS_RELEASE)
cmake_host_system_information(RESULT OS_VERSION QUERY OS_VERSION)

# Generate date and time stamp
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
string(TIMESTAMP BUILD_TIME "%H:%M:%S" UTC)

# Combine for a full identifier string
set(BUILD_INFO "${OS_NAME} ${OS_VERSION} - ${BUILD_DATE} ${BUILD_TIME}")

# prevents a warning message
cmake_policy(SET CMP0100 NEW)

# for FindQWT.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Network Core5Compat Qml)
qt_standard_project_setup()

find_package(PIRL 3.0.0 REQUIRED)

find_package(idaeim REQUIRED)
find_package(libJP2 REQUIRED)
find_package(Qwt REQUIRED)
find_package(Kakadu REQUIRED)

set(HEADERS
About_HiView_Dialog.hh
Activity_Indicator.hh
Coordinate.hh
Count_Sequence.hh
Data_Mapper_Tool.hh
Distance_Line.hh
Drawn_Line.hh
Equirectangular_Projection.hh
FunctionEvaluator.hh
Function_Nodes.hh
Graph_Tracker.hh
Help_Docs.hh
HiView_Application.hh
HiView_Config.hh
HiView_Utilities.hh
HiView_Window.hh
Histogram_Plot.hh
Icon_Button.hh
Image_Info_Panel.hh
Image_Blaster.hh
Image_Tile.hh
Image_Viewer.hh
JP2_Image.hh
Location_Mapper.hh
Mac_Voice_Adapter.hh
Metadata_Dialog.hh
Navigator_Tool.hh
Network_Status.hh
PDS_Metadata.hh
Parameter_Tree_Model.hh
Parameter_Tree_View.hh
Plastic_Image.hh
Plastic_Image_Factory.hh
Plastic_QImage.hh
Polar_Stereographic_Elliptical_Projection.hh
Preferences_Dialog.hh
Projection.hh
Qstream.hh
Rotated_Label.hh
Save_Image_Dialog.hh
Save_Image_Thread.hh
SpeechHandler.hh
Statistics_Tool.hh
Statistics_Tools.hh
Statistics_and_Bounds_Tool.hh
Stats.hh
Synchronized_Event.hh
Tiled_Image_Display.hh
URL_Checker.hh
Value_Tree_Model.hh
Voice_Adapter.hh
)

set(SOURCES
 HiView.cc
 HiView_Application.cc
 HiView_Config.cc
 HiView_Utilities.cc
 HiView_Window.cc
 About_HiView_Dialog.cc
 Image_Tile.cc
 Synchronized_Event.cc
 Image_Blaster.cc
 Plastic_Image.cc
 Plastic_QImage.cc
 JP2_Image.cc
 Plastic_Image_Factory.cc
 Tiled_Image_Display.cc
 Image_Viewer.cc
 Drawn_Line.cc
 Navigator_Tool.cc
 Count_Sequence.cc
 Histogram_Plot.cc
 Stats.cc
 Statistics_Tool.cc
 Statistics_and_Bounds_Tool.cc
 Statistics_Tools.cc
 Graph_Tracker.cc
 Data_Mapper_Tool.cc
 Function_Nodes.cc
 Rotated_Label.cc
 Activity_Indicator.cc
 Icon_Button.cc
 Image_Info_Panel.cc
 Parameter_Tree_Model.cc
 Value_Tree_Model.cc
 Parameter_Tree_View.cc
 Metadata_Dialog.cc
 PDS_Metadata.cc
 Qstream.cc
 Save_Image_Dialog.cc
 Save_Image_Thread.cc
 Network_Status.cc
 URL_Checker.cc
 Help_Docs.cc
 Preferences_Dialog.cc
 Coordinate.cc
 Location_Mapper.cc
 Projection.cc
 Equirectangular_Projection.cc
 Polar_Stereographic_Elliptical_Projection.cc
   SpeechHandler.cc
   FunctionEvaluator.cc
 Voice_Adapter.cc
 Distance_Line.cc
)

if(APPLE)
    list(APPEND HEADERS MacSpeechHandler.h MacSpeechHandlerDelegate.h)
    list(APPEND SOURCES MacSpeechHandler.mm MacSpeechHandlerDelegate.mm Mac_Voice_Adapter.mm)
endif()

list(LENGTH HEADERS HeaderCount)
list(LENGTH SOURCES SourceCount)
message(VERBOSE "Compiling ${HeaderCount} headers and ${SourceCount} Sources")

qt_add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_compile_definitions(${PROJECT_NAME} PRIVATE
APP_NAME="${PROJECT_NAME}"
APP_VERSION="${PROJECT_VERSION}"
APP_INFO="${BUILD_INFO}"
APP_ORGNAME="UAHiRISE.org")

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if  (DEFINED ENV{DEBUG_SECTION})
    message(STATUS "Setting DEBUG to $ENV{DEBUG_SECTION}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_SECTION=$ENV{DEBUG_SECTION})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE QWT_DLL=1)

#
# TODO try and understand why the linker says that QwtSeriesStore<QPointF> is defined twice, once
# in Function_Nodes.cc and once in Qwt, probably some inheritence issue
#
if (MSVC)
    # TODO disable these for release build
    #target_compile_definitions(${PROJECT_NAME} PRIVATE _DISABLE_VECTOR_ANNOTATION _DISABLE_STL_ANNOTATION)
    #target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address /Zi /Zc:preprocessor)

    target_link_options(${PROJECT_NAME} PRIVATE /FORCE)
endif()

if (APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE "/usr/local/include")
    #target_compile_definitions(${PROJECT_NAME} PRIVATE _DISABLE_VECTOR_ANNOTATION _DISABLE_STL_ANNOTATION)
    # use one of -fsanitize=address or -fsanitize=thread
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Wno-deprecated-declarations)
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE true)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    if(COREFOUNDATION_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${COREFOUNDATION_FRAMEWORK})
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${QWT_INCLUDE_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Core5Compat Qt6::Qml QWT
 KDU KDU_AUX idaeim::PVL idaeim::Strings idaeim::Utility PIRL::PIRL++ libJP2::JP2 libJP2::KakaduReaders)

qt_add_resources(${PROJECT_NAME} "${PROJECT_NAME}" PREFIX "/" FILES
 Images/HiView_Splash.png
 Images/HiView_Icon.png
 Images/X_button.png
 Images/OK.png
 Images/defaults_button.png
 Images/reset_button.png
 Images/reticule_cursor.png
 Images/reticule_cursor_mask.png
 Images/shift_cursor.png
 Images/shift_cursor_mask.png
 Images/shift_region_cursor.png
 Images/shift_vertical_cursor.png
 Images/shift_vertical_cursor_mask.png
 Images/shift_horizontal_cursor.png
 Images/shift_horizontal_cursor_mask.png
 Images/shift_region_vertical_cursor.png
 Images/shift_region_horizontal_cursor.png
 Images/shift_region_FDiag_cursor.png
 Images/shift_region_FDiag_cursor_mask.png
 Images/shift_region_BDiag_cursor.png
 Images/shift_region_BDiag_cursor_mask.png
 Images/scale_cursor.png
 Images/scale_cursor_mask.png
 Images/Blank_Bead_Red-32.png
 Images/Blank_Bead_Green-32.png
 Images/Blank_Bead_Blue-32.png
 Images/Blank_Bead_White-32.png
 Images/Blank_Bead_Grey-32.png
 Images/BW-32.png
 Images/Contrast_Stretch.png
 Images/redo_button.png
 Images/crosshair_cursor.png
 Images/crosshair_cursor_mask.png
 Images/greater_than_cursor_white.png
 Images/greater_than_cursor_red.png
 Images/greater_than_cursor_green.png
 Images/greater_than_cursor_blue.png
 Images/greater_than_cursor_yellow.png
 Images/greater_than_cursor_cyan.png
 Images/greater_than_cursor_magenta.png
 Images/greater_than_cursor_mask.png
 Images/less_than_cursor_white.png
 Images/less_than_cursor_red.png
 Images/less_than_cursor_green.png
 Images/less_than_cursor_blue.png
 Images/less_than_cursor_yellow.png
 Images/less_than_cursor_cyan.png
 Images/less_than_cursor_magenta.png
 Images/less_than_cursor_mask.png
 Images/About_Background.png
 Images/Expand_Down_Icon.png
 Images/Expand_Up_Icon.png
 Images/HiRISE_Logo.png
 Images/PIRL_Logo.png
 Images/LPL_Logo.png
 Images/UofA_Logo.png
 Images/NASA_Logo.png
)
