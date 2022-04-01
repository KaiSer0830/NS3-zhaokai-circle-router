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


#include "opportunistic-nano-routing-entity.h"
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
#include <stdio.h>
#include <stdlib.h>


NS_LOG_COMPONENT_DEFINE ("OpportunisticNanoRoutingEntity");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (OpportunisticNanoRoutingEntity);

TypeId OpportunisticNanoRoutingEntity::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::OpportunisticNanoRoutingEntity").SetParent<Object> ();
	return tid;
}


OpportunisticNanoRoutingEntity::OpportunisticNanoRoutingEntity ()
{
	SetDevice(0);
	m_receivedPacketListDim = 20;
	for (int i = 0; i < m_receivedPacketListDim; i++) {
		m_receivedPacketList.push_back (9999999);
	}
}


OpportunisticNanoRoutingEntity::~OpportunisticNanoRoutingEntity ()
{
  SetDevice(0);
}

void  OpportunisticNanoRoutingEntity::DoDispose (void)
{
  SetDevice (0);
}

void OpportunisticNanoRoutingEntity::SendPacket (Ptr<Packet> p)					//从MessageProcessUnit::CreteMessage中的m_device->SendPacket (p)进入
{
	if(GetDevice()->GetEnergyCapacity() >= GetDevice()->GetMinSatisfidSendEnergy()) {			//节点能量满足最小发送能量
		GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送自身探测数据包的能量，获取周围的邻居节点，此处过程简写
		GetDevice ()->GetMac ()->CheckForOpportNeighborss(p);
		std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;	//获取邻居节点，类型包括移动性(in_mobility)、index（in_index）、节点id（id)、节点类型，节点类型使用neighbor.type，2代表纳米节点，1代表路由节点，0代表网关节点
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;

		if (neighbors.size () != 0) {			//判断周围是否有网关节点或者其它符合要求的纳米节点，底层函数已过滤路由节点、index值不符合要求的节点、已携带数据包的节点、剩余能量不足的节点
			std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
			GetDevice()->ConsumeEnergyReceive(GetDevice()->GetTestSize() * neighbors.size());				//发送节点收到所有候选节点的探测数据包，需要消耗能量

			NanoL3Header l3Header;					//数据包添加路由头,header的size为24字节
			l3Header.SetSource (GetDevice ()->GetNode ()->GetId ());
			l3Header.SetDestination (0);
			l3Header.SetTtl (15);
			l3Header.SetPacketId (p->GetUid());
			l3Header.Setindex(GetDevice ()->index);
			p->AddHeader (l3Header);					//包的大小为136字节，Packet为100字节，seqTs为12字节，header为24字节

			SenderTypeTag tag;		//在nano-routing-entity.h中定义，大小为4字节，标记数据包的类型

			UpdateReceivedPacketId(p->GetUid());			//将数据包id记录队列，防止重复接收
			GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，能量是否足够判断已在候选节选择点中判断过
			bool localFlag = false;			//邻居节点是否有网关的标志位
			std::vector<NanoDetail> newNeighbors;
			std::vector<NanoDetail>::iterator it;
			for (it = neighbors.begin (); it != neighbors.end (); it++) {
				std::cout << Simulator::Now().GetSeconds() << " " << "neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "   " << "detail_index:" << (*it).detail_index << "   " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "priority:" << (*it).detail_priority_seq << std::endl;
				//std::cout << Simulator::Now().GetSeconds() << " " << "neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "   " << "detail_index:" << (*it).detail_index << "   " << "packetId:" << p->GetUid() << "   " << "priority:" << (*it).detail_priority_seq << std::endl;
				if((*it).detail_type == 0)	{		//如果是网关节点,将数据包发送给网关节点，其它节点不接收
					localFlag = true;
					tag.type = 0;
					p->AddPacketTag(tag);				//tag是放入packet中的m_packetTagList，不读入数据包长度
					GetDevice()->GetMac()->Send(p);
					break;
				} else {
					//剩余能量是否足够不需要再次判断，已在选择邻居节点中筛选过了
					if (GetDevice()->index < (*it).detail_index) {
						newNeighbors.push_back (*it);
					}
				}
			}
			if (localFlag == false && newNeighbors.size () != 0) {	 		//将数据包转发给邻居节点
				tag.type = 1;
				p->AddPacketTag (tag);
				GetDevice()->GetMac()->Send(p);
			}
		} else {
			Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &OpportunisticNanoRoutingEntity::SendPacket, this, p);			//0.1s
		}
	} else {
		std::cout << "SendPacket no energy to send" << std::endl;
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &OpportunisticNanoRoutingEntity::SendPacket, this, p);			//0.1s
	}
}

void OpportunisticNanoRoutingEntity::SendGatewaytestPacket (Ptr<Packet> p)		//目的节点发送探测包,数据包添加序列头和路由头
{
	GetDevice()->GetMac()->CheckForNeighbors();
	if (GetDevice()->GetMac()->m_neighbors.size() != 0) {				//给路由节点范围内所有节点发送探测包
		NanoL3Header l3Header;
		SenderTypeTag tag;
		tag.type = 2;				//在nano-routing-entity.h中定义，大小为4字节，为4代表为探测数据包
		l3Header.SetTtl(1);
		l3Header.SetPacketId(p->GetUid());
		l3Header.SetSource(0);				//探测数据包由网关节点发出，所有源节点id为0
		l3Header.SetDestination(999);
		l3Header.Setindex(GetDevice ()->index);
		p->AddHeader(l3Header);
		p->AddPacketTag (tag);
		GetDevice()->GetMac()->SendGatewaytestPacket(p);
	}
}

void OpportunisticNanoRoutingEntity::ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy)				//从ts-ook-based-nano-spectrum-phy.cc中的GetL3 ()->ReceivePacket (p)跳转过来的
{
	SenderTypeTag tag;
	p->RemovePacketTag(tag);
	NanoMacHeader macHeader;		//注意：需要按照堆栈的顺序来取，不能弄错，mac后放得先取，tag包除外
	p->RemoveHeader(macHeader);		//移除mac头Packet大小减8字节
	NanoL3Header l3Header;
	p->RemoveHeader(l3Header);		//移除路由头Packet大小减24字节

	bool alreadyReceived = CheckAmongReceivedPacket(p->GetUid());			//判断最近是否接收过该数据包
	if (!alreadyReceived) {
		if (GetDevice()->m_type == SimpleNanoDevice::NanoInterface) {
			if (!alreadyReceived) {
				sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive(GetDevice()->GetAckSize());		//发送节点消耗接收成功接收ACK应答包能量
				p->AddHeader(l3Header);
				UpdateReceivedPacketId(p->GetUid());			//将数据包id记录队列，防止重复接收
				sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

				//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
				sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
				std::cout << Simulator::Now().GetSeconds() << " " << "ProcessMessage" << "   " << "packetId:" << p->GetUid() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ----------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
				GetDevice()->GetMessageProcessUnit()->ProcessMessage(p);					//网关节点处理数据包
			} else {
				std::cout << Simulator::Now().GetSeconds() << " " << "###clear replicate packet###" << " " << "NodeId:" << sourcePhy->GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
				sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

				//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
				sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
			}
		} else if (GetDevice()->m_type == SimpleNanoDevice::NanoNode) {
			if (!alreadyReceived) {
				if (l3Header.GetSource() == 0 && tag.type == 2 && GetDevice()->m_type == SimpleNanoDevice::NanoNode) {			//Etag与index置0函数，发送节点是网关节点并且接收数据包的节点是纳米节点且Etag还未置0
					sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive(GetDevice()->GetAckSize());		//发送节点消耗接收成功接收ACK应答包能量
					GetDevice()->index = 0;		//接收探测包消耗能量写在了channel函数中
				} else if (l3Header.GetSource() != 0 && tag.type == 1 && GetDevice()->m_type == SimpleNanoDevice::NanoNode) {		//发送节点是普通纳米节点，Etag为0，自身没有数据包，将数据包转发给index比自己小的节点
					GetDevice()->ConsumeEnergyReceive(GetDevice()->GetPacketSize());				//范围内所有节点都需要先消耗能量接收数据包
					//筛选出index值、剩余能量与packetExistFlag符合要求的节点进行接收
					if (sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->index < GetDevice()->index && GetDevice()->packetExistFlag == false && GetDevice()->m_energy >= GetDevice()->GetMinSatisfidForwardEnergy()) {
						std::vector<NanoDetail> nowNeighbors = sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMac()->m_neighborss;
						std::vector<NanoDetail>::iterator it;
						for (it = nowNeighbors.begin (); it != nowNeighbors.end (); it++) {
							if ((*it).detail_id == GetDevice()->GetNode()->GetId() && (*it).detail_priority_seq == 0) {			//按优先级退避转发，如果优先级最高，直接发送
								GetDevice()->ConsumeEnergySend(GetDevice()->GetAckSize());				//接收节点消耗发送成功接收ACK应答包能量
								sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive(GetDevice()->GetAckSize());		//发送节点消耗接收成功接收ACK应答包能量
								p->AddHeader(l3Header);
								UpdateReceivedPacketId(p->GetUid());			//将数据包id记录队列，防止重复接收
								sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;			//自身数据包被别的节点接收，标志位置为true，节点此时没有数据包，可以进行转发别的节点数据包

								//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
								sourcePhy->GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
								GetDevice ()->packetExistFlag = true;			//接收节点接收了转发数据包，该节点该无法创建新的数据包，也无法再接收转发数据包，标志位置为false
								std::cout << Simulator::Now().GetSeconds() << " " << "forward1" << "   " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "sourcePhy:" << sourcePhy->GetDevice()->GetNode()->GetId() << " ------------------------> " << GetDevice()->GetNode()->GetId() << std::endl;
								ForwardPacket(p);
							} else {				//按优先级退避转发，简写，实验为理想情况，优先级最高的节点都能接收成功
								//std::cout << "BACKF-FORWARD" << std::endl;
							}
						}
					}
				}
			}
		}
	}
}

void OpportunisticNanoRoutingEntity::ForwardPacket (Ptr<Packet> p)
{
	if(GetDevice()->GetEnergyCapacity() >= GetDevice()->GetMinSatisfidSendEnergy()) {			//节点能量满足最小发送能量
		GetDevice()->ConsumeEnergySend(GetDevice()->GetTestSize());			//节点消耗发送候选节点探测数据包的能量，获取周围的邻居节点，此处过程简写，能量是否足够已在创建数据包时判断过了
		GetDevice ()->GetMac ()->CheckForOpportNeighborss(p);
		std::vector<NanoDetail> neighbors = GetDevice ()->GetMac ()->m_neighborss;	//获取邻居节点，类型包括移动性(in_mobility)、index（in_index）、节点id（id)、节点类型，节点类型使用neighbor.type，2代表纳米节点，1代表路由节点，0代表网关节点
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
		//std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;

		if (neighbors.size () != 0) {			//判断周围是否有网关节点或者其它符合要求的纳米节点，底层函数已过滤路由节点、index值不符合要求的节点、已携带数据包的节点、剩余能量不足的节点
			std::cout << Simulator::Now().GetSeconds() << " " << "GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << GetDevice ()->GetPhy()->GetMobility()->GetPosition() << "   " << "neighbors.size ():" << neighbors.size () << "   " << "index:" << GetDevice()->index << std::endl;
			GetDevice()->ConsumeEnergyReceive(GetDevice()->GetTestSize() * neighbors.size());				//发送节点收到所有候选节点的探测数据包，需要消耗能量
			NanoL3Header l3Header;
			p->RemoveHeader(l3Header);
			uint32_t ttl = l3Header.GetTtl();
			if (ttl > 0) {
				l3Header.SetTtl(ttl - 1);
				p->AddHeader(l3Header);
				GetDevice()->GetMac()->forward(p);

				SenderTypeTag tag;		//在nano-routing-entity.h中定义，大小为4字节，标记数据包的类型
				UpdateReceivedPacketId(p->GetUid());			//将数据包id记录队列，防止重复接收
				GetDevice()->ConsumeEnergySend(GetDevice()->GetPacketSize());				//节点消耗发送数据包的能量，能量是否足够判断已在候选节选择点中判断过
				bool localFlag = false;			//邻居节点是否有网关的标志位
				std::vector<NanoDetail> newNeighbors;
				std::vector<NanoDetail>::iterator it;
				for (it = neighbors.begin (); it != neighbors.end (); it++) {
					std::cout << Simulator::Now().GetSeconds() << " " << "neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "   " << "detail_index:" << (*it).detail_index << "   " << "packetId:" << p->GetUid() << "   " << "ttl:" << l3Header.GetTtl() << "   " << "priority:" << (*it).detail_priority_seq << std::endl;
					//std::cout << Simulator::Now().GetSeconds() << " " << "neighbor.id:" << (*it).detail_id << "   " << (*it).detail_phy->GetMobility()->GetPosition() << "   " << "detail_index:" << (*it).detail_index << "   " << "packetId:" << p->GetUid() << "   " << "priority:" << (*it).detail_priority_seq << std::endl;
					if((*it).detail_type == 0)	{		//如果是网关节点,将数据包发送给网关节点，其它节点不接收
						localFlag = true;
						tag.type = 0;
						p->AddPacketTag(tag);				//tag是放入packet中的m_packetTagList，不读入数据包长度
						GetDevice()->GetMac()->Send(p);
						break;
					} else {
						//剩余能量是否足够不需要再次判断，已在选择邻居节点中筛选过了
						if (GetDevice()->index < (*it).detail_index) {
							newNeighbors.push_back (*it);
						}
					}
				}
				if (localFlag == false && newNeighbors.size () != 0) {	 		//将数据包转发给邻居节点
					tag.type = 1;
					p->AddPacketTag (tag);
					GetDevice()->GetMac()->Send(p);
				}
			} else {
				std::cout << Simulator::Now().GetSeconds() << " " << "TTL expire" << "   " << "ttl:" << l3Header.GetTtl() << "   " << "GetId:" << GetDevice()->GetNode()->GetId() << "   " << "packetId:" << p->GetUid() << std::endl;
				GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag = false;				//数据包TTL用完，节点空闲

				//先判断发送节点之前定时产生数据包的事件是否还存在，如果不存在则重新产生，如果存在则不产生
				GetDevice()->GetObject<SimpleNanoDevice>()->EvendJudge();
			}
		} else {
			Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &OpportunisticNanoRoutingEntity::ForwardPacket, this, p);			//0.1s
		}
	} else {
		std::cout << GetDevice()->GetNode()->GetId() << "ForwardPacket no energy to send" << std::endl;
		Simulator::Schedule (Seconds (GetDevice ()->reSendTimeInterval), &OpportunisticNanoRoutingEntity::ForwardPacket, this, p);			//0.1s
	}
}

void OpportunisticNanoRoutingEntity::SetReceivedPacketListDim (int m)
{
	NS_LOG_FUNCTION (this);
	m_receivedPacketListDim = m;
}

void OpportunisticNanoRoutingEntity::UpdateReceivedPacketId (uint32_t id)
{
	NS_LOG_FUNCTION (this);
	m_receivedPacketList.pop_front ();
	m_receivedPacketList.push_back (id);
}

bool OpportunisticNanoRoutingEntity::CheckAmongReceivedPacket (uint32_t id)
{
  NS_LOG_FUNCTION (this);
  for (std::list<uint32_t>::iterator it = m_receivedPacketList.begin(); it != m_receivedPacketList.end (); it++)
    {
	  NS_LOG_FUNCTION (this << *it << id);
	  if (*it == id) return true;
    }
  return false;
}

} // namespace ns3
