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


#include "ns3/core-module.h"                        //build/ns3
#include "fsa-nano-mac-entity.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "simple-nano-device.h"
#include "nano-mac-queue.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "nano-routing-entity.h"
#include "ns3/message-process-unit.h"


NS_LOG_COMPONENT_DEFINE ("FSANanoMacEntity");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (FSANanoMacEntity);

TypeId FSANanoMacEntity::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::FSANanoMacEntity").SetParent<NanoMacEntity> ();
	return tid;
}

FSANanoMacEntity::FSANanoMacEntity ()
{
	SetMacQueue (CreateObject <NanoMacQueue> ());
	SetMacQueue1(CreateObject <NanoMacQueue> ());
	SetMacQueue2(CreateObject <NanoMacQueue> ());
	SetDevice (0);
}


FSANanoMacEntity::~FSANanoMacEntity ()
{
}

void FSANanoMacEntity::DoDispose (void)
{
	NanoMacEntity::DoDispose ();
}


void FSANanoMacEntity::DoSendPacket ()			//处理节点数据包与网关探测数据包
{
	NS_LOG_FUNCTION (this);
	Ptr<NanoSpectrumPhy> phy = GetDevice ()->GetPhy ();

	Ptr<Packet> p = (m_queue.front ())->Copy ();		//从m_queue队列中复制第一个数据包
	m_queue.pop_front ();								//m_queue队列中删除第一个数据包
	phy->StartTx (p);
	if (m_queue.size () > 0)							//m_queue.size ()不会大于0，进不去该判断，用不上退避时隙等待
	{
		double backoff = rand () % 100000;				//rand ()小于100000，所有退避时隙在0-100ns
		Simulator::Schedule (PicoSeconds (backoff), &FSANanoMacEntity::DoSendPacket, this);
	}
}

void FSANanoMacEntity::SendGatewaytestPacket(Ptr<Packet> p)			//纳米网关节点发送探测数据包
{
	NanoMacHeader macHeader;
	macHeader.SetDestination(999);
	macHeader.SetSource(0);
	p->AddHeader(macHeader);
	m_queue.push_back (p);
	DoSendPacket();
}

void FSANanoMacEntity::Send (Ptr<Packet> p)						//未使用该函数，使用的是下面的带有dst的Send函数
{
  NanoMacHeader macHeader;
  uint32_t src = 0;
  uint32_t dst = 999;
  macHeader.SetSource (src);
  macHeader.SetDestination (dst);
  p->AddHeader (macHeader);
  m_queue.push_back (p);
  if (m_queue.size () == 1) {
	  Simulator::Schedule (Seconds (0.0), &FSANanoMacEntity::DoSendPacket, this);
	}
}

void FSANanoMacEntity::Send (Ptr<Packet> p, uint32_t dst)
{
	m_queue.push_back (p);
	if (m_queue.size () == 1)
	{
		Simulator::Schedule (Seconds (0.0), &FSANanoMacEntity::DoSendPacket, this);
	}
}

void FSANanoMacEntity::Receive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
