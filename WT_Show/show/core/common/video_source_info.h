#pragma once
#ifndef VIDEO_SOURCE_H
#define VIDEO_SOURCE_H

#include <QObject>
#include <QSize>

namespace capture {
namespace common {

class VideoSourceInfo {
    Q_GADGET
    Q_PROPERTY(SourceType type MEMBER type)
    Q_PROPERTY(QString name MEMBER name)
public:
    enum class SourceType
    {
        Invalid,
        DownwardFacingCamera,
        ForwardFacingCamera,
        SproutCamera,
        PrimaryDesktop,
        MatDesktop,
        Webcamera
    };

    Q_ENUM(SourceType)

    explicit VideoSourceInfo(common::VideoSourceInfo::SourceType type = SourceType::Invalid, QString name = QString())
     : type(type), name(name), frameRate(0) {
        // Needed for proper cross-thread signals
        static struct Initialize {
            Initialize() {
                qRegisterMetaType<capture::common::VideoSourceInfo>();
                qRegisterMetaType<QVector<capture::common::VideoSourceInfo>>();
                qRegisterMetaType<capture::common::VideoSourceInfo::SourceType>();
            }
        } initialize;
    }

    SourceType type;
    QString name;
    QSize resolution;
    unsigned int frameRate;

    // Shortcut to types we know
    static VideoSourceInfo DownwardFacingCamera() { return VideoSourceInfo(SourceType::DownwardFacingCamera); }
    static VideoSourceInfo ForwardFacingCamera() { return VideoSourceInfo(SourceType::ForwardFacingCamera); }
    static VideoSourceInfo SproutCamera() { return VideoSourceInfo(SourceType::SproutCamera); }
    static VideoSourceInfo PrimaryDesktop() { return VideoSourceInfo(SourceType::PrimaryDesktop); }
    static VideoSourceInfo MatDesktop() { return VideoSourceInfo(SourceType::MatDesktop); }
};

inline bool operator == (const VideoSourceInfo& lhs, const VideoSourceInfo& rhs)
{
  return (lhs.type == rhs.type && lhs.name == rhs.name && lhs.resolution == rhs.resolution && lhs.frameRate == rhs.frameRate);
}

inline bool operator != (const VideoSourceInfo& lhs, const VideoSourceInfo& rhs)
{
  return !(lhs == rhs);
}

} // namespace common
} // namespace capture

Q_DECLARE_METATYPE(capture::common::VideoSourceInfo)
Q_DECLARE_METATYPE(capture::common::VideoSourceInfo::SourceType)

inline uint qHash(const capture::common::VideoSourceInfo &key, uint seed)
{
    return qHash(key.name, seed) ^ static_cast<int>(key.type);
}

inline QDebug operator << (QDebug debug, const capture::common::VideoSourceInfo& obj)
{
  debug << obj.type << obj.name << "[" << obj.resolution << "@" << obj.frameRate << "fps ]";
  return debug;
}

#endif // VIDEO_SOURCE_H
