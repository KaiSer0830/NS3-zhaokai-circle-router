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
	if(GetDevice()->GetEnergyCapacity() >= GetDevice()->GetMinSatisfidSendEnergy()) {			//节点能量满足最小发送能量
		GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
		GetDevice ()->GetMac ()->CheckForNeighborss(p);
		std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << std::endl;

		if (neighbors.size() != 0) {
			GetDevice()->ConsumeEnergyReceive(GetDevice()->GetTestSize() * neighbors.size());				//发送节点收到所有邻居节点的探测数据包，需要消耗能量
			std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;

			NanoL3Header l3Header;
			l3Header.SetSource(GetDevice()->GetNode()->GetId());
			l3Header.SetDestination(0);						//目的地固定为0，即网关节点
			l3Header.SetTtl(15);
			l3Header.SetPacketId(p->GetUid());
			l3Header.Setindex(GetDevice ()->index);
			p->AddHeader(l3Header);

			UpdateReceivedPacketId(p->GetUid());				//自己发送的自己不需要接收，也放入接收过的数据包队列
			GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，，能量是否足够判断已在候选节选择点中判断过
			bool localFlag = false;				//邻居节点是否存在网关节点标志
			std::vector<NanoDetail>::iterator it;
			for (it = neighbors.begin(); it != neighbors.end(); it++) {
				std::cout << Simulator::Now().GetSeconds() << " " << "SendPacket--neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "    " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "index:" << (*it).detail_index << std::endl;
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
			Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &RandomNanoRoutingEntity::SendPacket, this, p);			//0.1s
		}
	} else {	 		//没有能量,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		std::cout << GetDevice()->GetNode()->GetId() << "   " << "SendPacket no energy to send" << std::endl;
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &RandomNanoRoutingEntity::SendPacket, this, p);			//0.1s
	}
}

void RandomNanoRoutingEntity::ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy)
{
	NanoMacHeader macHeader;			//注意：需要按照堆栈的顺序来取，不能弄错，mac后放得先取
	p->RemoveHeader(macHeader);
	NanoL3Header l3Header;
	p->RemoveHeader(l3Header);


	bool alreadyReceived = CheckAmongReceivedPacket(p->GetUid());		//根据数据包的id判断该数据包是否已经被接收过
	if (GetDevice()->m_type == SimpleNanoDevice::NanoInterface) {		//如果节点有消息处理单元并且是纳米网关节点，则传输结束，进行消息处理
		if (!alreadyReceived) {
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive(GetDevice()->GetAckSize());		//发送节点消耗接收成功接收ACK应答包能量
			p->AddHeader(l3Header);
			UpdateReceivedPacketId(p->GetUid());				//将数据包id记录队列，防止重复接收
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

			//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
			std::cout << Simulator::Now().GetSeconds() << " " << "ProcessMessage" << "   " << "packetId:" << p->GetUid() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ----------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
			GetDevice()->GetMessageProcessUnit()->ProcessMessage(p);
		} else {
			std::cout << Simulator::Now().GetSeconds() << " " << "###clear replicate packet###" << " " << "NodeId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

			//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
			sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
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
						GetDevice()->ConsumeEnergySend(GetDevice()->GetAckSize());				//接收节点消耗发送成功接收ACK应答包能量
						sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive(GetDevice()->GetAckSize());		//发送节点消耗接收成功接收ACK应答包能量
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
			}
		} else {
			//std::cout << Simulator::Now().GetSeconds() << " " << "alreadyReceived" << "   " << "sourceId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "NodeId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
		}
	}
}

void RandomNanoRoutingEntity::ForwardPacket (Ptr<Packet> p)
{
	if(GetDevice()->GetEnergyCapacity() >= GetDevice()->GetMinSatisfidSendEnergy()) {			//节点能量满足最小发送能量
		GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够判断已在创建数据包时判断
		GetDevice ()->GetMac ()->CheckForNeighborss(p);
		std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << std::endl;

		if (neighbors.size() != 0) {
			GetDevice()->ConsumeEnergyReceive(GetDevice()->GetTestSize() * neighbors.size());				//发送节点收到所有邻居节点的探测数据包，需要消耗能量
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
					std::cout << Simulator::Now().GetSeconds() << " " << "SendPacket--neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "    " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "index:" << (*it).detail_index << std::endl;
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
				std::cout << Simulator::Now().GetSeconds() << " " << "TTL expire" << "   " << "ttl:" << l3Header.GetTtl() << "   " << "GetId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
				GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;				//数据包TTL用完，节点空闲

				//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
				GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
			}
		} else {	 		//没有邻居节点	,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
			Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &RandomNanoRoutingEntity::ForwardPacket, this, p);			//0.1s
		}
	} else {	 		//没有能量,间隔reSendTimeInterval之后再次调用该节点的mac协议发送
		std::cout << GetDevice()->GetNode()->GetId() << "   " << "ForwardPacket no energy to send" << std::endl;
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &RandomNanoRoutingEntity::ForwardPacket, this, p);			//0.1s
	}
}

void RandomNanoRoutingEntity::UpdateReceivedPacketId (uint32_t id)
{
	NS_LOG_FUNCTION (this);
	m_receivedPacketList.pop_front ();
	m_receivedPacketList.push_back (id);
}

bool RandomNanoRoutingEntity::CheckAmongReceivedPacket (uint32_t id)
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
