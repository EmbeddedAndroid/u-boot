.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Qualcomm Glymur CRD
===================

The Glymur CRD is the reference design for the Qualcomm Glymur (SC8480XP)
SoC. Its operating system drive is an NVMe SSD on PCIe 5 (Gen5 x4).

On this board U-Boot runs as BL33 of TF-A, with OP-TEE as BL32, instead of
the vendor UEFI firmware. TF-A loads U-Boot from its FIP and passes it no
device tree, so U-Boot uses the built-in ``qcom/glymur-crd`` device tree.

Building
--------

Configure with ``qcom_defconfig`` and the ``tfa-optee.config`` and
``glymur-crd.config`` fragments::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config glymur-crd.config
  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output -j$(nproc)

Use ``.output/u-boot.bin`` as the BL33 image when building the TF-A FIP.

Booting
-------

Only the PCIe 5 NVMe slot is supported: U-Boot has no driver for the
Gen4 x2 PHYs of PCIe 4 and PCIe 6, which fail to probe with "failed to
get pcie phy".

U-Boot brings up PCIe 5 and the NVMe drive, then boots the OS through
UEFI from the EFI system partition on the drive. The OS device tree
gets the ``/firmware/optee`` node from the Snapdragon device tree fixups.
