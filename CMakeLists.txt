cmake_minimum_required(VERSION 3.11)

project(HiView)

# TODO see https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# TODO put this in CMakeSettings.json
if (MSVC)
	set(Qt5_DIR "C:/Qt/5.11.2/msvc2017_64/lib/cmake/Qt5")
	set(Qwt_DIR "C:/Qwt-6.1.4")
	set(Kak_DIR "$ENV{USERPROFILE}\\kakadu\\kakadu-7.A.7")
endif()

# TODO add other components, e.g. Network?
find_package(Qt5 REQUIRED COMPONENTS Gui REQUIRED Widgets REQUIRED Network REQUIRED Script REQUIRED)
find_package(PIRL 3.0.0 REQUIRED)
find_package(idaeim 2.3.4 REQUIRED)
find_package(JP2 REQUIRED)

#
# Qwt
#
find_library(qwt_lib NAMES qwt PATHS "${Qwt_DIR}/lib" NO_DEFAULT_PATH)
find_path(qwt_inc qwt.h PATHS "${Qwt_DIR}/include")
add_library(Qwt STATIC IMPORTED)
set_target_properties(Qwt PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${qwt_inc} IMPORTED_LOCATION ${qwt_lib})

#
# Kakadu
#
add_library(KDU STATIC IMPORTED)
add_library(KDU_AUX STATIC IMPORTED)
find_library(kdu_lib NAMES "kdu_v7AR" PATHS "${Kak_DIR}/lib_x64" NO_DEFAULT_PATH)
set_target_properties(KDU PROPERTIES IMPORTED_LOCATION ${kdu_lib})
find_library(kdu_lib NAMES "kdu_a7AR" PATHS "${Kak_DIR}/lib_x64" NO_DEFAULT_PATH)
set_target_properties(KDU_AUX PROPERTIES IMPORTED_LOCATION ${kdu_lib})

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
Image_Renderer.hh
Image_Renderer_Thread.hh
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
	Image_Renderer.cc
	Image_Renderer_Thread.cc
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

list(LENGTH HEADERS HeaderCount)
list(LENGTH SOURCES SourceCount)
message("Compiling ${HeaderCount} headers and ${SourceCount} Sources")

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} "HiView.qrc")

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Script Qwt idaeim::PVL idaeim::Strings idaeim::Utility PIRL::PIRL++ UA::HiRISE::KakaduReaders)