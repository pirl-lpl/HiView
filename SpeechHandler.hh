/*	SpeechHandler.hh

HiROC CVS ID: $Id$

Copyright (C) 2009-2025  Arizona Board of Regents on behalf of the
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

*/
#pragma once

// forward reference
class Voice_Adapter;

class SpeechHandler
{
  public:
  explicit SpeechHandler(Voice_Adapter* adapter);

  virtual void listen() = 0;
  virtual void suspend() = 0;

  protected:
  virtual ~SpeechHandler();
  Voice_Adapter* voice_adapter;
};
