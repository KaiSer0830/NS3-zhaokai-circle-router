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

#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/packet-burst.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/double.h"
#include "ns3/mobility-model.h"
#include "ns3/spectrum-phy.h"
#include "ns3/spectrum-propagation-loss-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "nano-spectrum-channel.h"
#include "nano-spectrum-phy.h"
#include <iostream>
#include <utility>
#include "ns3/simple-nano-device.h"
#include <cmath>
#include "ns3/random-nano-routing-entity.h"
#include "ns3/flooding-nano-routing-entity.h"
#include "ns3/opportunistic-nano-routing-entity.h"

NS_LOG_COMPONENT_DEFINE("NanoSpectrumChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(NanoSpectrumChannel);

NanoSpectrumChannel::NanoSpectrumChannel() {
	m_transmissionRange = 0;
	NS_LOG_FUNCTION(this);
}

void NanoSpectrumChannel::DoDispose() {
	NS_LOG_FUNCTION(this);
	m_phyList.clear();
	m_spectrumModel = 0;
	m_propagationDelay = 0;
	m_propagationLoss = 0;
	m_spectrumPropagationLoss = 0;
	SpectrumChannel::DoDispose();
}

TypeId NanoSpectrumChannel::GetTypeId(void) {
	NS_LOG_FUNCTION_NOARGS ();
	static TypeId tid = TypeId("ns3::NanoSpectrumChannel").SetParent<SpectrumChannel>().AddConstructor<NanoSpectrumChannel>();
	return tid;
}

void NanoSpectrumChannel::AddRx(Ptr<SpectrumPhy> phy) {
	NS_LOG_FUNCTION(this << phy);
	m_phyList.push_back(phy);
}

void NanoSpectrumChannel::StartTx(Ptr<SpectrumSignalParameters> txParams) {
	NS_LOG_FUNCTION(this << txParams->psd << txParams->duration << txParams->txPhy);
	NS_ASSERT_MSG(txParams->psd, "NULL txPsd");
	NS_ASSERT_MSG(txParams->txPhy, "NULL txPhy");

	if (m_spectrumModel == 0) {
		m_spectrumModel = txParams->psd->GetSpectrumModel();
	} else {
		NS_ASSERT(*(txParams->psd->GetSpectrumModel()) == *m_spectrumModel);
	}

	Ptr<MobilityModel> senderMobility = txParams->txPhy->GetMobility();
	double txRange = txParams->txPhy->GetObject<NanoSpectrumPhy>()->GetTransmissionRange();				//获取物理层的节点传输范围

	for (PhyList::const_iterator rxPhyIterator = m_phyList.begin(); rxPhyIterator != m_phyList.end(); ++rxPhyIterator) {
		if ((*rxPhyIterator) != txParams->txPhy) {
			Time delay = MicroSeconds(0);						//delay为+0.0fs
			Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility();
			double distance = (receiverMobility->GetDistanceFrom(senderMobility));				//计算两个节点之间的距离

			NS_LOG_FUNCTION(this << "check txrange" << distance << txRange);					//将数据发送给传输范围内的所有节点
			if (distance <= txRange) {
//				if(txParams->txPhy->GetDevice()->GetNode()->GetId() > 1) {
//					std::cout << Simulator::Now().GetSeconds() << " " << "txPhy:" << txParams->txPhy->GetDevice()->GetNode()->GetId() << "   " << "index:" << (*rxPhyIterator)->GetDevice()->GetNode()->GetId() << "   " << "distance:" << distance << std::endl;
//				}
				NS_LOG_LOGIC("copying signal parameters " << txParams);
				Ptr<SpectrumSignalParameters> rxParams = txParams->Copy();

				Ptr<NetDevice> netDev = (*rxPhyIterator)->GetDevice();
				if (netDev) {
					uint32_t dstNode = netDev->GetNode()->GetId();
					Simulator::ScheduleWithContext(dstNode, delay, &NanoSpectrumChannel::StartRx, this, rxParams, *rxPhyIterator);
				} else {
					Simulator::Schedule(delay, &NanoSpectrumChannel::StartRx, this, rxParams, *rxPhyIterator);
				}
			}
		}
	}
}

void NanoSpectrumChannel::StartRx(Ptr<SpectrumSignalParameters> params, Ptr<SpectrumPhy> receiver) {
	NS_LOG_FUNCTION(this << params);
	receiver->StartRx(params);
}

uint32_t NanoSpectrumChannel::GetNDevices(void) const {
	NS_LOG_FUNCTION(this);
	return m_phyList.size();
}

Ptr<NetDevice> NanoSpectrumChannel::GetDevice(uint32_t i) const {
	NS_LOG_FUNCTION(this << i);
	return m_phyList.at(i)->GetDevice()->GetObject<NetDevice>();
}

void NanoSpectrumChannel::AddPropagationLossModel(Ptr<PropagationLossModel> loss) {
	NS_LOG_FUNCTION(this << loss);
	NS_ASSERT(m_propagationLoss == 0);
	m_propagationLoss = loss;
}

void NanoSpectrumChannel::AddSpectrumPropagationLossModel(Ptr<SpectrumPropagationLossModel> loss) {
	NS_LOG_FUNCTION(this << loss);
	NS_ASSERT(m_spectrumPropagationLoss == 0);
	m_spectrumPropagationLoss = loss;
}

void NanoSpectrumChannel::SetPropagationDelayModel(Ptr<PropagationDelayModel> delay) {
	NS_LOG_FUNCTION(this << delay);
	NS_ASSERT(m_propagationDelay == 0);
	m_propagationDelay = delay;
}

Ptr<SpectrumPropagationLossModel> NanoSpectrumChannel::GetSpectrumPropagationLossModel(void) {
	NS_LOG_FUNCTION(this);
	return m_spectrumPropagationLoss;
}

void NanoSpectrumChannel::SetTransmissionRange(double r) {
	NS_LOG_FUNCTION(this << r);
	m_transmissionRange = r;
}

double NanoSpectrumChannel::GetTransmissionRange(void) {
	NS_LOG_FUNCTION(this);
	return m_transmissionRange;
}

std::vector<NanoDetail> NanoSpectrumChannel::GetNeighbors(Ptr<SpectrumPhy> phy) {				//网关节点发送探测包时的获取邻居节点函数,获取的邻居节点类型包括index、节点id、节点类型
	std::vector<NanoDetail> neighbors;

	Ptr<MobilityModel> senderMobility = phy->GetMobility();
	double txRange = phy->GetObject<NanoSpectrumPhy>()->GetTransmissionRange();

	for (PhyList::const_iterator rxPhyIterator = m_phyList.begin(); rxPhyIterator != m_phyList.end(); ++rxPhyIterator) {
		if ((*rxPhyIterator) != phy) {
			Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility();
			double distance = (receiverMobility->GetDistanceFrom(senderMobility));
			if (distance <= txRange) {					//根据距离判断邻居节点
				(*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetTestSize());
				NanoDetail neighbor;
				neighbor.detail_index = (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->index;
				neighbor.detail_id = (*rxPhyIterator)->GetDevice()->GetNode()->GetId();
				if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoNode)			//将邻居节点进行分类
					neighbor.detail_type = 2;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoRouter)
					neighbor.detail_type = 1;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoInterface)
					neighbor.detail_type = 0;

				neighbors.push_back(neighbor);
			}
		}
	}
	return neighbors;
}

std::vector<NanoDetail> NanoSpectrumChannel::GetNeighborss(Ptr<SpectrumPhy> phy, Ptr<Packet> p) {				//泛洪与随机路由协议中普通纳米节点获取邻居节点函数，获取的邻居节点类型包括index、节点id、节点类型
	NS_LOG_FUNCTION(this);

	std::vector<NanoDetail> neighbors;
	Ptr<MobilityModel> senderMobility = phy->GetMobility();
	double txRange = phy->GetObject<NanoSpectrumPhy>()->GetTransmissionRange();

	for (PhyList::const_iterator rxPhyIterator = m_phyList.begin(); rxPhyIterator != m_phyList.end(); ++rxPhyIterator) {
		if ((*rxPhyIterator) != phy) {
			Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility();
			double distance = (receiverMobility->GetDistanceFrom(senderMobility));
			if (distance <= txRange) {					//根据距离判断邻居节点
				if(phy->GetDevice()->GetNode()->GetId() > 0 && (*rxPhyIterator)->GetDevice()->GetNode()->GetId() > 0) {				//根据条件过滤不符合条件的普通纳米节点
					(*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetTestSize());	//所有候选节点都需要消耗接收探测数据包的能量,除网关与路由节点外
					//过滤邻居节点中剩余能量不符合要求的，所有过滤条件在路由协议接收时都要重新判断一遍，因为这里筛选只是统计邻居节点数目，不决定具体接收节点
					if(((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_energy) < (phy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMinSatisfidForwardEnergy())) {
						continue;
					}
					//过滤邻居节点中已带有数据包的节点
					if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag) {
						continue;
					}
					//过滤邻居节点中已接收过该数据包的节点，需要先判断该路由对象存不存在，否则会报错
					if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->GetObject<FloodingNanoRoutingEntity>() != 0) {
						if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->GetObject<FloodingNanoRoutingEntity>()->CheckAmongReceivedPacket(p->GetUid())) {
							continue;
						}
					} else if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->GetObject<RandomNanoRoutingEntity>() != 0) {
						if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->GetObject<RandomNanoRoutingEntity>()->CheckAmongReceivedPacket(p->GetUid())) {
							continue;
						}
					}
				}
				NanoDetail neighbor;
				neighbor.detail_index = (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->index;
				neighbor.detail_id = (*rxPhyIterator)->GetDevice()->GetNode()->GetId();
				neighbor.detail_m_energy = (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetEnergyCapacity();
				neighbor.detail_phy = (*rxPhyIterator);
				if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoNode)			//将邻居节点进行分类
					neighbor.detail_type = 2;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoRouter)
					neighbor.detail_type = 1;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoInterface)
					neighbor.detail_type = 0;

				neighbors.push_back(neighbor);
			}
		}
	}
	return neighbors;
}

std::vector<NanoDetail> NanoSpectrumChannel::GetOpportNeighborss(Ptr<SpectrumPhy> phy, Ptr<Packet> p) {				//循环机会路由协议中普通纳米节点获取邻居节点函数，获取的邻居节点类型包括index、节点id、节点类型、剩余能量和优先级
	NS_LOG_FUNCTION(this);

	std::vector<NanoDetail> neighbors;
	Ptr<MobilityModel> senderMobility = phy->GetMobility();
	double txRange = phy->GetObject<NanoSpectrumPhy>()->GetTransmissionRange();
	for (PhyList::const_iterator rxPhyIterator = m_phyList.begin(); rxPhyIterator != m_phyList.end(); ++rxPhyIterator) {
		if ((*rxPhyIterator) != phy) {
			Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility();
			double distance = (receiverMobility->GetDistanceFrom(senderMobility));			//计算距离函数已经修改成循环计算方式
			if (distance <= txRange) {					//根据距离判断邻居节点
				if(phy->GetDevice()->GetNode()->GetId() > 0 && (*rxPhyIterator)->GetDevice()->GetNode()->GetId() > 0) {
					(*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->ConsumeEnergyReceive((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetTestSize());	//所有候选节点都需要消耗接收探测数据包的能量,除网关与路由节点外
					//过滤邻居节点中index值不符合要求的，所有过滤条件在路由协议接收时都要重新判断一遍，因为这里筛选只是统计邻居节点数目，不决定具体接收节点
					if((phy->GetDevice()->GetObject<SimpleNanoDevice>()->index >= (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->index)) {
						continue;
					}
					//过滤邻居节点中剩余能量不符合转发要求的
					if(((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_energy) < (phy->GetDevice()->GetObject<SimpleNanoDevice>()->GetMinSatisfidForwardEnergy())) {
						continue;
					}
					//过滤邻居节点中已带有数据包的节点
					if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->packetExistFlag) {
						continue;
					}
					//过滤邻居节点中已接收过该数据包的节点
					if((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetL3()->GetObject<OpportunisticNanoRoutingEntity>()->CheckAmongReceivedPacket(p->GetUid())) {
						continue;
					}
				}
				NanoDetail neighbor;
				neighbor.detail_index = (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->index;
				neighbor.detail_id = (*rxPhyIterator)->GetDevice()->GetNode()->GetId();
				neighbor.detail_m_energy = (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->GetEnergyCapacity();
				neighbor.detail_phy = (*rxPhyIterator);
				if((*rxPhyIterator)->GetDevice()->GetNode()->GetId() > 1) {
					neighbor.detail_priority_value = 					//计算候选节点的转发优先级
							0.5 * (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_energy +
							0.5 * abs(phy->GetDevice()->GetObject<SimpleNanoDevice>()->index - (*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->index) +
							0.0001 * (*rxPhyIterator)->GetDevice()->GetNode()->GetId();				//加节点id是为了降低优先级一致出现的概率
				}
				if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoNode)			//将邻居节点进行分类
					neighbor.detail_type = 2;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoRouter)
					neighbor.detail_type = 1;
				else if ((*rxPhyIterator)->GetDevice()->GetObject<SimpleNanoDevice>()->m_type == SimpleNanoDevice::NanoInterface)
					neighbor.detail_type = 0;
				neighbors.push_back(neighbor);
			}
		}
	}
	if(neighbors.size() == 1) {
		neighbors.at(0).detail_priority_seq = 0;
	} else if(neighbors.size() > 1) {						//候选节点数据超过1个，需要将转发优先级排序
		std::vector<NanoDetail>::iterator it1;
		std::vector<NanoDetail>::iterator it2;
		for (it1 = neighbors.begin (); it1 != neighbors.end (); it1++) {
			int priority = 0;
			for (it2 = neighbors.begin (); it2 != neighbors.end (); it2++) {
				if ((*it1).detail_id != (*it2).detail_id && (*it1).detail_priority_value < (*it2).detail_priority_value) {
					priority++;
				}
			}
			(*it1).detail_priority_seq = priority;			//detail_priority_seq，从0开始，越小转发优先级越高，暂时未考虑优先级值相等的情况
		}
	}
	return neighbors;
}

bool NanoSpectrumChannel::CheckAmongReceivedPacket (std::list<uint32_t> m_receivedPacketList, uint32_t PacketId)
{
	for (std::list<uint32_t>::iterator it = m_receivedPacketList.begin(); it != m_receivedPacketList.end (); it++) {
		if (*it == PacketId) return true;
	}
	return false;
}

} // namespace ns3
