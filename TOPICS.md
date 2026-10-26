# TOPICS: U-Boot qcom/integration

Base: main f97bb5c1ad78.
Every qcom/<platform> branch below is the pinned base plus
the LATEST revision of every in-flight upstream submission for that
platform (Gerrit patch set, GitHub PR head, lore series version) and our
not yet submitted patches unchanged (refresh 2026-10-25). This branch
merges every platform branch onto the base; each merge commit message
records its conflict resolutions, the ones redone in the refresh under
"Conflicts (refresh 2026-10-25)". Snapshot of the gated state before the
refresh: tag qcom/integration-20261024.

Added 2026-10-26: qcom/kailua (SM8550 HDK), merged on top of the gated
refresh; the state before it is tag qcom/integration-20261025.

## Platform branches

| platform | branch | on | carries | upstream status |
|---|---|---|---|---|
| glymur | qcom/glymur | base | Glymur CRD, carve-outs, boot environment | not submitted |
| mahua | qcom/mahua | glymur | Mahua CRD | not submitted |
| pakala | qcom/pakala | base | SM8750 MTP | not submitted |
| kaanapali | qcom/kaanapali | pakala | SM8850 MTP | not submitted |
| hamoa | qcom/hamoa | base | X1E80100 IoT EVK, RCG divider fix | not submitted |
| purwa | qcom/purwa | hamoa | X1P42100 IoT EVK | not submitted |
| monaco | qcom/monaco | base | Balaji's qcs8300/monaco-evk v4 (5), VENTUNO Q local v3 (v2 on lore), RCG fix, monaco-evk l8a, hwinfo and reboot-mode patches | open; VENTUNO Q v3 not posted yet |
| agatti | qcom/agatti | base | UNO Q board, OP-TEE reservation | not submitted |
| talos | qcom/talos | base | QCS615 Ride, l17a always on, qcs615 SDCC2 clock, watchdog autostart revert, qcs615 eMMC env v2 | open; our patches not submitted |
| lanai | qcom/lanai | base | SM8650 QRD | not submitted |
| kailua | qcom/kailua | lanai | nothing of its own: the Lanai eUSB2 repeater pair covers the SM8550 HDK (build: qcom_defconfig + tfa-optee.config, DEVICE_TREE=qcom/sm8550-hdk) | not submitted |
| kodiak | qcom/kodiak | base | RB3 Gen 2 fixes, shared RPMh clock driver (monaco v4 1/5) + sc7280 RPMh clocks | open; our fixes not submitted |
| lemans | qcom/lemans | base | IQ-9075 EVK fixes, watchdog revert, qcs9100 to lemans defconfig v2, QUPv3 W3 SPI clock v3, SPI-NOR node v2, snagboot v9 (8), snagboot SPI-NOR | open; our fixes not submitted |
| shikra | qcom/shikra | base | Shikra v4 (Aswin, 12) | open |
| nord | qcom/nord | base | Nord v3 (Aswin, 8) | open |

## Per-patch submission map

Each commit of a platform branch (from its parent: the base or the branch it builds on) and the upstream submission it is the latest revision of. "adapted": the patch differs from the posted revision because the pinned base or another series changed under it; the merge commit message or the commit itself says how. "not submitted": local work, carried unchanged.

### qcom/glymur (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 4c6d7b5a0ea4 | pci: pcie_dw_qcom: drop the storage class from the ops struct type | Tyler Baker | not submitted |  |
| 69f3c121f0c7 | soc: qcom: smem: skip partitions of unknown remote hosts | Tyler Baker | not submitted |  |
| 000ad18996b3 | soc: qcom: rpmh-rsc: use the v3 register layout on later RSCs | Tyler Baker | not submitted |  |
| 95d2705a0eb3 | spmi: msm: add the PMIC arbiter v8 | Tyler Baker | not submitted |  |
| 0260d78e24fb | gpio: qcom_spmi_gpio: support the LV/MV GPIO subtypes with clock | Tyler Baker | not submitted |  |
| bb2dad5133ee | gpio: qcom_spmi_gpio: add the Glymur PMIC GPIO compatibles | Tyler Baker | not submitted |  |
| d012e78639d7 | pci: pcie_dw_qcom: take PERST# and the PHY from the root port node | Tyler Baker | not submitted |  |
| 8f9f5688f492 | pci: pcie_dw_qcom: take the link width from num-lanes | Tyler Baker | not submitted |  |
| 62b44c533a48 | iommu: add a driver for an Arm SMMUv3 left in bypass | Tyler Baker | not submitted |  |
| 512a95851909 | phy: qcom: qmp-pcie: enable the optional pipediv2 clock | Tyler Baker | not submitted |  |
| 7e3e677bf99f | phy: qcom: qmp-pcie: add the Glymur Gen5 x4 PHY | Tyler Baker | not submitted |  |
| 6052a94f9eeb | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 835bc9cd1264 | clk: qcom: add the Glymur GCC and TCSR clock driver | Tyler Baker | not submitted |  |
| def5d6875271 | clk: stub: add the Glymur RPMh clock controller | Tyler Baker | not submitted |  |
| aaf856dccae2 | pinctrl: qcom: add the Glymur pinctrl driver | Tyler Baker | not submitted |  |
| 8c21dbc5e746 | power: regulator: qcom-rpmh: add the Glymur PMIC regulators | Tyler Baker | not submitted |  |
| e3508aec8b35 | nvme: wait for the controller to answer after the link comes up | Tyler Baker | not submitted |  |
| 8621a64e866a | board: qualcomm: hunt for NVMe drives before booting | Tyler Baker | not submitted |  |
| 73b4897ec8d6 | board: qualcomm: add the Glymur CRD | Tyler Baker | not submitted |  |
| e16b47a9f129 | arm64: dts: qcom: glymur: reserve the TZ carve-outs | Tyler Baker | not submitted |  |
| 56ab19a0930a | arm64: dts: qcom: glymur: reserve the boot firmware carve-outs | Tyler Baker | not submitted |  |
| b0f21f50625a | bootstd: honour the priority of the first global bootmeth | Tyler Baker | not submitted |  |
| 52016d17de9b | board: qualcomm: glymur-crd: boot from NVMe with bootstd | Tyler Baker | not submitted |  |

### qcom/mahua (on qcom/glymur)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 8b5310c460eb | pinctrl: qcom: glymur: add the Mahua TLMM compatible | Tyler Baker | not submitted |  |
| 8beaae95be4d | clk: qcom: glymur: add the Mahua TCSR compatible | Tyler Baker | not submitted |  |
| d4d04f66e1b9 | board: qualcomm: add the Mahua CRD | Tyler Baker | not submitted |  |

### qcom/pakala (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 6f839ea2bf27 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 3c9236390e1e | clk: qcom: add the SM8750 GCC and TCSR clock driver | Tyler Baker | not submitted |  |
| 17f7f712fe80 | clk: stub: add the SM8750 RPMh clock controller | Tyler Baker | not submitted |  |
| b7ba3e846451 | pinctrl: qcom: add the SM8750 pinctrl driver | Tyler Baker | not submitted |  |
| 04775c0f071c | phy: qcom: qmp-ufs: add the SM8750 UFS PHY | Tyler Baker | not submitted |  |
| 7a7a925b5e6a | mach-snapdragon: mark fused-off CPU cores in the OS device tree | Tyler Baker | not submitted |  |
| 282a59fd7871 | board: qualcomm: add the SM8750 MTP | Tyler Baker | not submitted |  |
| 8977339d8931 | dts: sm8750-mtp: reserve the DARE tag regions | Tyler Baker | not submitted |  |

### qcom/kaanapali (on qcom/pakala)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| cd6aae9f4c34 | soc: qcom: smem: skip partitions of unknown remote hosts | Tyler Baker | not submitted |  |
| 988a645c9a13 | clk: stub: add the Kaanapali RPMh clock controller | Tyler Baker | not submitted |  |
| f022b4746d75 | clk: qcom: add the Kaanapali GCC and TCSR drivers | Tyler Baker | not submitted |  |
| 9f00cfed8ee1 | pinctrl: qcom: add the Kaanapali TLMM driver | Tyler Baker | not submitted |  |
| 6ea5ca3cf2ab | board: qualcomm: add the Kaanapali MTP config fragment | Tyler Baker | not submitted |  |
| 4a13625631b8 | serial: msm-geni: skip a non-debug UART whose clock does not enable | Tyler Baker | not submitted |  |
| bd83a16b2be6 | dts: kaanapali-mtp: reserve the boot firmware carve-outs | Tyler Baker | not submitted |  |

### qcom/hamoa (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 565e179a42f3 | mach-snapdragon: mark fused-off CPU cores in the OS device tree | Tyler Baker | not submitted |  |
| 9b5427966983 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| ba3bcf3316fc | board: qualcomm: add a Hamoa IoT EVK TF-A config fragment | Tyler Baker | not submitted |  |
| e9622fc49732 | clk: stub: add the X1E80100 RPMh clock controller | Tyler Baker | not submitted |  |
| 6c9a024b0cee | clk: qcom: x1e80100: add the UFS PHY clocks | Tyler Baker | not submitted |  |
| ba41fcdb6664 | phy: qcom: qmp-ufs: enable every clock the device tree lists | Tyler Baker | not submitted |  |
| 3eaf3e052109 | clk: qcom: x1e80100: add the UFS AXI and UniPro core clock rates | Tyler Baker | not submitted |  |
| 77fc6c6cc011 | WIP: interconnect: qcom: add X1E80100 | Tyler Baker | not submitted |  |
| ebe8dfd153e6 | clk: qcom: x1e80100: add the PCIe 6A PHY aux and pipediv2 clocks | Tyler Baker | not submitted |  |
| a06c6366e71b | pci: pcie_dw_qcom: take PERST# and the PHY from the root port node | Tyler Baker | not submitted |  |
| 398df9cdf3a1 | board: qualcomm: hamoa-iot-evk: enable PCIe and NVMe | Tyler Baker | not submitted |  |
| da2670ba6dbd | clk: qcom: program the RCG divider as F() encodes it | Tyler Baker | not submitted |  |
| 71d59ae7af03 | clk: qcom: x1e80100: add the SDCC2 clocks | Tyler Baker | not submitted |  |
| 7fb5bc5d33b0 | clk: qcom: x1e80100: add the RPMh clocks | Tyler Baker | not submitted |  |
| 6430b9998047 | dts: hamoa-iot-evk: reserve the boot firmware carve-outs | Tyler Baker | not submitted |  |

### qcom/purwa (on qcom/hamoa)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 9e645226ebb5 | board: qualcomm: add the Purwa IoT EVK | Tyler Baker | not submitted |  |
| c2b080106d3c | dts: purwa-iot-evk: reserve the boot firmware carve-outs | Tyler Baker | not submitted |  |

### qcom/monaco (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 6825f01e2a99 | clk: qcom: Add shared RPMH clock driver for SM8550/SM8650/SA8775P | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-1-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| 55ed9613bbc8 | clk: qcom: qcs8300: Extend GCC clock driver to full gate/RCG coverage | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-2-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| 58bf78a510e7 | pinctrl: qcom: Add QCS8300 pinctrl driver | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-3-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| cf22e7479b11 | configs: qcom_qcs8300: Switch to monaco-evk DT and enable new drivers | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-4-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| cc9b3b1b6475 | configs: rename qcom_qcs8300_defconfig to qcom_monaco_defconfig | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-5-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| 7b53edd40081 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 2dfea17ae996 | mach-snapdragon: allow selecting the boot CPU | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-2-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 970d2ed6c75f | board: qualcomm: add Arduino VENTUNO Q support | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-3-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 22f9ce6678a2 | clk: qcom: program the RCG divider as F() encodes it | Tyler Baker | not submitted |  |
| d21480a1d2eb | dts: monaco-evk-u-boot: keep vreg_l8a always-on | Balaji Selvanathan | lore v1 <20261021-rpmh-regulator-refcount-v1-1-c9cbcc71d6a8@oss.qualcomm.com> monaco-evk-l8a |  |
| 45a3551fbea2 | arm: dts: monaco-evk: Add qcom-hwinfo node | Balaji Selvanathan | lore v1 <20261021-monaco-hwinfo-v1-1-d28858f6913b@oss.qualcomm.com> monaco-evk-hwinfo | adapted |
| 7d7ff934f727 | dts: monaco-evk-u-boot: add reboot-mode support via PON/SDAM | Balaji Selvanathan | lore v1 <20261021-monaco-reboot-mode-v1-1-41cd3c8f15d5@oss.qualcomm.com> monaco-evk-rebootmode | adapted |

### qcom/agatti (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 846965745134 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| e9f79df526d6 | board: qualcomm: add Arduino UNO Q config fragment | Tyler Baker | not submitted |  |
| cb661efeef49 | doc: board: qualcomm: document Arduino UNO Q | Tyler Baker | not submitted |  |
| b5f788ef8a0d | mach-snapdragon: of_fixup: set select-utmi-as-pipe-clk on the glue node | Tyler Baker | not submitted |  |
| 9d1d03897573 | dts: qrb2210-arduino-imola: reserve the OP-TEE memory | Tyler Baker | not submitted |  |

### qcom/talos (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 846965745134 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| c4120640396b | board: qualcomm: add QCS615 Ride config fragment | Tyler Baker | not submitted |  |
| ee5b70b15aed | doc: board: qualcomm: dragonwing: describe the QCS615 Ride TF-A build | Tyler Baker | not submitted |  |
| c1635400eebb | clk: qcom: program the RCG divider as F() encodes it | Tyler Baker | not submitted |  |
| 57c5b3581b8b | arm: dts: qcs615-ride: keep l17a always on | Tyler Baker | not submitted |  |
| 9102cbf8889a | clk: qcom: qcs615: add SDCC2 apps clock support | Balaji Selvanathan | lore v1 <20260911-pll-sdcc-talos-v1-1-403f61254a19@oss.qualcomm.com> talos-sdcc2 |  |
| 467ccee8455f | Revert "configs: qcs615/qcs9100: Enable watchdog autostart" | Aswin Murugan | lore v1 <20261008103308.707564-1-aswin.murugan@oss.qualcomm.com> wdog-revert |  |
| e0e37a1cc369 | qcom: qcs615: disable SCSI and use MMC for fastboot | Balaji Selvanathan | lore v2 <20261009-talos_mmc-v2-1-5d01c3428bee@oss.qualcomm.com> talos-emmc-env | adapted |
| 51a1d9be8b9f | qcom: qcs615: store environment in eMMC logfs partition | Balaji Selvanathan | lore v2 <20261009-talos_mmc-v2-2-5d01c3428bee@oss.qualcomm.com> talos-emmc-env |  |

### qcom/lanai (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 846965745134 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| dbc4fce4d87f | phy: qcom: add the PMIC eUSB2 repeater driver | Tyler Baker | not submitted |  |
| 021625196df8 | phy: qcom: snps-eusb2: initialize the repeater | Tyler Baker | not submitted |  |

### qcom/kodiak (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 846965745134 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 8cfc6ab96223 | mach-snapdragon: of_fixup: set select-utmi-as-pipe-clk on the glue node | Tyler Baker | not submitted |  |
| 4ee892542f16 | rtc: add a driver for the Qualcomm SPMI PMIC RTC | Tyler Baker | not submitted |  |
| 33c9027b3a88 | configs: qcom_defconfig: enable the PMIC RTC and EFI SetTime() | Tyler Baker | not submitted |  |
| e9974eee80e5 | arm: dts: qcs6490-rb3gen2: allow setting the RTC | Tyler Baker | not submitted |  |
| 2f77febebb55 | ufs: skip data cache maintenance for commands without data | Tyler Baker | not submitted |  |
| c47802d898c0 | mach-snapdragon: of_fixup: enable the APSS watchdog at EL2 | Tyler Baker | not submitted |  |
| c27b22d6e0f6 | clk: qcom: program the RCG divider as F() encodes it | Tyler Baker | not submitted |  |
| fa61754dcd39 | clk: qcom: Add shared RPMH clock driver for SM8550/SM8650/SA8775P | Balaji Selvanathan | lore v4 <20261021-b4-monaco-v4-1-c90f0c01d312@oss.qualcomm.com> monaco-balaji-v4 |  |
| c2a02c990bb4 | clk: qcom: sc7280: enable shared RPMH clock driver | Balaji Selvanathan | lore v1 <20260820-clk-qcom-sc7280-rpmh-v1-1-18100caea7f9@oss.qualcomm.com> sc7280-rpmh-clk |  |

### qcom/lemans (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 846965745134 | board: qualcomm: tfa-optee: enable the OP-TEE library | Tyler Baker | lore v2 <20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com> ventuno-v2 | posted as v2; the branch carries the unposted v3 revision |
| 6d7c574bdf3f | net: dwc_eth_qos: allow relocated MTL queue and DMA channel registers | Tyler Baker | not submitted |  |
| 5624d03ff79f | net: dwc_eth_qos: support 2.5G links | Tyler Baker | not submitted |  |
| e7d900228ed6 | phy: qcom: add the DWMAC SGMII SerDes PHY driver | Tyler Baker | not submitted |  |
| b536d68bedaa | net: phy: atheros: add QCA8081 support | Tyler Baker | not submitted |  |
| c3cdb86c75be | clk: qcom: sa8775p: add the EMAC0 clocks and GDSC | Tyler Baker | not submitted |  |
| e6ec756818ab | gpio: qcom: Support GPIOs on PMM8654AU PMIC | Tyler Baker | not submitted |  |
| 7fb03e9562f9 | misc: qcom_geni: do not dereference a QUP firmware blob not loaded yet | Tyler Baker | not submitted |  |
| a0c5875efb48 | net: dwc_eth_qos_qcom: add SA8775P SGMII and 2500BASE-X support | Tyler Baker | not submitted |  |
| ad408aa81ebf | configs: qcom: enable the SGMII SerDes and QCA8081 PHY drivers | Tyler Baker | not submitted |  |
| 42c7ffce740a | ufs: skip data cache maintenance for commands without data | Tyler Baker | not submitted |  |
| 4ce95b03e303 | rtc: add a driver for the Qualcomm SPMI PMIC RTC | Tyler Baker | not submitted |  |
| a74f2d0d5abe | configs: qcom_defconfig: enable the PMIC RTC and EFI SetTime() | Tyler Baker | not submitted |  |
| 27450e5b8923 | arm: dts: lemans-evk: allow setting the RTC | Tyler Baker | not submitted |  |
| cd03602f24de | Revert "configs: qcs615/qcs9100: Enable watchdog autostart" | Aswin Murugan | lore v1 <20261008103308.707564-1-aswin.murugan@oss.qualcomm.com> wdog-revert |  |
| c6a460c305b1 | configs: Rename qcs9100 defconfig to lemans defconfig | Aswin Murugan | lore v2 <20261009115920.2021086-1-aswin.murugan@oss.qualcomm.com> lemans-defconfig-rename |  |
| da799e67d5ed | clk: qcom: sa8775p: Add QUPv3 wrapper 3 SPI clock support | Vandhiadevan Karunamoorthy | lore v3 <20261016-b4-sa8775p-qup3-spi-clk-v3-1-7b9867dbf65c@oss.qualcomm.com> sa8775p-qup3-spi |  |
| e688169885cf | arm: dts: lemans-evk: Add SPI-NOR flash node | Vandhiadevan Karunamoorthy | lore v2 <20261016-lemans-spi-nor-dts-v2-1-70307aab80c5@oss.qualcomm.com> lemans-spinor-dts | adapted |
| 9e6037c6372a | arm: snapdragon: Make SYSRESET_PSCI optional | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-1-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| f8f91d0aea67 | arm: snapdragon: Guard PSCI functions with CONFIG_ARM_SMCCC | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-2-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| 7f16eb2fc170 | arm: snapdragon: Disable MMU early before U-Boot reset vector | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-3-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| 1154654f64de | arm: snapdragon: Add CONFIG_QCOM_SNAGBOOT_MODE option | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-4-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| 8939f10478d8 | soc: qcom: cmd-db: Allow boot without CMD DB data | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-5-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| b8bed09da01e | arm: dts: lemans-evk: drop the memory node | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-6-18ce3e431320@oss.qualcomm.com> lemans-snagboot | adapted |
| 31b32af62dcc | configs: add qcom_lemans_snagboot_defconfig for Lemans-EVK | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-7-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| e97ffb0536f4 | doc: qualcomm: Add snagboot mode documentation | Balaji Selvanathan | lore v9 <20261021-snagboot-v9-8-18ce3e431320@oss.qualcomm.com> lemans-snagboot |  |
| 7616b28dde85 | arm: mach-snapdragon: enable SPI-NOR in Snagboot mode | Vandhiadevan Karunamoorthy | lore v1 <20261006-review-lemans-spinor-fix-v1-1-f61b32c1a79b@oss.qualcomm.com> snagboot-spinor | adapted |

### qcom/shikra (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| a2502c8dc51b | clk: qcom: add GCC clock driver support for shikra SoC | Aswin Murugan | lore v4 <20261021160916.3894471-2-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| a9e1e7065011 | pinctrl: qcom: add driver for shikra SoC | Aswin Murugan | lore v4 <20261021160916.3894471-3-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 70069a72e3b7 | qcom_defconfig: Enable Shikra clock driver | Aswin Murugan | lore v4 <20261021160916.3894471-4-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 82dc59cdbe7d | phy: qcom: qusb2: add Shikra PHY configuration | Aswin Murugan | lore v4 <20261021160916.3894471-5-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| b7a407d2c3bf | phy: qcom: add QMP USB3 PHY driver for Shikra | Aswin Murugan | lore v4 <20261021160916.3894471-6-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 1bb14e823e9f | configs: qcom: enable PHY_QCOM_QMP_USB3_LEGACY | Aswin Murugan | lore v4 <20261021160916.3894471-7-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 6962f10a81e8 | configs: qcom: add debug UART config fragment for shikra-evk | Aswin Murugan | lore v4 <20261021160916.3894471-8-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| b0dcb1f4db3a | tools: qcom: mkmbn: add Shikra board mappings | Aswin Murugan | lore v4 <20261021160916.3894471-9-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 9e411050eb45 | configs: add shikra defconfig for IQS board | Aswin Murugan | lore v4 <20261021160916.3894471-10-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 6d2f34ad5a73 | qcom: smem: increase SMEM_HOST_COUNT for newer SMEM versions | Aswin Murugan | lore v4 <20261021160916.3894471-11-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| c91b18d1ccd5 | board: qualcomm: add MAINTAINERS entries for Shikra platforms | Aswin Murugan | lore v4 <20261021160916.3894471-12-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |
| 3f3b02b31609 | doc: board: qualcomm: document Shikra IQS (IQ2) | Aswin Murugan | lore v4 <20261021160916.3894471-13-aswin.murugan@oss.qualcomm.com> shikra-v4 |  |

### qcom/nord (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| f33d2fec6083 | clk: qcom: add GCC driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-2-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| e6ccd8ad5222 | clk: qcom: add NEGCC driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-3-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| f351450ec6ff | clk: qcom: add SEGCC driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-4-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| 5465f059d8cc | clk: qcom: add NWGCC driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-5-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| b1fd63f9085a | clk: qcom: add TCSRCC driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-6-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| 7023117d58f9 | configs: qcom_defconfig: enable nord clock drivers | Aswin Murugan | lore v3 <20261021160448.3893858-7-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| b494c6ee0e04 | pinctrl: qcom: add TLMM driver for nord | Aswin Murugan | lore v3 <20261021160448.3893858-8-aswin.murugan@oss.qualcomm.com> nord-v3 |  |
| c35dc976b5c2 | phy: qcom: snps-eusb2: add Nord compatible | Aswin Murugan | lore v3 <20261021160448.3893858-9-aswin.murugan@oss.qualcomm.com> nord-v3 |  |


## In-flight submissions not carried

- Lemans SPL v4 and its generic prerequisites (ipq5210 SPL architecture v10, spl-qcom-phase, spl-ufs v4, NVMEM v5): the gated Lemans flow keeps the lab SPL.
- Shikra SPL series (eMMC SPL v2, SD-card SPL v2, CDT v2, DDR training v1): SPL replaces XBL; same SPL architecture dependency.
- Gunyah exit on QCS615/QCM6490/QCS8300: needs exit-gunyah.config from the unmerged EL2 switch v5 series; stock boot only.
- Generic mach-snapdragon series (boot time v3, EL2 switch v5, SKIP_EARLY_DM, smem available_length v2, block cache, NVMEM v5, HW-Info v5, HS200/HS400 v2, PS_HOLD PON v2, SYS_MALLOC_LEN v2, capsule v2, SCSI scan v2, GENI SPI v6, qmp-combo orientation v2 and others): not platform submissions.

## LAVA results

Build: one fresh firmware fingerprint per build (QCDFP-<date>-<n>), read
back from BL2, BL31, OP-TEE and U-Boot on the cold boot and on the second
boot where the board comes back (Lemans: SPL instead of BL2, Shikra: U-Boot
only). Suite: the OSS suite minus ACS, compared case by case with the
earlier integration gate and the platform baseline. Every job ends on the
lab stock firmware (in-job restore block, or a pinned restore job where the
job cannot reach it). Jobs: https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | stack | build | job | result vs baseline |
|---|---|---|---|---|
| pakala | internal | QCDFP-261025-93 | 512788 | equal to 511913 (earlier gate) and 507296 (baseline) |
| hamoa | internal | QCDFP-261025-94 | 512789 | equal to 511915 and 505458 |
| purwa | internal | QCDFP-261025-95 | 512790 | equal to 511910 and 507756 (9 fails, as stock) |
| glymur | internal | QCDFP-261025-96 | 512791 | equal to 511917 and 510585 |
| mahua | internal | QCDFP-261025-97 | 512792 | equal to 511918 and 510593 |
| kaanapali | internal | QCDFP-261025-98 | 512810 | equal to 511465 and 505311 (first run 512793 lost the second boot to a one-off PMIC over-current reset of the board, not repeated) |
| kodiak | internal | QCDFP-261025-81 | 512596 | equal to 511914 except the known big-core cpufreq runtime-ratio flake |
| kodiak | public | QCDFP-261025-83 | 512685 | equal to 502713 except policy7-min fail -> pass |
| agatti | internal | QCDFP-261025-85 | 512739 | equal to 511909 and 490882 |
| agatti | public | QCDFP-261025-86 | 512743 | equal to 511900 and 490883 |
| monaco | internal | QCDFP-261025-87 | 512745 | equal to 511890 row for row; warm reboot still hangs (SAIL acknowledge, open); backstop 512746 restored stock |
| monaco | public | QCDFP-261025-89 | 512751 | equal to 511893; same warm reboot gap; backstop 512754 |
| lemans | internal | QCDFP-261025-82 | 512684 | equal to 511911 except the fastrpc unsigned-PD flake; memory map now from SMEM; SPL not gated |
| lemans | public | QCDFP-261025-84 | 512687 | equal to 511912 except the same flake |
| talos | internal | QCDFP-261025-90 | 512757 | equal to 511898 except the known KVM PMU timing race (now passing) and the OSM cpufreq checks (not carried, skip) |
| talos | public | QCDFP-261025-92 | 512759 | equal to 511901 except the known KVM PMU timing race (now passing) and the OSM cpufreq checks (not carried, skip) |
| shikra | public | QCDFP-261025-91 | 512770 | U-Boot only (Shikra v4) on the stock XBL/QSEE/Gunyah health image: FP, Linux, login and the health smoke tests pass as health job 511579; no upstream TF-A/OP-TEE boot path yet |
| lanai | internal |  |  | untested: no LAVA board |
| lanai | public |  |  | untested: no LAVA board |
| kailua | public |  |  | untested: no board yet (lab access pending) |
| nord | internal |  |  | untested: no LAVA board |
