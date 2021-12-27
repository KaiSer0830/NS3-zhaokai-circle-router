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
#include <stdint.h>
#include <iostream>
#include "ns3/spectrum-phy.h"

#ifndef NANO_DETAIL_H
#define NANO_DETAIL_H


namespace ns3 {
	//typedef __uint32_t uint32_t;
	class NanoDetail{
		public:
			int detail_type;
			uint32_t detail_id;
			uint32_t detail_index;
			double detail_m_energy;
			double detail_priority_value;
			int detail_priority_seq;
			Ptr<SpectrumPhy> detail_phy;
	};

} // namespace ns3

#endif /* NANO_MAC_ENTITY_H */
