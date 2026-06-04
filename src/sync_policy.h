#pragma once
// Sync if we've never synced, or the calendar day changed since last sync.
// yearDay = day-of-year (0..365) read from the RTC.
inline bool needsNtpSync(bool everSynced, int lastSyncYearDay, int currentYearDay) {
    if (!everSynced) return true;
    return lastSyncYearDay != currentYearDay;
}
