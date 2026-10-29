.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Qualcomm Eliza MTP
==================

The Eliza MTP is the reference design for the Qualcomm Eliza (SM7750) SoC.
It boots from UFS.

On this board U-Boot runs as BL33 of TF-A, with OP-TEE as BL32, instead of
the vendor UEFI firmware. TF-A loads U-Boot from its FIP and passes it no
device tree, so U-Boot uses the built-in ``qcom/eliza-mtp`` device tree.

Building
--------

Configure with ``qcom_defconfig`` and the ``tfa-optee.config`` fragment, and
select the device tree on the command line::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config
  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output -j$(nproc) DEVICE_TREE=qcom/eliza-mtp

Use ``.output/u-boot.bin`` as the BL33 image when building the TF-A FIP.
Add ``debug-eliza.config`` for the early debug UART on QUPv3 wrap 2 SE5.

Booting
-------

U-Boot boots the OS through UEFI from the EFI system partition on UFS.
