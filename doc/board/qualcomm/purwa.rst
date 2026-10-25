.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Qualcomm Purwa IoT EVK
======================

The Purwa IoT EVK is the evaluation kit for the Qualcomm IQ-X5121, based on
the X1P42100 (Purwa) SoC, a variant of X1E80100 (Hamoa) with two of its
three CPU clusters. Its operating system boots from UFS.

On this board U-Boot runs as BL33 of TF-A, with OP-TEE as BL32, instead of
the vendor UEFI firmware. TF-A loads U-Boot from its FIP and passes it no
device tree, so U-Boot is built with the ``qcom/purwa-iot-evk`` device tree.
The X1E80100 clock and pin controller drivers serve Purwa through the
X1E80100 compatibles its device tree keeps.

Building
--------

Configure with ``qcom_defconfig`` and the ``tfa-optee.config`` and
``purwa-iot-evk.config`` fragments::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config purwa-iot-evk.config
  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output -j$(nproc)

Use ``.output/u-boot.bin`` as the BL33 image when building the TF-A FIP.

Booting
-------

U-Boot boots the OS through UEFI from the EFI system partition on UFS,
with the ``/firmware/optee`` node added to the OS device tree by the OP-TEE
library.

The microSD slot (SDHC 2) probes; card access is untested.

The M.2 NVMe slot is on PCIe 6A, whose PHY
(``qcom,x1p42100-qmp-gen4x4-pcie-phy``) has no X1E80100 fallback compatible
and no U-Boot driver, so this configuration does not enable PCIe.

USB host is not supported: the X1E80100 clock driver lacks
``GCC_CFG_NOC_USB2_PRIM_AXI_CLK`` and ``GCC_CFG_NOC_USB3_MP_AXI_CLK``.
