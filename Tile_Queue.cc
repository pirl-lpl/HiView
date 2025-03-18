/* Tile_Queue.cc

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

#include "Tile_Queue.hh"

#include <QMutexLocker>

namespace UA::HiRISE
{
void Tile_Queue::enqueue(Image_Tile* tile, bool prioritize)
{
    QMutexLocker locker(&mutex);

    if (!prioritize)
    {
        queue.append(tile);
        return;
    }

    auto area = tile->area();

    for (int i = 0; i < queue.size(); i++)
    {
        if (queue[i]->is_high_priority() && queue[i]->area() > area) continue;
        queue.insert(i, tile);
        return;
    }
}

bool Tile_Queue::dequeue(Image_Tile* tile)
{
    QMutexLocker locker(&mutex);

    if (queue.isEmpty())
    {
        return false;
    }

    int index = find(tile->Image);

    if (index >= 0)
    {
        queue.remove(index);
        return true;
    }

    return false;
}

Image_Tile* Tile_Queue::dequeue()
{
    QMutexLocker locker(&mutex);

    return queue.isEmpty() ? NULL : queue.takeFirst();
}

int Tile_Queue::find(const Dynamic_Image* image) const
{
    QMutexLocker locker(&mutex);

    for (int index = queue.size(); index > 0;)
    {
        if (queue[--index]->Image == image) return index;
    }

    return -1;
}

Image_Tile* Tile_Queue::getTile(int index) const
{
    QMutexLocker locker(&mutex);

    return (queue.isEmpty() || index >= queue.size()) ? NULL : queue.at(index);
}

void Tile_Queue::remove(Dynamic_Image* image)
{
    QMutexLocker locker(&mutex);

    for (int index = queue.size(); index > 0;)
    {
        if (queue[--index]->Image == image) delete queue.takeAt(index);
    }
}

void Tile_Queue::clear()
{
    QMutexLocker locker(&mutex);

    for (int index = queue.size(); index > 0; --index)
    {
        delete queue.takeLast();
    }
}

void Tile_Queue::remove(int index)
{
    QMutexLocker locker(&mutex);

    if (queue.size() > index) delete queue.takeAt(index);
}

int Tile_Queue::priorityStatus() const
{
    return (queue.size() > 0) ? queue.first()->status() : 0;
}

void Tile_Queue::replace(int index, Image_Tile* tile)
{
    QMutexLocker locker(&mutex);

    queue.remove(index);
    queue.insert(index, tile);
}

int Tile_Queue::size() const
{
    QMutexLocker locker(&mutex);

    return queue.size();
}

bool Tile_Queue::isEmpty() const
{
    QMutexLocker locker(&mutex);

    return queue.isEmpty();
}

} // namespace UA::HiRISE
