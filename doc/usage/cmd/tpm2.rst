.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: tpm2 (command)

tpm2 command
============

Synopsis
--------

::

    tpm2 device [num device]
    tpm2 info
    tpm2 state
    tpm2 autostart
    tpm2 init
    tpm2 startup <mode> [off]
    tpm2 self_test <type>
    tpm2 clear <hierarchy>
    tpm2 pcr_extend <pcr> <digest_addr> [<digest_algo>]
    tpm2 pcr_read <pcr> <digest_addr> [<digest_algo>]
    tpm2 get_capability <capability> <property> <addr> <count>
    tpm2 dam_reset [<password>]
    tpm2 dam_parameters <max_tries> <recovery_time> <lockout_recovery> [<password>]
    tpm2 change_auth <hierarchy> <new_pw> [<old_pw>]
    tpm2 pcr_setauthpolicy|pcr_setauthvalue <pcr> <key> [<password>]
    tpm2 pcr_allocate <algorithm> <on|off> [<password>]

Description
-----------

The *tpm2* command issues TPM 2.0 commands to a TPM of the TPM uclass:
a TPM on SPI or I2C driven by U-Boot, an MMIO TPM, or a TPM reached
through OP-TEE (the firmware TPM TA, or a discrete TPM that OP-TEE owns,
see below). Commands that take a digest read or write it at a memory
address, in the size of the digest algorithm (SHA256 by default).

tpm2 device [num device]
    Without argument, list the TPM devices and mark the current one.
    With *num device*, select that device for the following commands.

tpm2 info
    Show the description of the current TPM.

tpm2 state
    Show the internal state of the TPM, if its driver can report it.

tpm2 autostart
    Initialize the TPM and run a full self test; if the TPM is not started
    yet, issue TPM2_Startup(TPM2_SU_CLEAR) first. A TPM that earlier
    firmware already started is used as it is.

tpm2 init
    Initialize the software stack. Only *tpm2 startup* may follow.

tpm2 startup <mode> [off]
    Issue TPM2_Startup with *mode* TPM2_SU_CLEAR or TPM2_SU_STATE, or
    TPM2_Shutdown with that mode when *off* is given.

tpm2 self_test <type>
    Issue TPM2_SelfTest: *full* tests everything, *continue* only what
    has not been tested yet.

tpm2 clear <hierarchy>
    Issue TPM2_Clear with authorization of TPM2_RH_LOCKOUT or
    TPM2_RH_PLATFORM.

tpm2 pcr_extend <pcr> <digest_addr> [<digest_algo>]
    Extend PCR *pcr* with the digest at *digest_addr*. *digest_algo* is
    one of sha1, sha256 (default), sha384, sha512.

tpm2 pcr_read <pcr> <digest_addr> [<digest_algo>]
    Read PCR *pcr* of the bank *digest_algo* (default sha256) to
    *digest_addr* and print it.

tpm2 get_capability <capability> <property> <addr> <count>
    Issue TPM2_GetCapability and print *count* 4-byte values of
    *capability* starting at *property*; they are also stored at *addr*.

tpm2 dam_reset [<password>]
    Reset the dictionary attack lockout counter.

tpm2 dam_parameters <max_tries> <recovery_time> <lockout_recovery> [<password>]
    Set the dictionary attack parameters.

tpm2 change_auth <hierarchy> <new_pw> [<old_pw>]
    Change the authorization value of a hierarchy.

tpm2 pcr_setauthpolicy|pcr_setauthvalue <pcr> <key> [<password>]
    Set the authorization policy or value of a PCR.

tpm2 pcr_allocate <algorithm> <on|off> [<password>]
    Enable or disable the PCR bank of *algorithm* (sha1, sha256, sha384,
    sha512 or sm3_256); takes effect after the next TPM reset.

TPM owned by OP-TEE
-------------------

On some platforms a discrete TPM sits on a bus that only the secure
world can reach, for instance an SPI TPM on a serial engine assigned to
OP-TEE. OP-TEE drives the chip and relays raw TPM commands through its
TPM passthrough pseudo TA; with CONFIG_TPM2_OPTEE_DTPM the OP-TEE driver
binds the *optee_dtpm* TPM device when OP-TEE enumerates that pseudo TA,
so the TPM needs no node in the U-Boot device tree. All commands above
work on it; they run at TPM locality 0. OP-TEE starts the TPM itself, so
use *tpm2 autostart* rather than *tpm2 startup*, which would fail with
TPM_RC_INITIALIZE.

With CONFIG_EFI_TCG2_PROTOCOL (the default when CONFIG_TPM_V2 is set) the
UEFI sub-system starts the TPM and measures what it boots. If the TPM
does not respond, the UEFI sub-system does not initialize and EFI boot
is not possible: a platform whose TPM may be absent must leave
EFI_TCG2_PROTOCOL disabled.

Example
-------

Read the sha256 bank of PCR 16, extend it with a digest and read it
back, on a TPM owned by OP-TEE (IQ-9075 EVK, ST33 TPM)::

    => setexpr rd ${loadaddr} + 0x100
    => tpm2 device
    device 0: OP-TEE discrete TPM
    => tpm2 autostart
    => mw.b ${loadaddr} 0x5a 0x20
    => tpm2 pcr_read 16 ${rd}
    PCR #16 sha256 32 byte content (22 known updates):
     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    => tpm2 pcr_extend 16 ${loadaddr}
    PCR #16 extended with 32 byte sha256 digest
     5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a
     5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a
    => tpm2 pcr_read 16 ${rd}
    PCR #16 sha256 32 byte content (22 known updates):
     d3 42 b8 b5 fd da bf c1 d9 4e 5c 8c 53 38 82 11
     df 37 97 91 08 9b 77 2e c0 2a 15 d9 4a dc c7 f5

The *rng* command reads random bytes from the TPM through its RNG
device: find its index with *rng list* and pass it to *rng*.

Configuration
-------------

The command is available if CONFIG_CMD_TPM=y and CONFIG_TPM_V2=y. A TPM
owned by OP-TEE needs CONFIG_TPM2_OPTEE_DTPM=y (and CONFIG_OPTEE=y).

Return value
------------

The return value $? is 0 (true) on success and 1 (false) on failure.
