namespace MARTe {

/**
 * @file wrtd-common.h
 *
 * Copyright (c) 2018-2019 CERN (home.cern)
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef __WRTD_COMMON_H
#define __WRTD_COMMON_H

/* Maximum number of CPUS per MT (allow less than the MT arch).  */
#define WRTD_MAX_CPUS 4

/* Maximum number of devices per CPU. */
#define WRTD_MAX_DEVS 4

/* Length of WRTD ID strings (not null-terminated).  */
#define  WRTD_ID_LEN 16

#define WRTD_ID_NET 0xff

/**
 * WRTD timestamp format
 */
typedef struct wrtd_tstamp {
        /** TAI seconds since 1/Jan/1970 (Unix Epoch Time).
            This will overflow in 7/Feb/2106... */
        uint32 seconds;
        /** Number of nanoseconds. Wraps at 1e9. */
        uint32 ns;
        /** Number of fractional nanoseconds. Unit is 2^(-32) ns. */
        uint32 frac;
} wrtd_tstamp;

#define WRTD_TSTAMP_FRAC_PS 0x418937

/**
 * WRTD Event
 */
struct wrtd_event {
        /** Time of the event.  */
        struct wrtd_tstamp ts;

        /** Event id.  */
        char id[WRTD_ID_LEN];

        /** Sequence number.  */
        uint32 seq;

        /** Associated flags.  */
        unsigned char flags;
};

inline static unsigned char wrtd_id_hash(const char *id)
{
        /* This assumes ID is properly padded. */
        return id[0] ^ id[3] ^ id[4] ^ id[7];
}

#define WRTD_DEST_CPU_LOCAL 0xfe
#define WRTD_DEST_CH_NET 0xff

struct wrtd_rule_config {
        /* Id of the rule.  */
        char id[WRTD_ID_LEN];

        char source_id[WRTD_ID_LEN];

        char dest_id[WRTD_ID_LEN];

        /* Device destination.  */
        uint8 dest_cpu;
        uint8 dest_ch;

        uint8 enabled;
        uint8 send_late;

        uint32 repeat_count;

        uint32 delay_ns;
        uint32 hold_off_ns;

        uint32 resync_period_ns;
        uint32 resync_factor;

        /* Next rule id for the same hash; -1 for last.  */
        int32 hash_chain;
};

struct wrtd_rule_stats {
        uint32 rx_events;
        struct wrtd_tstamp rx_last;

        uint32 tx_events;
        struct wrtd_tstamp tx_last;

        /* Latency. */
        uint32 lat_min_ns;
        uint32 lat_max_ns;
        uint32 lat_lo_ns;
        uint32 lat_hi_ns;
        uint32 lat_nbr;

        uint32 miss_holdoff;
        uint32 miss_late;
        uint32 miss_nosync;
        uint32 miss_overflow;
        struct wrtd_tstamp miss_last;

        /* Events that occur before this time are discarded.  */
        struct wrtd_tstamp hold_off;

        /* Sequence number over the network.  */
        uint32 seq;
};

struct wrtd_rule {
        struct wrtd_rule_config conf;
        struct wrtd_rule_stats  stat;
};

struct wrtd_alarm {
        /* Time when to generate the event.  */
        struct wrtd_tstamp setupime;

        /* Next time and id.  */
        struct wrtd_event event;

        uint32 enabled;
        int32 repeat_count;
        uint32 period_ns;
};

struct wrtd_message {
        unsigned char hw_detect[3]; /* LXI */
        unsigned char domain;       /* 0 */
        unsigned char event_id[WRTD_ID_LEN];
        uint32 seq;
        uint32 ts_sec;
        uint32 ts_ns;
        uint16 ts_frac;
        uint16 ts_hi_sec;
        uint8 flags;
        uint8 zero[2];
        uint8 pad[1];
};

enum wrtd_wr_link {
        WR_LINK_OFFLINE = 0,
        WR_LINK_ONLINE,
        WR_LINK_SYNCING,
        WR_LINK_WAIT,
        WR_LINK_SYNCED
};

#define WRTD_VERSION_MAJOR 1
#define WRTD_VERSION_MINOR 0

#define WRTD_CH_DIR_IN  0
#define WRTD_CH_DIR_OUT 1

#define WRTD_CAP_NET_RX   (1 << 0)
#define WRTD_CAP_NETX   (1 << 1)
#define WRTD_CAP_LOCAL_RX (1 << 2)
#define WRTD_CAP_LOCALX (1 << 3)

struct wrtd_root {
        /* Version.  */
        uint8 ver_major;
        uint8 ver_minor;
        uint8 pad0_0;
        uint8 pad0_1;

        char     fw_name[WRTD_ID_LEN];
        uint32 fw_id;

        /* Config.  */
        uint8 nbr_devices;
        uint8 nbr_alarms;
        uint8 nbr_rules;
        /* Made of WRTD_CAP_* flags. */
        uint8 capabilities;

        /* Status from firmware.  Must not be modified by the user.  */
        enum wrtd_wr_link wr_state;

        /* Status to the firmware.  Not modified by the firmware.  */
        uint8 log_flags;
        uint8 freeze_flag;
        uint8 pad3_0;
        uint8 pad3_1;

        uint8  devices_nbr_chs[WRTD_MAX_DEVS];
        /* Array of WRTD_CH_DIR_* values */
        uint8  devices_chs_dir[WRTD_MAX_DEVS];

        uint32 rules_addr;
        uint32 alarms_addr;

};

struct wrtd_config_msg {
        /* Always the first one.  */
        uint32 root_addr;
        uint32 sync_flag;
        struct wrtd_tstamp now;
};

struct wrtd_io_msg {
        uint32 addr;
        uint32 nwords;
        uint32 data[];
};

enum wrtd_trtl_actions {
        /* Always the first one, to get root and version.  */
        WRTD_ACTION_GET_CONFIG,
        WRTD_ACTION_READW,
        WRTD_ACTION_WRITEW
};

#define WRTD_ACTION_LOG 0x20

enum wrtd_log_msg_type {
        WRTD_LOG_MSG_EV_NONE      = 0,
        WRTD_LOG_MSG_EV_GENERATED = 1,
        WRTD_LOG_MSG_EV_CONSUMED  = 2,
        WRTD_LOG_MSG_EV_DISCARDED = 3,
        WRTD_LOG_MSG_EV_NETWORK   = 4
};

enum wrtd_log_reason_type {
        /* For GENERATED: */
        WRTD_LOG_GENERATED_ALARM = 1,
        WRTD_LOG_GENERATED_DEVICE = 8,
        WRTD_LOG_GENERATED_DEVICE_0 = WRTD_LOG_GENERATED_DEVICE + 0,
        WRTD_LOG_GENERATED_DEVICE_1 = WRTD_LOG_GENERATED_DEVICE + 8,
        WRTD_LOG_GENERATED_DEVICE_2 = WRTD_LOG_GENERATED_DEVICE + 16,
        WRTD_LOG_GENERATED_DEVICE_3 = WRTD_LOG_GENERATED_DEVICE + 24,
        WRTD_LOG_GENERATED_DEVICE_4 = WRTD_LOG_GENERATED_DEVICE + 32,
        WRTD_LOG_GENERATED_DEVICE_5 = WRTD_LOG_GENERATED_DEVICE + 40,
        WRTD_LOG_GENERATED_DEVICE_6 = WRTD_LOG_GENERATED_DEVICE + 48,
        WRTD_LOG_GENERATED_DEVICE_7 = WRTD_LOG_GENERATED_DEVICE + 56,

        /* For CONSUMED: */
        WRTD_LOG_CONSUMED_START = 1,
        WRTD_LOG_CONSUMED_DONE,

        /* For DISCARDED: */
        /* Not synchronized.  */
        WRTD_LOG_DISCARD_NO_SYNC = 1,
        /* Hold-off violation.  */
        WRTD_LOG_DISCARD_HOLDOFF,
        /* Timeout: event after time.  */
        WRTD_LOG_DISCARD_TIMEOUT,
        /* No space in a queue.  */
        WRTD_LOG_DISCARD_OVERFLOW,

        /* For NETWORK: */
        WRTD_LOG_NETWORK_TX = 1,
        WRTD_LOG_NETWORK_RX

};

/**
 * Log event descriptor
 */
struct wrtd_log_entry {
        enum wrtd_log_msg_type type;
        enum wrtd_log_reason_type reason;
        struct wrtd_event event;
        struct wrtd_tstamp ts;
};


/* Remote message queue for host and log.  */
#define WRTD_HMQ 0

#endif
}
