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
#include "simple-nano-device.h"
#include "nano-spectrum-phy.h"
#include "nano-mac-entity.h"
#include "nano-mac-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "nano-routing-entity.h"
#include "message-process-unit.h"
#include "math.h"

NS_LOG_COMPONENT_DEFINE ("SimpleNanoDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimpleNanoDevice);

TypeId SimpleNanoDevice::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::SimpleNanoDevice").SetParent<NetDevice> ().AddConstructor<SimpleNanoDevice> ();
	return tid;
}

SimpleNanoDevice::SimpleNanoDevice ()			//变量初始化
{
	NS_LOG_FUNCTION(this);
	m_node = 0;
	m_mac = 0;
	m_phy = 0;
	m_l3 = 0;
	m_mpu = 0;
	index = 0;
	M = 0;
	U = 0;
	parameter1 = 0;
	parameter2 = 0;
	averageindex = 0;
	mobile = 0;				//flow-guided,节点相对移动性
	packetExistFlag = false;		//节点当前是否携带数据包，初始值为false，不携带数据包

	m_energy = 0;
	m_maxenergy = 0;
	m_capEnergyInterval = 0;
	m_capEnergySpeed = 0;
	m_EnergySendPerByte = 0;
	m_EnergyReceivePerByte = 0;
	m_PacketSize = 100;
	m_TestSize = 50;
	m_type = SimpleNanoDevice::NanoInterface;
	m_ifIndex = 0;
	m_randv = 0;
	m_txrange = 0;
}

SimpleNanoDevice::~SimpleNanoDevice ()
{
	NS_LOG_FUNCTION (this);
}

void SimpleNanoDevice::DoDispose ()
{
	NS_LOG_FUNCTION (this);
	m_node = 0;
	m_mac = 0;
	m_phy = 0;
	m_l3 = 0;
	m_mpu = 0;
	index = 0;
	M = 0;
	U = 0;
	parameter1 = 0;
	parameter2 = 0;
	NetDevice::DoDispose ();
}

void SimpleNanoDevice::SetIfIndex (const uint32_t index)
{
	NS_LOG_FUNCTION (index);
	m_ifIndex = index;
}

uint32_t SimpleNanoDevice::GetIfIndex(void) const {
	NS_LOG_FUNCTION(this);
	return m_ifIndex;
}

bool SimpleNanoDevice::SetMtu (uint16_t mtu)
{
	NS_LOG_FUNCTION (mtu);
	return (mtu == 0);
}

uint16_t SimpleNanoDevice::GetMtu (void) const
{
	NS_LOG_FUNCTION (this);
	return 0;
}

Ptr<Channel> SimpleNanoDevice::GetChannel (void) const
{
	NS_LOG_FUNCTION (this);
	return 0;
}

void SimpleNanoDevice::SetAddress (Address address)
{
	NS_LOG_FUNCTION (this);
}

Address SimpleNanoDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return Address ();
}

bool SimpleNanoDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Address SimpleNanoDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Address ();
}

bool SimpleNanoDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Address SimpleNanoDevice::GetMulticast (Ipv4Address addr) const
{
  NS_LOG_FUNCTION (addr);
  return Address ();
}

Address SimpleNanoDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (addr);
  return Address ();
}

bool SimpleNanoDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool SimpleNanoDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Ptr<Node> SimpleNanoDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void SimpleNanoDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);
  m_node = node;
}

bool SimpleNanoDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool SimpleNanoDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void SimpleNanoDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (&callback);
}

void SimpleNanoDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
}

void SimpleNanoDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
}

bool SimpleNanoDevice::SupportsSendFrom () const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool SimpleNanoDevice::Send (Ptr<Packet> packet,const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return false;
}

bool SimpleNanoDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
  return false;
}

void SimpleNanoDevice::SetPhy (Ptr<NanoSpectrumPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phy = phy;
}

Ptr<NanoSpectrumPhy> SimpleNanoDevice::GetPhy () const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void SimpleNanoDevice::SendPacket (Ptr<Packet> p)			//数据包在CreteMessage发送数据包时调用此函数
{
	NS_LOG_FUNCTION (this << p << "node" << GetNode ()->GetId ());
	GetL3 ()->SendPacket (p);
}

void SimpleNanoDevice::SendGatewaytestPacket (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p << "node" << GetNode ()->GetId ());
  GetL3 ()->SendGatewaytestPacket (p);
}

void SimpleNanoDevice::SendackPacket (Ptr<Packet> p,uint32_t dst)
{
  NS_LOG_FUNCTION (this << p << "node" << GetNode ()->GetId ());
  GetL3 ()->SendackPacket (p, dst);
}

void SimpleNanoDevice::ReceivePacket (Ptr<Packet> p, Ptr<SpectrumPhy> sourcePhy)			//数据包在ts-ook物理层协议完成数据包接收时调用此函数
{
  NS_LOG_FUNCTION (this << p);
  GetL3 ()->ReceivePacket (p, sourcePhy);
}

void SimpleNanoDevice::SetMac (Ptr<NanoMacEntity> mac)
{
  NS_LOG_FUNCTION (this);
  m_mac = mac;
}

Ptr<NanoMacEntity> SimpleNanoDevice::GetMac () const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

void SimpleNanoDevice::SetL3 (Ptr<NanoRoutingEntity> l3)
{
  NS_LOG_FUNCTION (this);
  m_l3 = l3;
}

Ptr<NanoRoutingEntity> SimpleNanoDevice::GetL3 () const
{
  NS_LOG_FUNCTION (this);
  return m_l3;
}

void SimpleNanoDevice::SetMessageProcessUnit (Ptr<MessageProcessUnit> mpu)
{
  NS_LOG_FUNCTION (this);
  m_mpu = mpu;
  if(m_type == SimpleNanoDevice::NanoNode) {
	  m_mpu->SetPacketSize(m_PacketSize);			//普通纳米节点设置节点传输数据包的大小
  } else if(m_type == SimpleNanoDevice::NanoInterface) {
	  m_mpu->SetTestPacketSize(m_TestSize);			//网关节点设置节点传输探测数据包的大小
  }
}

Ptr<MessageProcessUnit> SimpleNanoDevice::GetMessageProcessUnit (void)
{
  NS_LOG_FUNCTION (this);
  return m_mpu;
}

void SimpleNanoDevice::SetEnergyCapacity(double energy) {
	NS_LOG_FUNCTION(this);
	m_energy = energy;
}

double SimpleNanoDevice::GetEnergyCapacity() const {
	NS_LOG_FUNCTION(this);
	return m_energy;
}

void SimpleNanoDevice::SetMaxEnergy(double maxenergy) {
	NS_LOG_FUNCTION(this);
	m_maxenergy = maxenergy;
}

void SimpleNanoDevice::SetCapEnergyInterval(double t) {
	NS_LOG_FUNCTION(this);
	m_capEnergyInterval = t;
}

void SimpleNanoDevice::SetCapEnergySpeed(int speed) {
	NS_LOG_FUNCTION(this);
	m_capEnergySpeed = speed;
}

void SimpleNanoDevice::HarvestEnergy() {
	NS_LOG_FUNCTION(this);
	Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable> ();			//类UniformRandomVariable从RandomVariableStream继承
	double nowCapEnergySpeed = random->GetValue(1.0, m_capEnergySpeed);			//根据随机数产生不同的能量捕获率,1-4pJ/s
	double HarEnergyThisTime = m_capEnergyInterval * nowCapEnergySpeed;
	//std::cout << "before: " << m_energy << " " << "HarEnergyThisTime: " << HarEnergyThisTime << std::endl;
	m_energy = m_energy + HarEnergyThisTime ;
	//std::cout << "after: " << m_energy << std::endl;

	if (m_energy > m_maxenergy) {
		m_energy = m_maxenergy;
	}
	Simulator::Schedule(Seconds(m_capEnergyInterval), &SimpleNanoDevice::HarvestEnergy, this);			//能量捕获间隔为0.1s
}

double SimpleNanoDevice::ConsumeEnergySend(double packetsize) {
	NS_LOG_FUNCTION(this);
	double ConsumeEnergyThisTime = packetsize * m_EnergySendPerByte;

	//计算公式：每个字节8位，平均一半1一半0，每个pulse100aJ，能量效率25%，
	double before_energy = m_energy;
	m_energy = m_energy - ConsumeEnergyThisTime;

	if(osEnergy.is_open() == 0) {							//使用energy.log文件记录所有普通纳米节点消耗的能量信息
		osEnergy.open(energyLog.c_str(), std::ios::app);			//c_str是string类的一个函数，可以把string类型变量转换成char*变量,open()要求的是一个char*字符串，std::ios::app代表以附加的方式输入内容
	}
	osEnergy << m_node->GetId() << " " << before_energy << " " << ConsumeEnergyThisTime << " " << m_energy << std::endl;

	if (m_energy < 0) {
		m_energy = 0;
	}
	return ConsumeEnergyThisTime;
}

double SimpleNanoDevice::ConsumeEnergyReceive(double packetsize) {
	NS_LOG_FUNCTION(this);
	double ConsumeEnergyThisTime = packetsize * m_EnergyReceivePerByte;

	//计算公式：每个字节8位，平均一半1一半0，每个pulse100aJ，能量效率25%，
	double before_energy = m_energy;
	m_energy = m_energy - ConsumeEnergyThisTime;

	if(osEnergy.is_open() == 0) {							//使用energy.log文件记录所有普通纳米节点消耗的能量信息
		osEnergy.open(energyLog.c_str(), std::ios::app);			//c_str是string类的一个函数，可以把string类型变量转换成char*变量,open()要求的是一个char*字符串，std::ios::app代表以附加的方式输入内容
	}
	osEnergy << m_node->GetId() << " " << before_energy << " " << ConsumeEnergyThisTime << " " << m_energy << std::endl;

	if (m_energy < 0) {
		m_energy = 0;
	}
	return ConsumeEnergyThisTime;
}

void SimpleNanoDevice::SetEnergySendPerByte(double energysendperbyte) {
	NS_LOG_FUNCTION(this);
	m_EnergySendPerByte = energysendperbyte;
}

double SimpleNanoDevice::GetEnergySendPerByte() {
	NS_LOG_FUNCTION(this);
	return m_EnergySendPerByte ;
}

void SimpleNanoDevice::SetEnergyReceivePerByte(double energyreceiveperbyte) {
	NS_LOG_FUNCTION(this);
	m_EnergyReceivePerByte = energyreceiveperbyte;
}

double SimpleNanoDevice::GetEnergyReceivePerByte() {
	NS_LOG_FUNCTION(this);
	return m_EnergyReceivePerByte ;
}

void SimpleNanoDevice::SetPacketSize(double packetsize) {
	NS_LOG_FUNCTION(this);
	m_PacketSize = packetsize;
}

double SimpleNanoDevice::GetPacketSize() {
	NS_LOG_FUNCTION(this);
	return m_PacketSize;
}

double SimpleNanoDevice::GetM() {					//求候选节点转发的优先级
	NS_LOG_FUNCTION(this);
	double x = (m_maxenergy - m_energy) * 1.0 / m_maxenergy;
	M = parameter1 * (m_capEnergySpeed * log(U) * 1.0) / (m_maxenergy * (pow(U, x) - 1)) + parameter2 * (index);					//m_maxenergy * (pow(U, x) + 1)有误，改成m_maxenergy * (pow(U, x) - 1)
	return M;
}

void SimpleNanoDevice::SetU(double u) {
	NS_LOG_FUNCTION(this);
	U = u;
}

void SimpleNanoDevice::SetTestSize(double testsize) {
	NS_LOG_FUNCTION(this);
	m_TestSize = testsize;
}

int SimpleNanoDevice::GetTestSize() {
	NS_LOG_FUNCTION(this);
	return m_TestSize;
}

void SimpleNanoDevice::SetParameter(double param1,double param2) {
	parameter1 = param1;
	parameter2 = param2;
}

void SimpleNanoDevice::SetaverageIndex(double averageIndex) {
	averageindex = averageIndex;
}

double SimpleNanoDevice::GetaverageIndex() {
	return averageindex;
}

void SimpleNanoDevice::Setmobility(double Mobile) {
	mobile = Mobile;
}

double SimpleNanoDevice::Getmobility() {
	return mobile;
}

double SimpleNanoDevice::GetMinSatisfidSendEnergy() {			//邻居节点最小发送能量为发送数据包能量+发送候选节点探测包能量+接收网关与路由探测包能量
	return (m_PacketSize + m_TestSize) * m_EnergySendPerByte + 2 * m_TestSize * m_EnergyReceivePerByte;
}

double SimpleNanoDevice::GetMinSatisfidForwardEnergy() {			//邻居节点最小转发能量需要为接收数据包能量+接收候选节点探测包能量+接收网关与路由探测包能量+发送数据包能量+发送探测包能量
	return (m_PacketSize + 3 * m_TestSize) * m_EnergyReceivePerByte + (m_PacketSize + m_TestSize) * m_EnergySendPerByte;
}

} // namespace ns3
