.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Qualcomm Eliza MTP and CQS EVK
==============================

The Eliza MTP is the reference design for the Qualcomm Eliza (SM7750) SoC.
It boots from UFS. The Eliza CQS EVK (CQ7790S SoM on the EVK carrier board)
boots from eMMC.

On these boards U-Boot runs as BL33 of TF-A, with OP-TEE as BL32, instead of
the vendor UEFI firmware. This needs the Eliza ports of TF-A (platform
``eliza_mtp``, used for both boards) and OP-TEE (``PLATFORM=qcom-eliza``).
TF-A loads U-Boot from its FIP and passes it no device tree, so U-Boot uses
its built-in device tree: ``qcom/eliza-mtp`` for the MTP,
``qcom/eliza-cqs-evk`` for the EVK.

Building
--------

Configure with ``qcom_defconfig`` and the ``tfa-optee.config`` fragment, and
select the device tree on the command line (``DEVICE_TREE=qcom/eliza-cqs-evk``
for the EVK)::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config
  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output -j$(nproc) DEVICE_TREE=qcom/eliza-mtp

Use ``.output/u-boot.bin`` as the BL33 image when building the TF-A FIP.
Add ``debug-eliza.config`` for the early debug UART on QUPv3 wrap 2 SE5.

Booting
-------

U-Boot boots the OS through UEFI from the EFI system partition, on UFS on
the MTP and on eMMC on the EVK. The EVK's SD card slot is not supported.
