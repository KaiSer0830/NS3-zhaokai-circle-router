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

#ifndef SIMPLE_NANO_DEVICE_H
#define SIMPLE_NANO_DEVICE_H

#include <string.h>
#include <ns3/node.h>
#include <ns3/address.h>
#include <ns3/net-device.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <list>
#include "ns3/nano-spectrum-phy.h"
#include "ns3/core-module.h"

namespace ns3 {

	class NanoSpectrumPhy;
	class NanoMacEntity;
	class NanoRoutingEntity;
	class MessageProcessUnit;

	/**
	 * \ingroup nanonetworks
	 *
	 * This class provides a template implementation
	 * of a a generic nano device
	 */

	class SimpleNanoDevice : public NetDevice
	{
		public:
			static TypeId GetTypeId (void);

			SimpleNanoDevice ();
			virtual ~SimpleNanoDevice ();

			virtual void DoDispose (void);

			// inherited from NetDevice
			virtual void SetIfIndex (const uint32_t index);
			virtual uint32_t GetIfIndex (void) const;
			virtual Ptr<Channel> GetChannel (void) const;
			virtual bool SetMtu (const uint16_t mtu);
			virtual uint16_t GetMtu (void) const;
			virtual void SetAddress (Address address);
			virtual Address GetAddress (void) const;
			virtual bool IsLinkUp (void) const;
			virtual void AddLinkChangeCallback (Callback<void> callback);
			virtual bool IsBroadcast (void) const;
			virtual Address GetBroadcast (void) const;
			virtual bool IsMulticast (void) const;
			virtual bool IsPointToPoint (void) const;
			virtual bool IsBridge (void) const;
			virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
			virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
			virtual Ptr<Node> GetNode (void) const;
			virtual void SetNode (Ptr<Node> node);
			virtual bool NeedsArp (void) const;
			virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
			virtual Address GetMulticast (Ipv4Address addr) const;
			virtual Address GetMulticast (Ipv6Address addr) const;
			virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
			virtual bool SupportsSendFrom (void) const;

			/**
			* Set the Phy object which is attached to this device.
			* @param phy the Phy object embedded within this device.
			*/
			void SetPhy (Ptr<NanoSpectrumPhy> phy);

			/**
			* @return a reference to the PHY object embedded in this NetDevice.
			*/
			Ptr<NanoSpectrumPhy> GetPhy () const;

			/**
			* Set the
			* @param mac
			*/
			void SetMac (Ptr<NanoMacEntity> mac);

			/**
			* @return a reference to
			*/
			Ptr<NanoMacEntity> GetMac () const;

			/**
			* Set the
			* @param routing
			*/
			void SetL3 (Ptr<NanoRoutingEntity> l3);

			/**
			* @return a reference to
			*/
			Ptr<NanoRoutingEntity> GetL3 () const;


			void SendPacket (Ptr<Packet> p);
			void SendGatewaytestPacket (Ptr<Packet> p);					//flow-guided,发送探测数据包
			void SendackPacket (Ptr<Packet> p,uint32_t dst);		//flow-guided,发送响应数据包
			void ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy);

			void SetMessageProcessUnit (Ptr<MessageProcessUnit> mpu);
			Ptr<MessageProcessUnit> GetMessageProcessUnit (void);

			void SetEnergyCapacity(double energy);					//flow-guided,设置存储能量大小
			double GetEnergyCapacity() const;							//flow-guided,获取存储能量大小
			void SetMaxEnergy(double maxenergy);						//flow-guided,设置最大存储能量大小
			void SetInterarrivalTestTime(double t);						//flow-guided,设置探测数据包产生时隙
			void SetCapEnergyInterval(double t);					//flow-guided,设置能量捕获时隙
			void SetCapEnergySpeed(int speed);						//flow-guided,设置能量捕获速率
			void HarvestEnergy();				//flow-guided,模拟接收能量随机的情况

			double ConsumeEnergySend(double packetsize);					//flow-guided
			double ConsumeEnergyReceive(double packetsize);				//flow-guided
			void SetEnergySendPerByte(double energysendperbyte);			//flow-guided,设置发送能耗/字节
			void SetEnergyReceivePerByte(double energyreceiveperbyte);	//flow-guided,设置接收能耗/字节
			double GetEnergySendPerByte();								//flow-guided,获取发送能耗/字节
			double GetEnergyReceivePerByte();								//flow-guided,获取接收能耗/字节

			void SetPacketSize(double packetsize);		//flow-guided,设置数据包大小
			int GetPacketSize();						//flow-guided,获取数据包大小
			void SetTestSize(double testsize);		//flow-guided,设置探测数据包大小
			int GetTestSize();						//flow-guided,获取探测数据包大小
			void SetAckSize(double acksize);		//flow-guided,设置ack数据包大小
			int GetAckSize();						//flow-guided,获取ack数据包大小

			double GetInterarrivalTestTime();		//flow-guided,获取探测数据包产生时隙
			void SetParameter(double parameter1,double parameter2);
			double GetM();
			void SetU(double u);

			void SetaverageIndex(double averageIndex);					//flow-guided,设置平均Index值
			void SetEtag(bool Etag);
			bool GetEtag();
			double GetMinSatisfidSendEnergy();						//flow-guided,设置最小发送能量
			double GetMinSatisfidForwardEnergy();					//flow-guided,设置最小转发能量
			void SetLogName(std::string name);
			void SetOsFstream(std::ofstream& os, std::string s);
			void EvendJudge();

			enum NodeType
			{
				NanoInterface,NanoRouter,NanoNode
			};
			NodeType m_type;			//节点类型,0代表网关节点，1代表路由节点，2代表纳米节点

		private:
			Ptr<Node> m_node;
			uint32_t m_ifIndex;
			Ptr<NanoSpectrumPhy> m_phy;
			Ptr<NanoMacEntity> m_mac;
			Ptr<NanoRoutingEntity> m_l3;
			Ptr<MessageProcessUnit> m_mpu;
			uint32_t m_randv;				//flow-guided,使用m_randv变量来作为srand()不同的种子，从而产生不同能量捕获速度的随机值

		public:
			double m_txrange;

			uint32_t index;				//flow-guided,节点index值
			bool packetExistFlag;		//节点当前是否携带数据包，初始值为false，方便第一次创建

			double m_energy;					//flow-guided,节点当前能量
			double m_maxenergy;					//flow-guided,节点最大存储能量
			double m_capEnergyInterval;			//flow-guided,节点能量捕获时隙
			double m_capEnergySpeed;			//flow-guided,节点能量捕获速度
			double m_EnergySendPerByte;			//flow-guided,发送能耗/字节
			double m_EnergyReceivePerByte;		//flow-guided,接收能耗/字节
			double m_interarrivalTestTime;		//flow-guided,探测数据包产生时隙0.01s
			int m_PacketSize;				//flow-guided,数据包大小
			int m_TestSize;					//flow-guided,探测数据包大小
			int m_AckSize;					//flow-guided,ack数据包大小

			double M;					//flow-guided,候选节点转发的优先级
			double U,parameter1,parameter2;				//flow-guided,计算候选节点转发优先级的系统参数
			double reSendTimeInterval;					//flow-guided,发送节点重新发送检测邻居节点的时间间隔

			std::string energyLog;				//能量消耗输出日志名称
			std::ofstream osEnergy;

			EventId nodeSchedule;				//节点的创建数据包Schedule事件
	};	//SimpleNanoDevice
} 		//namespace ns3

#endif /* SIMPLE_NANO_DEVICE_H */
