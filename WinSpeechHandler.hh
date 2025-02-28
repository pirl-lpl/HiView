/*	WinSpeechHandler.hh

HiROC CVS ID: $Id: Image_Info_Panel.hh,v 1.14 2014/08/05 17:58:09 stephens Exp $

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
#pragma once

#include <QObject>

//#include <sphelper.h>
#include <sapi.h>

#include "Voice_Adapter.hh"

class WinSpeechHandler : public ISpNotifyCallback, public SpeechHandler
{
 public:
    explicit WinSpeechHandler(Voice_Adapter* adapter);
    virtual ~WinSpeechHandler();
    void listen();
    void suspend();

  HRESULT NotifyCallback(WPARAM   wParam, LPARAM   lParam);

 private:
    Voice_Adapter* voice_adapter;
    ISpRecoGrammar* recoGrammar;
    ISpRecoContext* recoContext;
    ISpRecognizer* recoInstance;
};
