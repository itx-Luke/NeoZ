#ifndef NEOZ_INPUTSTATE_H
#define NEOZ_INPUTSTATE_H

#include <QObject>
#include <QPointF>
#include <chrono>

namespace NeoZ {

/**
 * @brief Represents mouse input state at any stage of the pipeline.
 * 
 * This is the fundamental data structure that flows through the
 * Neo-Z input pipeline. It captures raw deltas, velocity, and
 * timing information needed for sensitivity calculations.
 */
struct InputState
{
    // Raw delta values (can be in any unit depending on pipeline stage)
    double deltaX = 0.0;
    double deltaY = 0.0;
    
    // Calculated velocity magnitude (pixels/ms or normalized)
    double velocity = 0.0;
    
    // Timestamp for velocity calculations
    std::chrono::steady_clock::time_point timestamp;
    
    // Pipeline stage tracking (for debugging)
    enum Stage {
        Raw,            // Direct from HID
        WindowsScaled,  // After Windows pointer speed applied
        HostNormalized, // After DPI normalization
        EmulatorMapped, // After emulator translation
        Final           // After velocity curve and multipliers
    };
    Stage stage = Raw;
    
    // Factory methods
    static InputState fromRawDelta(double dx, double dy) {
        InputState state;
        state.deltaX = dx;
        state.deltaY = dy;
        state.timestamp = std::chrono::steady_clock::now();
        state.stage = Raw;
        state.velocity = std::sqrt(dx * dx + dy * dy);
        return state;
    }
    
    static InputState fromAbsolutePositions(const QPointF& current, const QPointF& last) {
        return fromRawDelta(current.x() - last.x(), current.y() - last.y());
    }
    
    // Utility methods
    double magnitude() const {
        return std::sqrt(deltaX * deltaX + deltaY * deltaY);
    }
    
    InputState scaled(double factorX, double factorY) const {
        InputState result = *this;
        result.deltaX *= factorX;
        result.deltaY *= factorY;
        result.velocity = result.magnitude();
        return result;
    }
    
    InputState scaled(double factor) const {
        return scaled(factor, factor);
    }
    
    // Calculate time delta in milliseconds from another state
    double timeDeltaMs(const InputState& previous) const {
        auto duration = timestamp - previous.timestamp;
        return std::chrono::duration<double, std::milli>(duration).count();
    }
    
    // Calculate velocity from time delta (pixels per millisecond)
    double calculateVelocity(const InputState& previous) const {
        double dt = timeDeltaMs(previous);
        if (dt <= 0.0) return 0.0;
        return magnitude() / dt;
    }
};

} // namespace NeoZ

#endif // NEOZ_INPUTSTATE_H
