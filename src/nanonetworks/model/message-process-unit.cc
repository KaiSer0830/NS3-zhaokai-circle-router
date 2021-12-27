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


#include "message-process-unit.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "simple-nano-device.h"
#include "nano-mac-queue.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-header.h"
#include "nano-seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/nano-l3-header.h"

NS_LOG_COMPONENT_DEFINE ("MessageProcessUnit");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (MessageProcessUnit);

TypeId MessageProcessUnit::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MessageProcessUnit").SetParent<Object> ();
  return tid;
}


MessageProcessUnit::MessageProcessUnit ()
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_packetSize = 100;
  m_testPacketSize = 50;
  m_interarrivalTime = 0.1;
  m_interarrivalTestTime = 0.01;
}


MessageProcessUnit::~MessageProcessUnit ()
{
  NS_LOG_FUNCTION (this);
}

void  MessageProcessUnit::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
}

void MessageProcessUnit::SetDevice (Ptr<SimpleNanoDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_device = d;
}

Ptr<SimpleNanoDevice> MessageProcessUnit::GetDevice (void)
{
  return m_device;
}

void MessageProcessUnit::CreteMessage ()				//从主函数进入
{
	std::cout << Simulator::Now().GetSeconds() << " " << "MessageProcessUnit::CreteMessage" << "   " << "GetDevice ()->GetNode()->GetId():" << GetDevice ()->GetNode()->GetId() << "   " << "GetDevice ()->packetExistFlag:" << GetDevice ()->packetExistFlag << std::endl;
	if(GetDevice()->packetExistFlag == false) {			//前一个数据包是否被接收完毕，一个节点只能拥有一个数据包，无论是自己产生的还是转发别人的
		NS_LOG_FUNCTION (this);
		uint8_t *buffer  = new uint8_t[m_packetSize];				//数据包大小100字节
		for (int i = 0; i < m_packetSize; i++) {
			buffer[i] = 129;
		}
		Ptr<Packet> p = Create<Packet>(buffer, m_packetSize);
		NanoSeqTsHeader seqTs;				//seqTs的size为12字节
		seqTs.SetSeq (p->GetUid ());
		p->AddHeader (seqTs);				//p->GetSize()为初始数据包大小+12
		if (GetDevice()->GetEnergyCapacity() >= GetDevice()->GetMinSatisfidSendEnergy()) {			//节点能量满足最小发送能量

			if(osTx.is_open() == 0) {				//使用Tx.log日志记录节点所有产生的数据包信息
				osTx.open(txLogName.c_str(), std::ios::app);			//c_str是string类的一个函数，可以把string类型变量转换成char*变量,open()要求的是一个char*字符串，std::ios::app代表以附加的方式输入内容
			}
			osTx << GetDevice ()->GetNode ()->GetId () << " " << p->GetUid () << " " << Now().GetSeconds () << std::endl;

			GetDevice()->packetExistFlag = true;
			m_device->SendPacket (p);
		}else {
			std::cout << "CreteMessage no energy to send" << std::endl;
		}
	}
}

void MessageProcessUnit::ComputeIndex ()					//节点移动模型计算，0.1s执行一次，在主程序调用
{
	GetDevice()->index ++;
	//std::cout << Simulator::Now().GetSeconds() << " " << "MessageProcessUnit::ComputeIndex" << "   " << "Id:" << GetDevice ()->GetNode()->GetId() << "   " << "index:" << GetDevice ()->index << std::endl;
    Simulator::Schedule (Seconds (0.1), &MessageProcessUnit::ComputeIndex, this);			//每隔0.1秒目的节点的index自增加1
}

void MessageProcessUnit::CreateGatewaytestMessage ()			//网关节点创建探测数据包，在主程序调用
{
    NS_LOG_FUNCTION (this);
	uint8_t *buffer  = new uint8_t[m_testPacketSize];				//由原先的100改为50
	for (int i = 0; i < m_testPacketSize; i++) {
	  buffer[i] = 0;
	}
	Ptr<Packet> p = Create<Packet>(buffer, m_testPacketSize);
	NanoSeqTsHeader seqTs;					//seqTs的size为12
	seqTs.SetSeq (p->GetUid ());
	p->AddHeader (seqTs);
	m_device->SendGatewaytestPacket (p);			//p->GetSize()为初始探测包大小+12
    Simulator::Schedule (Seconds (m_interarrivalTestTime), &MessageProcessUnit::CreateGatewaytestMessage, this);		//每隔0.01秒目的节点发送一次探测数据包
}

void MessageProcessUnit::ProcessMessage (Ptr<Packet> p)
{
	NanoL3Header l3Header;
	p->RemoveHeader (l3Header);

	NanoSeqTsHeader seqTs;
	p->RemoveHeader (seqTs);				//会将原seqTs数据赋值到新创建的seqTs对象

	double delay = Simulator::Now ().GetSeconds() - seqTs.GetTs().GetSeconds ();			//seqTs的开始时间戳是在路由协议中的发送数据包函数确定的

	if(osRx.is_open() == 0) {				//使用Rx.log日志记录所有的接收数据包信息
		osRx.open(rxLogName.c_str(), std::ios::app);			//c_str是string类的一个函数，可以把string类型变量转换成char*变量,open()要求的是一个char*字符串，std::ios::app代表以附加的方式输入内容
	}
	osRx << l3Header.GetSource() << " " << seqTs.GetSeq() << " " << GetDevice ()->GetNode ()->GetId () << " " << p->GetSize() << " " << delay << " " << seqTs.GetTs().GetSeconds() << " " << Now().GetSeconds () << std::endl;
}


void MessageProcessUnit::SetPacketSize (int s)
{
	NS_LOG_FUNCTION (this);
	m_packetSize = s;
}

void MessageProcessUnit::SetTestPacketSize (int s)
{
	NS_LOG_FUNCTION (this);
	m_testPacketSize = s;
}

void MessageProcessUnit::SetInterarrivalTime (double t)
{
	NS_LOG_FUNCTION (this);
	m_interarrivalTime = t;
}

void MessageProcessUnit::SetInterarrivalTestTime (double t)
{
	NS_LOG_FUNCTION (this);
	m_interarrivalTestTime = t;
}

} // namespace ns3
