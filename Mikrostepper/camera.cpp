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

void MockCamera::saveBuffer(const QString& fileName) {
	m_buffer.save(fileName);
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
	cv::Mat fr{ sz.height(), sz.width(), CV_8UC3, (uchar*)m_buffer.bits(), (size_t)m_buffer.bytesPerLine() };
	recorder.setFrame(fr);
	emit frameReady(m_buffer);
}

QSize DSCamera::size() const {
	return m_size;
}

void DSCamera::setResolution(int res) {
	deinitialize();
	bool status = CameraInit(SnapThreadCallback, (DS_RESOLUTION)res, 0, 1, 0);
	if (status) {
		m_resolution = res;
		CameraSetB2RGBMode(DS_B2RGB_MODE::B2RGB_MODE_LINE);
		CameraSetColorEnhancement(TRUE);
		CameraSetLightFrquency(DS_LIGHT_FREQUENCY::LIGHT_FREQUENCY_60HZ);
		CameraSetFrameSpeed(DS_FRAME_SPEED::FRAME_SPEED_SUPER);
		CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_HORIZONTAL, FALSE);
		CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_VERTICAL, FALSE);
		CameraPlay();
		int W, H;
		CameraGetImageSize(&W, &H);
		m_size = QSize(W, H);
		m_available = true;
		emit sourceSizeChanged(m_size);

		recorder.frameSize = cv::Size(W, H);
	}
}

void DSCamera::initialize() {
	//Start camera:
	setResolution(m_resolution);
}

void DSCamera::deinitialize() {
	CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_HORIZONTAL, FALSE);
	CameraSetMirror(DS_MIRROR_DIRECTION::MIRROR_DIRECTION_VERTICAL, FALSE);
	CameraStop();
	CameraUnInit();
}

void DSCamera::capture(int res, const QString &fileName) {
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
	//cout << duration_cast<milliseconds>(elapsed).count() << " ms\n";
	cout << res << "\n";
	return res;
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
	update();
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
