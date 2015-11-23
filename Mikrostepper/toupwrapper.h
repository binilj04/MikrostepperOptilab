#ifndef TOUPWRAPPER_H
#define TOUPWRAPPER_H

#include <QObject>
#include <QSize>

#include "toupcam.h"

class Image
{
public:
	Image();
	Image(int w, int h, int c);
	~Image();

	Image(const Image&);
	Image& operator=(const Image&);

	Image(Image&& im);
	Image& operator=(Image&&);

	QImage image();
	uchar* buffer();
	uchar* buffer() const;

	QSize size() const;

protected:
	int width, height, channel;
	uchar* m_buffer;
};

void CALLBACK toup_callback(unsigned nEvent, void* pCallbackCtx);

class ToupWrapper : public QObject
{
	Q_OBJECT
	ToupcamInst arrinst[TOUPCAM_MAX];
	HToupCam htoupcam;
	bool m_available;
	QSize m_size;
	size_t m_still;
	Image m_image;
public:
	ToupWrapper(QObject* parent = nullptr);
	~ToupWrapper();

	void cbImageReady();
	void cbStillImageReady();
	void cbCameraError();

	bool isAvailable() const;

	size_t resolution();
	QSize size() const;
	std::vector<QSize> sizes();

	size_t stillResolution() const;
	QSize stillSize();
	std::vector<QSize> stillSizes();

	Image& pullImage();
	Image pullStillImage();

	QRect roi() const;
	void setRoi(const QRect& roi);

	unsigned int rawFormat() const;

	unsigned int option(unsigned int optId) const;
	void setOption(unsigned int optId, unsigned int value);

	bool realTime() const;
	void setRealTime(bool enable);

	bool autoExposure() const;
	void setAutoExposure(bool enable);
	unsigned short autoExposureTarget() const;
	void setAutoExposureTarget(unsigned short target);
	void setMaxAutoExposureTime(unsigned int maxTime);
	void setMaxAutoExposureAGain(unsigned int maxGain);

	unsigned int exposureTime() const;
	void setExposureTime(unsigned int time);
	std::pair<size_t, size_t> exposureTimeRange() const;

	unsigned short exposureAGain() const;
	void setExposureAGain(unsigned short gain);
	std::pair<size_t, size_t> exposureAGainRange() const;

	int hue() const;
	void setHue(int val);
	int saturation() const;
	void setSaturation(int val);
	int brightness() const;
	void setBrightness(int val);
	int contrast() const;
	void setContrast(int val);
	int gamma() const;
	void setGamma(int gamma);

	bool chrome() const;
	void setChrome(bool enable);

	bool hFlip() const;
	void setHFlip(bool enable);
	bool vFlip() const;
	void setVFlip(bool enable);

	unsigned short speed() const;
	void setSpeed(unsigned short speed);

	int frequency() const;
	void setFrequency(int hz);

	bool isBinMode() const;
	void toggleMode();

	std::unique_ptr<int> whiteBalanceGain() const;
	void setWhiteBalanceGain(std::unique_ptr<int> gain);

	int rGain() const;
	void setRGain(int r);
	int gGain() const;
	void setGGain(int g);
	int bGain() const;
	void setBGain(int b);

	int whiteBalanceTemperature() const;
	void setWhiteBalanceTemperature(int temp);
	int whiteBalanceTint() const;
	void setWhiteBalanceTint(int tint);

	QRect whiteBalanceRect() const;
	void setWhiteBalanceRect(const QRect& rect);
	QRect autoExposureRect() const;
	void setAutoExposureRect(const QRect& rect);

	bool isMono() const;

	std::string serialNumber() const;
	std::string fwVersion() const;
	std::string hwVersion() const;
	std::string productionDate() const;

public slots:
	void setResolution(size_t res);
	void start();
	void stop();

	void snap(size_t res);

	void awbOnePush();
	void autoEqualize();

signals:
	void imageReady();
	void stillImageReady();
	void cameraError();
	void sizeChanged(const QSize&);

private:
	void init();
	void close();
	void updateSize();
};


#endif // TOUPWRAPPER_H
