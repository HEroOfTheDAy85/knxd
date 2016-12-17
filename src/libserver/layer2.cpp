/*
    EIBD eib bus access and management daemon
    Copyright (C) 2015 Matthias Urlichs <matthias@urlichs.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "layer2.h"
#include "layer3.h"

Layer2::Layer2 (L2options *opt, Trace *tr)
{
  l3 = 0;
  t = opt ? opt->t : 0;
  if (! t)
    t = tr;
  mode = BUSMODE_DOWN;
  allow_monitor = !(opt && (opt->flags & FLAG_B_NO_MONITOR));
  if (opt) opt->flags &=~ FLAG_B_NO_MONITOR;
}

bool
Layer2::init (Layer3 *layer3)
{
  l3 = layer3;
  if (! l3)
    return false;
  l3->registerLayer2 (shared_from_this());
  return true;
}

void
Layer2::Recv_L_Data (LPDU * l)
{
  l3->recv_L_Data (l);
}

void
Layer2::RunStop()
{
  if (l3) {
    l3->deregisterLayer2 (shared_from_this());
    l3 = 0;
  }
}

bool
Layer2::addAddress (eibaddr_t addr)
{
  unsigned i;
  for (i = 0; i < indaddr (); i++)
    if (indaddr[i] == addr)
      return false;
  indaddr.add (addr);
  return true;
}

bool
Layer2::addGroupAddress (eibaddr_t addr)
{
  unsigned i;
  for (i = 0; i < groupaddr (); i++)
    if (groupaddr[i] == addr)
      return false;
  groupaddr.add (addr);
  return true;
}

bool
Layer2::removeAddress (eibaddr_t addr)
{
  unsigned i;
  for (i = 0; i < indaddr (); i++)
    if (indaddr[i] == addr)
      {
        indaddr.deletepart (i, 1);
        return true;
      }
  return false;
}

bool
Layer2::removeGroupAddress (eibaddr_t addr)
{
  unsigned i;
  for (i = 0; i < groupaddr (); i++)
    if (groupaddr[i] == addr)
      {
        groupaddr.deletepart (i, 1);
        return true;
      }
  return false;
}

bool
Layer2::hasAddress (eibaddr_t addr)
{
  for (unsigned int i = 0; i < indaddr (); i++)
    if (indaddr[i] == addr)
      return true;
  return false;
}

bool
Layer2::hasGroupAddress (eibaddr_t addr)
{
  for (unsigned int i = 0; i < groupaddr (); i++)
    if (groupaddr[i] == addr || groupaddr[i] == 0)
      return true;
  return false;
}

bool
Layer2::enterBusmonitor ()
{
  if (!allow_monitor)
    return false;
  if (mode != BUSMODE_DOWN)
    return false;
  mode = BUSMODE_MONITOR;
  return true;
}

bool
Layer2::leaveBusmonitor ()
{
  if (mode != BUSMODE_MONITOR)
    return false;
  mode = BUSMODE_DOWN;
  return true;
}

bool
Layer2::Open ()
{
  if (mode != BUSMODE_DOWN)
    return false;
  mode = BUSMODE_UP;
  return true;
}

bool
Layer2::Close ()
{
  if (mode != BUSMODE_UP)
    return false;
  mode = BUSMODE_DOWN;
  return true;
}

bool
Layer2::Send_Queue_Empty ()
{
  return true;
}

void Layer2Single::addReverseAddress (eibaddr_t src, eibaddr_t dest)
{
  for (unsigned int i = 0; i < revaddr (); i++)
    if (revaddr[i].dest == dest) {
      revaddr[i].src = src;
      return;
    }
  phys_comm srcdest = (phys_comm) { .src=src, .dest=dest };
  revaddr.add(srcdest);
}

eibaddr_t Layer2Single::getDestinationAddress (eibaddr_t src)
{
  for (unsigned int i = 0; i < revaddr (); i++)
    if (revaddr[i].dest == src)
      return revaddr[i].src;

  return 0;
}

void Layer2Single::Send_L_Data (LPDU * l)
{
  /* Sending a packet to this interface: record address pair, clear source */
  if (l->getType () == L_Data) {
    L_Data_PDU *l1 = dynamic_cast<L_Data_PDU *>(l);
    if (l1->AddrType == IndividualAddress) {
      addReverseAddress (l1->source, l1->dest);
      l1->source = 0;
    }
  }
  Send_L_Data_(l);
}

void Layer2Single::Recv_L_Data (LPDU * l) {
  /* Receiving a packet from this interface: reverse-lookup real destination from source */
  if (l->getType () == L_Data) {
    L_Data_PDU *l1 = dynamic_cast<L_Data_PDU *>(l);
    if (l1->AddrType == IndividualAddress) {
      l1->dest = getDestinationAddress (l1->source);
    }
  }
  Layer2::Recv_L_Data(l);
}

