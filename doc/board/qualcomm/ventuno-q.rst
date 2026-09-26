.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Arduino VENTUNO Q
=================

The Arduino VENTUNO Q is based on the Qualcomm QCS8275 SoC, with four
Cortex-A78C and four Cortex-A55 cores, and boots from eMMC.

U-Boot runs as BL33 after TF-A and OP-TEE. XBL loads TF-A BL2 from the ``tz``
partition and the FIP from the ``uefi`` partition. TF-A BL2 must already be
installed as described in the `TF-A documentation`_.

.. _TF-A documentation: https://trustedfirmware-a.readthedocs.io/en/latest/plat/qti/monza.html

Boot CPU
--------

The boot firmware enters U-Boot on a Cortex-A78C. A kernel with pointer
authentication enabled cannot bring up the Cortex-A55 cores when booted on a
Cortex-A78C, so U-Boot moves to the first Cortex-A55 (MPIDR ``0x10000``) with
``CONFIG_BOOT0_QCOM_BOOT_CPU``.

Memory
------

``monaco-arduino-monza-u-boot.dtsi`` leaves memory reserved by the boot
firmware out of the RAM map.

Installation
------------

Build U-Boot with the TF-A/OP-TEE and board configuration fragments::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_defconfig tfa-optee.config arduino-ventuno-q.config
  $ make -j8

Pass ``u-boot.bin`` to the TF-A build as ``BL33`` and build ``fip.elf`` as
described in the TF-A documentation.

Put the board in EDL mode and write ``fip.elf`` to both ``uefi`` slots with
`qdl <https://github.com/linux-msm/qdl>`_ and the board's eMMC firehose
programmer::

  $ qdl --storage emmc prog_firehose_ddr.elf \
      write uefi_a fip.elf write uefi_b fip.elf
