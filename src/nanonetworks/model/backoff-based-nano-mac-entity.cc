/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013,2014 TELEMATICS LAB, DEE - Politecnico di Bari
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



#include "backoff-based-nano-mac-entity.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "simple-nano-device.h"
#include "nano-l3-header.h"
#include "nano-mac-queue.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("BackoffBasedNanoMacEntity");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (BackoffBasedNanoMacEntity);

TypeId BackoffBasedNanoMacEntity::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BackoffBasedNanoMacEntity")
    .SetParent<NanoMacEntity> ();
  return tid;
}


BackoffBasedNanoMacEntity::BackoffBasedNanoMacEntity ()
{
  SetMacQueue (CreateObject <NanoMacQueue> ());
  SetDevice (0);
}


BackoffBasedNanoMacEntity::~BackoffBasedNanoMacEntity ()
{
}

void BackoffBasedNanoMacEntity::DoDispose (void)
{
  NanoMacEntity::DoDispose ();
}

void BackoffBasedNanoMacEntity::DoSendPacket ()
{
	if (m_queue.size () > 0)
	{
		NS_LOG_FUNCTION (this);
		Ptr<NanoSpectrumPhy> phy = GetDevice ()->GetPhy ();

		Ptr<Packet> p = (m_queue.front ())->Copy ();				//从m_queue队列中复制第一个数据包
		m_queue.pop_front ();										//m_queue队列中删除第一个数据包
		phy->StartTx (p);

		if (m_queue.size () > 0)
		{
			double txtime = phy->GetTxDuration (p);					//GetTxDuration在ts-ook-based-nano-spectrum-phy.cc实现，为发送一个数据包的总时间
			Simulator::Schedule (FemtoSeconds (txtime), &BackoffBasedNanoMacEntity::DoSendPacket, this);
		}
	}

}

void BackoffBasedNanoMacEntity::Send (Ptr<Packet> p)
{
	NanoMacHeader macHeader;
	uint32_t src = GetDevice()->GetNode()->GetId();
	uint32_t dst = 0;
	macHeader.SetSource(src);
	macHeader.SetDestination(dst);

	p->AddHeader(macHeader);
	m_queue.push_back(p);

	if (m_queue.size() == 1) {
		Simulator::Schedule(Seconds(0.0), &BackoffBasedNanoMacEntity::DoSendPacket, this);
	}
}

void BackoffBasedNanoMacEntity::Send (Ptr<Packet> p, uint32_t dst)
{
	NanoMacHeader macHeader;
	uint32_t src = GetDevice()->GetNode()->GetId();
	macHeader.SetSource(src);
	macHeader.SetDestination(dst);

	p->AddHeader(macHeader);
	m_queue.push_back(p);

	if (m_queue.size() == 1) {
		Simulator::Schedule(Seconds(0.0), &BackoffBasedNanoMacEntity::DoSendPacket, this);
	}
}

void BackoffBasedNanoMacEntity::Receive (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this);
}

} // namespace ns3
