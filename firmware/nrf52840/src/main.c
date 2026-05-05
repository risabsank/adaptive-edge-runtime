#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#define ADV_INTERVAL_MS 2000
#define COMPANY_ID_NORDIC 0x0059
#define FRAME_MAGIC_0 0xAE
#define FRAME_MAGIC_1 0x10

struct __packed event_frame {
    uint16_t sequence;
    uint8_t feature_0;
    uint8_t feature_1;
    uint8_t feature_2;
    uint8_t priority;
    uint8_t queue_depth;
};

static uint8_t advertisement_payload[2 + 2 + sizeof(struct event_frame)];
static uint16_t sequence_number;

static void update_event_frame(void)
{
    struct event_frame frame = {
        .sequence = sys_cpu_to_le16(sequence_number++),
        .feature_0 = sys_rand32_get() % 256,
        .feature_1 = sys_rand32_get() % 256,
        .feature_2 = sys_rand32_get() % 256,
        .priority = (sys_rand32_get() % 10) > 7 ? 1 : 0,
        .queue_depth = sys_rand32_get() % 10,
    };

    advertisement_payload[0] = COMPANY_ID_NORDIC & 0xFF;
    advertisement_payload[1] = (COMPANY_ID_NORDIC >> 8) & 0xFF;
    advertisement_payload[2] = FRAME_MAGIC_0;
    advertisement_payload[3] = FRAME_MAGIC_1;
    memcpy(&advertisement_payload[4], &frame, sizeof(frame));
}

void main(void)
{
    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    while (1) {
        update_event_frame();

        const struct bt_data adv[] = {
            BT_DATA(BT_DATA_FLAGS, (uint8_t[]) { BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL }, 1),
            BT_DATA(BT_DATA_MANUFACTURER_DATA, advertisement_payload, sizeof(advertisement_payload))
        };

        bt_le_adv_stop();
        err = bt_le_adv_start(BT_LE_ADV_NCONN, adv, ARRAY_SIZE(adv), NULL, 0);
        if (err) {
            printk("Advertising failed to start (err %d)\n", err);
        } else {
            const struct event_frame* frame = (const struct event_frame*)&advertisement_payload[4];
            printk(
                "advertised event seq=%u features=[%u,%u,%u] priority=%u queue=%u\n",
                sys_le16_to_cpu(frame->sequence),
                frame->feature_0,
                frame->feature_1,
                frame->feature_2,
                frame->priority,
                frame->queue_depth
            );
        }

        k_sleep(K_MSEC(ADV_INTERVAL_MS));
    }
}
