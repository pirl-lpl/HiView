/*	Script_Evaluator.hh

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
//#include <QThread>
#include <QString>
#include <QSet>
#include <QJSEngine>
#include <QJSValue>
#include <QException>

#include <array>
#include <string>

#include "Stats.hh"
#include "Script_Utility.hh"

namespace idaeim::PVL
{
class Array;
class Aggregate;
}

namespace UA::HiRISE
{

const std::array<QString, 3> PDSMETA_LOW_VALUE_NAMES =
{{
    "CORE_NULL",
    "CORE_LOW_REPR_SATURATION",
    "CORE_LOW_INSTR_SATURATION"
}};

const std::array<QString, 2> PDSMETA_HIGH_VALUE_NAMES =
{{
    "CORE_HIGH_REPR_SATURATION",
    "CORE_HIGH_INSTR_SATURATION"
}};

class Script_Exception : public QException
{
 public:
    explicit Script_Exception(const QString& msg);
    void raise() const override;
    Script_Exception *clone() const override;

 private:
    QString message;
};

class Script_Evaluator : public QObject
{
    Q_OBJECT

 public:
    static const QString
    VAR_X_PX,
    VAR_Y_PX,
    VAR_BAND0_VAL,
    VAR_BAND1_VAL,
    VAR_BAND2_VAL,
    VAR_SCALE_METERS,
    VAR_COUNT_PIXELS,
    VAR_AREA_METERS2,
    VAR_WIDTH_METERS,
    VAR_HEIGHT_METERS,
    VAR_WIDTH_PIXELS,
    VAR_HEIGHT_PIXELS,
    VAR_LENGTH_PIXELS,
    VAR_LENGTH_METERS;

    static const QString
    TOOLTIP_VAR_X_PX,
    TOOLTIP_VAR_Y_PX,
    TOOLTIP_VAR_BAND0_VAL,
    TOOLTIP_VAR_BAND1_VAL,
    TOOLTIP_VAR_BAND2_VAL,
    TOOLTIP_VAR_SCALE_METERS,
    TOOLTIP_VAR_AREA_METERS2,
    TOOLTIP_VAR_COUNT_PIXELS,
    TOOLTIP_VAR_WIDTH_METERS,
    TOOLTIP_VAR_HEIGHT_METERS,
    TOOLTIP_VAR_WIDTH_PIXELS,
    TOOLTIP_VAR_HEIGHT_PIXELS,
    TOOLTIP_VAR_LENGTH_PIXELS,
    TOOLTIP_VAR_LENGTH_METERS;


  enum class BandOrdinal
  {
    NONE, FIRST, SECOND, THIRD
  };



    explicit Script_Evaluator(QObject* parent = NULL);

    virtual ~Script_Evaluator();

    void setProperty(const QString& name, const QJSValue& value);

    void run();
    void erred();
    void clearScriptProperties();

    long double getCount() const;
    long double getSum() const;

    double getAvg() const;
    double getMin() const;
    double getMax() const;
    int getMinAt() const;
    int getMaxAt() const;

    QString getPixelText() const;
    QString getScriptText() const;

    friend void array_to_string(idaeim::PVL::Array &array, QJSValue &engine_array);


 public slots:
    inline void setData(Stats& data) { stats = &data; }
    void setMetadata(idaeim::PVL::Aggregate* metadata);
    void setValidMin(const int value);
    void setValidMax(const int value);
    void setScript(const QString& input);

 private:
    void reset();


    QString function;

    long double total_pixel_count;
    long double evaluated_sum;
    double evaluated_avg;
    double evaluated_min;
    double evaluated_max;

    int min_at;
    int max_at;

    int valid_min;
    int valid_max;

    QJSEngine* engine;

    QSet <QString> imagevars;
    QSet <QString> labelvars;
    QJSValue global, undef;

    BandOrdinal selectedBand;

    Stats* stats;
};


} // namespace UA::HiRISE
