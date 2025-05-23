/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VehicleBatteryFactGroup.h"
#include "QmlObjectListModel.h"
#include "Vehicle.h"

VehicleBatteryFactGroup::VehicleBatteryFactGroup(uint8_t batteryId, QObject *parent)
    : FactGroup(1000, QStringLiteral(":/json/Vehicle/BatteryFact.json"), parent)
{
    _addFact(&_batteryIdFact);
    _addFact(&_batteryFunctionFact);
    _addFact(&_batteryTypeFact);
    _addFact(&_voltageFact);
    _addFact(&_currentFact);
    _addFact(&_mahConsumedFact);
    _addFact(&_temperatureFact);
    _addFact(&_percentRemainingFact);
    _addFact(&_timeRemainingFact);
    _addFact(&_timeRemainingStrFact);
    _addFact(&_chargeStateFact);
    _addFact(&_instantPowerFact);

    _batteryIdFact.setRawValue(batteryId);
    _batteryFunctionFact.setRawValue(MAV_BATTERY_FUNCTION_UNKNOWN);
    _batteryTypeFact.setRawValue(MAV_BATTERY_TYPE_UNKNOWN);
    _voltageFact.setRawValue(qQNaN());
    _currentFact.setRawValue(qQNaN());
    _mahConsumedFact.setRawValue(qQNaN());
    _temperatureFact.setRawValue(qQNaN());
    _percentRemainingFact.setRawValue(qQNaN());
    _timeRemainingFact.setRawValue(qQNaN());
    _chargeStateFact.setRawValue(MAV_BATTERY_CHARGE_STATE_UNDEFINED);
    _instantPowerFact.setRawValue(qQNaN());

    (void) connect(&_timeRemainingFact, &Fact::rawValueChanged, this, &VehicleBatteryFactGroup::_timeRemainingChanged);
}

void VehicleBatteryFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    switch (message.msgid) {
    case MAVLINK_MSG_ID_HIGH_LATENCY:
        _handleHighLatency(vehicle, message);
        break;
    case MAVLINK_MSG_ID_HIGH_LATENCY2:
        _handleHighLatency2(vehicle, message);
        break;
    case MAVLINK_MSG_ID_BATTERY_STATUS:
        _handleBatteryStatus(vehicle, message);
        break;
    default:
        break;
    }
}

void VehicleBatteryFactGroup::_handleHighLatency(Vehicle *vehicle, const mavlink_message_t &message)
{
    mavlink_high_latency_t highLatency{};
    mavlink_msg_high_latency_decode(&message, &highLatency);

    VehicleBatteryFactGroup *const group = _findOrAddBatteryGroupById(vehicle, 0);
    group->percentRemaining()->setRawValue((highLatency.battery_remaining == UINT8_MAX) ? qQNaN() : highLatency.battery_remaining);

    group->_setTelemetryAvailable(true);
}

void VehicleBatteryFactGroup::_handleHighLatency2(Vehicle *vehicle, const mavlink_message_t &message)
{
    mavlink_high_latency2_t highLatency2{};
    mavlink_msg_high_latency2_decode(&message, &highLatency2);

    VehicleBatteryFactGroup *const group = _findOrAddBatteryGroupById(vehicle, 0);
    group->percentRemaining()->setRawValue((highLatency2.battery == -1) ? qQNaN() : highLatency2.battery);

    group->_setTelemetryAvailable(true);
}

void VehicleBatteryFactGroup::_handleBatteryStatus(Vehicle *vehicle, const mavlink_message_t &message)
{
    mavlink_battery_status_t batteryStatus{};
    mavlink_msg_battery_status_decode(&message, &batteryStatus);

    VehicleBatteryFactGroup *const group = _findOrAddBatteryGroupById(vehicle, batteryStatus.id);

    double totalVoltage = qQNaN();
    for (int i = 0; i < 10; i++) {
        const double cellVoltage = ((batteryStatus.voltages[i] == UINT16_MAX)) ? qQNaN() : (static_cast<double>(batteryStatus.voltages[i]) / 1000.0);
        if (qIsNaN(cellVoltage)) {
            break;
        }
        if (i == 0) {
            totalVoltage = cellVoltage;
        } else {
            totalVoltage += cellVoltage;
        }
    }

    for (int i = 0; i < 4; i++) {
        const double cellVoltage = ((batteryStatus.voltages_ext[i] == 0)) ? qQNaN() : (static_cast<double>(batteryStatus.voltages_ext[i]) / 1000.0);
        if (qIsNaN(cellVoltage)) {
            break;
        }
        totalVoltage += cellVoltage;
    }

    group->function()->setRawValue(batteryStatus.battery_function);
    group->type()->setRawValue(batteryStatus.type);
    group->temperature()->setRawValue((batteryStatus.temperature == INT16_MAX) ? qQNaN() : (static_cast<double>(batteryStatus.temperature) / 100.0));
    group->voltage()->setRawValue(totalVoltage);
    group->current()->setRawValue((batteryStatus.current_battery == -1) ? qQNaN() : (static_cast<double>(batteryStatus.current_battery) / 100.0));
    group->mahConsumed()->setRawValue((batteryStatus.current_consumed == -1) ? qQNaN() : batteryStatus.current_consumed);
    group->percentRemaining()->setRawValue((batteryStatus.battery_remaining == -1) ? qQNaN() : batteryStatus.battery_remaining);
    group->timeRemaining()->setRawValue((batteryStatus.time_remaining == 0) ? qQNaN() : batteryStatus.time_remaining);
    group->chargeState()->setRawValue(batteryStatus.charge_state);
    group->instantPower()->setRawValue(totalVoltage * group->current()->rawValue().toDouble());

    group->_setTelemetryAvailable(true);
}

void VehicleBatteryFactGroup::handleMessageForFactGroupCreation(Vehicle *vehicle, const mavlink_message_t &message)
{
    switch (message.msgid) {
    case MAVLINK_MSG_ID_HIGH_LATENCY:
    case MAVLINK_MSG_ID_HIGH_LATENCY2:
        _findOrAddBatteryGroupById(vehicle, 0);
        break;
    case MAVLINK_MSG_ID_BATTERY_STATUS:
    {
        mavlink_battery_status_t batteryStatus{};
        mavlink_msg_battery_status_decode(&message, &batteryStatus);
        _findOrAddBatteryGroupById(vehicle, batteryStatus.id);
    }
    default:
        break;
    }
}

VehicleBatteryFactGroup *VehicleBatteryFactGroup::_findOrAddBatteryGroupById(Vehicle *vehicle, uint8_t batteryId)
{
    QmlObjectListModel *const batteries = vehicle->batteries();

    // We maintain the list in order sorted by battery id so the ui shows them sorted.
    for (int i = 0; i < batteries->count(); i++) {
        VehicleBatteryFactGroup *const group = batteries->value<VehicleBatteryFactGroup*>(i);
        const int listBatteryId = group->id()->rawValue().toInt();
        if (listBatteryId > batteryId) {
            VehicleBatteryFactGroup *const newBatteryGroup = new VehicleBatteryFactGroup(batteryId, batteries);
            batteries->insert(i, newBatteryGroup);
            vehicle->_addFactGroup(newBatteryGroup, QStringLiteral("%1%2").arg(_batteryFactGroupNamePrefix).arg(batteryId));
            return newBatteryGroup;
        } else if (listBatteryId == batteryId) {
            return group;
        }
    }

    VehicleBatteryFactGroup *const newBatteryGroup = new VehicleBatteryFactGroup(batteryId, batteries);
    batteries->append(newBatteryGroup);
    vehicle->_addFactGroup(newBatteryGroup, QStringLiteral("%1%2").arg(_batteryFactGroupNamePrefix).arg(batteryId));

    return newBatteryGroup;
}

void VehicleBatteryFactGroup::_timeRemainingChanged(const QVariant &value)
{
    if (qIsNaN(value.toDouble())) {
        _timeRemainingStrFact.setRawValue("--:--:--");
    } else {
        const int totalSeconds = value.toInt();
        const int hours = totalSeconds / 3600;
        const int minutes = (totalSeconds % 3600) / 60;
        const int seconds = totalSeconds % 60;

        _timeRemainingStrFact.setRawValue(QString::asprintf("%02dH:%02dM:%02dS", hours, minutes, seconds));
    }
}
