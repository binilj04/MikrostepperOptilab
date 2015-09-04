#ifndef OPTILABVIEWER_H
#define OPTILABVIEWER_H

#include <QObject>
#include "camera.h"
#include "baserecorder.h"

using Command = std::function < void() > ;
using CommandPool = std::queue < Command >;

class OptilabViewer : public QObject
{
    Q_OBJECT
    Q_ENUMS(RecordingStatus)
    Q_PROPERTY(RecordingStatus recordingStatus READ recordingStatus NOTIFY recordingStatusChanged)
public:
    explicit OptilabViewer(Camera *parent);
    ~OptilabViewer();

    enum RecordingStatus {
        WaitForFile, Recording, Paused
    };

    void addCommand(Command cmd);
    RecordingStatus recordingStatus() const { return en_recording; }
signals:
    void imageSaved(const QUrl& imgPath);
    void remainingCommand(int remaining);
    void recordingTime(const QString& time);
    void recordingStatusChanged();

	void captureReady(const QUrl& filename);

public slots:
    QSize calculateAspectRatio(int screenWidth, int screenHeight) const;
    QUrl captureToTemp(const QString& imgName);
	QUrl saveToTemp(const QString& imgName);
    void copyFromTemp(const QString& imgName, const QUrl& fullPath);
    void copyToFolder(const QUrl& image, const QUrl& folder);
	void scaleImage(const QString& image, int w, int h, 
		Qt::AspectRatioMode as = Qt::IgnoreAspectRatio, 
		Qt::TransformationMode md = Qt::SmoothTransformation);
	QSize imageSize(const QString& image);

    void nextCommand();

    void addCaptureCommand(const QString& imgName);
    void addWaitCommand(int msecond);
    void addCaptureWaitCommand(const QString& imgName, int msecond);

    void flushCommands();

    QStringList startSerialCapture(int interval, int fcount);
	QStringList startSerialCaptureAsync(int interval, int fcount);

    void initRecorder(const QUrl& video);
    void pauseRecording();
    void resumeRecording();
    void stopRecording();

private:
    Camera* m_camera;

    CommandPool commandPool;
    RecordingStatus en_recording;
};

#endif // OPTILABVIEWER_H
