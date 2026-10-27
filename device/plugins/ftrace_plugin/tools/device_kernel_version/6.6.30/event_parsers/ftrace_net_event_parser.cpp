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
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
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
template <typename FtraceEventType>
void ParseNapiGroFragsEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NapiGroFragsEntryData eventData {};
    ParseNapiGroFragsEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_frags_entry_format();
    if (msg == nullptr) {
        return;
    }
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(napi_gro_frags_entry, ParseNapiGroFragsEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(napi_gro_frags_entry, ParseNapiGroFragsEntry<ProtoEncoder::FtraceEvent>);

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
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
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
template <typename FtraceEventType>
void ParseNapiGroReceiveEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NapiGroReceiveEntryData eventData {};
    ParseNapiGroReceiveEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_napi_gro_receive_entry_format();
    if (msg == nullptr) {
        return;
    }
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(napi_gro_receive_entry, ParseNapiGroReceiveEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(napi_gro_receive_entry, ParseNapiGroReceiveEntry<ProtoEncoder::FtraceEvent>);

struct NetDevQueueData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetDevQueueData(NetDevQueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseNetDevQueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetDevQueueData eventData {};
    ParseNetDevQueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_queue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_queue, ParseNetDevQueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_queue, ParseNetDevQueue<ProtoEncoder::FtraceEvent>);

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
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
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
template <typename FtraceEventType>
void ParseNetDevStartXmit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetDevStartXmitData eventData {};
    ParseNetDevStartXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_start_xmit_format();
    if (msg == nullptr) {
        return;
    }
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_start_xmit, ParseNetDevStartXmit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_start_xmit, ParseNetDevStartXmit<ProtoEncoder::FtraceEvent>);

struct NetDevXmitData {
    uint64_t skbaddr {};
    uint32_t len {};
    int32_t rc {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetDevXmitData(NetDevXmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.rc = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseNetDevXmit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetDevXmitData eventData {};
    ParseNetDevXmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_net_dev_xmit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_rc(eventData.rc);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(net_dev_xmit, ParseNetDevXmit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(net_dev_xmit, ParseNetDevXmit<ProtoEncoder::FtraceEvent>);

struct NetifReceiveSkbData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetifReceiveSkbData(NetifReceiveSkbData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseNetifReceiveSkb(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetifReceiveSkbData eventData {};
    ParseNetifReceiveSkbData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_receive_skb, ParseNetifReceiveSkb<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_receive_skb, ParseNetifReceiveSkb<ProtoEncoder::FtraceEvent>);

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
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
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
template <typename FtraceEventType>
void ParseNetifReceiveSkbEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetifReceiveSkbEntryData eventData {};
    ParseNetifReceiveSkbEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_receive_skb_entry_format();
    if (msg == nullptr) {
        return;
    }
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_receive_skb_entry, ParseNetifReceiveSkbEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_receive_skb_entry, ParseNetifReceiveSkbEntry<ProtoEncoder::FtraceEvent>);

struct NetifRxData {
    uint64_t skbaddr {};
    uint32_t len {};
    std::string name {};
};
__attribute__((noinline)) void ParseNetifRxData(NetifRxData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.skbaddr = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseNetifRx(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetifRxData eventData {};
    ParseNetifRxData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_skbaddr(eventData.skbaddr);
    msg->set_len(eventData.len);
    msg->set_name(eventData.name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_rx, ParseNetifRx<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_rx, ParseNetifRx<ProtoEncoder::FtraceEvent>);

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
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
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
template <typename FtraceEventType>
void ParseNetifRxEntry(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    NetifRxEntryData eventData {};
    ParseNetifRxEntryData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_netif_rx_entry_format();
    if (msg == nullptr) {
        return;
    }
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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(netif_rx_entry, ParseNetifRxEntry<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(netif_rx_entry, ParseNetifRxEntry<ProtoEncoder::FtraceEvent>);
}  // namespace
FTRACE_NS_END
