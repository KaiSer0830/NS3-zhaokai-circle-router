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


#include "flooding-nano-routing-entity.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "simple-nano-device.h"
#include "nano-mac-queue.h"
#include "nano-l3-header.h"
#include "nano-mac-entity.h"
#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/channel.h"
#include "simple-nano-device.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-entity.h"
#include "nano-mac-header.h"
#include "nano-seq-ts-header.h"
#include "ns3/simulator.h"
#include "nano-routing-entity.h"
#include "message-process-unit.h"


NS_LOG_COMPONENT_DEFINE ("FloodingNanoRoutingEntity");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (FloodingNanoRoutingEntity);

TypeId FloodingNanoRoutingEntity::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::FloodingNanoRoutingEntity").SetParent<Object>();
	return tid;
}


FloodingNanoRoutingEntity::FloodingNanoRoutingEntity ()
{
	SetDevice(0);
	m_receivedPacketListDim = 100;
	for (int i = 0; i < m_receivedPacketListDim; i++)
	{
		m_receivedPacketList.push_back (9999999);
	}
}


FloodingNanoRoutingEntity::~FloodingNanoRoutingEntity ()
{
	SetDevice(0);
}

void  FloodingNanoRoutingEntity::DoDispose (void)
{
	SetDevice (0);
}

void FloodingNanoRoutingEntity::SendPacket (Ptr<Packet> p)
{
	GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
	GetDevice ()->GetMac ()->CheckForNeighborss(p);
	std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
	std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index  << std::endl;

	if (neighbors.size() != 0) {
		NanoL3Header l3Header;
		l3Header.SetSource(GetDevice()->GetNode()->GetId());
		l3Header.SetDestination(0);						//目的地固定为0，即网关节点
		l3Header.SetTtl(15);
		l3Header.SetPacketId(p->GetUid());
		l3Header.Setindex(GetDevice ()->index);
		p->AddHeader(l3Header);

		UpdateReceivedPacketId(p->GetUid());				//自己发送的自己不需要接收，也放入接收过的数据包队列
		GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，能量是否足够判断已在候选节选择点中判断过
		std::vector<NanoDetail>::iterator it;
		for (it = neighbors.begin(); it != neighbors.end(); it++) {
			std::cout << Simulator::Now().GetSeconds() << " " << "SendPacket--neighbor.id:" << (*it).detail_id << "    " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "index:" << (*it).detail_index << std::endl;
		}
		GetDevice()->GetMac()->Send(p);				//发送一遍即可，周围邻居节点都会收到
	} else {	 		//没有邻居节点	,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &FloodingNanoRoutingEntity::SendPacket, this, p);			//0.1s
	}
}

void FloodingNanoRoutingEntity::ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy)
{
	NanoMacHeader macHeader;					//注意：需要按照堆栈的顺序来取，不能弄错，mac后放得先取
	p->RemoveHeader(macHeader);
	NanoL3Header l3Header;
	p->RemoveHeader(l3Header);

	bool alreadyReceived = CheckAmongReceivedPacket(p->GetUid());
	if (GetDevice()->m_type == SimpleNanoDevice::NanoInterface) {				//如果是接收节点是网关节点
		if (!alreadyReceived) {			//如果没有接收过该数据包，则网关节点处理该数据包
			p->AddHeader(l3Header);
			UpdateReceivedPacketId(p->GetUid());				//将数据包id记录队列，防止重复接收
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

			//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
			std::cout << Simulator::Now().GetSeconds() << " " << "ProcessMessage" << "   " << "packetId:" << p->GetUid() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ----------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
			GetDevice()->GetMessageProcessUnit()->ProcessMessage(p);
		} else {						//遇到重复的数据包需要清除该节点数据包，方便下次产生数据包，否则该节点数据包一直不会清除
			std::cout << Simulator::Now().GetSeconds() << " " << "###clear replicate packet###" << " " << "NodeId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

			//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
		}
	} else if(GetDevice()->m_type == SimpleNanoDevice::NanoNode){					//如果是接收节点是普通纳米节点
		GetDevice()->ConsumeEnergyReceive(GetDevice()->GetPacketSize());				//范围内所有节点都需要先消耗能量接收数据包
		if (!alreadyReceived) {
			std::vector<NanoDetail> nowNeighbors = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMac()->m_neighborss;
			std::vector<NanoDetail>::iterator it;
			for (it = nowNeighbors.begin (); it != nowNeighbors.end (); it++) {
				//遍历候选节点，候选节点中有节点id跟自己相等的则接收数据包
				if((*it).detail_id == GetDevice()->GetNode()->GetId() && GetDevice()->packetExistFlag == false && GetDevice()->m_energy >= GetDevice()->GetMinSatisfidForwardEnergy()) {
					p->AddHeader(l3Header);
					UpdateReceivedPacketId(p->GetUid());				//将数据包id记录队列，防止重复接收
					sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

					//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
					sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
					GetDevice ()->packetExistFlag = true;			//接收节点接收了转发数据包，该节点该无法创建新的数据包，也无法再接收转发数据包，标志位置为false
					std::cout << Simulator::Now().GetSeconds() << " " << "forward" << "   " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ------------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
					ForwardPacket(p);
				}
			}
		} else {
			//std::cout << Simulator::Now().GetSeconds() << " " << "alreadyReceived" << "   " << "sourceId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "NodeId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
		}
	}
}

void FloodingNanoRoutingEntity::ForwardPacket (Ptr<Packet> p)
{
	GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
	GetDevice ()->GetMac ()->CheckForNeighborss(p);
	std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
	std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;

	if (neighbors.size() != 0) {
		NanoL3Header l3Header;
		p->RemoveHeader(l3Header);
		uint32_t ttl = l3Header.GetTtl();
		if (ttl > 1) {
			l3Header.SetTtl(ttl - 1);
			p->AddHeader(l3Header);

			std::vector<NanoDetail>::iterator it;
			GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，，能量是否足够判断已在候选节选择点中判断过
			for (it = neighbors.begin(); it != neighbors.end(); it++) {
				std::cout << Simulator::Now().GetSeconds() << " " << "ForwardPacket--neighbor.id:" << (*it).detail_id << "    " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "index:" << (*it).detail_index << std::endl;
			}
			GetDevice()->GetMac()->Send(p);
		} else {
			std::cout << Simulator::Now().GetSeconds() << " " << "TTL expire" << "   " << "ttl:" << l3Header.GetTtl() << "   " << "GetId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
			GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;				//数据包TTL用完，节点空闲

			//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
			GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
		}
	} else {	 		//没有邻居节点	,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &FloodingNanoRoutingEntity::ForwardPacket, this, p);			//0.1s
	}
}

void FloodingNanoRoutingEntity::UpdateReceivedPacketId (uint32_t id)			//弹出一个最早的数据，push一个新数据
{
	m_receivedPacketList.pop_front ();				//pop一个又push一个，保持m_receivedPacketList的大小为20
	m_receivedPacketList.push_back (id);
}

bool FloodingNanoRoutingEntity::CheckAmongReceivedPacket (uint32_t id)
{
	for (std::list<uint32_t>::iterator it = m_receivedPacketList.begin(); it != m_receivedPacketList.end (); it++)
	{
		NS_LOG_FUNCTION (this << *it << id);
		if (*it == id) return true;
	}
	return false;
}

void FloodingNanoRoutingEntity::SetReceivedPacketListDim (int m) {
	NS_LOG_FUNCTION (this);
	m_receivedPacketListDim = m;
}

} // namespace ns3
