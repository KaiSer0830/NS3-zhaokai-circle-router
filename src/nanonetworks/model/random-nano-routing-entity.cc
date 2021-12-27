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


#include "random-nano-routing-entity.h"
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
#include "nano-routing-entity.h"
#include "message-process-unit.h"
#include <stdio.h>
#include <stdlib.h>


NS_LOG_COMPONENT_DEFINE ("RandomNanoRoutingEntity");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (RandomNanoRoutingEntity);

TypeId RandomNanoRoutingEntity::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::RandomNanoRoutingEntity").SetParent<Object> ();
	return tid;
}


RandomNanoRoutingEntity::RandomNanoRoutingEntity ()
{
	SetDevice(0);
	m_receivedPacketListDim = 20;
	for (int i = 0; i < m_receivedPacketListDim; i++) {			//未使用
		m_receivedPacketList.push_back(9999999);
	}
}


RandomNanoRoutingEntity::~RandomNanoRoutingEntity ()
{
	SetDevice(0);
}

void  RandomNanoRoutingEntity::DoDispose (void)
{
	SetDevice (0);
}

void RandomNanoRoutingEntity::SendPacket (Ptr<Packet> p)
{
	GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
	GetDevice ()->GetMac ()->CheckForNeighborss(p);
	std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
	//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << std::endl;

	if (neighbors.size() != 0) {
		std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
		NanoSeqTsHeader seqTs;
		p->RemoveHeader(seqTs);

		NanoL3Header header;
		header.SetSource(GetDevice()->GetNode()->GetId());
		header.SetDestination(0);						//目的地固定为0，即网关节点
		header.SetTtl(15);
		header.SetPacketId(p->GetUid());
		p->AddHeader(seqTs);
		p->AddHeader(header);

		UpdateReceivedPacketId(p->GetUid());				//自己发送的自己不需要接收，也放入接收过的数据包队列
		GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，，能量是否足够判断已在候选节选择点中判断过
		bool localFlag = false;				//邻居节点是否存在网关节点标志
		std::vector<NanoDetail>::iterator it;
		for (it = neighbors.begin(); it != neighbors.end(); it++) {
			std::cout << Simulator::Now().GetSeconds() << " " << "SendPacket--neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "    " << "packetId:" << p->GetUid() << "   " << "index:" << (*it).detail_index << std::endl;
			if((*it).detail_type == 0)	{		//如果是网关节点,将数据包发送给网关节点，其它节点不接收
				localFlag = true;
				GetDevice()->GetMac()->Send(p);
			}
		}
		if(localFlag == false && neighbors.size() == 1) {
			GetDevice ()->GetMac ()->m_neighborss.at(0).detail_priority_seq = 0;				//只有一个元素时也需要设置优先级
			GetDevice()->GetMac()->Send(p);
		} else if(localFlag == false && neighbors.size() > 1) {
			srand(time(NULL));
			int i = rand() % neighbors.size();
			GetDevice ()->GetMac ()->m_neighborss.at(i).detail_priority_seq = 0;				//随机选中的候选节点优先级最高
			GetDevice()->GetMac()->Send(p);
		}
	} else {	 		//没有邻居节点	,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		Simulator::Schedule (Seconds (GetDevice ()->GetMac ()->reSendTimeInterval), &RandomNanoRoutingEntity::SendPacket, this, p);
	}
}

void RandomNanoRoutingEntity::ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy)
{
	NanoL3Header l3Header;
	p->RemoveHeader(l3Header);
	NanoMacHeader macHeader;
	p->RemoveHeader(macHeader);

	bool alreadyReceived = CheckAmongReceivedPacket(p->GetUid());		//根据数据包的id判断该数据包是否已经被接收过
	if (GetDevice()->m_type == SimpleNanoDevice::NanoInterface) {		//如果节点有消息处理单元并且是纳米网关节点，则传输结束，进行消息处理
		if (!alreadyReceived) {
			p->AddHeader(l3Header);
			UpdateReceivedPacketId(p->GetUid());				//将数据包id记录队列，防止重复接收
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包
			double nextTime = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->getNextMessageGenerateTime();							//获取节点下一次产生数据包的间隔，间隔是1-4s之内的随机值
			Simulator::Cancel(sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule);					//清除原有的创建节点数据包Simulator事件，防止存在好几个创建数据包Simulator事件
			//发送节点延时1-4s之后重新产生新的数据包
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule = Simulator::Schedule (Seconds (nextTime), &MessageProcessUnit::CreteMessage, sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMessageProcessUnit());
			std::cout << Simulator::Now().GetSeconds() << " " << "ProcessMessage" << "   " << "packetId:" << p->GetUid() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ----------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
			GetDevice()->GetMessageProcessUnit()->ProcessMessage(p);
		} else {
			std::cout << Simulator::Now().GetSeconds() << " " << "###clear replicate packet###" << " " << "NodeId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包
			double nextTime = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->getNextMessageGenerateTime();						//获取节点下一次产生数据包的间隔，间隔是1-4s之内的随机值
			Simulator::Cancel(sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule);					//清除原有的创建节点数据包Simulator事件，防止存在好几个创建数据包Simulator事件
			//发送节点延时1-4s之后重新产生新的数据包
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule = Simulator::Schedule (Seconds (nextTime), &MessageProcessUnit::CreteMessage, sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMessageProcessUnit());
		}
	} else if (GetDevice()->m_type == SimpleNanoDevice::NanoNode){										//接收的不是网关节点，进行数据包转发
		GetDevice()->ConsumeEnergyReceive(GetDevice()->GetPacketSize());				//范围内所有节点都需要先消耗能量接收数据包
		if (!alreadyReceived) {
			std::vector<NanoDetail> nowNeighbors = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMac()->m_neighborss;
			std::vector<NanoDetail>::iterator it;
			//遍历候选节点，候选节点中有节点id跟自己相等的则接收数据包
			for (it = nowNeighbors.begin (); it != nowNeighbors.end (); it++) {
				//遍历候选节点，候选节点中有节点id跟自己相等的则接收数据包
				if((*it).detail_id == GetDevice()->GetNode()->GetId() && GetDevice()->packetExistFlag == false && GetDevice()->m_energy >= GetDevice()->GetMinSatisfidForwardEnergy()) {
					if((*it).detail_priority_seq == 0) {			//只让候选节点优先级最高的接收
						p->AddHeader(l3Header);
						UpdateReceivedPacketId(p->GetUid());				//将数据包id记录队列，防止重复接收
						sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包
						double nextTime = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->getNextMessageGenerateTime();						//获取节点下一次产生数据包的间隔，间隔是1-4s之内的随机值
						Simulator::Cancel(sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule);					//清除原有的创建节点数据包Simulator事件，防止存在好几个创建数据包Simulator事件
						//发送节点延时1-4s之后重新产生新的数据包
						sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule = Simulator::Schedule (Seconds (nextTime), &MessageProcessUnit::CreteMessage, sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMessageProcessUnit());
						GetDevice ()->packetExistFlag = true;			//接收节点接收了转发数据包，该节点该无法创建新的数据包，也无法再接收转发数据包，标志位置为false
						std::cout << Simulator::Now().GetSeconds() << " " << "forward" << "   " << "packetId:" << p->GetUid() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ------------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
						ForwardPacket(p);
					}
				}
			}
		} else {
			//std::cout << Simulator::Now().GetSeconds() << " " << "alreadyReceived" << "   " << "sourceId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "NodeId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
		}
	}
}

void RandomNanoRoutingEntity::ForwardPacket (Ptr<Packet> p)
{
	GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
	GetDevice ()->GetMac ()->CheckForNeighborss(p);
	std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
	//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << std::endl;

	if (neighbors.size() != 0) {
		std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
		NanoL3Header l3Header;
		p->RemoveHeader(l3Header);
		uint32_t ttl = l3Header.GetTtl();
		if (ttl > 1) {
			l3Header.SetTtl(ttl - 1);
			p->AddHeader(l3Header);

			GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，，能量是否足够判断已在候选节选择点中判断过
			std::vector<NanoDetail>::iterator it;
			bool localFlag = false;				//邻居节点是否存在网关节点标志
			for (it = neighbors.begin(); it != neighbors.end(); it++) {
				std::cout << Simulator::Now().GetSeconds() << " " << "SendPacket--neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "    " << "packetId:" << p->GetUid() << "   " << "index:" << (*it).detail_index << std::endl;
				if((*it).detail_type == 0)	{		//如果是网关节点,将数据包发送给网关节点，其它节点不接收
					localFlag = true;
					GetDevice()->GetMac()->Send(p);
				}
			}
			if(localFlag == false && neighbors.size() == 1) {
				GetDevice ()->GetMac ()->m_neighborss.at(0).detail_priority_seq = 0;				//只有一个元素时也需要设置优先级
				GetDevice()->GetMac()->Send(p);
			} else if(localFlag == false && neighbors.size() > 1) {
				srand(time(NULL));
				int i = rand() % neighbors.size();
				GetDevice ()->GetMac ()->m_neighborss.at(i).detail_priority_seq = 0;				//随机选中的候选节点优先级最高
				GetDevice()->GetMac()->Send(p);
			}
		} else {
			std::cout << Simulator::Now().GetSeconds() << " " << "TTL expire" << "   " << "GetId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
			GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;				//数据包TTL用完，节点空闲
			double nextTime = GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->getNextMessageGenerateTime();						//获取节点下一次产生数据包的间隔，间隔是1-4s之内的随机值
			Simulator::Cancel(GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule);					//清除原有的创建节点数据包Simulator事件，防止存在好几个创建数据包Simulator事件
			//发送节点延时1-4s之后重新产生新的数据包
			GetDevice()->GetObject<SimpleNanoDevice>()->nodeSchedule = Simulator::Schedule (Seconds (nextTime), &MessageProcessUnit::CreteMessage, GetDevice()->GetObject<SimpleNanoDevice>()->GetMessageProcessUnit());
		}
	} else {	 		//没有邻居节点	,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		Simulator::Schedule (Seconds (GetDevice ()->GetMac ()->reSendTimeInterval), &RandomNanoRoutingEntity::ForwardPacket, this, p);
	}
}

void RandomNanoRoutingEntity::UpdateReceivedPacketId (uint32_t id)					//未使用
{
	NS_LOG_FUNCTION (this);
	m_receivedPacketList.pop_front ();
	m_receivedPacketList.push_back (id);
}

bool RandomNanoRoutingEntity::CheckAmongReceivedPacket (uint32_t id)					//未使用
{
	NS_LOG_FUNCTION(this);
	for (std::list<uint32_t>::iterator it = m_receivedPacketList.begin(); it != m_receivedPacketList.end(); it++) {
		NS_LOG_FUNCTION(this << *it << id);
		if (*it == id)
			return true;
	}
	return false;
}

void RandomNanoRoutingEntity::SetReceivedPacketListDim (int m)
{
  NS_LOG_FUNCTION (this);
  m_receivedPacketListDim = m;
}

} // namespace ns3
