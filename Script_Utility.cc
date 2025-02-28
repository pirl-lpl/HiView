/*	Function_Evaluator.cc

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
#include <QDateTime>
#include <QString>
#include <QJSValue>

#include "Script_Utility.hh"
#include "PDS_Metadata.hh"

#ifndef QT_NO_DEBUG_OUTPUT
#include <QDebug>
#endif

#include <string>

namespace UA::HiRISE
{

    void array_to_string(idaeim::PVL::Array &array, QJSValue &engine_array)
    {
        auto end = array.end_depth();
        int index = 0;

        for (auto i = array.begin_depth(); i != end; ++i)
        {
            if (i->is_Array())
            {
                array_to_string(static_cast<idaeim::PVL::Array &>(*i), engine_array);
                continue;
            }

            engine_array.setProperty(index, QString::fromStdString(static_cast<std::string>(*i).c_str()));
            ++index;
        }
    }

    QList<bool> *parse_variable_names(const QStringRef &input, const QList<QString> *list)
    {
        // It is not the variable if:
        //  1)the name is contained within another variable
        //    -Variable names can contain letters, digits, underscores, and dollar signs.
        //  2)If the a field with the same name is being used in a custom object
        //    -Will use '.' to access the field and ':' to declare a value in an initializer list
        //       -Dont include ':' in reject list incase someone uses ? operator
        //  3)If a function with the same name is being used
        //    -Will have '(' after the name
        //  4)If the name is used in a comment or a string
        auto script = input.toString();

        int loc = 0;
        while (loc < script.length())
        {
            int min_type = 0;
            int min = script.indexOf('\"', loc);
            min = (min == -1) ? script.length() : min;
            int single_quote = script.indexOf('\'', loc);
            if (single_quote != -1 && single_quote < min)
            {
                min = single_quote;
                min_type = 1;
            }
            int line_comment = script.indexOf("//", loc);
            if (line_comment != -1 && line_comment < min)
            {
                min = line_comment;
                min_type = 2;
            }
            int multiline_comment = script.indexOf("/*", loc);
            if (multiline_comment != -1 && multiline_comment < min)
            {
                min = multiline_comment;
                min_type = 3;
            }
            if (min != script.length())
            {
                int j;
                switch (min_type)
                {
                case 0:
                    j = script.indexOf('\"', min + 1);
                    if (j != -1)
                    {
                        script.remove(min, j - min + 1);
                    }
                    else
                    {
                        script.remove(min, script.length() - min);
                    }
                    break;
                case 1:
                    j = script.indexOf('\'', min + 1);
                    if (min != -1)
                    {
                        script.remove(min, j - min + 1);
                    }
                    else
                    {
                        script.remove(min, script.length() - min);
                    }
                    break;
                case 2:
                    j = script.indexOf('\n', min + 2);
                    if (j != -1)
                    {
                        // leave in the new line
                        script.remove(min, j - min);
                    }
                    else
                    {
                        script.remove(min, script.length() - min);
                    }
                    break;
                case 3:
                    j = script.indexOf("*/", min + 2);
                    if (j != -1)
                    {
                        script.remove(min, j - min + 2);
                    }
                    else
                    {
                        script.remove(min, script.length() - min);
                    }
                    break;
                }
            }
            loc = min;
        }

        // check for exceptions
        QList<bool> *inList = new QList<bool>();
        QString reject_chars_before_var = "_$.";
        QString reject_chars_after_var = "_$.(";

        for (int i = 0; i < list->size(); ++i)
        {
            int j = 0;
            int len = list->at(i).length();
            while ((j = script.indexOf(list->at(i), j)) != -1)
            {
                if ((j == 0 || !(script[j - 1].isLetterOrNumber() ||
                reject_chars_before_var.contains(script[j - 1]))) &&

                    ((j + len) >= script.length() || !(script[j + len].isLetterOrNumber() ||
                    reject_chars_after_var.contains(script[j + len]))))
                {
                    break;
                }
                ++j;
            }

            inList->push_back(j > -1);
        }
        return inList;
    } // function declaration

} // namespace UA::HiRISE
