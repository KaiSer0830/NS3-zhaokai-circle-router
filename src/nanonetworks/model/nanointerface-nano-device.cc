/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013,2014 TELEMATICS LAB, DEI - Politecnico di Bari
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Giuseppe Piro <peppe@giuseppepiro.com>, <g.piro@poliba.it>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/channel.h"
#include "nanointerface-nano-device.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-entity.h"
#include "nano-mac-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "nano-routing-entity.h"

NS_LOG_COMPONENT_DEFINE("NanoInterfaceDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(NanoInterfaceDevice);

TypeId NanoInterfaceDevice::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::NanoInterfaceDevice").SetParent<SimpleNanoDevice>().AddConstructor<NanoInterfaceDevice>();
	return tid;
}

NanoInterfaceDevice::NanoInterfaceDevice() {
	NS_LOG_FUNCTION(this);
	m_type = SimpleNanoDevice::NanoInterface;
}

NanoInterfaceDevice::~NanoInterfaceDevice() {
	NS_LOG_FUNCTION(this);
}

void NanoInterfaceDevice::DoDispose() {
	NS_LOG_FUNCTION(this);
	SimpleNanoDevice::DoDispose();
}

} // namespace ns3
