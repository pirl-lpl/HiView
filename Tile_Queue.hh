/* Tile_Queue.hh

Copyright (C) 2010-2025 Arizona Board of Regents on behalf of the
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

#include <QList>
#include <QMutex>

#include "Image_Tile.hh"

namespace UA::HiRISE
{
class Tile_Queue
{
    public:
    void enqueue(Image_Tile* tile, bool prioritize = false);
    bool dequeue(Image_Tile* tile);
    Image_Tile* getTile(int index) const;
    Image_Tile* dequeue();
    void clear();
    void remove(int index);
    void replace(int index, Image_Tile* tile);
    int priorityStatus() const;
    int find(const Dynamic_Image* image) const;
    void remove(Dynamic_Image* image);

    int size() const;

    bool isEmpty() const;
    private:
    mutable QMutex mutex;
    QList<Image_Tile*> queue;
};
} // namespace UA::HiRISE
