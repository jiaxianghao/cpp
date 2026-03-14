/*
 * Copyright 2016-2024. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */

#include <tcp_network/tcp_communicator_config.h>

using namespace tcp_network;

ConnectionConfig::ConnectionConfig(const ConnectionConfig& other)
    : connect_timeout_ms(other.connect_timeout_ms)
    , read_timeout_ms(other.read_timeout_ms)
    , retry_count(other.retry_count)
    , retry_interval_ms(other.retry_interval_ms)
    , enable_keepalive(other.enable_keepalive)
    , keepalive_idle(other.keepalive_idle)
    , keepalive_interval(other.keepalive_interval)
    , keepalive_count(other.keepalive_count)
    , message_boundary(other.message_boundary)
    , heartbeat_strategy(other.heartbeat_strategy ? other.heartbeat_strategy->clone() : nullptr)
    , heartbeat_interval_ms(other.heartbeat_interval_ms)
    , heartbeat_timeout_ms(other.heartbeat_timeout_ms)
{
}

ConnectionConfig& ConnectionConfig::operator=(const ConnectionConfig& other) {
    if (this != &other) {
        connect_timeout_ms = other.connect_timeout_ms;
        read_timeout_ms = other.read_timeout_ms;
        retry_count = other.retry_count;
        retry_interval_ms = other.retry_interval_ms;
        enable_keepalive = other.enable_keepalive;
        keepalive_idle = other.keepalive_idle;
        keepalive_interval = other.keepalive_interval;
        keepalive_count = other.keepalive_count;
        message_boundary = other.message_boundary;
        heartbeat_strategy = other.heartbeat_strategy ? other.heartbeat_strategy->clone() : nullptr;
        heartbeat_interval_ms = other.heartbeat_interval_ms;
        heartbeat_timeout_ms = other.heartbeat_timeout_ms;
    }
    return *this;
}
