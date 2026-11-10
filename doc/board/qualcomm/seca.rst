.. SPDX-License-Identifier: GPL-2.0+

SecaAU Ride SX
==============

The SecaAU is a Wildcat family automotive SoC closely related to Nord: one
fewer CPU clusters (two clusters of six Oryon cores), two DDR slices and
two UFS controllers. The Ride SX boots XBL, then TF-A BL2/BL31 and OP-TEE,
and U-Boot runs as BL33 at EL2, loaded by BL2 from the FIP in the uefi
partition.

Devicetree
----------

Until the Linux SecaAU devicetree lands in dts/upstream, the board uses a
local devicetree (``arch/arm/dts/seca-ride-sx.dts`` and ``seca.dtsi``)
that describes what U-Boot drives: the debug UART (QUP WRAP2 SE1), TLMM,
the NE_GCC, NW_GCC and TCSR clock controllers, UFS_0 with its PHY, USB0
in peripheral mode, the volume-down key and the reserved memory map.
The /memory node is empty; U-Boot reads the DDR map from the SMEM RAM
partition table written by XBL.

Building
--------

.. code-block:: bash

    make qcom_defconfig tfa-optee.config seca-ride-sx.config
    make CROSS_COMPILE=aarch64-linux-gnu-

``u-boot.bin`` (with the devicetree appended) is the BL33 image passed to
TF-A's fiptool as ``--nt-fw``.

Fastboot
--------

Holding volume down (TLMM GPIO 136, active high) while U-Boot starts runs
``fastboot usb 0``. ``fastboot flash`` writes partitions on UFS LUN 0
through the block backend.
