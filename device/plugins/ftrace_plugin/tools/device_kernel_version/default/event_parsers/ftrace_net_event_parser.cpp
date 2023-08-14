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

FTRACE_NS_BEGIN
namespace {
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    napi_gro_frags_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_napi_gro_frags_entry_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_napi_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_l4_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_truesize(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_nr_frags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    napi_gro_receive_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_napi_gro_receive_entry_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_napi_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_l4_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_truesize(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_nr_frags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    net_dev_queue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_net_dev_queue_format();
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    net_dev_start_xmit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_net_dev_start_xmit_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_network_offset(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_transport_offset_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_transport_offset(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_tx_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_segs(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    net_dev_xmit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_net_dev_xmit_format();
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_rc(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    netif_receive_skb,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_netif_receive_skb_format();
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    netif_receive_skb_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_netif_receive_skb_entry_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_napi_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_l4_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_truesize(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_nr_frags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    netif_rx,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_netif_rx_format();
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    netif_rx_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_netif_rx_entry_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_napi_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_l4_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_truesize(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_nr_frags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    netif_rx_ni_entry,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_netif_rx_ni_entry_format();
        msg->set_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_napi_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_queue_mapping(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_skbaddr(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_vlan_tagged(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_proto(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vlan_tci(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_protocol(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ip_summed(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_l4_hash(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_data_len(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_truesize(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header_valid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_mac_header(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_nr_frags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_size(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_gso_type(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
