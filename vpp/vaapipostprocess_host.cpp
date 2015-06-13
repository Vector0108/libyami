/*
 *  vaapidecoder_host.cpp - create specific type of video decoder
 *
 *  Copyright (C) 2013 Intel Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common/log.h"
#include "interface/VideoPostProcessHost.h"
#include "vaapipostprocess_factory.h"

using namespace YamiMediaCodec;

extern "C" {

IVideoPostProcess *createVideoPostProcess(const char *mimeType)
{
    yamiTraceInit();

    if (!mimeType) {
        ERROR("NULL mime type.");
        return NULL;
    }

    VaapiPostProcessFactory::Type vpp =
        VaapiPostProcessFactory::create(mimeType);

    if (!vpp)
        ERROR("Failed to create vpp for mimeType: '%s'", mimeType);
    else
        INFO("Created vpp for mimeType: '%s'", mimeType);

    return vpp;
}

void releaseVideoPostProcess(IVideoPostProcess * p)
{
    delete p;
}

} // extern "C"
