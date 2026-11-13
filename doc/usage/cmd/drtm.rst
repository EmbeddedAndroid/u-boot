.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: drtm (command)

drtm command
============

Synopsis
--------

::

    drtm info
    drtm launch <dlme> <size> <image size> <data offset> [<entry offset> [<features>]]

Description
-----------

The drtm command drives the Arm Dynamic Root of Trust for Measurement
(DRTM, Arm DEN0113) service of the secure firmware, the Dynamic
Configuration Environment (DCE). A dynamic launch measures the DLME
(Dynamically Launched Measured Environment, for example a hypervisor) into
the TPM's D-RTM PCRs and starts it with DMA protection, independently of
everything that booted before it.

drtm info
    Print the DRTM version, the TPM, minimum memory, DMA protection, boot
    PE and DLME image authentication features the firmware reports (raw
    values as defined in DEN0113), and the error code recorded with
    DRTM_SET_ERROR, which the firmware keeps across a reset if the platform
    supports it.

drtm launch
    Start a dynamic launch of a DLME image already loaded in memory. All
    numbers are hexadecimal.

    dlme
        physical address of the DLME region, 4 KiB aligned; the image starts
        at its first byte
    size
        size of the DLME region, 4 KiB aligned
    image size
        size of the DLME image the DCE measures
    data offset
        offset of the DLME data in the region, 4 KiB aligned and above the
        image; the DCE writes the DLME data header, its event log and the
        address map there; the region from it to the end must hold at least
        the minimum memory 'drtm info' reports (in 4 KiB pages)
    entry offset
        offset of the DLME entry point in the image, 4 byte aligned, 0 by
        default
    features
        the DEN0113 launch feature bits (hashing type, PCR schema, DMA
        protection type, DLME image authentication), 0 by default; they
        must match the TPM, DMA protection and image authentication
        features 'drtm info' reports

    The launch parameters are written to the 4 KiB page just below the
    DLME region. That page and the DLME region must be free memory: not
    used by U-Boot and not reserved by the device tree.

    Arguments U-Boot can check, memory that is not free, launch features
    that do not match the firmware, and a firmware without dynamic launch
    are refused first. Then U-Boot quiesces as for an OS boot (devices
    that do DMA or need preparing for the OS removed, caches cleaned and
    disabled) and issues DRTM_DYNAMIC_LAUNCH. The DCE only accepts the
    call from the normal world, so the command refuses to run when U-Boot
    runs at EL3. The DLME starts at EL2 when EL2 is implemented, at EL1
    otherwise. On success the DLME runs and the command does not return.

    A launch the firmware refuses prints the DEN0113 return code and
    resets the board. The firmware may restrict the DLME region further
    than free memory (for example to one DDR bank); such a region is only
    refused after the quiesce. A launch the firmware stops
    after it has measured the DLME does not return either: the firmware
    records an error code, which 'drtm info' shows after the reset.

Example
-------

::

    => drtm info
    DRTM version 1.0
      TPM              0x20000000b
      minimum memory   0x11
      DMA protection   0x1
      boot PE          0x0
      image auth       0x0
      error code       0x0
    => load mmc 0:1 0xe0100000 dlme.bin
    => drtm launch e0100000 200000 1000 100000
    DRTM: launching DLME at 0xe0100000

    Starting kernel ...

Configuration
-------------

The command is available with CONFIG_CMD_DRTM=y on arm64. It needs EL3
firmware that implements the DRTM service (for TF-A, DRTM_SUPPORT=1); on a
CPU without EL3 both subcommands refuse to run.

Return value
------------

drtm info returns 0 when the firmware implements DRTM, and 1 otherwise,
without EL3 or when U-Boot runs at EL3. drtm launch only returns when it
refused the launch before the quiesce, with 1.
