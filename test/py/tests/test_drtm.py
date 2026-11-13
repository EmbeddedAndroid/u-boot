# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

"""
Test the drtm command (Arm DRTM, DEN0113). The tests that call the firmware
need the DRTM service, so they only run on boards that declare it:

env__drtm = {
    'present': True,
}

The argument and memory checks of drtm launch run on any board with the
command; a successful launch is not tested here, it does not return to
U-Boot.
"""

import re
import pytest

def drtm_env(ubman):
    env = ubman.config.env.get('env__drtm')
    if not env or not env.get('present'):
        pytest.skip('DRTM firmware not declared in env__drtm')
    return env

@pytest.mark.buildconfigspec('cmd_drtm')
def test_drtm_info(ubman):
    """The firmware answers DRTM_VERSION and reports a TPM and DMA protection"""
    drtm_env(ubman)
    out = ubman.run_command('drtm info')
    assert 'DRTM version 1.' in out
    assert 'TPM              not supported' not in out
    assert 'DMA protection   not supported' not in out

@pytest.mark.buildconfigspec('cmd_drtm')
def test_drtm_info_no_firmware(ubman):
    """Without DRTM firmware drtm info fails cleanly instead of aborting"""
    if ubman.config.env.get('env__drtm', {}).get('present'):
        pytest.skip('DRTM firmware declared in env__drtm')
    out = ubman.run_command('drtm info')
    if 'DRTM version' in out:
        pytest.skip('DRTM firmware present, env__drtm not declared')
    assert out.startswith('DRTM: ')
    assert ubman.run_command('echo $?') == '1'

@pytest.mark.buildconfigspec('cmd_drtm')
@pytest.mark.parametrize('args', [
    'e0100800 200000 1000 100000',      # DLME region not 4 KiB aligned
    'e0100000 200000 101000 100000',    # image overlaps the DLME data
    'e0100000 200000 1000 200000',      # DLME data outside the region
    'e0100000 200000 1000 100000 2000', # entry point past the image
    'e0100000 200000 1000 100000 2',    # entry point not aligned
])
def test_drtm_launch_refused(ubman, args):
    """A launch U-Boot can tell is invalid is refused before quiescing"""
    out = ubman.run_command('drtm launch ' + args)
    assert 'invalid DLME region' in out
    assert ubman.run_command('echo $?') == '1'

@pytest.mark.buildconfigspec('cmd_drtm')
def test_drtm_launch_refused_reserved(ubman):
    """A DLME region over U-Boot itself is refused before quiescing"""
    out = ubman.run_command('bdinfo')
    reloc = int(re.search(r'relocaddr\s*=\s*(0x[0-9a-f]+)', out).group(1), 16)
    out = ubman.run_command('drtm launch %x 200000 1000 100000' %
                            (reloc & ~0xfff))
    assert 'not free memory' in out
    assert ubman.run_command('echo $?') == '1'

@pytest.mark.buildconfigspec('cmd_drtm')
def test_drtm_launch_refused_features(ubman):
    """Launch features the firmware would refuse are refused before quiescing"""
    drtm_env(ubman)
    # PCR schema 1 (DLME authorities) without DLME image authentication
    out = ubman.run_command('drtm launch ${loadaddr} 200000 1000 100000 0 2')
    assert 'features 0x2 refused' in out
    assert ubman.run_command('echo $?') == '1'
