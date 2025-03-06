/*	Voice_Adapter

HiROC CVS ID: $Id: Voice_Adapter.hh,v 2.1 2013/04/08 19:29:11 guym Exp $

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
#include "Image_Viewer.hh"
#include "Statistics_Tool.hh"
#include "Data_Mapper_Tool.hh"
#include "Voice_Adapter.hh"
#ifdef Q_OS_WIN32
#include "WinSpeechHandler.hh"
#endif

using UA::HiRISE::Image_Viewer, UA::HiRISE::Statistics_Tool, UA::HiRISE::Data_Mapper_Tool;

#include <cmath>

#ifndef QT_NO_DEBUG_OUTPUT
#include <QDebug>
#endif

/*==============================================================================
    Constants
*/
const char *const
    Voice_Adapter::ID =
        "Voice_Adapter ($Revision: 2.4 $ $Date: 2014/05/23 00:49:35 $)";

const std::array<std::string, 10> Voice_Adapter::COMMANDS =
    {
        {"zoom in", "zoom out", "full size", "fit image",
         "pan left", "pan right", "pan up", "pan down",
         "enhance", "restore"}};

#define SHIFT_FRACTION 0.95

/*==============================================================================
    Constructors
*/
Voice_Adapter::Voice_Adapter(
    Image_Viewer *viewer,
    Statistics_Tool *stattool,
    Data_Mapper_Tool *mapper)
    : viewer(viewer), stattool(stattool), mapper(mapper)
{
#ifdef Q_OS_WIN32
    speechHandler = new WinSpeechHandler(this);
#endif
}

/*==============================================================================
    Callback Methods
*/
void Voice_Adapter::receiveCommand(const std::string &input)
{
    if (input == COMMANDS[0])
        doZoomIn();
    else if (input == COMMANDS[1])
        doZoomOut();
    else if (input == COMMANDS[2])
        doFullSize();
    else if (input == COMMANDS[3])
        doFitImage();
    else if (input == COMMANDS[4])
        doPanLeft();
    else if (input == COMMANDS[5])
        doPanRight();
    else if (input == COMMANDS[6])
        doPanUp();
    else if (input == COMMANDS[7])
        doPanDown();
    else if (input == COMMANDS[8])
        doEnhance();
    else if (input == COMMANDS[9])
        doRestore();
    else
    {
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "Unhandled input " << input.c_str();
#endif
    }
}

void Voice_Adapter::toggle(bool on)
{
    if (speechHandler)
        on ? speechHandler->listen() : speechHandler->suspend();
}

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

    // QSizeF image_scaling = viewer->image_scaling();

    viewer->scale_up(/*image_scaling.height()/2*/);
}

void Voice_Adapter::doZoomOut()
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Zooming out from " << viewer->image_scaling();
#endif

    // QSizeF image_scaling = viewer->image_scaling();

    viewer->scale_down(/*image_scaling.height()*1.5*/);
}

void Voice_Adapter::doPanUp()
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Received pan up command";
#endif

    QSize size = viewer->image_display_size();

    if (size.height() < 1)
        return;

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

    if (size.height() < 1)
        return;

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

    if (size.width() < 1)
        return;

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

    if (size.width() < 1)
        return;

    size.setHeight(0);
    size.setWidth(std::ceil(-size.width() * SHIFT_FRACTION));

    viewer->shift_image(size);
}
