/*
 * Copyright (C) 2014  Xiao-Long Chen <chenxiaolong@cxl.epac.to>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ramdiskpatchers/hammerhead/hammerheadramdiskpatcher.h"

#include "ramdiskpatchers/common/coreramdiskpatcher.h"
#include "ramdiskpatchers/qcom/qcomramdiskpatcher.h"


class HammerheadBaseRamdiskPatcher::Impl
{
public:
    const PatcherPaths *pp;
    const FileInfo *info;
    CpioFile *cpio;

    PatcherError error;
};


HammerheadBaseRamdiskPatcher::HammerheadBaseRamdiskPatcher(const PatcherPaths * const pp,
                                                           const FileInfo * const info,
                                                           CpioFile * const cpio) :
    m_impl(new Impl())
{
    m_impl->pp = pp;
    m_impl->info = info;
    m_impl->cpio = cpio;
}

HammerheadBaseRamdiskPatcher::~HammerheadBaseRamdiskPatcher()
{
}

PatcherError HammerheadBaseRamdiskPatcher::error() const
{
    return m_impl->error;
}

////////////////////////////////////////////////////////////////////////////////

const std::string HammerheadAOSPRamdiskPatcher::Id = "hammerhead/AOSP/AOSP";

HammerheadAOSPRamdiskPatcher::HammerheadAOSPRamdiskPatcher(const PatcherPaths *const pp,
                                                           const FileInfo *const info,
                                                           CpioFile *const cpio)
    : HammerheadBaseRamdiskPatcher(pp, info, cpio)
{
}

std::string HammerheadAOSPRamdiskPatcher::id() const
{
    return Id;
}

bool HammerheadAOSPRamdiskPatcher::patchRamdisk()
{
    CoreRamdiskPatcher corePatcher(m_impl->pp, m_impl->info, m_impl->cpio);
    QcomRamdiskPatcher qcomPatcher(m_impl->pp, m_impl->info, m_impl->cpio);

    if (!corePatcher.patchRamdisk()) {
        m_impl->error = corePatcher.error();
        return false;
    }

    if (!qcomPatcher.modifyInitRc()) {
        m_impl->error = qcomPatcher.error();
        return false;
    }

    if (!qcomPatcher.modifyFstab()) {
        m_impl->error = qcomPatcher.error();
        return false;
    }

    if (!qcomPatcher.modifyInitTargetRc("init.hammerhead.rc")) {
        m_impl->error = qcomPatcher.error();
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

const std::string HammerheadNoobdevRamdiskPatcher::Id = "hammerhead/AOSP/cxl";

HammerheadNoobdevRamdiskPatcher::HammerheadNoobdevRamdiskPatcher(const PatcherPaths *const pp,
                                                                 const FileInfo *const info,
                                                                 CpioFile *const cpio)
    : HammerheadBaseRamdiskPatcher(pp, info, cpio)
{
}

std::string HammerheadNoobdevRamdiskPatcher::id() const
{
    return Id;
}

bool HammerheadNoobdevRamdiskPatcher::patchRamdisk()
{
    CoreRamdiskPatcher corePatcher(m_impl->pp, m_impl->info, m_impl->cpio);
    QcomRamdiskPatcher qcomPatcher(m_impl->pp, m_impl->info, m_impl->cpio);

    if (!corePatcher.patchRamdisk()) {
        m_impl->error = corePatcher.error();
        return false;
    }

    // /raw-cache needs to always be mounted rw so OpenDelta can write to
    // /cache/recovery
    QcomRamdiskPatcher::FstabArgs args;
    args[QcomRamdiskPatcher::ArgForceCacheRw] = true;
    args[QcomRamdiskPatcher::ArgKeepMountPoints] = true;
    args[QcomRamdiskPatcher::ArgSystemMountPoint] = "/raw-system";
    args[QcomRamdiskPatcher::ArgCacheMountPoint] = "/raw-cache";
    args[QcomRamdiskPatcher::ArgDataMountPoint] = "/raw-data";

    if (!qcomPatcher.modifyFstab(args)) {
        m_impl->error = qcomPatcher.error();
        return false;
    }

    return true;
}
