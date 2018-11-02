/*	Count_Sequence

HiROC CVS ID: $Id: Count_Sequence.cc,v 1.5 2012/03/09 02:13:55 castalia Exp $

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

#include	"Count_Sequence.hh"


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_MANIPULATORS		(1 << 2)
#define	DEBUG_HELPERS			(1 << 3)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constructors
*/
Count_Sequence::Count_Sequence ()
	:	QwtSeriesData (),
		Max_Count (0),
		Base (0.0),
		Increment (1.0)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Count_Sequence @ " << (void*)this << endl
	 << "    Data size = " << Data.size () << endl
	 << "    Max_Count = " << Max_Count << endl
	 << "    Base = " << Base << endl
	 << "    Increment = " << Increment << endl
	 << "<<< Count_Sequence" << endl;
#endif
}

Count_Sequence::Count_Sequence
	(
	const Count_Sequence&	count_sequence,
	double					base,
	double					increment
):	QwtSeriesData (),
		Data (count_sequence.Data),
		Max_Count (count_sequence.Max_Count),
		Base (base),
		Increment (increment)
{}


Count_Sequence::Count_Sequence
	(
	const QVector<unsigned long long>&	data_values,
	double								base,
	double								increment
):	QwtSeriesData (),
		Data (),
		Max_Count (0),
		Base (base),
		Increment (increment)
{data (data_values.data (), data_values.size ());}


Count_Sequence::Count_Sequence
	(
	const unsigned long long*	data_values,
	int							size,
	double						base,
	double						increment
	):	QwtSeriesData (),
		Data (),
		Max_Count (0),
		Base (base),
		Increment (increment)
{data (data_values, size);}


Count_Sequence::~Count_Sequence ()
{}

/*==============================================================================
	Local functions
*/
#ifndef	DOXYGEN_PROCESSING
namespace
{
unsigned long long
max_count_value
	(
	const QVector<unsigned long long>&	data
	)
{
unsigned long long
	datum,
	max_datum = 0;
int
	index = data.size ();
while (index--)
	{
	datum = data.at (index);
	if (max_datum < datum)
		max_datum = datum;
	}
return max_datum;
}
}
#endif

/*==============================================================================
	Accessors
*/
QwtSeriesData<QPointF>*
Count_Sequence::copy () const
{
Count_Sequence
	*data_copy = new Count_Sequence;
data_copy->Data = Data;
data_copy->Max_Count = Max_Count;
return data_copy;
}


Count_Sequence&
Count_Sequence::operator=
	(
	const Count_Sequence&	count_sequence
	)
{
data (count_sequence.Data);
return *this;
}


Count_Sequence&
Count_Sequence::data
	(
	const Count_Sequence&	count_sequence
	)
{data (count_sequence.Data); return *this;}


Count_Sequence&
Count_Sequence::data
	(
	const QVector<unsigned long long>&	data_sequence
	)
{
if (&Data != &data_sequence)
	Data = data_sequence;

//	The data sequence values may have changed even if the container didn't.
calculate_max_count ();
return *this;
}


Count_Sequence&
Count_Sequence::data
	(
	const unsigned long long*	data_sequence,
	int							size
	)
{
Data.clear ();
Max_Count = 0;
if (size)
	{
	Data.reserve (size);
	while (size--)
		{
		if (Max_Count < *data_sequence)
			Max_Count = *data_sequence;
		Data.append (*data_sequence++);
		}
	}
return *this;
}


void
Count_Sequence::size
	(
	int		amount
	)
{
if (amount <= 0)
	{
	Data.clear ();
	Max_Count = 0;
	}
else
if (Data.size () != amount)
	{
	Data.resize (amount);
	calculate_max_count ();
	}
}


size_t
Count_Sequence::size () const
{return Data.size ();}


double
Count_Sequence::x
	(
	size_t	index
	) const
{return Base + (index * Increment);}


double
Count_Sequence::y
	(
	size_t	index
	) const
{return (index < (size_t)Data.size ()) ? Data.at (index) : -1.0;}

QPointF 
Count_Sequence::sample (size_t index) const
{
   return QPointF(x(index), y(index));
}

QwtDoubleRect
Count_Sequence::boundingRect () const
{
return QwtDoubleRect
	(
	Base,						//	Left.
	Max_Count,					//	Top.
	Data.size () * Increment,	//	Width.
	Max_Count					//	Height.
	);
}


unsigned long long
Count_Sequence::calculate_max_count ()
{return (Max_Count = max_count_value (Data));}

}	//	namespace HiRISE
}	//	namespace UA
