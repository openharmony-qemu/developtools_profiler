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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    napi_gro_frags_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_napi_gro_frags_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.napi_gro_frags_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "napi_gro_frags_entry: dev=%s napi_id=%#x queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x "
            "vlan_tci=0x%04x protocol=0x%04x ip_summed=%d hash=0x%08x l4_hash=%d len=%u data_len=%u truesize=%u "
            "mac_header_valid=%d mac_header=%d nr_frags=%d gso_size=%d gso_type=%#x",
            msg.name().c_str(), msg.napi_id(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(),
            msg.vlan_tci(), msg.protocol(), msg.ip_summed(), msg.hash(), msg.l4_hash(), msg.len(), msg.data_len(),
            msg.truesize(), msg.mac_header_valid(), msg.mac_header(), msg.nr_frags(), msg.gso_size(), msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(napi_gro_frags_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    napi_gro_receive_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_napi_gro_receive_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.napi_gro_receive_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "napi_gro_receive_entry: dev=%s napi_id=%#x queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x "
            "vlan_tci=0x%04x protocol=0x%04x ip_summed=%d hash=0x%08x l4_hash=%d len=%u data_len=%u truesize=%u "
            "mac_header_valid=%d mac_header=%d nr_frags=%d gso_size=%d gso_type=%#x",
            msg.name().c_str(), msg.napi_id(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(),
            msg.vlan_tci(), msg.protocol(), msg.ip_summed(), msg.hash(), msg.l4_hash(), msg.len(), msg.data_len(),
            msg.truesize(), msg.mac_header_valid(), msg.mac_header(), msg.nr_frags(), msg.gso_size(), msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(napi_gro_receive_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    net_dev_queue,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_net_dev_queue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.net_dev_queue_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "net_dev_queue: dev=%s skbaddr=%p len=%u",
            msg.name().c_str(), msg.skbaddr(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(net_dev_queue) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    net_dev_start_xmit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_net_dev_start_xmit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.net_dev_start_xmit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "net_dev_start_xmit: dev=%s queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x vlan_tci=0x%04x "
            "protocol=0x%04x ip_summed=%d len=%u data_len=%u network_offset=%d transport_offset_valid=%d "
            "transport_offset=%d tx_flags=%d gso_size=%d gso_segs=%d gso_type=%#x",
            msg.name().c_str(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(), msg.vlan_tci(),
            msg.protocol(), msg.ip_summed(), msg.len(), msg.data_len(), msg.network_offset(),
            msg.transport_offset_valid(), msg.transport_offset(), msg.tx_flags(), msg.gso_size(), msg.gso_segs(),
            msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(net_dev_start_xmit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    net_dev_xmit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_net_dev_xmit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.net_dev_xmit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "net_dev_xmit: dev=%s skbaddr=%p len=%u rc=%d",
            msg.name().c_str(), msg.skbaddr(), msg.len(), msg.rc());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(net_dev_xmit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    netif_receive_skb,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_netif_receive_skb_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.netif_receive_skb_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "netif_receive_skb: dev=%s skbaddr=%p len=%u",
            msg.name().c_str(), msg.skbaddr(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(netif_receive_skb) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    netif_receive_skb_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_netif_receive_skb_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.netif_receive_skb_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "netif_receive_skb_entry: dev=%s napi_id=%#x queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x "
            "vlan_tci=0x%04x protocol=0x%04x ip_summed=%d hash=0x%08x l4_hash=%d len=%u data_len=%u truesize=%u "
            "mac_header_valid=%d mac_header=%d nr_frags=%d gso_size=%d gso_type=%#x",
            msg.name().c_str(), msg.napi_id(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(),
            msg.vlan_tci(), msg.protocol(), msg.ip_summed(), msg.hash(), msg.l4_hash(), msg.len(), msg.data_len(),
            msg.truesize(), msg.mac_header_valid(), msg.mac_header(), msg.nr_frags(), msg.gso_size(), msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(netif_receive_skb_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    netif_rx,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_netif_rx_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.netif_rx_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "netif_rx: dev=%s skbaddr=%p len=%u",
            msg.name().c_str(), msg.skbaddr(), msg.len());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(netif_rx) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    netif_rx_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_netif_rx_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.netif_rx_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "netif_rx_entry: dev=%s napi_id=%#x queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x "
            "vlan_tci=0x%04x protocol=0x%04x ip_summed=%d hash=0x%08x l4_hash=%d len=%u data_len=%u truesize=%u "
            "mac_header_valid=%d mac_header=%d nr_frags=%d gso_size=%d gso_type=%#x",
            msg.name().c_str(), msg.napi_id(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(),
            msg.vlan_tci(), msg.protocol(), msg.ip_summed(), msg.hash(), msg.l4_hash(), msg.len(), msg.data_len(),
            msg.truesize(), msg.mac_header_valid(), msg.mac_header(), msg.nr_frags(), msg.gso_size(), msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(netif_rx_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    netif_rx_ni_entry,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_netif_rx_ni_entry_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.netif_rx_ni_entry_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "netif_rx_ni_entry: dev=%s napi_id=%#x queue_mapping=%u skbaddr=%p vlan_tagged=%d vlan_proto=0x%04x "
            "vlan_tci=0x%04x protocol=0x%04x ip_summed=%d hash=0x%08x l4_hash=%d len=%u data_len=%u truesize=%u "
            "mac_header_valid=%d mac_header=%d nr_frags=%d gso_size=%d gso_type=%#x",
            msg.name().c_str(), msg.napi_id(), msg.queue_mapping(), msg.skbaddr(), msg.vlan_tagged(), msg.vlan_proto(),
            msg.vlan_tci(), msg.protocol(), msg.ip_summed(), msg.hash(), msg.l4_hash(), msg.len(), msg.data_len(),
            msg.truesize(), msg.mac_header_valid(), msg.mac_header(), msg.nr_frags(), msg.gso_size(), msg.gso_type());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(netif_rx_ni_entry) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
