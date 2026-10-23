.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Tyler Baker <tyler.baker@oss.qualcomm.com>

Qualcomm Glymur and Mahua CRDs
==============================

The Glymur CRD is the reference design for the Qualcomm Glymur (SC8480XP)
SoC. Its operating system drive is an NVMe SSD on PCIe 5 (Gen5 x4).

Mahua is a Glymur with two of its three CPU clusters. The Mahua CRD is
built and booted as the Glymur CRD, with its own device tree
(``qcom/mahua-crd``); everything below applies to both boards.

On these boards U-Boot runs as BL33 of TF-A, with OP-TEE as BL32, instead of
the vendor UEFI firmware. TF-A loads U-Boot from its FIP and passes it no
device tree, so U-Boot uses its built-in device tree (``qcom/glymur-crd`` or
``qcom/mahua-crd``).

Building
--------

Configure with ``qcom_defconfig`` and the ``tfa-optee.config`` and
``glymur-crd.config`` fragments::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config glymur-crd.config
  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output -j$(nproc)

For the Mahua CRD, add the ``mahua-crd.config`` fragment after
``glymur-crd.config``; it only selects the Mahua CRD device tree::

  $ make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig tfa-optee.config glymur-crd.config mahua-crd.config

Use ``.output/u-boot.bin`` as the BL33 image when building the TF-A FIP.

Booting
-------

Only the PCIe 5 NVMe slot is supported: U-Boot has no driver for the
Gen4 x2 PHYs of PCIe 4 and PCIe 6, which fail to probe with "failed to
get pcie phy".

``glymur-crd.config`` selects ``board/qualcomm/glymur-crd.env`` as the
default environment. Its boot command is ``bootflow scan -lb`` with
``bootmeths`` set to ``extlinux efi_mgr``. Standard boot hunts the
bootdevs (MMC, the NVMe with PCI enumeration and namespace scan, SCSI,
USB, network), looking for an ``extlinux.conf``, and then runs the EFI
boot manager, which sees the EFI system partition on the drive. Unless an
``extlinux.conf`` is found first, ``BootNext`` and ``BootOrder`` decide
what boots. The boot manager adds a boot option for each block device it finds,
so with no options of your own it starts ``\EFI\BOOT\BOOTAA64.EFI``
from the drive, for example a unified kernel image. To boot another image
first, add an option and list it ahead of the generated one (shown by
``efidebug boot dump``, here ``Boot0000``), for example::

  => bootdev hunt nvme
  => efidebug boot add -b 1000 Linux nvme 0:1 EFI/Linux/linux.efi
  => efidebug boot order 1000 0000

``efidebug boot order`` replaces the whole list, and a generated option
that already exists is not added back, so leaving ``0000`` out removes the
fallback. The variables are kept in ``ubootefi.var`` on the EFI system
partition.

The device tree passed to the OS is the one the boot option names, else
``$fdtfile`` (``qcom/<soc>-<board>.dtb``) under one of the distro device
tree directories such as ``/dtb`` on the partition the image was loaded
from, else the one at ``$fdt_addr`` if set, else U-Boot's own. The OS
device tree gets the ``/firmware/optee`` node from the Snapdragon device
tree fixups.

There is no ``preboot``: the CRD has no UFS, and standard boot starts the
SCSI and USB bootdevs itself when the scan reaches them.
