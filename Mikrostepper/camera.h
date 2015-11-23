#ifndef CAMERA_H
#define CAMERA_H

/* 
Base implementation of Camera
- Buffer current frame as QImage
- Capture to file and to buffer
- Set and get resolution
*/

#include <QObject>
#include "toupwrapper.h"
#include "camprop.h"
#include "baserecorder.h"

class Camera : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize sourceSize READ size NOTIFY sourceSizeChanged)
public:
	Camera(QObject *parent = 0);
	virtual ~Camera();

	BaseRecorder recorder;

public slots:
    virtual bool isAvailable() = 0;

	virtual void setResolution(int res) = 0;

    virtual QSize size() const = 0;

	virtual void capture(int resolution, const QString &fileName) = 0;
	virtual void saveBuffer(const QString& fileName) = 0;

	virtual double focusValue();

signals :
	void frameReady(const QImage& frame);
	void captureReady(const QString& filename);
    void sourceSizeChanged(const QSize& sz);

protected:
    virtual void initialize() = 0;
	virtual void deinitialize() = 0;
};

class MockCamera : public Camera
{
    Q_OBJECT
    QImage m_buffer;
    QTimer* emitter;
    int state;
    bool m_available = false;
public:
	MockCamera(QObject *parent = 0);
	~MockCamera();

public slots:
    bool isAvailable() { return m_available; }

    void setResolution(int res) { Q_UNUSED(res) }

    QSize size() const { return m_buffer.size(); }

    void capture(int resolution, const QString &fileName);
	void saveBuffer(const QString& fileName);
    
    void imageProc();

protected:
    void initialize() { }
    void deinitialize() { }
};

class BuffObj;

class DSCamera : public Camera
{
	Q_OBJECT
	QSize m_size;
	int m_resolution;
	bool m_available;
	QImage m_buffer;
public:
	DSCamera(QObject *parent = 0);
	~DSCamera();

public slots:
	void setResolution(int res);

	bool isAvailable() { return m_available; }

	QSize size() const;

	void capture(int resolution, const QString &fileName);
	void saveBuffer(const QString& fileName);

	void imageProc(const BuffObj& pBuffer);

	double focusValue() override;

protected:
	void initialize();
	void deinitialize();

};

class DSCameraProp : public NullCamProp
{
	Q_OBJECT
public:
	DSCameraProp(QObject* parent = nullptr);
	~DSCameraProp();

	double saturation() const;
	void setSaturation(double val);
	double contrast() const;
	void setContrast(double);
	double gamma() const;
	void setGamma(double val);

	bool autoexposure() const;
	void setAutoexposure(bool);
	double aeGain() const;
	void setAeGain(double val);
	double exposureTime() const;
	void setExposureTime(double);
	double aeTarget() const;
	void setAeTarget(double);
	double maxExposureTime();

	double rGain() const;
	void setRGain(double val);
	double gGain() const;
	void setGGain(double val);
	double bGain() const;
	void setBGain(double val);

	bool isColor() const;
	void setColorMode(bool);
	bool isHFlip() const;
	void setHFlip(bool);
	bool isVFlip() const;
	void setVFlip(bool);

	int frameRate() const;
	void setFrameRate(int);

public slots:
	void oneShotWB();

	void loadDefaultParameters();

	CameraType cameraType() const;

private:
	void reloadParams();
};

// ToupCamera
class ToupCamera : public Camera
{
	Q_OBJECT
public:
	ToupCamera(QObject* parent = 0);

public slots:
	void setResolution(int res);

	bool isAvailable();

	QSize size() const;

	void capture(int resolution, const QString &fileName);
	void saveBuffer(const QString& fileName);

	double focusValue() override;

	ToupWrapper* wrapper();

protected:
	void initialize(){ };
	void deinitialize(){ };
	
private:
	ToupWrapper m_camera;
	QImage m_buffer;
	QString m_filename;

private slots:
	void pullImage();
	void pullStillImage();
};

class ToupCameraProp : public CamProp
{
	Q_OBJECT
	ToupWrapper* cam;
public:
	ToupCameraProp(ToupWrapper* camera);
	~ToupCameraProp();

	double hue() const;
	void setHue(double);
	double saturation() const;
	void setSaturation(double val);
	double brightness() const;
	void setBrightness(double);
	double contrast() const;
	void setContrast(double);
	double gamma() const;
	void setGamma(double val);

	bool autoexposure() const;
	void setAutoexposure(bool);
	double aeGain() const;
	void setAeGain(double val);
	double exposureTime() const;
	void setExposureTime(double);
	double aeTarget() const;
	void setAeTarget(double);
	double maxExposureTime();

	double rGain() const;
	void setRGain(double val);
	double gGain() const;
	void setGGain(double val);
	double bGain() const;
	void setBGain(double val);

	double whiteBalanceTemperature() const;
	void setWhiteBalanceTemperature(double);
	double whiteBalanceTint() const;
	void setWhiteBalanceTint(double);

	int frameRate() const;
	void setFrameRate(int);

	bool isColor() const;
	void setColorMode(bool);
	bool isHFlip() const;
	void setHFlip(bool);
	bool isVFlip() const;
	void setVFlip(bool);
	bool isBin() const;
	void setSamplingMode(bool);

	QRect whiteBalanceBox() const;
	void setWhiteBalanceBox(const QRect& r);

public slots:
	void oneShotWB();

	void loadDefaultParameters();

	CameraType cameraType() const;
};

/*
QuickCam - QQuickItem to render camera stream
- Image stream is stretched, aspect ratio calculation is done elsewhere
- Can selectively render a part of image
- Can process image in rendering thread

To use simply connect Camera::frameReady signals to QuickCam::updateImage
*/

#include <qquickitem.h>

class QuickCam : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QImage source READ currentFrame WRITE updateImage NOTIFY sourceChanged)
	Q_PROPERTY(bool blocked READ isBlocked WRITE block NOTIFY blockedChanged)
	Q_PROPERTY(RenderParams renderParams MEMBER renderParams)
    Q_PROPERTY(double overlap READ overlap WRITE setOverlap NOTIFY overlapChanged)
	Q_ENUMS(RenderParams)
	QImage m_frame;
	bool m_blocked;
    double m_overlap;
public:
	QuickCam(QQuickItem* parent = 0);
	~QuickCam();

	enum RenderParams {
		OriginalSize, ScaledToItem, Halved
	} renderParams;

	bool isBlocked();
	void block(bool bl);

	QImage currentFrame() const;
    double overlap() const;

public slots:
	void updateImage(const QImage &frame);
    void setOverlap(double trim);

signals:
	void blockedChanged(bool block);
	void sourceChanged(const QImage &nframe);
    void overlapChanged(double ovl);

protected:
	QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);
};

#endif // CAMERA_H
