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
	Q_PROPERTY(bool scRunning READ scRunning NOTIFY scRunningChanged)
public:
    explicit OptilabViewer(Camera *parent);
    ~OptilabViewer();

    enum RecordingStatus {
        WaitForFile, Recording, Paused
    };

    void addCommand(Command cmd);
    RecordingStatus recordingStatus() const { return en_recording; }

	bool scRunning() const;
signals:
    void imageSaved(const QString& imgPath);
    void remainingCommand(int remaining);
    void recordingTime(const QString& time);
    void recordingStatusChanged();

	void captureReady(const QUrl& filename);

	void preciseTimerTriggered();
	void scRunningChanged(bool);

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

	void captureAsync(const QUrl& filename);
	void capture(const QUrl&);

    void initRecorder(const QUrl& video);
    void pauseRecording();
    void resumeRecording();
    void stopRecording();

	// An utility function! Qt really should add this to Qt namespace!!
	QUrl fromLocalFile(const QString& localfile);
	QString toLocalFile(const QUrl& url);
	void startPreciseTimer(int duration);
	void stopPreciseTimer();

	QString currentPath();
	QString homePath();
	QString tempPath();
	QString rootPath();

	bool exists(const QString& file);

private:
    Camera* m_camera;

    CommandPool commandPool;
    RecordingStatus en_recording;
	QTimer m_timer;
};

#endif // OPTILABVIEWER_H
