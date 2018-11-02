/*	Stats

HiROC CVS ID: $Id: Stats.cc,v 1.15 2014/08/05 17:58:09 stephens Exp $

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

#include	"Stats.hh"

#include	<QtAlgorithms>

#include	<cmath>
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::invalid_argument;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_VALUE_AT			(1 << 2)
#define DEBUG_CALCULATE			(1 << 3)
#define DEBUG_VALUES			(1 << 4)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
using std::setw;

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Stats::ID =
		"UA::HiRISE::Stats ($Revision: 1.15 $ $Date: 2014/08/05 17:58:09 $)";

/*==============================================================================
	Constructors
*/
Stats::Stats ()
	:	Lower_Limit (0),
		Upper_Limit (0),
		Histograms (MAX_STATISTICS_SETS, NULL)
{clear ();}


Stats::Stats
	(
	const Stats&	stats
	)
{*this = stats;}


Stats&
Stats::operator=
	(
	const Stats&	stats
	)
{
if (this != &stats)
	{
	for (int
			index = 0;
			index < MAX_STATISTICS_SETS;
			index++)
		{
		Lowest_Value[index]      = stats.Lowest_Value[index];
		Highest_Value[index]     = stats.Highest_Value[index];
		Minimum_Count[index]     = stats.Minimum_Count[index];
		Maximum_Count[index]     = stats.Maximum_Count[index];
		Lower_Area[index]        = stats.Lower_Area[index];
		Valid_Area[index]        = stats.Valid_Area[index];
		Upper_Area[index]        = stats.Upper_Area[index];
		Mean_Value[index]        = stats.Mean_Value[index];
		Median_Value[index]      = stats.Median_Value[index];
		Std_Dev_of_Values[index] = stats.Std_Dev_of_Values[index];
		}
	Histograms = stats.Histograms;
	}
return *this;
}


Stats::~Stats ()
{}

/*==============================================================================
	Accessors
*/
void
Stats::limits
	(
	int		min,
	int		max
	)
{
if (min < 0)
	min = 0;
if (max < 0)
	max = 0;
bool
	recalculate =
		(min != Lower_Limit ||
		 max != Upper_Limit);
Lower_Limit = min;
Upper_Limit = max;
if (recalculate)
	calculate ();
}


Stats&
Stats::histogram
	(
	Histogram*	data,
	int			index
	)
{
index_range_check (index);
if (Histograms[index] != data)
	{
	Histograms[index] = data;
	calculate (index);
	}
return *this;
}


int
Stats::sets () const
{
int
	count = 0,
	index = MAX_STATISTICS_SETS;
while (index--)
	if (Histograms[index])
		++count;
return count;
}


unsigned long long
Stats::area
	(
	int				index,
	Range_Selection	range
	) const
{
index_range_check (index);
return
	((range == VALID_RANGE) ?
		Valid_Area[index] :
	((range == LOWER_RANGE) ?
		Lower_Area[index] :
		Upper_Area[index]));
}


double
Stats::upper_percent_at_value
	(
	int		value,
	int		index
	) const
{
#if ((DEBUG_SECTION) & DEBUG_VALUE_AT)
clog << ">>> Stats::upper_percent_at_value: value " << value
		<< ", index " << index << endl;
#endif
index_range_check (index);

#if ((DEBUG_SECTION) & DEBUG_VALUE_AT)
clog << "       Valid_Area[" << index << "] = " << Valid_Area[index] << endl
	 << "    Highest_Value[" << index << "] = " << Highest_Value[index] << endl;
#endif
if (value < 0)
	value = 0;
double
	percent = 0;
if (Histograms[index] &&
	Valid_Area[index] &&
	value < Highest_Value[index])
	{
	Histogram
		*histogram = Histograms[index];
	unsigned long long
		count = 0;
	for (int
			bin = Highest_Value[index];
			bin > value;
		  --bin)
		count += histogram->at (bin);

	#if ((DEBUG_SECTION) & DEBUG_VALUE_AT)
	clog << "    accumulated histogram count = " << count << endl;
	#endif
	percent = ((double)count / Valid_Area[index]) * 100.0;
	}
#if ((DEBUG_SECTION) & DEBUG_VALUE_AT)
clog << "<<< Stats::upper_percent_at_value: " << percent << endl;
#endif
return percent;
}


double
Stats::lower_percent_at_value
	(
	int		value,
	int		index
	) const
{
index_range_check (index);

if (value < 0)
	value = 0;
double
	percent = 0;
if (Histograms[index] &&
	Valid_Area[index] &&
	value > Lowest_Value[index])
	{
	Histogram
		*histogram = Histograms[index];
	unsigned long long
		count = 0;
	for (int
			bin = Lowest_Value[index];
			bin < value;
		  ++bin)
		count += histogram->at (bin);
	percent = ((double)count / Valid_Area[index]) * 100.0;
	}
return percent;
}


int
Stats::upper_value_at_percent
	(
	double	percent,
	int		index
	) const
{
index_range_check (index);

int
	value;
if (Histograms[index] &&
	percent < 100.0)
	{
	value = Highest_Value[index];

	if (Valid_Area[index] &&
		percent > 0.0)
		{
		Histogram
			*histogram = Histograms[index];
		unsigned long long
			count = histogram->at (value),
			area = static_cast<unsigned long long>
				((percent / 100.0) * Valid_Area[index]);
		while (count < area)
			count += histogram->at (--value);
		}
	}
else
	value = Lowest_Value[index];
return value;
}


int
Stats::lower_value_at_percent
	(
	double	percent,
	int		index
	) const
{
index_range_check (index);

int
	value;
if (Histograms[index] &&
	percent < 100.0)
	{
	value = Lowest_Value[index];

	if (Valid_Area[index] &&
		percent > 0.0)
		{
		Histogram
			*histogram = Histograms[index];
		unsigned long long
			count = histogram->at (value),
			area = static_cast<unsigned long long>
				((percent / 100.0) * Valid_Area[index]);
		while (count < area)
			count += histogram->at (++value);
		}
	}
else
	value = Highest_Value[index];
return value;
}

/*==============================================================================
	Manipulators
*/
void
Stats::calculate
	(
	int		index
	)
{
#if ((DEBUG_SECTION) & (DEBUG_CALCULATE | DEBUG_VALUES))
clog << ">>> Stats::calculate: " << index << endl;
#endif
int
	last_index;
if (index < 0)
	{
	//	All statistics sets will be scanned.
	last_index = MAX_STATISTICS_SETS;
	index = -1;
	}
else
	last_index = index_range_check (index) + 1;

Histogram
	*histogram;
int
	value,
	counted_values,
	lowest_counted,
	highest_counted,
	upper_limit_value;
Histogram::value_type
	count,
	min_count,
	max_count;
unsigned long long
	area_count,
	lower_area_count,
	upper_area_count;
double
	mean,
	offset,
	accumulator;

while (++index < last_index)
	{
	#if ((DEBUG_SECTION) & (DEBUG_CALCULATE | DEBUG_VALUES))
	clog << "    index " << index << " -" << endl;
	#endif
	//	Clear the summary values.
	Values_Counted[index]    = 0;
	Lowest_Value[index]      = -1;
	Highest_Value[index]     = -1;
	Minimum_Count[index]     = 0;
	Maximum_Count[index]     = 0;
	Valid_Area[index]        = 0;
	Lower_Area[index]        = 0;
	Upper_Area[index]        = 0;
	Mean_Value[index]        = 0;
	Median_Value[index]      = 0;
	Std_Dev_of_Values[index] = 0;

	if (Histograms[index])
		{
		//	Scan the histogram to update the summary values.
		counted_values   = 0;
		lowest_counted   = -1;
		highest_counted  = -1;
		//	N.B.: Assumes Histogram::value_type is unsigned.
		min_count        = static_cast<Histogram::value_type>(-1);
		max_count        = 0;
		area_count       = 0;
		lower_area_count = 0;
		upper_area_count = 0;
		accumulator      = 0;

		histogram = Histograms[index];
		upper_limit_value = histogram->size () - Upper_Limit - 1;
		value = histogram->size ();
		#if ((DEBUG_SECTION) & (DEBUG_CALCULATE | DEBUG_VALUES))
		clog << "    histogram size = " << value << endl;
		#endif
		while (--value > upper_limit_value)
			{
			#if ((DEBUG_SECTION) & DEBUG_VALUES)
			clog << "     " << setw (5) << value << ": "
					<< histogram->at (value) << endl;
			#endif
			upper_area_count += histogram->at (value);
			}

		while (value >= Lower_Limit)
			{
			count = histogram->at (value);
			#if ((DEBUG_SECTION) & DEBUG_VALUES)
			clog << "    ";
			if (value == upper_limit_value)
				clog << '+';
			else
			if (value == Lower_Limit)
				clog << '-';
			else
				clog << ' ';
			clog << setw (5) << value << ": " << count << endl;
			#endif
			if (count)
				{
				++counted_values;
				area_count += count;
				accumulator += (double)value * count;

				lowest_counted = value;
				if (highest_counted < 0)
					highest_counted = value;

				if (min_count > count)
					min_count = count;
				if (max_count < count)
					max_count = count;
				}
			else
				min_count = 0;
			--value;
			}

		while (value >= 0)
			{
			#if ((DEBUG_SECTION) & DEBUG_VALUES)
			clog << "     " << setw (5) << value << ": "
					<< histogram->at (value) << endl;
			#endif
			lower_area_count += histogram->at (value--);
			}

		Values_Counted[index] = counted_values;
		Lowest_Value[index]   = lowest_counted;
		Highest_Value[index]  = highest_counted;
		if (min_count == static_cast<unsigned long long>(-1))
			min_count = 0;
		Minimum_Count[index]  = min_count;
		Maximum_Count[index]  = max_count;
		Valid_Area[index]     = area_count;
		Lower_Area[index]     = lower_area_count;
		Upper_Area[index]     = upper_area_count;
		if (area_count)
			Mean_Value[index] = accumulator / area_count;

		if (counted_values)
			{
			max_count = area_count % 2;	//	Odd area_count?
			min_count = (area_count >> 1) + max_count;

			mean = Mean_Value[index];
			accumulator = 0;

			value = upper_limit_value + 1;
			while (value-- > Lower_Limit)
				{
				count = histogram->at (value);
				if (count)
					{
					if (! Median_Value[index])
						{
						if (count > min_count)
							{
							/*	At least half the counts lie above
								the current value.
							*/
							Median_Value[index] = value;
							if (min_count ||
								max_count)
								{
								//	Distribute the over count.
								Median_Value[index] +=
									(min_count + (max_count / 2.0)) / count;
								}
							}
						else
							min_count -= count;
						}

					offset = value - mean;
					accumulator += offset * offset;
					}
				}
			Std_Dev_of_Values[index] =
				sqrt (accumulator / counted_values);
			}
		}
	#if ((DEBUG_SECTION) & (DEBUG_CALCULATE | DEBUG_VALUES))
	clog << "             Lower_Area = " << Lower_Area[index] << endl
		 << "             Valid_Area = " << Valid_Area[index] << endl
		 << "             Upper_Area = " << Upper_Area[index] << endl
		 << "         Values_Counted = " << Values_Counted[index] << endl
		 << "           Lowest_Value = " << Lowest_Value[index] << endl
		 << "          Highest_Value = " << Highest_Value[index] << endl
		 << "          Minimum_Count = " << Minimum_Count[index] << endl
		 << "          Maximum_Count = " << Maximum_Count[index] << endl
		 << "             Mean_Value = " << Mean_Value[index] << endl
		 << "           Median_Value = " << Median_Value[index] << endl
		 << "      Std_Dev_of_Values = " << Std_Dev_of_Values[index] << endl;

	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_CALCULATE | DEBUG_VALUES))
clog << "<<< Stats::calculate" << endl;
#endif
}


void
Stats::clear ()
{
int
	index = MAX_STATISTICS_SETS;
while (index--)
	{
	Values_Counted[index]    = 0;
	Lowest_Value[index]      = -1;
	Highest_Value[index]     = -1;
	Minimum_Count[index]     = 0;
	Maximum_Count[index]     = 0;
	Lower_Area[index]        = 0;
	Valid_Area[index]        = 0;
	Upper_Area[index]        = 0;
	Mean_Value[index]        = 0;
	Median_Value[index]      = 0;
	Std_Dev_of_Values[index] = 0;

	if (Histograms[index])
		Histograms[index]->fill (0);
	}
}

/*==============================================================================
	Helpers
*/
int
Stats::index_range_check
	(
	int		index,
	int		max
	)
{
if (max == 0)
	max =  Stats::MAX_STATISTICS_SETS;
if (index < 0 ||
	index >= max)
	{
	ostringstream
		message;
	message << Stats::ID << endl
			<< "Invalid index number: " << index << endl
			<< "Maximum index number: " << max;
	throw invalid_argument (message.str ());
	}
return index;
}


}	//	namespace HiRISE
}	//	namespace UA
