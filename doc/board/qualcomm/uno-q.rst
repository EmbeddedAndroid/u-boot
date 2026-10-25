.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Arduino UNO Q
=============

The Arduino UNO Q is based on the Qualcomm QRB2210 SoC, with four Cortex-A53
cores, and boots from eMMC.

U-Boot runs as BL33 after TF-A and OP-TEE. XBL loads TF-A BL2 from the ``tz``
partition and the FIP from the ``uefi`` partition. TF-A BL2 must already be
installed as described in the `TF-A Qualcomm platform documentation`_.

.. _TF-A Qualcomm platform documentation:
   https://trustedfirmware-a.readthedocs.io/en/latest/plat/qti/index.html

Installation
------------

Build U-Boot with the TF-A/OP-TEE and board configuration fragments::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_defconfig tfa-optee.config arduino-uno-q.config
  $ make -j8

Pass ``u-boot.bin`` to the TF-A build as ``BL33`` and build ``fip.elf`` as
described in the TF-A documentation.

Put the board in EDL mode and write ``fip.elf`` to the ``uefi_a`` partition
with `qdl <https://github.com/linux-msm/qdl>`_ and the board's eMMC firehose
programmer::

  $ qdl --storage emmc prog_firehose_ddr.elf write uefi_a fip.elf
