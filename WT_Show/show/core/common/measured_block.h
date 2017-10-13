#pragma once
#ifndef MEASURED_BLOCK_H
#define MEASURED_BLOCK_H

#include <QElapsedTimer>
#include <global_utilities.h>

namespace capture {
namespace common {

#define MEASURED_BLOCK capture::common::MeasuredBlock measuredBlock = capture::common::measure_block(__FUNCTION__); Q_UNUSED(measuredBlock);

class MeasuredBlock
{
public:
    explicit MeasuredBlock(const QString& name) noexcept : m_name(name), m_reference(0) {
        if (!m_name.isEmpty()) {
            m_timer.start();
        }
    }

    explicit MeasuredBlock() noexcept {}

    MeasuredBlock(MeasuredBlock&& other) noexcept : MeasuredBlock(other.m_name) {
        if (!m_name.isEmpty() && other.m_timer.isValid()) {
            m_timer.start();
            m_reference = other.m_timer.elapsed();
        }
        other.m_timer.invalidate();
    }

    MeasuredBlock(const MeasuredBlock&) = delete;
    MeasuredBlock& operator=(const MeasuredBlock&) = delete;

    ~MeasuredBlock() noexcept {
        if (!m_name.isEmpty() && m_timer.isValid()) {
            qDebug() << m_name << "took" << (m_reference + m_timer.elapsed()) << "milliseconds";
        }
    }

private:
    QElapsedTimer m_timer;
    QString m_name;
    qint64 m_reference;
};

inline MeasuredBlock measure_block(const QString& name) noexcept {
    static bool initialized = false;
    static bool allowBlocks = false;

    if (!initialized) {
        initialized = true;
        allowBlocks = GlobalUtilities::applicationSettings()->value("measured_blocks_enabled", false).toBool();
    }

    return allowBlocks ? MeasuredBlock(name) : MeasuredBlock();
}

} // namespace common
} // namespace capture

#endif // MEASURED_BLOCK_H
