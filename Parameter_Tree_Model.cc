/*	Parameter_Tree_Model

HiROC CVS ID: $Id: Parameter_Tree_Model.cc,v 1.23 2012/11/13 22:29:37 guym Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
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

#include	"Parameter_Tree_Model.hh"

#include	"PVL.hh"
using idaeim::PVL::Aggregate;
using idaeim::PVL::Parameter;
using idaeim::PVL::Value;

#include	<QTreeView>

#include	<string>
using std::string;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:

	Note: In the context of Makefiles generated by the Qt qmake utility
	the DEBUG_SECTION token is always defined when a "debug" build is
	done. The token will be defined as empty by default in the Makefile
	unless DEBUG_SECTION is defined by the user as an environment
	variable or on the make command line. Since an empty definition can
	not be distinguished from a 0 definition the default when a debug
	build is done is to use DEBUG_OFF.
*/
#define	DEBUG_OFF				0
#define	DEBUG_ALL				(-1)
#define	DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_MODEL				(1 << 1)
#define DEBUG_DATA				(1 << 2)
#define DEBUG_ARRAY_SUBTREES	(1 << 3)

#if (DEBUG_SECTION + 0) == 0
#undef	DEBUG_SECTION
#define	DEBUG_SECTION DEBUG_OFF

#else
#include <iostream>
#include <iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
using std::hex;
using std::dec;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Parameter_Tree_Model::ID = "UA::HiRISE::Parameter_Tree_Model ($Revision: 1.23 $ $Date: 2012/11/13 22:29:37 $)";


#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)		"" #__VA_ARGS__ ""
#define AS_STRING(...)			STRINGIFIED(__VA_ARGS__)
#endif

#ifndef DEFAULT_PARAMETER_TREE_ROOT_NAME
#define DEFAULT_PARAMETER_TREE_ROOT_NAME	Root
#endif
#define _DEFAULT_ROOT_NAME_	AS_STRING(DEFAULT_PARAMETER_TREE_ROOT_NAME)
const char* const
	Parameter_Tree_Model::DEFAULT_ROOT_NAME		= _DEFAULT_ROOT_NAME_;

/*==============================================================================
	Constructors
*/
Parameter_Tree_Model::Parameter_Tree_Model
	(
	Aggregate*	root,
	QObject*	parent
	)
	:	QAbstractItemModel (parent),
		Root (root),
		Root_Visible (false),
		Root_Name (tr (DEFAULT_ROOT_NAME))
{
#if (DEBUG_SECTION & DEBUG_CONSTRUCTORS)
clog << "<-> Parameter_Tree_Model::Parameter_Tree_Model @ " << (void*)this
		<< ": root @ " << (void*)root << endl;
#endif
#if (DEBUG_SECTION != 0)
clog << boolalpha;
QHash<int, QByteArray>
	role_names;
role_names.insert (0, "Display");
role_names.insert (1, "Decoration");
role_names.insert (2, "Edit");
role_names.insert (3, "ToolTip");
role_names.insert (4, "StatusTip");
role_names.insert (5, "WhatsThis");
role_names.insert (6, "Font");
role_names.insert (7, "TextAlignment");
role_names.insert (8, "Background");
role_names.insert (9, "Foreground");
role_names.insert (10, "CheckState");
role_names.insert (11, "AccessibleText");
role_names.insert (12, "AccesibleDescription");
role_names.insert (13, "SizeHint");
//TODO QT4 OBSOLETE setRoleNames (role_names);
#endif
}


Parameter_Tree_Model::Parameter_Tree_Model
	(
	const Parameter_Tree_Model&	model
	)
	:	QAbstractItemModel (),
		Root (model.Root),
		Root_Visible (model.Root_Visible),
		Root_Name (model.Root_Name)
{
#if (DEBUG_SECTION & DEBUG_CONSTRUCTORS)
clog << "<-> Parameter_Tree_Model::Parameter_Tree_Model: copy @ "
		<< (void*)(&model) << " with root @ " << (void*)Root << endl;
#endif
#if (DEBUG_SECTION != 0)
clog << boolalpha;
QHash<int, QByteArray>
	role_names;
role_names.insert (0, "Display");
role_names.insert (1, "Decoration");
role_names.insert (2, "Edit");
role_names.insert (3, "ToolTip");
role_names.insert (4, "StatusTip");
role_names.insert (5, "WhatsThis");
role_names.insert (6, "Font");
role_names.insert (7, "TextAlignment");
role_names.insert (8, "Background");
role_names.insert (9, "Foreground");
role_names.insert (10, "CheckState");
role_names.insert (11, "AccessibleText");
role_names.insert (12, "AccesibleDescription");
role_names.insert (13, "SizeHint");
//TODO QT4 OBSOLETE setRoleNames (role_names);
#endif
}


Parameter_Tree_Model&
Parameter_Tree_Model::operator= (const Parameter_Tree_Model& model)
{
if (this != &model)
	{
	Root = model.Root;
	Root_Visible = model.Root_Visible;
	Root_Name = model.Root_Name;
	}
return *this;
}

/*==============================================================================
	QAbstractItemModel interface implementation
*/
QModelIndex
Parameter_Tree_Model::index
	(
	int					row,
	int					column,
	const QModelIndex&	parent_index
	) const
{
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << ">>> Parameter_Tree_Model::index: row " << row << " col " << column
		<< endl
	 << "    parent " << parent_index  << endl
	 << "      is valid = " << parent_index.isValid () << endl;
#endif
QModelIndex
	index;
if (column >= 0 &&
	column <= 1 &&
	parent_index.column () != 1)	//	Value column has no children.
	{
	Aggregate*
		parent = NULL;
	if (parent_index.isValid ())
		//	Non-root parent.
		parent = static_cast<Aggregate*>(parent_index.internalPointer ());
	else
		{
		//	Root parent.
		if (Root_Visible &&
			row == 0)
			{
			#if (DEBUG_SECTION & DEBUG_MODEL)
			clog << "    createIndex (" << row << ", " << column << ", "
					<< (void*)(Root) << ')' << endl
				 << "    at Root";
			#endif
			index = createIndex (row, column, Root);
			}
		else
			parent = Root;
		}

	if (parent &&
		row < (int)parent->size ())
		{
		// Set the internal pointer to the Parameter for the row.
		#if (DEBUG_SECTION & DEBUG_MODEL)
		clog << "    createIndex (" << row << ", " << column << ", "
				<< (void*)(&(*parent)[row]) << ')' << endl
			 << "    parameter " << parent->name () << '/'
			 	<< parent->at(row).name () << endl;
		#endif
		index = createIndex (row, column, &(*parent)[row]);
		}
	}
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << "<<< Parameter_Tree_Model::index: " << index << endl;
#endif
return index;
}


QVariant
Parameter_Tree_Model::data
	(
	const QModelIndex&	index,
	int					role
	) const
{
#if (DEBUG_SECTION & DEBUG_DATA)
clog << ">>> Parameter_Tree_Model::data:" << endl
	 << "    " << index << endl
	 << "    role = " << role
	 	<< " (" << qPrintable (QString (roleNames ().value (role)))
		<< ')' << endl;
#endif
QVariant
	datum;
if (role == Qt::TextAlignmentRole)
	{
	#if (DEBUG_SECTION & DEBUG_DATA)
	clog << "    TextAlignmentRole" << endl;
	#endif
	datum = static_cast<int>(Qt::AlignTop | Qt::AlignLeft);
	}
else
if (role == Qt::DisplayRole &&
	index.isValid ())
	{
	Parameter*
		parameter = static_cast<Parameter*>(index.internalPointer ());
	if (index.column () == 0)
		{
		//	Name column.
		if (Root_Visible &&
			Root == parameter)
			{
			#if (DEBUG_SECTION & DEBUG_DATA)
			clog << "    Root_Name " << qPrintable (Root_Name) << endl;
			#endif
			datum = Root_Name;
			}
		else
			{
			#if (DEBUG_SECTION & DEBUG_DATA)
			clog << "    parameter name " << parameter->name () << endl;
			#endif
			datum = QString::fromStdString (parameter->name ());
			}
		}
	else
	if (index.column () == 1)
		{
		//	Value column.
		if (parameter->is_Aggregate ())
			{
			#if (DEBUG_SECTION & DEBUG_DATA)
			clog << "    " << parameter->type_name () << " Aggregate "
					<< parameter->name () << endl;
			#endif
			datum = tr (parameter->type_name ().c_str ());
			}
		else
			{
			Value*
				value = &parameter->value ();
			if (value->is_Array ())
				{
				#if (DEBUG_SECTION & DEBUG_DATA)
				clog << "    " << value->type_name () << " Array" << endl;
				#endif
				datum = tr (value->type_name ().c_str ());
				}
			else
				{
				#if (DEBUG_SECTION & DEBUG_DATA)
				clog << "    Value = " << *value << endl;
				#endif
				datum = QString::fromStdString (static_cast<string>(*value));
				}
			}
		}
	}
#if (DEBUG_SECTION & DEBUG_DATA)
clog << "<<< Parameter_Tree_Model::data: "
		<< qPrintable (datum.toString ()) << endl;
#endif
return datum;
}


QModelIndex
Parameter_Tree_Model::parent
	(
	const QModelIndex&	index
	) const
{
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << ">>> Parameter_Tree_Model::parent: " << index << endl
	 << "      is valid = " << index.isValid () << endl;
#endif
QModelIndex
	parent_index;
if (index.isValid ())
	{
	Parameter*
		parameter = static_cast<Parameter*>(index.internalPointer ());
	if (parameter)
		parameter = parameter->parent ();
	if (parameter &&
		(parameter != Root ||
		 Root_Visible))
		parent_index = createIndex (parameter_row (parameter), 0, parameter);
	}
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << "<<< Parameter_Tree_Model::parent: " << parent_index << endl;
#endif
return parent_index;
}


int
Parameter_Tree_Model::rowCount
	(
	const QModelIndex&	index
	) const
{
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << ">>> Parameter_Tree_Model::rowCount: " << index << endl
	 << "      is valid = " << index.isValid () << endl;
#endif
int
	count = 0;

if (index.isValid ())
	{
	if (index.column () == 0)
		{
		Parameter*
			parameter = static_cast<Parameter*>(index.internalPointer ());
		if (parameter)
			{
			#if (DEBUG_SECTION & DEBUG_MODEL)
			clog << "    " << parameter->type_name () << " parameter "
					<< parameter->name () << endl;
			#endif
			if (parameter->is_Aggregate ())
				count = static_cast<Aggregate*>(parameter)->size ();
			}
		}
	}
else
	{
	#if (DEBUG_SECTION & DEBUG_MODEL)
	clog << "    tree root" << endl;
	#endif
	if (Root_Visible)
		count = 1;
	else
	if (Root)
		count = Root->size ();
	}
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << "<<< Parameter_Tree_Model::rowCount: " << count << endl;
#endif
return count;
}


QVariant
Parameter_Tree_Model::headerData
	(
	int				section,
	Qt::Orientation	orientation,
	int				role
	) const
{
QVariant
	data;
if (orientation == Qt::Horizontal &&
	role == Qt::DisplayRole &&
	section >= 0 &&
	section <= 1)
	data = ((section == 0) ? tr ("Name") : tr ("Value"));
return data;
}

/*==============================================================================
	Accessors
*/
void
Parameter_Tree_Model::root_visible
	(
	bool	enable
	)
{
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << ">>> Parameter_Tree_Model::root_visible: " << enable << endl;
#endif
if (Root_Visible != enable)
	{
	#if (DEBUG_SECTION & DEBUG_MODEL)
	clog << "    visibility changed" << endl;
	#endif
	//	>>> SIGNAL <<<
	emit layoutAboutToBeChanged ();
	Root_Visible = enable;
	//	>>> SIGNAL <<<
	emit layoutChanged ();
	}
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << "<<< Parameter_Tree_Model::root_visible" << endl;
#endif
}


void
Parameter_Tree_Model::root_name
	(
	const QString&	name
	)
{
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << ">>> Parameter_Tree_Model::root_name: " << name.toStdString() << endl;
#endif
if (Root_Name != name)
	{
	#if (DEBUG_SECTION & DEBUG_MODEL)
	clog << "    name changed from " << Root_Name.toStdString() << endl;
	#endif
	Root_Name = name;
	if (Root_Visible)
		{
		#if (DEBUG_SECTION & DEBUG_MODEL)
		clog << "    root is visible" << endl;
		#endif
		//	>>> SIGNAL <<<	Nudge the viewer to redisplay the model.
		emit dataChanged
			(index (0, 0, QModelIndex ()), index (0, 0, QModelIndex ()));
		}
	}
#if (DEBUG_SECTION & DEBUG_MODEL)
clog << "<<< Parameter_Tree_Model::root_name" << endl;
#endif
}


QString
Parameter_Tree_Model::comments
	(
	const QModelIndex&	index
	) const
{
QString
	comments;
Parameter*
	parameter = static_cast<Parameter*>(index.internalPointer ());
if (parameter)
	comments = parameter->comment ().c_str ();
return comments;
}

/*==============================================================================
	Helpers
*/
int
Parameter_Tree_Model::parameter_row
	(
	Parameter*	parameter
	) const
{
int
	row = -1;	//	Not found.
if (parameter)
	{
	Aggregate*
		parent = parameter->parent ();
	if (parent)
		{
		row = parent->size ();
		while (row--)
			if (&(*parent)[row] == parameter)
				break;
		}
	else
	if (Root_Visible)
		row = 0;
	}
return row;
}

/*==============================================================================
	Utility
*/
std::ostream&
operator<<
	(
	std::ostream&		stream,
	const QModelIndex&	index
	)
{
stream
	<< "QModelIndex = row "
		<< index.row () << " col " << index.column () << " object @ "
		<< (void*)index.internalPointer ();
return stream;
}


}	// end namespace HiRISE
}	// end namespace UA
