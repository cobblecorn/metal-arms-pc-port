// Per-weapon secondary tap/hold gesture, shared by the Mining Laser and L3 Rivet.
// The caller supplies retail timing and energy readiness; this owns no input device.
#pragma once

class PCLaserCharge {
public:
    enum Shot { NONE, SECONDARY_TAP, CHARGED };
    struct Result { Shot shot; float power; };

    PCLaserCharge() : m_analog(false) { Cancel(false); }

    void Cancel(bool waitForRelease = true) {
        m_held = false;
        m_waitForRelease = waitForRelease;
        m_seconds = 0.0f;
        m_heldSeconds = 0.0f;
        m_power = 0.0f;
        m_charging = false;
        m_chargeIntent = false;
    }

    Result Sample(float trigger, bool canFire, bool energyReady, float seconds,
                  float delay, float chargeSeconds, bool analog = false) {
        if (m_analog != analog) { Cancel(false); m_analog = analog; }
        // Retail starts ordinary fire with a full primary pull. Once charging
        // has begun, squeezing further retains that charge until release.
        if (analog && trigger >= 1.0f && !m_charging) {
            Cancel(false);
            return { NONE, 0.0f };
        }
        const bool down = analog ? trigger > 0.0f : trigger >= 0.25f;
        if (!canFire) {
            Cancel(down);
            return { NONE, 0.0f };
        }
        if (m_waitForRelease) {
            if (!down) Cancel(false);
            return { NONE, 0.0f };
        }
        if (down) {
            m_held = true;
            if (seconds > 0.0f) m_heldSeconds += seconds;
            // A hold cannot become a grenade throw when energy/cooldown blocks charging.
            if (m_heldSeconds >= delay) m_chargeIntent = true;
            if (energyReady && chargeSeconds > 0.0f) {
                if (seconds > 0.0f) m_seconds += seconds;
                if (m_seconds > delay + chargeSeconds) m_seconds = delay + chargeSeconds;
                m_charging = m_seconds >= delay;
                m_power = m_charging ? (m_seconds - delay) / chargeSeconds : 0.0f;
                if (m_seconds >= delay + chargeSeconds) m_power = 1.0f;
            } else {
                m_seconds = m_power = 0.0f;
                m_charging = false;
            }
            return { NONE, 0.0f };
        }
        const Shot shot = !m_held ? NONE : (m_chargeIntent ?
            (m_charging && energyReady ? CHARGED : NONE) : (analog ? NONE : SECONDARY_TAP));
        const Result result = { shot, m_power };
        Cancel(false);
        return result;
    }

    bool IsCharging() const { return m_charging; }
    float Power() const { return m_power; }

private:
    bool m_held, m_waitForRelease, m_charging, m_chargeIntent, m_analog;
    float m_seconds, m_heldSeconds, m_power;
};
