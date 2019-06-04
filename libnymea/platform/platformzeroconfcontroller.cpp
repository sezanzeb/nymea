/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "platformzeroconfcontroller.h"

#include "network/zeroconf/zeroconfservicebrowser.h"
#include "network/zeroconf/zeroconfservicepublisher.h"

PlatformZeroConfController::PlatformZeroConfController(QObject *parent):
    HardwareResource("ZeroConf", parent)
{
    m_zeroConfPublisherDummy = new ZeroConfServicePublisher(this);
}

ZeroConfServiceBrowser *PlatformZeroConfController::createServiceBrowser(const QString &serviceType)
{
    return new ZeroConfServiceBrowser(serviceType, this);
}

ZeroConfServicePublisher *PlatformZeroConfController::servicePublisher() const
{
    return m_zeroConfPublisherDummy;
}

bool PlatformZeroConfController::available() const
{
    return false;
}

bool PlatformZeroConfController::enabled() const
{
    return false;
}

void PlatformZeroConfController::setEnabled(bool enabled)
{
    Q_UNUSED(enabled)
}
