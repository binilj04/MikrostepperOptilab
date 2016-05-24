#include "stdafx.h"
#include "camera.h"

#include <future>
#include <thread>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <chrono>

#include <qsgsimpletexturenode.h>
#include <qquickwindow.h>

#include "opencv2\opencv.hpp"
#include "DSCAMAPI.h"
using namespace std;

Camera::Camera(QObject *parent)
	: QObject(parent), recorder(this)
{
    
}

Camera::~Camera()
{
}

double Camera::focusValue() {
	return 0.0;
}

//MockCamera implementation
MockCamera::MockCamera(QObject *parent)
    : Camera(parent), state(0), m_buffer(QSize(1280, 1024), QImage::Format_RGB888)
{
    m_buffer.fill(qRgb(0, 255, 0));
    emitter = new QTimer(this);
    emitter->setInterval(1000/10);
    connect(emitter, &QTimer::timeout, this, &MockCamera::imageProc);
    emitter->start();
    m_available = true;
}

MockCamera::~MockCamera()
{
}

void MockCamera::capture(int resolution, const QString &fileName) {
    Q_UNUSED(resolution)
    m_buffer.save(fileName);
}

void MockCamera::saveBuffer(const QString& filename) {
	if (QFile::exists(filename))
		QFile::remove(filename);
	packaged_task<bool(QString)> task{ [&](const QString& fn) -> bool {
		mutex m;
		m.lock();
		auto im = m_buffer.copy();
		m.unlock();
		return im.save(fn);
	} };
	task(filename);
}

void MockCamera::imageProc() {
    auto rgb = QColor::fromHsv(state, 255, 255);
    m_buffer.fill(rgb);
    emit frameReady(m_buffer);
    if (state >= 355)
        state = 0;
    else
        state += 5;
}

//DSCamera implementation
DSCamera* dscamera;
DSCamera::DSCamera(QObject *parent)
	: Camera(parent), m_resolution(1), m_available(false)
{
	dscamera = this;
	initialize();
}

DSCamera::~DSCamera()
{
	deinitialize();
}

class BuffObj
{
	BYTE* data;
	int w, h, c;
public:
	BuffObj(BYTE* pData, int w, int h, int c) {
		size_t sz = w * h *c;
		data = new BYTE[sz];
		memcpy(data, pData, sz);
	}
	~BuffObj() {
		delete[] data;
	}
	BuffObj(BuffObj&) = delete;
	BuffObj(BuffObj&&) = delete;
	BuffObj operator=(BuffObj&) = delete;
	uchar* getData() const { return data; }
};

int CALLBACK SnapThreadCallback(BYTE* pBuffer) {
	int w, h;
	CameraGetImageSize(&w, &h);
	BYTE *pBmp24 = CameraISP(pBuffer);
	BuffObj bf(pBmp24, w, h, 3);
	if (pBmp24 && dscamera)
		dscamera->imageProc(bf);
	return TRUE;
}

void DSCamera::imageProc(const BuffObj& pBuffer) {
	auto sz = size();
	m_buffer = QImage(pBuffer.getData(), sz.width(), sz.height(), QImage::Format_RGB888).copy().rgbSwapped().mirrored(false, true);
	cv::Mat fr{ sz.height(), sz.width(), CV_8UC3, (uchar*)pBuffer.getData() };
	recorder.setFrame(fr);
	emit frameReady(m_buffer);
}

QSize DSCamera::size() const {
	return m_size;
}

void DSCamera::setResolution(int res) {
	deinitialize();
	bool status = CameraInit(SnapThreadCallback, (DS_RESOLUTION)res, 0, 1, 0) == DS_CAMERA_STATUS::STATUS_OK;
	if (status) {
		m_resolution = res;
		CameraPlay();
		int W, H;
		CameraGetImageSize(&W, &H);
		m_size = QSize(W, H);
		m_available = true;
		emit sourceSizeChanged(m_size);

		recorder.frameSize = cv::Size(W, H);
	}
	else
	{
		m_available = false;
		CameraStop();
		CameraUnInit();
	}
}

void DSCamera::initialize() {
	//Start camera:
	setResolution(m_resolution);
}

void DSCamera::deinitialize() {
	CameraStop();
	CameraUnInit();
}

void DSCamera::capture(int res, const QString &fileName) {
	auto job = [&](int res, const QString& fileName) {
		LPCTSTR fn = L"Z";
		if (CameraCaptureFile(fn, FILE_PNG, 80, (DS_RESOLUTION)res) != STATUS_OK)
			return;
		if (QFile::exists(fileName))
			QFile::remove(fileName);
		if (fileName.contains(".png"))
			QFile::copy("Z.png", fileName);
		else if (fileName.contains(".jpg") || fileName.contains(".bmp")) {
			QImage zz("Z.png");
			bool success = zz.save(fileName);
			if (!success) QFile::copy("Z.png", fileName);
		}
		else
			QFile::copy("Z.png", fileName);
		emit captureReady(fileName);
	};
	async(launch::async, job, res, fileName);
}

void DSCamera::saveBuffer(const QString& fileName) {
	std::packaged_task<bool(QString)> task{ [&](const QString& fn) -> bool {
		std::mutex m;
		m.lock();
		auto im = m_buffer.copy();
		m.unlock();
		return im.save(fn);
	} 
	};
	auto result = task.get_future();
	task(fileName);
	if (result.get()) return;
	else 
		m_buffer.save(fileName);
}

cv::Mat toMat(const QImage& im, int w, int h) {
	return cv::Mat{ h, w, CV_8UC3, (uchar*)im.bits(), (size_t)im.bytesPerLine() };
}

cv::Mat toGray(const cv::Mat& im) {
	cv::Mat gr;
	cv::cvtColor(im, gr, cv::COLOR_RGB2GRAY);
	return gr;
}

double DSCamera::focusValue() {
	using namespace cv;
	using namespace std;
	using namespace std::chrono;
	auto mat = toMat(m_buffer, size().width(), size().height());
	auto gray = toGray(mat);
	//auto now = steady_clock::now();
	Scalar mean, stddev;
	meanStdDev(gray, mean, stddev);
	auto res = stddev[0] * stddev[0] / mean[0];
	//auto elapsed = steady_clock::now() - now;
	//cout << duration_cast<microseconds>(elapsed).count() << " us\n";
	//cout << res << "\n";
	return res;
}

//DSCameraProp implementation
DSCameraProp::DSCameraProp(QObject* parent)
	: NullCamProp(parent)
{
	min["gamma"] = 10;
	max["gamma"] = 250;
	min["saturation"] = 0;
	max["saturation"] = 255;
	min["contrast"] = 0;
	max["contrast"] = 100;
	min["aetarget"] = 50;
	max["aetarget"] = 200;
	min["aeexposure"] = 0.001;
	max["aeexposure"] = 4;
	min["aegain"] = 1;
	max["aegain"] = 80;
	min["red"] = 10;
	max["red"] = 255;
	min["green"] = 10;
	max["green"] = 255;
	min["blue"] = 10;
	max["blue"] = 255;
	reloadParams();
}

DSCameraProp::~DSCameraProp()
{
}

double DSCameraProp::rGain() const {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
    return rg;
}

void DSCameraProp::setRGain(double val) {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
    if (rg != int(val)) {
		rg = int(val);
		CameraSetGain(rg, gg, bg);
        emit rGainChanged(val);
    }
}

double DSCameraProp::gGain() const {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
	return 1.0 * gg;
}

void DSCameraProp::setGGain(double val) {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
	if (gg != int(val)) {
		gg = int(val);
		CameraSetGain(rg, gg, bg);
		emit rGainChanged(val);
	}
}

double DSCameraProp::bGain() const {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
	return 1.0 * bg;
}

void DSCameraProp::setBGain(double val) {
	int rg, gg, bg;
	CameraGetGain(&rg, &gg, &bg);
	if (bg != int(val)) {
		bg = int(val);
		CameraSetGain(rg, gg, bg);
		emit rGainChanged(val);
	}
}

double DSCameraProp::gamma() const {
	uchar ga;
	CameraGetGamma(&ga);
	return 1.0 * ga;
}

void DSCameraProp::setGamma(double val) {
    if (gamma() != val) {
		CameraSetGamma((uchar)val);
        emit gammaChanged(val);
    }
}

double DSCameraProp::contrast() const {
	uchar cont;
	CameraGetContrast(&cont);
	return 1.0 * cont;
}

void DSCameraProp::setContrast(double val) {
    if (contrast() != val) {
		CameraSetContrast((uchar)val);
        emit contrastChanged(val);
    }
}

double DSCameraProp::saturation() const {
	uchar sat;
	CameraGetSaturation(&sat);
	return 1.0 * sat;
}

void DSCameraProp::setSaturation(double val) {
    if (saturation() != val) {
		CameraSetSaturation((uchar)val);
        emit saturationChanged(val);
    }
}

bool DSCameraProp::autoexposure() const {
	BOOL AE;
	CameraGetAeState(&AE);
	return (AE == TRUE);
}

void DSCameraProp::setAutoexposure(bool val) {
    if (autoexposure() != val) {
		BOOL v = val ? TRUE : FALSE;
		CameraSetAeState(v);
        emit autoexposureChanged(val);
		emit aeGainChanged(aeGain());
		emit aeTargetChanged(aeTarget());
		emit exposureTimeChanged(exposureTime());
    }
}

double DSCameraProp::aeGain() const {
	ushort AEG;
	CameraGetAnalogGain(&AEG);
	return 1.0 * AEG;
}

void DSCameraProp::setAeGain(double val) {
    if (aeGain() != val) {
		CameraSetAnalogGain((ushort)val);
        emit aeGainChanged(val);
    }
}

double DSCameraProp::exposureTime() const {
	int tm;
	CameraGetExposureTime(&tm);
	return 1.0 * tm;
}

double DSCameraProp::maxExposureTime() {
	ushort tmx;
	CameraGetMaxExposureTime(&tmx);
	return 1.0 * tmx;
}

void DSCameraProp::setExposureTime(double val) {
    if (exposureTime() != val) {
		CameraSetExposureTime((int)val);
        emit exposureTimeChanged(val);
    }
}

double DSCameraProp::aeTarget() const {
	uchar tgt;
	CameraGetAeTarget(&tgt);
	return 1.0 * tgt;
}

void DSCameraProp::setAeTarget(double val) {
    if (aeTarget() != val) {
		CameraSetAeTarget((uchar)val);
        emit aeTargetChanged(val);
    }
}

void DSCameraProp::oneShotWB() {
	CameraSetAWBState(TRUE);
}

bool DSCameraProp::isColor() const
{
	BOOL enable;
	CameraGetMonochrome(&enable);
	return enable == FALSE;
}

void DSCameraProp::setColorMode(bool enable)
{
	if (enable != isColor())
	{
		CameraSetMonochrome(!enable);
		emit isColorChanged(enable);
	}
}

bool DSCameraProp::isHFlip() const
{
	BOOL flip;
	CameraGetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_HORIZONTAL, &flip);
	return flip == TRUE;
}

void DSCameraProp::setHFlip(bool flip)
{
	if (flip != isHFlip())
	{
		CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_HORIZONTAL, flip);
		emit isHFlipChanged(flip);
	}
}

bool DSCameraProp::isVFlip() const
{
	BOOL flip;
	CameraGetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_VERTICAL, &flip);
	return flip == TRUE;
}

void DSCameraProp::setVFlip(bool flip)
{
	if (flip != isVFlip())
	{
		CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_VERTICAL, flip);
		emit isVFlipChanged(flip);
	}
}

int DSCameraProp::frameRate() const
{
	uchar pt;
	CameraGetFrameSpeed(&pt);
	return pt;
}

void DSCameraProp::setFrameRate(int speed)
{
	if (speed != frameRate())
	{
		auto spd = (speed == 0) ? 0 : 2;
		CameraSetFrameSpeed(static_cast<DS_FRAME_SPEED>(spd));
		emit frameRateChanged(spd);
	}
}

void DSCameraProp::reloadParams() 
{
	emit rGainChanged(rGain());
	emit gGainChanged(gGain());
	emit bGainChanged(bGain());
	emit gammaChanged(gamma());
	emit contrastChanged(contrast());
	emit saturationChanged(saturation());
	emit autoexposureChanged(autoexposure());
	emit aeGainChanged(aeGain());
	emit aeTargetChanged(aeTarget());
	emit exposureTimeChanged(exposureTime());
}

void DSCameraProp::saveParametersA()
{
	CameraSaveParameter(PARAMETER_TEAM_A);
}

void DSCameraProp::loadParametersA()
{
	CameraReadParameter(PARAMETER_TEAM_A);
	setLastParam(0);
}

void DSCameraProp::saveParametersB()
{
	CameraSaveParameter(PARAMETER_TEAM_B);
}

void DSCameraProp::loadParametersB()
{
	CameraReadParameter(PARAMETER_TEAM_B);
	setLastParam(1);
}

void DSCameraProp::saveParametersC()
{
	CameraSaveParameter(PARAMETER_TEAM_C);
}

void DSCameraProp::loadParametersC()
{
	CameraReadParameter(PARAMETER_TEAM_C);
	setLastParam(2);
}

void DSCameraProp::saveParametersD()
{
	CameraSaveParameter(PARAMETER_TEAM_D);
}

void DSCameraProp::loadParametersD()
{
	CameraReadParameter(PARAMETER_TEAM_D);
	setLastParam(3);
}

void DSCameraProp::loadDefaultParameters()
{
	CameraLoadDefault();
	reloadParams();
}

int DSCameraProp::getCurrentParameterTeam()
{
	uchar p;
	CameraGetCurrentParameterTeam(&p);
	return p;
}

CamProp::CameraType DSCameraProp::cameraType() const
{
	return DS;
}

double DSCameraProp::controlMin(const QString& control) const
{
	if (min.find(control) != end(min))
		return min.at(control);
	return 0;
}

double DSCameraProp::controlMax(const QString& control) const
{
	if (max.find(control) != end(max))
		return max.at(control);
	return 100;
}

bool DSCameraProp::controlAvailable(const QString& control) const
{
	if (max.find(control) != end(max))
		return true;
	return false;
}

//ToupCamera implementation
ToupCamera::ToupCamera(QObject* parent)
	: Camera{ parent }, m_camera{ this }
{
	connect(&m_camera, &ToupWrapper::imageReady, this, &ToupCamera::pullImage);
	connect(&m_camera, &ToupWrapper::stillImageReady, this, &ToupCamera::pullStillImage);
}

bool ToupCamera::isAvailable()
{
	return m_camera.isAvailable();
}

QSize ToupCamera::size() const
{
	return m_camera.size();
}

void ToupCamera::setResolution(int res)
{
	m_camera.setResolution(res);
}

void ToupCamera::capture(int resolution, const QString& filename)
{
	m_filename = filename;
	m_camera.snap(resolution);
}

void ToupCamera::saveBuffer(const QString& filename)
{
	if (QFile::exists(filename))
		QFile::remove(filename);
	packaged_task<bool(QString)> task{ [&](const QString& fn) -> bool {
		mutex m;
		m.lock();
		auto im = m_buffer.copy();
		m.unlock();
		return im.save(fn);
	} };
	task(filename);
}

double ToupCamera::focusValue()
{
	using namespace cv;
	using namespace std;
	using namespace std::chrono;
	auto mat = toMat(m_buffer, size().width(), size().height());
	auto gray = toGray(mat);
	//auto now = steady_clock::now();
	Scalar mean, stddev;
	meanStdDev(gray, mean, stddev);
	auto res = stddev[0] * stddev[0] / mean[0];
	//auto elapsed = steady_clock::now() - now;
	//cout << duration_cast<microseconds>(elapsed).count() << " us\n";
	//cout << res << "\n";
	return res;
}

void ToupCamera::pullImage()
{
	auto img = m_camera.pullImage();
	m_buffer = img.image().copy().rgbSwapped().mirrored(false, true);
	cv::Mat fr{ img.size().height(), img.size().width(), CV_8UC3, (uchar*)img.buffer(), (size_t)img.image().bytesPerLine() };
	recorder.setFrame(fr.clone());
	//qDebug() << "osc: " << m_buffer.bits();
	emit frameReady(m_buffer);
}

void ToupCamera::pullStillImage()
{
	auto still = m_camera.pullStillImage();
	if (QFile::exists(m_filename))
		QFile::remove(m_filename);
	//qDebug() << "sav: " << still.buffer();
    auto ext = m_filename.right(3).toUpper().toStdString().c_str();
    still.image().rgbSwapped().mirrored().save(m_filename, ext);
	emit captureReady(m_filename);
}

ToupWrapper* ToupCamera::wrapper()
{
	return &m_camera;
}

//ToupCameraProp implementation
ToupCameraProp::ToupCameraProp(ToupWrapper* camera)
	: CamProp{ camera }, cam{ camera }
{
	min["hue"] = -180;
	max["hue"] = 180;
	min["saturation"] = 0;
	max["saturation"] = 255;
	min["brightness"] = -64;
	max["brightness"] = 64;
	min["gamma"] = 20;
	max["gamma"] = 180;
	min["contrast"] = -100;
	max["contrast"] = 100;
	min["aetarget"] = 16;
	max["aetarget"] = 235;
	min["aeexposure"] = 0.3;
	max["aeexposure"] = 2000;
	min["aegain"] = 100;
	max["aegain"] = 300;
	min["awbtemp"] = 2000;
	max["awbtemp"] = 15000;
	min["awbtint"] = 200;
	max["awbtint"] = 2500;
	min["framerate"] = 0;
	max["framerate"] = 3;
}

ToupCameraProp::~ToupCameraProp()
{

}

void ToupCameraProp::oneShotWB()
{
	cam->awbOnePush();
	emit whiteBalanceTemperatureChanged(whiteBalanceTemperature());
	emit whiteBalanceTintChanged(whiteBalanceTint());
	emit rGainChanged(rGain());
	emit gGainChanged(gGain());
	emit bGainChanged(bGain());
}

double ToupCameraProp::hue() const
{
	return cam->hue();
}

void ToupCameraProp::setHue(double val)
{
	if (val != hue())
	{
		cam->setHue(val);
		emit hueChanged(val);
	}
}

double ToupCameraProp::saturation() const
{
	return cam->saturation();
}

void ToupCameraProp::setSaturation(double val)
{
	if (val != saturation())
	{
		cam->setSaturation(val);
		emit saturationChanged(val);
	}
}

double ToupCameraProp::brightness() const
{
	return cam->brightness();
}

void ToupCameraProp::setBrightness(double val)
{
	if (val != brightness())
	{
		cam->setBrightness(val);
		emit brightnessChanged(val);
	}
}

double ToupCameraProp::contrast() const
{
	return cam->contrast();
}

void ToupCameraProp::setContrast(double val)
{
	if (val != contrast())
	{
		cam->setContrast(val);
		emit contrastChanged(val);
	}
}

double ToupCameraProp::gamma() const
{
	return cam->gamma();
}

void ToupCameraProp::setGamma(double val)
{
	if (val != gamma())
	{
		cam->setGamma(val);
		emit gammaChanged(val);
	}
}

bool ToupCameraProp::autoexposure() const
{
	return cam->autoExposure();
}

void ToupCameraProp::setAutoexposure(bool val)
{
	if (val != autoexposure())
	{
		cam->setAutoExposure(val);
		emit autoexposureChanged(val);
	}
}

double ToupCameraProp::aeGain() const
{
	return cam->exposureAGain();
}

void ToupCameraProp::setAeGain(double val)
{
	if (val != aeGain())
	{
		cam->setExposureAGain(val);
		emit aeGainChanged(val);
	}
}

double ToupCameraProp::exposureTime() const
{
	return cam->exposureTime();
}

void ToupCameraProp::setExposureTime(double val)
{
	if (val != exposureTime())
	{
		cam->setExposureTime(val);
		emit exposureTimeChanged(val);
	}
}

double ToupCameraProp::aeTarget() const
{
	return cam->autoExposureTarget();
}

void ToupCameraProp::setAeTarget(double val)
{
	if (val != aeTarget())
	{
		cam->setAutoExposureTarget(val);
		emit aeTargetChanged(val);
	}
}

double ToupCameraProp::maxExposureTime()
{
	return cam->exposureTimeRange().second;
}

double ToupCameraProp::rGain() const
{
	return cam->rGain();
}

void ToupCameraProp::setRGain(double val)
{
	if (val != rGain())
	{
		cam->setRGain(val);
		emit rGainChanged(val);
	}
}

double ToupCameraProp::gGain() const
{
	return cam->gGain();
}

void ToupCameraProp::setGGain(double val)
{
	if (val != gGain())
	{
		cam->setGGain(val);
		emit gGainChanged(val);
	}
}

double ToupCameraProp::bGain() const
{
	return cam->bGain();
}

void ToupCameraProp::setBGain(double val)
{
	if (val != bGain())
	{
		cam->setBGain(val);
		emit bGainChanged(val);
	}
}

double ToupCameraProp::whiteBalanceTemperature() const
{
	return cam->whiteBalanceTemperature();
}

void ToupCameraProp::setWhiteBalanceTemperature(double val)
{
	if (val != cam->whiteBalanceTemperature())
	{
		cam->setWhiteBalanceTemperature(val);
		emit whiteBalanceTintChanged(val);
	}
}

double ToupCameraProp::whiteBalanceTint() const
{
	return cam->whiteBalanceTint();
}

void ToupCameraProp::setWhiteBalanceTint(double val)
{
	if (val != whiteBalanceTint())
	{
		cam->setWhiteBalanceTint(val);
		emit whiteBalanceTintChanged(val);
	}
}

int ToupCameraProp::frameRate() const
{
	return cam->speed();
}

void ToupCameraProp::setFrameRate(int fr)
{
	if (fr != cam->speed())
	{
		cam->setSpeed(fr);
		emit frameRateChanged(fr);
	}
}

bool ToupCameraProp::isColor() const
{
	return !cam->chrome();
}

void ToupCameraProp::setColorMode(bool mode)
{
	if (mode != isColor())
	{
		cam->setChrome(!mode);
		emit isColorChanged(mode);
	}
}

bool ToupCameraProp::isHFlip() const
{
	return cam->hFlip();
}

void ToupCameraProp::setHFlip(bool flip)
{
	if (flip != isHFlip())
	{
		cam->setHFlip(flip);
		emit isHFlipChanged(flip);
	}
}

bool ToupCameraProp::isVFlip() const
{
	return cam->vFlip();
}

void ToupCameraProp::setVFlip(bool flip)
{
	if (flip != isVFlip())
	{
		cam->setVFlip(flip);
		emit isVFlipChanged(flip);
	}
}

bool ToupCameraProp::isBin() const
{
	return cam->isBinMode();
}

void ToupCameraProp::setSamplingMode(bool mode)
{
	if (mode != isBin())
	{
		cam->toggleMode();
		emit isBinChanged(mode);
	}
}

QRect ToupCameraProp::whiteBalanceBox() const
{
	return cam->whiteBalanceRect();
}

void ToupCameraProp::setWhiteBalanceBox(const QRect& r)
{
	if (r != whiteBalanceBox())
	{
		cam->setWhiteBalanceRect(r);
		emit whiteBalanceBoxChanged(r);
	}
}

void ToupCameraProp::loadDefaultParameters()
{
	setHue(0.0);
	setSaturation(128.0);
	setBrightness(0.0);
	setContrast(0.0);
	setGamma(100);
	setAutoexposure(true);
	setAeGain(100.0);
	setExposureTime(70000.0);
	setAeTarget(120.0);
	setRGain(1.0);
	setGGain(1.0);
	setBGain(1.0);
	setWhiteBalanceTemperature(6503.0);
	setWhiteBalanceTint(1000.0);
	setFrameRate(3);
	setColorMode(true);
	setHFlip(false);
	setVFlip(false);
	setSamplingMode(true);
	setWhiteBalanceBox(QRectF{ 50.0, 50.0, 50.0, 50.0 }.toRect());
}

CamProp::CameraType ToupCameraProp::cameraType() const
{
	return Toup;
}

double ToupCameraProp::controlMin(const QString& control) const
{
	if (min.find(control) != end(min))
		return min.at(control);
	return 0;
}

double ToupCameraProp::controlMax(const QString& control) const
{
	if (max.find(control) != end(max))
		return max.at(control);
	return 100;
}

bool ToupCameraProp::controlAvailable(const QString& control) const
{
	if (max.find(control) != end(max))
		return true;
	return false;
}


//QuickCam implementation
QuickCam::QuickCam(QQuickItem* parent)
    : QQuickItem(parent), m_frame(QSize(10, 10), QImage::Format_RGB888), m_blocked(false),
      m_overlap(0.0)
{
	m_frame.fill(Qt::white);
	renderParams = OriginalSize;
	setFlag(QQuickItem::ItemHasContents, true);
}

QuickCam::~QuickCam() {
}

bool QuickCam::isBlocked() {
	return m_blocked;
}

void QuickCam::block(bool bl) {
	if (m_blocked != bl) {
		m_blocked = bl;
		emit blockedChanged(bl);
	}
}

QImage QuickCam::currentFrame() const {
	return m_frame;
}

void QuickCam::updateImage(const QImage &frame) {
	if (!m_blocked) {
		auto src = frame;
        if (m_overlap > 0.00) {
			int tlx = m_overlap * frame.width();
			int tly = m_overlap * frame.height();
			int brx = frame.width() - tlx;
			int bry = frame.height() - tly;
            src = frame.copy(tlx, tly, brx, bry);
        }
        int w = (width() > 0) ? width() : src.width() / 10;
        int h = (height() > 0) ? height() : src.height() / 10;
		if (renderParams == ScaledToItem)
			m_frame = src.scaled(QSize(w, h));
		else if (renderParams == Halved)
			m_frame = src.scaled(QSize(src.width() / 2, src.height() / 2));
        else
            m_frame = src;
	}
	if (m_frame.bits())
		update();
	//qDebug() << "tex: " << m_frame.bits();
	emit sourceChanged(m_frame);
}

QSGNode* QuickCam::updatePaintNode(QSGNode* node, UpdatePaintNodeData* u) {
	Q_UNUSED(u)
	QSGSimpleTextureNode* n = static_cast<QSGSimpleTextureNode*>(node);
	if (!n) {
		n = new QSGSimpleTextureNode();
	}
	n->setRect(boundingRect());

	auto texture = n->texture();
	if (texture) texture->deleteLater();
	n->setTexture(this->window()->createTextureFromImage(m_frame));
	return n;
}

double QuickCam::overlap() const {
    return m_overlap;
}

void QuickCam::setOverlap(double trim) {
    if (m_overlap != trim) {
        m_overlap = trim;
        emit overlapChanged(trim);
    }
}
