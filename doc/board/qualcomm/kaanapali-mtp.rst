.. SPDX-License-Identifier: GPL-2.0+

Qualcomm Kaanapali MTP
======================

On the Kaanapali (SM8850) MTP, U-Boot can run as the BL33 image of an
open boot stack: TF-A BL2 loads it from the FIP and BL31 enters it at
EL2, with OP-TEE as BL32, instead of the vendor UEFI. U-Boot then uses its built-in device
tree and reads the memory map from SMEM.

Building
--------

::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_defconfig tfa-optee.config kaanapali-mtp.config
  $ make -j8

Pass the resulting ``u-boot.bin`` to the TF-A build as ``BL33``. U-Boot is
position independent, so the address TF-A loads it at does not matter.

The console is the GENI debug UART (uart7, 115200 8N1). The UFS host and
PHY drivers are enabled, for booting an EFI payload from the LUN 0 ESP
with ``bootflow scan`` or ``bootefi bootmgr``. USB is not supported yet:
there is no driver for the M31 eUSB2 PHY.
