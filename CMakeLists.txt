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
	HiView_Application.hh
	HiView_Config.hh
	HiView_Utilities.hh
	HiView_Window.hh
	About_HiView_Dialog.hh
	Image_Tile.hh
	Synchronized_Event.hh
	Image_Renderer.hh
	Image_Renderer_Thread.hh
	Plastic_Image.hh
	Plastic_QImage.hh
	JP2_Image.hh
	Plastic_Image_Factory.hh
	Tiled_Image_Display.hh
	Image_Viewer.hh
	Drawn_Line.hh
	Navigator_Tool.hh
	Count_Sequence.hh
	Histogram_Plot.hh
	Stats.hh
	Statistics_Tool.hh
	Statistics_and_Bounds_Tool.hh
	Statistics_Tools.hh
	Graph_Tracker.hh
	Data_Mapper_Tool.hh
	Function_Nodes.hh
	Rotated_Label.hh
	Activity_Indicator.hh
	Icon_Button.hh
	Image_Info_Panel.hh
	Parameter_Tree_Model.hh
	Value_Tree_Model.hh
	Parameter_Tree_View.hh
	Metadata_Dialog.hh
	PDS_Metadata.hh
	Qstream.hh
	Save_Image_Dialog.hh
	Save_Image_Thread.hh
	URL_Checker.hh
	Help_Docs.hh
	Preferences_Dialog.hh
	Coordinate.hh
	Location_Mapper.hh
	Projection.hh
	Equirectangular_Projection.hh
	Polar_Stereographic_Elliptical_Projection.hh
	SpeechHandler.hh
   FunctionEvaluator.hh
	Voice_Adapter.hh
	Distance_Line.hh
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

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Script Qwt idaeim::PVL PIRL::PIRL++ UA::HiRISE::KakaduReaders KDU KDU_AUX)