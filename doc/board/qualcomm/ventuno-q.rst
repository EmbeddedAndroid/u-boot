.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Arduino VENTUNO Q
=================

The Arduino VENTUNO Q is based on the Qualcomm QCS8275 SoC, with four
Cortex-A78C and four Cortex-A55 cores, and boots from eMMC.

U-Boot runs as BL33 after TF-A and OP-TEE. XBL loads TF-A BL2 from the ``tz``
partition and the FIP from the ``uefi`` partition. TF-A BL2 must already be
installed as described in the `TF-A Qualcomm platform documentation`_.

.. _TF-A Qualcomm platform documentation:
   https://trustedfirmware-a.readthedocs.io/en/latest/plat/qti/index.html

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

U-Boot SPL as the TZ stage
--------------------------

U-Boot SPL can replace TF-A BL2 as the image in the ``tz`` partition. XBL
starts it at EL3 in system IMEM at ``0x14680000``, where the TZ image runs;
SPL, its BSS, stack and early malloc pool stay between ``0x14680000`` and
``0x1469a000``, the region the QTI TZ image and TF-A BL2 use.
XBL checks the QTI signature of the TZ image even with secure boot disabled,
so ``spl/u-boot-spl.elf`` has to be signed as a TZ image with QTI signing,
including the SW image version (SWIV) segment; an OEM test signature is not
accepted.

XBL also loads the image in the ``uefi`` partition to DDR at ``0xaf000000``
before it starts the TZ image. For SPL this image is a FIT with TF-A BL31,
OP-TEE and U-Boot, wrapped in an ELF loaded at ``0xaf000000``. SPL reads the
FIT from memory, so it needs no storage driver, loads the images and starts
BL31 with the OP-TEE and U-Boot entry points. SPL prints on the debug UART
that XBL leaves set up.

Build SPL with its own configuration, and U-Boot proper as above::

  $ make qcom_monaco_spl_defconfig
  $ make -j8

Build TF-A BL31 alone (``PLAT=monza SPD=opteed bl31``) and assemble the FIT
from ``bl31.bin``, OP-TEE ``tee-raw.bin`` and ``u-boot.bin`` with external data,
so SPL only copies the FIT header into its malloc pool. U-Boot must come
before OP-TEE in ``loadables``: SPL records the entry points in the device
tree it hands to BL31 only after it has loaded an image that takes one::

  /dts-v1/;

  / {
          description = "TF-A BL31, OP-TEE and U-Boot";
          #address-cells = <1>;

          images {
                  atf {
                          data = /incbin/("bl31.bin");
                          type = "firmware";
                          arch = "arm64";
                          os = "arm-trusted-firmware";
                          compression = "none";
                          load = <0x1c200000>;
                          entry = <0x1c200000>;
                  };
                  uboot {
                          data = /incbin/("u-boot.bin");
                          type = "firmware";
                          arch = "arm64";
                          os = "u-boot";
                          compression = "none";
                          load = <0xaf400000>;
                          entry = <0xaf400000>;
                  };
                  tee {
                          data = /incbin/("tee-raw.bin");
                          type = "tee";
                          arch = "arm64";
                          os = "tee";
                          compression = "none";
                          load = <0x1c300000>;
                          entry = <0x1c300000>;
                  };
          };

          configurations {
                  default = "conf";
                  conf {
                          firmware = "atf";
                          loadables = "uboot", "tee";
                  };
          };
  };

::

  $ mkimage -E -B 0x1000 -f uefi.its uefi.itb

U-Boot is loaded at ``0xaf400000`` while SPL still reads the FIT at
``0xaf000000``, so the FIT has to stay below 4 MiB. Wrap it in an ELF loaded
at ``0xaf000000`` and sign that with qtestsign using TF-A's
``tools/qti/generate_fip_elf.sh``, then flash the signed SPL and the FIT ELF
to both slots::

  $ generate_fip_elf.sh uefi.itb 0xaf000000 && mv fip.elf uefi.elf
  $ qdl --storage emmc prog_firehose_ddr.elf \
      write tz_a u-boot-spl.mbn write tz_b u-boot-spl.mbn \
      write uefi_a uefi.elf write uefi_b uefi.elf
