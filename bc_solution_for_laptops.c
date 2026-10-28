// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/skbuff.h>

#include <uapi/linux/virtio_ids.h>
#include <uapi/linux/virtio_bt.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#define VERSION "0.1"
#define VIRTBT_RX_BUF_SIZE 1000

enum {
	VIRTBT_VQ_TX,
	VIRTBT_VQ_RX,
	VIRTBT_NUM_VQS,
};

struct virtio_bluetooth {
	struct virtio_device *vdev;
	struct virtqueue *vqs[VIRTBT_NUM_VQS];
	struct work_struct rx;
	struct hci_dev *hdev;
};

static int virtbt_add_inbuf(struct virtio_bluetooth *vbt)
{
	struct virtqueue *vq = vbt->vqs[VIRTBT_VQ_RX];
	struct scatterlist sg[1];
	struct sk_buff *skb;
	int err;

	skb = alloc_skb(VIRTBT_RX_BUF_SIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	sg_init_one(sg, skb->data, VIRTBT_RX_BUF_SIZE);

	err = virtqueue_add_inbuf(vq, sg, 1, skb, GFP_KERNEL);
	if (err < 0) {
		kfree_skb(skb);
		return err;
	}

	return 0;
}

static int virtbt_open(struct hci_dev *hdev)
{
	return 0;
}

static int virtbt_open_vdev(struct virtio_bluetooth *vbt)
{
	if (virtbt_add_inbuf(vbt) < 0)
		return -EIO;

	virtqueue_kick(vbt->vqs[VIRTBT_VQ_RX]);
	return 0;
}

static int virtbt_close(struct hci_dev *hdev)
{
	return 0;
}

static int virtbt_close_vdev(struct virtio_bluetooth *vbt)
{
	int i;

	cancel_work_sync(&vbt->rx);

	for (i = 0; i < ARRAY_SIZE(vbt->vqs); i++) {
		struct virtqueue *vq = vbt->vqs[i];
		struct sk_buff *skb;

		while ((skb = virtqueue_detach_unused_buf(vq)))
			kfree_skb(skb);
		cond_resched();
	}

	return 0;
}

static int virtbt_flush(struct hci_dev *hdev)
