/*	ScriptUtility

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

#include <QStringList>
#include <QJSEngine>
#include <QJSValue>
#include <QList>
#include <QStringRef>

#include "PVL.hh"


namespace UA::HiRISE
{
void array_to_string(idaeim::PVL::Array &array, QJSValue &engine_array);

QList<bool> * parse_variable_names(const QStringRef& input, const QList<QString> *list);

} // namespace UA::HiRISE
