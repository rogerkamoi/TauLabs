/**
 ******************************************************************************
 * @addtogroup TauLabsLibraries Tau Labs Libraries
 * @{
 *
 * @file       alarms.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2014
 * @brief      Library for setting and clearing system alarms
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "openpilot.h"
#include "alarms.h"
#include "pios_mutex.h"

// Private constants

// Private types

// Private variables
static struct pios_mutex *lock;

// Private functions
static int32_t hasSeverity(SystemAlarmsAlarmOptions severity);

/**
 * Initialize the alarms library
 */
int32_t AlarmsInitialize(void)
{
	SystemAlarmsInitialize();
	lock = PIOS_Mutex_Create();
	PIOS_Assert(lock != NULL);
	return 0;
}

/**
 * Set an alarm
 * @param alarm The system alarm to be modified
 * @param severity The alarm severity
 * @return 0 if success, -1 if an error
 */
int32_t AlarmsSet(SystemAlarmsAlarmElem alarm, SystemAlarmsAlarmOptions severity)
{
	SystemAlarmsData alarms;

	// Check that this is a valid alarm
	if (alarm >= SYSTEMALARMS_ALARM_NUMELEM)
	{
		return -1;
	}

	// Lock
	PIOS_Mutex_Lock(lock, PIOS_MUTEX_TIMEOUT_MAX);

    // Read alarm and update its severity only if it was changed
    SystemAlarmsGet(&alarms);
    if ( alarms.Alarm[alarm] != severity )
    {
    	alarms.Alarm[alarm] = severity;
    	SystemAlarmsSet(&alarms);
    }

    // Release lock
    PIOS_Mutex_Unlock(lock);
    return 0;

}

/**
 * Get an alarm
 * @param alarm The system alarm to be read
 * @return Alarm severity
 */
SystemAlarmsAlarmOptions AlarmsGet(SystemAlarmsAlarmElem alarm)
{
	SystemAlarmsData alarms;

	// Check that this is a valid alarm
	if (alarm >= SYSTEMALARMS_ALARM_NUMELEM)
	{
		return 0;
	}

    // Read alarm
    SystemAlarmsGet(&alarms);
    return alarms.Alarm[alarm];
}

/**
 * Set an alarm to it's default value
 * @param alarm The system alarm to be modified
 * @return 0 if success, -1 if an error
 */
int32_t AlarmsDefault(SystemAlarmsAlarmElem alarm)
{
	return AlarmsSet(alarm, SYSTEMALARMS_ALARM_DEFAULT);
}

/**
 * Default all alarms
 */
void AlarmsDefaultAll()
{
	uint32_t n;
    for (n = 0; n < SYSTEMALARMS_ALARM_NUMELEM; ++n)
    {
    	AlarmsDefault(n);
    }
}

/**
 * Clear an alarm
 * @param alarm The system alarm to be modified
 * @return 0 if success, -1 if an error
 */
int32_t AlarmsClear(SystemAlarmsAlarmElem alarm)
{
	return AlarmsSet(alarm, SYSTEMALARMS_ALARM_OK);
}

/**
 * Clear all alarms
 */
void AlarmsClearAll()
{
	uint32_t n;
    for (n = 0; n < SYSTEMALARMS_ALARM_NUMELEM; ++n)
    {
    	AlarmsClear(n);
    }
}

/**
 * Check if there are any alarms with the given or higher severity
 * @return 0 if no alarms are found, 1 if at least one alarm is found
 */
int32_t AlarmsHasWarnings()
{
	return hasSeverity(SYSTEMALARMS_ALARM_WARNING);
}

/**
 * Check if there are any alarms with error or higher severity
 * @return 0 if no alarms are found, 1 if at least one alarm is found
 */
int32_t AlarmsHasErrors()
{
	return hasSeverity(SYSTEMALARMS_ALARM_ERROR);
};

/**
 * Check if there are any alarms with critical or higher severity
 * @return 0 if no alarms are found, 1 if at least one alarm is found
 */
int32_t AlarmsHasCritical()
{
	return hasSeverity(SYSTEMALARMS_ALARM_CRITICAL);
};

/**
 * Check if there are any alarms with the given or higher severity
 * @return 0 if no alarms are found, 1 if at least one alarm is found
 */
static int32_t hasSeverity(SystemAlarmsAlarmOptions severity)
{
	SystemAlarmsData alarms;
	uint32_t n;

	// Lock
	PIOS_Mutex_Lock(lock, PIOS_MUTEX_TIMEOUT_MAX);

    // Read alarms
    SystemAlarmsGet(&alarms);

    // Go through alarms and check if any are of the given severity or higher
    for (n = 0; n < SYSTEMALARMS_ALARM_NUMELEM; ++n)
    {
    	if ( alarms.Alarm[n] >= severity)
    	{
    		PIOS_Mutex_Unlock(lock);
    		return 1;
    	}
    }

    // If this point is reached then no alarms found
	PIOS_Mutex_Unlock(lock);
    return 0;
}

/**
 * @}
 */

