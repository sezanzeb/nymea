/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include <QSignalSpy>

class TestDevices : public GuhTestBase
{
    Q_OBJECT

private slots:
    void getPlugins();

    void getPluginConfig_data();
    void getPluginConfig();

    void setPluginConfig_data();
    void setPluginConfig();

    void getSupportedVendors();

    void getSupportedDevices_data();
    void getSupportedDevices();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void getConfiguredDevices();

    void storedDevices();

    void discoverDevices_data();
    void discoverDevices();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void getStateValue_data();
    void getStateValue();

    void getStateValues_data();
    void getStateValues();

    void editDevices_data();
    void editDevices();

    void editByDiscovery_data();
    void editByDiscovery();

    // Keep this the last one! It'll remove the configured mock device
    void removeDevice_data();
    void removeDevice();

};

void TestDevices::getPlugins()
{
    QVariant response = injectAndWait("Devices.GetPlugins");

    QVariantList plugins = response.toMap().value("params").toMap().value("plugins").toList();

    QCOMPARE(plugins.count() > 0, true);
    bool found = false;
    foreach (const QVariant &listEntry, plugins) {
        if (PluginId(listEntry.toMap().value("id").toString()) == mockPluginId) {
            found = true;
        }
    }
    QCOMPARE(found, true);
}

void TestDevices::getPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid plugin") << mockPluginId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << DeviceManager::DeviceErrorPluginNotFound;
}

void TestDevices::getPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);
    QVariant response = injectAndWait("Devices.GetPluginConfiguration", params);
    verifyDeviceError(response, error);
}

void TestDevices::setPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid") << mockPluginId << QVariant(13) << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << QVariant(13) <<  DeviceManager::DeviceErrorPluginNotFound;
    QTest::newRow("too big") << mockPluginId << QVariant(130) << DeviceManager::DeviceErrorInvalidParameter;
    QTest::newRow("too small") << mockPluginId << QVariant(-13) << DeviceManager::DeviceErrorInvalidParameter;
    QTest::newRow("wrong type") << mockPluginId << QVariant("wrontType") << DeviceManager::DeviceErrorInvalidParameter;
}

void TestDevices::setPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariant, value);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);

    QVariantList configuration;
    QVariantMap configParam;
    configParam.insert("name", "configParamInt");
    configParam.insert("value", value);
    configuration.append(configParam);
    params.insert("configuration", configuration);
    QVariant response = injectAndWait("Devices.SetPluginConfiguration", params);
    verifyDeviceError(response, error);

    if (error == DeviceManager::DeviceErrorNoError) {
        params.clear();
        params.insert("pluginId", pluginId);
        response = injectAndWait("Devices.GetPluginConfiguration", params);
        verifyDeviceError(response);
        qDebug() << "222" << response.toMap().value("params").toMap().value("configuration").toList().first();
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("name") == "configParamInt", "Value not set correctly");
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("value") == value, "Value not set correctly");
    }
}

void TestDevices::getSupportedVendors()
{
    QVariant supportedVendors = injectAndWait("Devices.GetSupportedVendors");
    qDebug() << "response" << supportedVendors;

    // Make sure there is exactly 1 supported Vendor named "guh"
    QVariantList vendorList = supportedVendors.toMap().value("params").toMap().value("vendors").toList();
    QCOMPARE(vendorList.count() > 0, true);
    bool found = false;
    foreach (const QVariant &listEntry, vendorList) {
        if (VendorId(listEntry.toMap().value("id").toString()) == guhVendorId) {
            found = true;
        }
    }
    QCOMPARE(found, true);
}

void TestDevices::getSupportedDevices_data()
{
    QTest::addColumn<VendorId>("vendorId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("vendor guh") << guhVendorId << 1;
    QTest::newRow("no filter") << VendorId() << 1;
    QTest::newRow("invalid vendor") << VendorId("93e7d361-8025-4354-b17e-b68406c800bc") << 0;
}

void TestDevices::getSupportedDevices()
{
    QFETCH(VendorId, vendorId);
    QFETCH(int, resultCount);

    QVariantMap params;
    if (!vendorId.isNull()) {
        params.insert("vendorId", vendorId);
    }
    QVariant result = injectAndWait("Devices.GetSupportedDevices", params);

    QVariantList supportedDevices = result.toMap().value("params").toMap().value("deviceClasses").toList();
    // Make sure there are the right amount of supported device classes with the name Mock Device
    QCOMPARE(supportedDevices.count() >= resultCount, true);
    if (resultCount > 0) {
        bool found = false;
        foreach (const QVariant &listEntry, supportedDevices) {
            if (listEntry.toMap().value("name").toString().startsWith("Mock Device")) {
                found = true;
            }
        }
        QVERIFY2(found, "Mock Device not found");
    }
}

void TestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", m_mockDevice1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("name", "async");
    asyncParam.insert("value", true);
    QVariantMap brokenParam;
    brokenParam.insert("name", "broken");
    brokenParam.insert("value", true);

    QVariantList deviceParams;

    deviceParams.clear(); deviceParams << httpportParam;
    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorNoError;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam;
    QTest::newRow("User, JustAdd, Async") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("Invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << deviceParams << DeviceManager::DeviceErrorDeviceClassNotFound;
    deviceParams.clear(); deviceParams << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorSetupFailed;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorSetupFailed;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorMissingParameter;

    QVariantMap fakeparam;
    fakeparam.insert("name", "tropptth");
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorInvalidParameter;

    fakeparam.insert("value", "buhuu");
    invalidDeviceParams.clear();
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, wrong param") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorInvalidParameter;

}

void TestDevices::addConfiguredDevice()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(QVariantList, deviceParams);
    QFETCH(DeviceManager::DeviceError, deviceError);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    qDebug() << "response is" << response;

    verifyDeviceError(response, deviceError);

    if (deviceError == DeviceManager::DeviceErrorNoError) {
        QUuid deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::getConfiguredDevices()
{
    QVariant response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    QCOMPARE(devices.count(), 2); // There should be one auto created mock device and the one created in initTestcase()
}

void TestDevices::storedDevices()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariantList deviceParams;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Blub Blub device");
    deviceParams.append(nameParam);
    QVariantMap asyncParam;
    asyncParam.insert("name", "async");
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("name", "broken");
    brokenParam.insert("value", false);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", 8888);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);
    DeviceId addedDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!addedDeviceId.isNull());

    // Restart the core instance to check if settings are loaded at startup
    restartServer();

    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    bool found = false;
    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        if (DeviceId(device.toMap().value("id").toString()) == addedDeviceId) {
            qDebug() << "found added device" << device.toMap().value("params");
            qDebug() << "expected deviceParams:" << deviceParams;
            verifyParams(deviceParams, device.toMap().value("params").toList());
            found = true;
            break;
        }
    }
    QVERIFY2(found, "Device missing in config!");


    params.clear();
    params.insert("deviceId", addedDeviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}

void TestDevices::discoverDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("name", "resultCount");
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid deviceClassId") << mockDeviceClassId << 2 << DeviceManager::DeviceErrorNoError << QVariantList();
    QTest::newRow("valid deviceClassId with params") << mockDeviceClassId << 1 << DeviceManager::DeviceErrorNoError << discoveryParams;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << DeviceManager::DeviceErrorDeviceClassNotFound << QVariantList();
}

void TestDevices::discoverDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // If we found something, lets try to add it
    if (DeviceManager::DeviceErrorNoError) {
        DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());

        params.clear();
        params.insert("deviceClassId", deviceClassId);
        params.insert("deviceDescriptorId", descriptorId.toString());
        response = injectAndWait("Devices.AddConfiguredDevice", params);

        verifyDeviceError(response);

        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::getActionTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 5;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getActionTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetActionTypes", params);

    QVariantList actionTypes = response.toMap().value("params").toMap().value("actionTypes").toList();
    QCOMPARE(actionTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(actionTypes.first().toMap().value("id").toString(), mockActionIdWithParams.toString());
    }
}

void TestDevices::getEventTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 4;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getEventTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetEventTypes", params);

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(eventTypes.first().toMap().value("id").toString(), mockEvent1Id.toString());
    }
}

void TestDevices::getStateTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 2;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getStateTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetStateTypes", params);

    QVariantList stateTypes = response.toMap().value("params").toMap().value("stateTypes").toList();
    QCOMPARE(stateTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(stateTypes.first().toMap().value("id").toString(), mockIntStateId.toString());
    }
}

void TestDevices::getStateValue_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<DeviceManager::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << m_mockDeviceId << mockIntStateId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << mockIntStateId << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetypeId") << m_mockDeviceId << StateTypeId("120514f1-343e-4621-9bff-dac616169df9") << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestDevices::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(DeviceManager::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Devices.GetStateValue", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), JsonTypes::deviceErrorToString(statusCode));
    if (statusCode == DeviceManager::DeviceErrorNoError) {
        QVariant value = response.toMap().value("params").toMap().value("value");
        QCOMPARE(value.toInt(), 10); // Mock device has value 10 by default...
    }
}

void TestDevices::getStateValues_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<DeviceManager::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << m_mockDeviceId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << DeviceManager::DeviceErrorDeviceNotFound;
}

void TestDevices::getStateValues()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(DeviceManager::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    QVariant response = injectAndWait("Devices.GetStateValues", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), JsonTypes::deviceErrorToString(statusCode));
    if (statusCode == DeviceManager::DeviceErrorNoError) {
        QVariantList values = response.toMap().value("params").toMap().value("values").toList();
        QCOMPARE(values.count(), 2); // Mock device has two states...
    }
}

void TestDevices::editDevices_data()
{
    QVariantList asyncChangeDeviceParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("name", "async");
    asyncParamDifferent.insert("value", true);
    asyncChangeDeviceParams.append(asyncParamDifferent);

    QVariantList httpportChangeDeviceParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("name", "httpport");
    httpportParamDifferent.insert("value", 666);
    httpportChangeDeviceParams.append(httpportParamDifferent);

    QVariantList brokenChangedDeviceParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("name", "broken");
    brokenParamDifferent.insert("value", true);
    brokenChangedDeviceParams.append(brokenParamDifferent);

    QVariantList nameChangedDeviceParams;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Awesome Mockdevice");
    nameChangedDeviceParams.append(nameParam);


    QVariantList asyncAndPortChangeDeviceParams;
    asyncAndPortChangeDeviceParams.append(asyncParamDifferent);
    asyncAndPortChangeDeviceParams.append(httpportParamDifferent);


    QVariantList changeAllEditableDeviceParams;
    changeAllEditableDeviceParams.append(nameParam);
    changeAllEditableDeviceParams.append(asyncParamDifferent);
    changeAllEditableDeviceParams.append(httpportParamDifferent);
    //changeAllEditableDeviceParams.append(brokenParamDifferent);


    QTest::addColumn<bool>("broken");
    QTest::addColumn<QVariantList>("newDeviceParams");
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QTest::newRow("valid - change async param") << false << asyncChangeDeviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeDeviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid - change httpport and async param") << false << asyncAndPortChangeDeviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid - change name param (not editable)") << false << nameChangedDeviceParams << DeviceManager::DeviceErrorParameterNotEditable;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllEditableDeviceParams << DeviceManager::DeviceErrorParameterNotEditable;
}

void TestDevices::editDevices()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newDeviceParams);
    QFETCH(DeviceManager::DeviceError, deviceError);

    // add device
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariantList deviceParams;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Test edit mockdevice");
    deviceParams.append(nameParam);
    QVariantMap asyncParam;
    asyncParam.insert("name", "async");
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("name", "broken");
    brokenParam.insert("value", broken);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", 8890);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);

    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // now EDIT the added device
    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    // edit the added and verified device
    QVariantMap editParams;
    editParams.insert("deviceId", deviceId);
    editParams.insert("deviceParams", newDeviceParams);

    response.clear();
    response = injectAndWait("Devices.EditDevice", editParams);
    verifyDeviceError(response, deviceError);

    // if the edit should have been successfull
    if (deviceError == DeviceManager::DeviceErrorNoError) {

        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        bool found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the edit was ok
                verifyParams(newDeviceParams, device.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the edit was ok
                verifyParams(newDeviceParams, device.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // delete it
        params.clear();
        params.insert("deviceId", deviceId);
        response.clear();
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
        return;
    } else {
        // The edit was not ok, check if the old params are still there
        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        bool found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the params are unchanged
                verifyParams(deviceParams, device.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if after the reboot the settings are unchanged
                verifyParams(deviceParams, device.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");
    }

    // delete it
    params.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}


void TestDevices::editByDiscovery_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("name", "resultCount");
    resultCountParam.insert("value", 2);
    discoveryParams.append(resultCountParam);

    QTest::newRow("discover 2 devices with params") << mockDeviceClassId << 2 << DeviceManager::DeviceErrorNoError << discoveryParams;
}

void TestDevices::editByDiscovery()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // add Discovered Device 1 port 55555
    QVariantList deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();

    DeviceDescriptorId descriptorId1;
    foreach (const QVariant &descriptor, deviceDescriptors) {
        // find the device with port 55555
        if (descriptor.toMap().value("description").toString() == "55555") {
            descriptorId1 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            qDebug() << descriptorId1.toString();
            break;
        }
    }

    qDebug() << "adding descriptorId 1" << descriptorId1;

    QVERIFY(!descriptorId1.isNull());

    params.clear();
    response.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceDescriptorId", descriptorId1);
    response = injectAndWait("Devices.AddConfiguredDevice", params);

    DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // and now rediscover, and edit the first device with the second
    params.clear();
    response.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // get the second device
    DeviceDescriptorId descriptorId2;
    foreach (const QVariant &descriptor, deviceDescriptors) {
        // find the device with port 55556
        if (descriptor.toMap().value("description").toString() == "55556") {
            descriptorId2 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            break;
        }
    }
    QVERIFY(!descriptorId2.isNull());

    qDebug() << "edit device 1 (55555) with descriptor 2 (55556) " << descriptorId2;

    // EDIT
    response.clear();
    params.clear();
    params.insert("deviceId", deviceId.toString());
    params.insert("deviceDescriptorId", descriptorId2);
    response = injectAndWait("Devices.EditDevice", params);
    verifyDeviceError(response, error);

    response.clear();
    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    QVariantMap deviceMap;
    bool found = false;
    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
            qDebug() << "found added device" << device.toMap().value("params");
            found = true;
            deviceMap = device.toMap();
            break;
        }
    }

    printJson(deviceMap);

    QVERIFY2(found, "Device missing in config!");
    QCOMPARE(deviceMap.value("id").toString(), deviceId.toString());
    if (deviceMap.contains("setupComplete")) {
        QVERIFY2(deviceMap.value("setupComplete").toBool(), "Setup not completed after edit");
    }

    // Note: this shows that by discovery a not editable param (name) can be changed!
    foreach (QVariant param, deviceMap.value("params").toList()) {
        if (param.toMap().value("name") == "name") {
            QCOMPARE(param.toMap().value("value").toString(), QString("Discovered Mock Device 2"));
        }
        if (param.toMap().value("name") == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), 55556);
        }
    }


    params.clear();
    params.insert("deviceId", deviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}


void TestDevices::removeDevice_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QTest::newRow("Existing Device") << m_mockDeviceId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("Not existing Device") << DeviceId::createDeviceId() << DeviceManager::DeviceErrorDeviceNotFound;
}

void TestDevices::removeDevice()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(DeviceManager::DeviceError, deviceError);

    QSettings settings(m_deviceSettings);
    settings.beginGroup("DeviceConfig");
    if (deviceError == DeviceManager::DeviceErrorNoError) {
        settings.beginGroup(m_mockDeviceId.toString());
        // Make sure we have some config values for this device
        QVERIFY(settings.allKeys().count() > 0);
    }

    QVariantMap params;
    params.insert("deviceId", deviceId);

    QVariant response = injectAndWait("Devices.RemoveConfiguredDevice", params);

    verifyDeviceError(response, deviceError);

    if (DeviceManager::DeviceErrorNoError) {
        // Make sure the device is gone from settings too
        QCOMPARE(settings.allKeys().count(), 0);
    }
}

#include "testdevices.moc"

QTEST_MAIN(TestDevices)
