/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct NapiGroFragsEntryData {
    std::string name {};
    uint32_t napi_id {};
    uint32_t queue_mapping {};
    uint64_t skbaddr {};
    uint32_t vlan_tagged {};
    uint32_t vlan_proto {};
    uint32_t vlan_tci {};
    uint32_t protocol {};
    uint32_t ip_summed {};
    uint32_t hash {};
    uint32_t l4_hash {};
    uint32_t len {};
    uint32_t data_len {};
    uint32_t truesize {};
    uint32_t mac_header_valid {};
    int32_t mac_header {};
    uint32_t nr_frags {};
    uint32_t gso_size {};
    uint32_t gso_type {};
};
__attribute__((noinline)) void ParseNapiGroFragsEntryData(NapiGroFragsEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.napi_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.queue_mapping = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vlan_tagged = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_proto = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_tci = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.protocol = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ip_summed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.l4_hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.truesize = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header_valid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_frags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_size = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseNapiGroFragsEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NapiGroFragsEntryData eventData {};
    ParseNapiGroFragsEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_frags_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
void ParseNapiGroFragsEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NapiGroFragsEntryData eventData {};
    ParseNapiGroFragsEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_frags_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(napi_gro_frags_entry, ParseNapiGroFragsEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(napi_gro_frags_entry, ParseNapiGroFragsEntryEncoder);

struct NapiGroReceiveEntryData {
    std::string name {};
    uint32_t napi_id {};
    uint32_t queue_mapping {};
    uint64_t skbaddr {};
    uint32_t vlan_tagged {};
    uint32_t vlan_proto {};
    uint32_t vlan_tci {};
    uint32_t protocol {};
    uint32_t ip_summed {};
    uint32_t hash {};
    uint32_t l4_hash {};
    uint32_t len {};
    uint32_t data_len {};
    uint32_t truesize {};
    uint32_t mac_header_valid {};
    int32_t mac_header {};
    uint32_t nr_frags {};
    uint32_t gso_size {};
    uint32_t gso_type {};
};
__attribute__((noinline)) void ParseNapiGroReceiveEntryData(NapiGroReceiveEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.napi_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.queue_mapping = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vlan_tagged = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_proto = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_tci = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.protocol = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ip_summed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.l4_hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.truesize = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header_valid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_frags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_size = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseNapiGroReceiveEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NapiGroReceiveEntryData eventData {};
    ParseNapiGroReceiveEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_receive_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
void ParseNapiGroReceiveEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NapiGroReceiveEntryData eventData {};
    ParseNapiGroReceiveEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_receive_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(napi_gro_receive_entry, ParseNapiGroReceiveEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(napi_gro_receive_entry, ParseNapiGroReceiveEntryEncoder);

struct NetDevQueueData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetDevQueueData(NetDevQueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseNetDevQueue(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevQueueData eventData {};
    ParseNetDevQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_queue_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
void ParseNetDevQueueEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevQueueData eventData {};
    ParseNetDevQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_queue_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_queue, ParseNetDevQueue);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_queue, ParseNetDevQueueEncoder);

struct NetDevStartXmitData {
    std::string name {};
    uint32_t queue_mapping {};
    uint64_t skbaddr {};
    uint32_t vlan_tagged {};
    uint32_t vlan_proto {};
    uint32_t vlan_tci {};
    uint32_t protocol {};
    uint32_t ip_summed {};
    uint32_t len {};
    uint32_t data_len {};
    int32_t network_offset {};
    uint32_t transport_offset_valid {};
    int32_t transport_offset {};
    uint32_t tx_flags {};
    uint32_t gso_size {};
    uint32_t gso_segs {};
    uint32_t gso_type {};
};
__attribute__((noinline)) void ParseNetDevStartXmitData(NetDevStartXmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.queue_mapping = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vlan_tagged = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_proto = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_tci = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.protocol = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ip_summed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.network_offset = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.transport_offset_valid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.transport_offset = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.tx_flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_size = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_segs = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseNetDevStartXmit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevStartXmitData eventData {};
    ParseNetDevStartXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_start_xmit_format();
    msg->set_name(eventData.name);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_network_offset(eventData.network_offset);
    msg->set_transport_offset_valid(eventData.transport_offset_valid);
    msg->set_transport_offset(eventData.transport_offset);
    msg->set_tx_flags(eventData.tx_flags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_segs(eventData.gso_segs);
    msg->set_gso_type(eventData.gso_type);
}
void ParseNetDevStartXmitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevStartXmitData eventData {};
    ParseNetDevStartXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_start_xmit_format();
    msg->set_name(eventData.name);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_network_offset(eventData.network_offset);
    msg->set_transport_offset_valid(eventData.transport_offset_valid);
    msg->set_transport_offset(eventData.transport_offset);
    msg->set_tx_flags(eventData.tx_flags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_segs(eventData.gso_segs);
    msg->set_gso_type(eventData.gso_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_start_xmit, ParseNetDevStartXmit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_start_xmit, ParseNetDevStartXmitEncoder);

struct NetDevXmitData {
    uint64_t skbaddr {};
    uint32_t len {};
    int32_t rc {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetDevXmitData(NetDevXmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseNetDevXmit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevXmitData eventData {};
    ParseNetDevXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_xmit_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_rc(eventData.rc);
    msg->set_name(eventData.name);
}
void ParseNetDevXmitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetDevXmitData eventData {};
    ParseNetDevXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_xmit_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_rc(eventData.rc);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_xmit, ParseNetDevXmit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_xmit, ParseNetDevXmitEncoder);

struct NetifReceiveSkbData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetifReceiveSkbData(NetifReceiveSkbData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseNetifReceiveSkb(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifReceiveSkbData eventData {};
    ParseNetifReceiveSkbData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
void ParseNetifReceiveSkbEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifReceiveSkbData eventData {};
    ParseNetifReceiveSkbData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_receive_skb, ParseNetifReceiveSkb);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_receive_skb, ParseNetifReceiveSkbEncoder);

struct NetifReceiveSkbEntryData {
    std::string name {};
    uint32_t napi_id {};
    uint32_t queue_mapping {};
    uint64_t skbaddr {};
    uint32_t vlan_tagged {};
    uint32_t vlan_proto {};
    uint32_t vlan_tci {};
    uint32_t protocol {};
    uint32_t ip_summed {};
    uint32_t hash {};
    uint32_t l4_hash {};
    uint32_t len {};
    uint32_t data_len {};
    uint32_t truesize {};
    uint32_t mac_header_valid {};
    int32_t mac_header {};
    uint32_t nr_frags {};
    uint32_t gso_size {};
    uint32_t gso_type {};
};
__attribute__((noinline)) void ParseNetifReceiveSkbEntryData(NetifReceiveSkbEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.napi_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.queue_mapping = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vlan_tagged = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_proto = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_tci = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.protocol = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ip_summed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.l4_hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.truesize = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header_valid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_frags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_size = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseNetifReceiveSkbEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifReceiveSkbEntryData eventData {};
    ParseNetifReceiveSkbEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
void ParseNetifReceiveSkbEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifReceiveSkbEntryData eventData {};
    ParseNetifReceiveSkbEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_receive_skb_entry, ParseNetifReceiveSkbEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_receive_skb_entry, ParseNetifReceiveSkbEntryEncoder);

struct NetifRxData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetifRxData(NetifRxData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseNetifRx(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifRxData eventData {};
    ParseNetifRxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
void ParseNetifRxEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifRxData eventData {};
    ParseNetifRxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_format();
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_rx, ParseNetifRx);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_rx, ParseNetifRxEncoder);

struct NetifRxEntryData {
    std::string name {};
    uint32_t napi_id {};
    uint32_t queue_mapping {};
    uint64_t skbaddr {};
    uint32_t vlan_tagged {};
    uint32_t vlan_proto {};
    uint32_t vlan_tci {};
    uint32_t protocol {};
    uint32_t ip_summed {};
    uint32_t hash {};
    uint32_t l4_hash {};
    uint32_t len {};
    uint32_t data_len {};
    uint32_t truesize {};
    uint32_t mac_header_valid {};
    int32_t mac_header {};
    uint32_t nr_frags {};
    uint32_t gso_size {};
    uint32_t gso_type {};
};
__attribute__((noinline)) void ParseNetifRxEntryData(NetifRxEntryData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.napi_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.queue_mapping = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.vlan_tagged = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_proto = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vlan_tci = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.protocol = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ip_summed = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.l4_hash = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.data_len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.truesize = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header_valid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.mac_header = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.nr_frags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_size = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.gso_type = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseNetifRxEntry(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifRxEntryData eventData {};
    ParseNetifRxEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
void ParseNetifRxEntryEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    NetifRxEntryData eventData {};
    ParseNetifRxEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_entry_format();
    msg->set_name(eventData.name);
    msg->set_napi_id(eventData.napi_id);
    msg->set_queue_mapping(eventData.queue_mapping);
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_vlan_tagged(eventData.vlan_tagged);
    msg->set_vlan_proto(eventData.vlan_proto);
    msg->set_vlan_tci(eventData.vlan_tci);
    msg->set_protocol(eventData.protocol);
    msg->set_ip_summed(eventData.ip_summed);
    msg->set_hash(eventData.hash);
    msg->set_l4_hash(eventData.l4_hash);
    msg->set_len(eventData.len);
    msg->set_data_len(eventData.data_len);
    msg->set_truesize(eventData.truesize);
    msg->set_mac_header_valid(eventData.mac_header_valid);
    msg->set_mac_header(eventData.mac_header);
    msg->set_nr_frags(eventData.nr_frags);
    msg->set_gso_size(eventData.gso_size);
    msg->set_gso_type(eventData.gso_type);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_rx_entry, ParseNetifRxEntry);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_rx_entry, ParseNetifRxEntryEncoder);
}  // namespace
FTRACE_NS_END
