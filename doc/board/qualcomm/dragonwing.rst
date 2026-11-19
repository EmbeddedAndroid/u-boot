.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Balaji Selvanathan <balaji.selvanathan@oss.qualcomm.com>

Qualcomm DragonWing
========================================

Qualcomm DragonWing boards are industrial-grade platforms targeting factory
and edge applications. They span multiple product families, including
IQ6, IQ8, IQ9, and Q8, which are based on Qualcomm QCS615, QCS8300 (Monaco),
QCS9100, and QCS8550 series SoCs respectively

More information can be found on the `Qualcomm's IQ6 product page`_,
`Qualcomm's IQ8 product page`_, `Qualcomm's IQ9 product page`_ and `Qualcomm's Q8 product page`_.

.. _Qualcomm's IQ6 product page: https://docs.qualcomm.com/bundle/publicresource/87-83838-1_REV_A_Qualcomm_IQ6_Series_Product_Brief.pdf
.. _Qualcomm's IQ8 product page: https://docs.qualcomm.com/bundle/publicresource/87-83839-1_REV_A_Qualcomm_IQ8_Series_Product_Brief________.pdf
.. _Qualcomm's IQ9 product page: https://docs.qualcomm.com/bundle/publicresource/87-83840-1_REV_A_Qualcomm_IQ9_Series_Product_Brief.pdf
.. _Qualcomm's Q8 product page: https://docs.qualcomm.com/doc/87-61717-1/87-61717-1_REV_D_Qualcomm_Dragonwing_QCS8550_QCM8550_Processors_Product_Brief.pdf

Installation
------------

Build Steps
~~~~~~~~~~~

Download the U-Boot source code. See the :doc:`../../build/source` documentation
for instructions on obtaining the U-Boot source.

Go to the U-Boot source directory:

.. code-block:: bash

  $ cd u-boot

Configure the build for desired platform. This command creates the required
configuration files in the .output directory. Following gives command for each
platform (choose based on your platform):

Generate the default configuration for the IQ-615 platform:

.. code-block:: bash

  $ make O=.output qcom_qcs615_defconfig

Generate the default configuration for the IQ-8 platform:

.. code-block:: bash

  $ make O=.output qcom_monaco_defconfig

Generate the default configuration for the IQ-9 platform:

.. code-block:: bash

  $ make O=.output qcom_lemans_defconfig

Generate the default configuration for the Q8 platform:

.. code-block:: bash

  $ make O=.output qcom_sm8550_defconfig

Compile the U-Boot source using the GCC cross compile toolchain:

.. code-block:: bash

  $ make -j8 CROSS_COMPILE=<aarch64 toolchain prefix> O=.output u-boot.mbn

Flashing
~~~~~~~~

Although the board does not have secure boot set up by default,
the firmware still expects firmware ELF images to be "signed" in the MBN format.
This is handled automatically with mkmbn (see :doc:`signing` for more details).

Just flash the resulting ``u-boot.mbn`` to the ``uefi_a`` partition
on your device with ``fastboot flash uefi_a .output/u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

A tool like bkerler's `edl`_ can be used for flashing with the firehose loader (for example, for QCS9100
the firehose loader can be obtained from `dragonwing IQ9 bootbinaries`).

For example:

.. code-block:: bash

  $ edl.py --loader /path/to/prog_firehose_ddr.elf w uefi_a u-boot.mbn

Ethernet
--------

The IQ-9 EVK (``qcom/lemans-evk``) and the Arduino VENTUNO Q
(``qcom/monaco-arduino-monza``) connect EMAC0 through the SGMII SerDes to a
Qualcomm QCA8081 PHY. ``qcom_defconfig`` builds the drivers for both.
``qcom_lemans_defconfig`` builds the IQ-9 EVK devicetree.

On the VENTUNO Q, U-Boot runs as BL33 of the TF-A and OP-TEE stack. Build it
from ``qcom_defconfig`` with the ``tfa-optee.config`` fragment and select the
VENTUNO Q devicetree:

.. code-block:: bash

  $ make O=.output qcom_defconfig tfa-optee.config
  $ make -j8 CROSS_COMPILE=<aarch64 toolchain prefix> O=.output DEVICE_TREE=qcom/monaco-arduino-monza

The resulting ``.output/u-boot.bin`` is the BL33 image of the TF-A FIP.

The IQ-9 EVK reads its MAC address from the on-board EEPROM, through the
``mac-address`` nvmem cell of its devicetree. The VENTUNO Q devicetree
describes no MAC address source, so set one before using the network: a
locally administered address, unique on the network, for example:

.. code-block:: none

  => setenv ethaddr 02:00:00:00:00:01
  => dhcp

.. _edl: https://github.com/bkerler/edl
.. _dragonwing IQ9 bootbinaries: https://artifacts.codelinaro.org/ui/native/qli-ci/flashable-binaries/qimpsdk/qcs9075-rb8-core-kit
