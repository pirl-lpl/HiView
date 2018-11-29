#	HiView
#
#	CVS ID: $Id: HiView.pro,v 1.93 2016/01/07 22:13:32 guym Exp $

TEMPLATE 		=	app
TARGET			=	HiView

QT				+=	core \
					gui \
               widgets \
					network \
					script

CONFIG			+=	qt \
					thread \
               c++1z

#	Include support for large files.
unix:CONFIG		+=	largefile

#	Host system machine architecture.
MACHINE = $$system(uname -m)
! build_pass:message (Host machine architecture: $$MACHINE)
DEFINES			+=	MACHINE=$$MACHINE

#	Host system OS.
HOST_OS = $$system(uname -s)
! build_pass:message (Host operating system: $$HOST_OS)
contains (HOST_OS, CYGWIN.+): OS = WIN
else: OS = $$join(HOST_OS, _)
DEFINES			+=	OS=$$OS


#	Software versions.

! build_pass:message (QT_VERSION $$[QT_VERSION])

#	Version of the HiView module.
! exists (VERSION) {
	exists (../VERSION) {
		system(ln -s ../VERSION)
		}
	else {
		! build_pass:warning (-------------------------)
		! build_pass:warning (!!!> No VERSION file <!!!)
		! build_pass:warning (MODULE_VERSION set to 0.0)
		! build_pass:warning (-------------------------)
		MODULE_VERSION = 0.0
		}
	}
exists (VERSION) {
	MODULE_VERSION = $$system(tail -1 VERSION)
	isEmpty(MODULE_VERSION) {
		! build_pass:warning (-------------------------)
		! build_pass:warning (No module version found!!)
		! build_pass:warning (MODULE_VERSION set to 0.0)
		! build_pass:warning (-------------------------)
		MODULE_VERSION = 0.0
		}
	}
! build_pass:message (MODULE_VERSION $$MODULE_VERSION)
DEFINES			+=	MODULE_VERSION=$$MODULE_VERSION


defineTest(addDef) {
   key = $$1
   val = $$2

   !isEmpty(val) {
      !build_pass:message($$key = $$val)
      DEFINES += $$key='\\"'$$val'\\"'
   }
   export(DEFINES)
}

addDef(DOCUMENTATION_SEARCH_LOCATIONS, $$(DOCUMENTATION_SEARCH_LOCATIONS))
addDef(DEFAULT_DOCUMENTATION_FILENAME, $$(DEFAULT_DOCUMENTATION_FILENAME))

macx: addDef(EXPORT_QMAKE_MAC_SDK, macosx)

PIRL_ROOT = $$(PIRL_ROOT)
IDAEIM_ROOT = $$(IDAEIM_ROOT)
KAKADU_ROOT = $$(KAKADU_ROOT)
QWT_ROOT = $$(QWT_ROOT)

isEmpty(QWT_ROOT) {
   ! build_pass:warning("QWT_ROOT undefined, using")

   unix:!macx {
      QWT_ROOT = /usr/local/qwt
   }

   macx {
      QWT_ROOT = /opt/qwt
      DYLD_FRAMEWORK_PATH += $(QWT_ROOT)/lib
   }

   win32 {
      QWT_ROOT = $(HOME)/qwt
   }

   ! build_pass:message($$QWT_ROOT)
}

include ( $(QWT_ROOT)/features/qwt.prf )

isEmpty(IDAEIM_ROOT) {
   ! build_pass:warning("IDAEIM_ROOT undefined, using")

   unix:!macx {
      IDAEIM_ROOT = $(HOME)/idaeim
   }

   macx {
      IDAEIM_ROOT = /opt/idaeim
   }

   win32 {
      IDAEIM_ROOT = $(HOME)/idaeim
   }

   ! build_pass:message($$IDAEIM_ROOT)
}

isEmpty(PIRL_ROOT) {
   ! build_pass:warning("PIRL_ROOT undefined, using")

   unix:!macx {
      PIRL_ROOT = $(HOME)/PIRL++
   }   

   macx {
      PIRL_ROOT = /opt/local/PIRL
   }   

   win32 {
      PIRL_ROOT = $(HOME)/PIRL++
   }

   ! build_pass:message($$PIRL_ROOT)

}

isEmpty(KAKADU_ROOT) {
   ! build_pass:warning("KAKADU_ROOT undefined, using")

   unix:!macx {
      KAKADU_ROOT = $(HOME)/Kakadu
   }   

   macx {
      KAKADU_ROOT = /opt/Kakadu
   }   

   win32 {
      KAKADU_ROOT = $(HOME)/Kakadu
   }

   ! build_pass:message($$KAKADU_ROOT)

}

CONFIG			+=	debug_and_release
CONFIG(debug, debug|release) {
	DEFINES			+=	DEBUG_SECTION=$(DEBUG_SECTION)
	
	#	Selective by-QObject name DEBUG.
	DEFINES			+=	DEBUG_OBJECT=$(DEBUG_OBJECT)

	win32 {
		#	Save debugging information in the debug directory
		#QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:LIBCMTD
		QMAKE_LFLAGS_DEBUG	+= /MAP:debug/mapfile.txt /MAPINFO:exports
		QMAKE_CXXFLAGS		+=	/Fd"debug/HiView.pdb"
		#	Provide console output on MS/Windows.
		#	Debug output (stdout) is always provided remotely through cygwin,
		#	but not on a local physical console without the console option.

		CONFIG				+=	console

		}
	}


#	Special compiler flags.

#	Disable MSVC "Function call with parameters that may be unsafe" warning.
win32: DEFINES		+= _SCL_SECURE_NO_WARNINGS

#macx:QMAKE_CXXFLAGS += -std=c++17

#	Mac application bundle assembly.
macx {
	#	Adds the Mac icons to the application bundle.
	ICON				=	Images/HiView_Icons.icns

	#	Info.plist file values.
	#	Application signature (4 characters).
	HiView_SIGNATURE	=	HiHV

	Info_plist.target	=	Info.plist
	Info_plist.depends	=	Info.plist.template $${TARGET}.app/Contents/Info.plist
	Info_plist.commands	=	@$(DEL_FILE) $${TARGET}.app/Contents/Info.plist$$escape_expand(\\n\\t) \
							@sed -e "s,@EXECUTABLE@,$$TARGET,g" -e "s,@VERSION@,$$MODULE_VERSION,g" -e "s,@TYPEINFO@,$$HiView_SIGNATURE,g" -e "s,@ICON@,$$basename(ICON),g" Info.plist.template > $${TARGET}.app/Contents/Info.plist
	QMAKE_EXTRA_TARGETS +=	Info_plist
	PRE_TARGETDEPS		+=	$$Info_plist.target

	PkgInfo.target		=	PkgInfo
	PkgInfo.depends		=	$${TARGET}.app/Contents/PkgInfo
	PkgInfo.commands	=	@$(DEL_FILE) $$PkgInfo.depends$$escape_expand(\\n\\t) \
							@echo "APPL$$HiView_SIGNATURE" > $$PkgInfo.depends
	QMAKE_EXTRA_TARGETS +=	PkgInfo
	PRE_TARGETDEPS		+=	$$PkgInfo.target

	#	Adds the Users Guide files to the application bundle Resources.
	user_docs.target	=	user_docs
	user_docs.depends	=	../Users_Guide
	user_docs.commands	=	@echo$$escape_expand(\\n\\t) \
							@echo "HiView Users Guide documents"$$escape_expand(\\n\\t) \
							@$(CHK_DIR_EXISTS) HiView.app/Contents/Resources || $(MKDIR) HiView.app/Contents/Resources$$escape_expand(\\n\\t) \
							rsync -avC ../Users_Guide/ $${TARGET}.app/Contents/Resources
	QMAKE_EXTRA_TARGETS	+=	user_docs
	}


#	Provides the specification of the MS/Windows application icon.
win32:RC_FILE 	= 	HiView.rc

#	Compiled application resources (icons).
RESOURCES		+=	HiView.qrc


#	External (non-Qt) dependencies:

DEPENDPATH += .

INCLUDEPATH += .

unix:INCLUDEPATH += \
	../JP2 \
	$$KAKADU_ROOT/include \
	$$PIRL_ROOT/include \
	$$IDAEIM_ROOT/include

win32:INCLUDEPATH += \
	../JP2 \
	$$KAKADU_ROOT/include \
	$$PIRL_ROOT/include \
	$$IDAEIM_ROOT/include 

#	Building with static libraries.
#  N.B. per Qwt License, static linking of Qwt is permitted

unix:LIBS += \
	../JP2/libJP2.a \
	../JP2/libJP2_Reader.a \
	../JP2/Kakadu/libKakadu_JP2.a \
	$$KAKADU_ROOT/lib/libkdu_aux.a \
	$$KAKADU_ROOT/lib/libkdu.a \
	$$PIRL_ROOT/lib/libPIRL++.a \
   $$IDAEIM_ROOT/lib/libQt_Utility.a \
	$$IDAEIM_ROOT/lib/libPVL.a \
	$$IDAEIM_ROOT/lib/libString.a \
	$$IDAEIM_ROOT/lib/libidaeim.a

win32:LIBS += \
	../JP2/libJP2.lib \
	../JP2/libJP2_Reader.lib \
	../JP2/Kakadu/libKakadu_JP2.lib \
	$$KAKADU_ROOT/lib/libkdu_a.lib \ 
	$$KAKADU_ROOT/lib/libkdu.lib \
	$$PIRL_ROOT/lib/libPIRL++.lib \ 
	$$IDAEIM_ROOT/lib/libPVL.lib \
	$$IDAEIM_ROOT/lib/libString.lib \
	$$IDAEIM_ROOT/lib/libidaeim.lib \
	Wsock32.lib \
	Advapi32.lib \
	Userenv.lib \
	winspool.lib \
	shell32.lib \
	Ws2_32.lib


#	Source code files:
HEADERS +=	\
	HiView_Application.hh \
	HiView_Config.hh \
	HiView_Utilities.hh \
	HiView_Window.hh \
	About_HiView_Dialog.hh \
	Image_Tile.hh \
	Synchronized_Event.hh \
	Image_Renderer.hh \
	Image_Renderer_Thread.hh \
	Plastic_Image.hh \
	Plastic_QImage.hh \
	JP2_Image.hh \
	Plastic_Image_Factory.hh \
	Tiled_Image_Display.hh \
	Image_Viewer.hh \
	Drawn_Line.hh \
	Navigator_Tool.hh \
	Count_Sequence.hh \
	Histogram_Plot.hh \
	Stats.hh \
	Statistics_Tool.hh \
	Statistics_and_Bounds_Tool.hh \
	Statistics_Tools.hh \
	Graph_Tracker.hh \
	Data_Mapper_Tool.hh \
	Function_Nodes.hh \
	Rotated_Label.hh \
	Activity_Indicator.hh \
	Icon_Button.hh \
	Image_Info_Panel.hh \
	Parameter_Tree_Model.hh \
	Value_Tree_Model.hh \
	Parameter_Tree_View.hh \
	Metadata_Dialog.hh \
	PDS_Metadata.hh \
	Qstream.hh \
	Save_Image_Dialog.hh \
	Save_Image_Thread.hh \
	URL_Checker.hh \
	Help_Docs.hh \
	Preferences_Dialog.hh \
	Coordinate.hh \
	Location_Mapper.hh \
	Projection.hh \
	Equirectangular_Projection.hh \
	Polar_Stereographic_Elliptical_Projection.hh \
	SpeechHandler.hh \
   FunctionEvaluator.hh \
	Voice_Adapter.hh \
	Distance_Line.hh


SOURCES +=	\
	HiView.cc \
	HiView_Application.cc \
	HiView_Config.cc \
	HiView_Utilities.cc \
	HiView_Window.cc \
	About_HiView_Dialog.cc \
	Image_Tile.cc \
	Synchronized_Event.cc \
	Image_Renderer.cc \
	Image_Renderer_Thread.cc \
	Plastic_Image.cc \
	Plastic_QImage.cc \
	JP2_Image.cc \
	Plastic_Image_Factory.cc \
	Tiled_Image_Display.cc \
	Image_Viewer.cc \
	Drawn_Line.cc \
	Navigator_Tool.cc \
	Count_Sequence.cc \
	Histogram_Plot.cc \
	Stats.cc \
	Statistics_Tool.cc \
	Statistics_and_Bounds_Tool.cc \
	Statistics_Tools.cc \
	Graph_Tracker.cc \
	Data_Mapper_Tool.cc \
	Function_Nodes.cc \
	Rotated_Label.cc \
	Activity_Indicator.cc \
	Icon_Button.cc \
	Image_Info_Panel.cc \
	Parameter_Tree_Model.cc \
	Value_Tree_Model.cc \
	Parameter_Tree_View.cc \
	Metadata_Dialog.cc \
	PDS_Metadata.cc \
	Qstream.cc \
	Save_Image_Dialog.cc \
	Save_Image_Thread.cc \
	Network_Status.cc \
	URL_Checker.cc \
	Help_Docs.cc \
	Preferences_Dialog.cc \
	Coordinate.cc \
	Location_Mapper.cc \
	Projection.cc \
	Equirectangular_Projection.cc \
	Polar_Stereographic_Elliptical_Projection.cc \
   SpeechHandler.cc \
   FunctionEvaluator.cc \
	Voice_Adapter.cc \
	Distance_Line.cc
   
mac {
    HEADERS += Mac_Voice_Adapter.hh MacSpeechHandler.h Voice_Adapter.hh \
               MacSpeechHandlerDelegate.h
                
    OBJECTIVE_SOURCES +=   Mac_Voice_Adapter.mm MacSpeechHandler.mm \
                         MacSpeechHandlerDelegate.mm
                         
    LIBS += -framework Foundation -framework AppKit
}    
