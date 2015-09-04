#include "stdafx.h"
#include "toupwrapper.h"
using namespace std;

Image::Image(int w, int h, int c)
	: width{ w }, height{ h }, channel{ c }, m_buffer{ new uchar[w * h * c] }
{ }

Image::~Image()
{
	if (m_buffer) delete[] m_buffer;
}

Image::Image(const Image& o)
	: width{ o.width }, height{ o.height }, channel{ o.channel }, 
	m_buffer{ new uchar[width * height * channel] }
{
	memcpy(m_buffer, o.m_buffer, width * height * channel);
}

Image& Image::operator=(const Image& o)
{
	width = o.width;
	height = o.height;
	channel = o.channel;
	if (m_buffer)
		delete[] m_buffer;
	m_buffer = new uchar[width*height*channel];
	memcpy(m_buffer, o.m_buffer, width * height * channel);
	return *this;
}

Image::Image(Image&& im)
	: width{ im.width }, height{ im.height }, channel{ im.channel }, m_buffer{ im.m_buffer }
{
	im.width = 0;
	im.height = 0;
	im.channel = 0;
	im.m_buffer = nullptr;
}

Image& Image::operator=(Image&& im)
{
	width = im.width;
	height = im.height;
	channel = im.channel;
	m_buffer = im.m_buffer;
	im.m_buffer = nullptr;
	return *this;
}

QImage Image::image()
{
	return QImage{ m_buffer, width, height, QImage::Format_RGB888 };
}

uchar* Image::buffer()
{
	return m_buffer;
}

uchar* Image::buffer() const
{
	return m_buffer;
}

QSize Image::size() const
{
	return QSize{ width, height };
}

ToupWrapper::ToupWrapper(QObject* parent)
	: QObject(parent), m_available{ false }
{
	init();
	start();
	auto res = sizes();
	size_t select;
	for (size_t i = res.size() - 1; i >= 0; --i)
	{
		if (res.at(i).width() * res.at(i).height() > 1000000)
		{
			select = i;
			break;
		}
	}
	setResolution(select);
}

ToupWrapper::~ToupWrapper()
{
	stop();
	close();
}

void ToupWrapper::setResolution(size_t res)
{
	stop();
	if (m_available) Toupcam_put_eSize(htoupcam, res);
	start();
}

void ToupWrapper::init()
{
	auto cnt = Toupcam_Enum(arrinst);
	if (cnt > 0)
	{
		htoupcam = Toupcam_Open(0);
		m_available = true;
	}
	else
		m_available = false;
}

void ToupWrapper::close()
{
	if (m_available) Toupcam_Close(htoupcam);
	m_available = false;
}

void ToupWrapper::updateSize()
{
	int w, h;
	Toupcam_get_Resolution(htoupcam, resolution(), &w, &h);
	m_size = QSize{ w, h };
	m_image = Image{ w, h, 3 };
	//qDebug() << m_size;
	emit sizeChanged(m_size);
}

void CALLBACK toup_callback(unsigned nEvent, void* pCallbackCtx)
{
	auto toup = (ToupWrapper*)pCallbackCtx;
	switch (nEvent)
	{
	case TOUPCAM_EVENT_IMAGE:
		toup->cbImageReady();
		break;
	case TOUPCAM_EVENT_STILLIMAGE:
		toup->cbStillImageReady();
		break;
	case TOUPCAM_EVENT_ERROR:
		toup->cbCameraError();
		break;
	case TOUPCAM_EVENT_DISCONNECTED:
		toup->cbCameraError();
		break;
	}
}

void ToupWrapper::start()
{
	if (m_available)
	{
		updateSize();
		Toupcam_StartPullModeWithCallback(htoupcam, toup_callback, this);
	}
}

void ToupWrapper::stop()
{
	if (m_available)
		Toupcam_Stop(htoupcam);
}

bool ToupWrapper::isAvailable() const
{
	return m_available;
}

void ToupWrapper::cbImageReady()
{
	emit imageReady();
}

Image& ToupWrapper::pullImage()
{
	Toupcam_PullImage(htoupcam, m_image.buffer(), 24, nullptr, nullptr);
	//qDebug() << "buf: " << m_image.buffer();
	return m_image;
}

void ToupWrapper::snap(size_t res)
{
	m_still = res;
	Toupcam_Snap(htoupcam, res);
}

void ToupWrapper::cbStillImageReady()
{
	emit stillImageReady();
}

Image ToupWrapper::pullStillImage()
{
	auto still_sz = stillSize();
	Image im{ still_sz.width(), still_sz.height(), 3 };
	Toupcam_PullStillImage(htoupcam, im.buffer(), 24, nullptr, nullptr);
	//qDebug() << "buf: " << im.buffer();
	return move(im);
}

void ToupWrapper::cbCameraError()
{
	m_available = false;
	emit cameraError();
}

size_t ToupWrapper::resolution()
{
	size_t res;
	Toupcam_get_eSize(htoupcam, &res);
	return res;
}

QSize ToupWrapper::size() const 
{
	return m_size;
}

vector<QSize> ToupWrapper::sizes()
{
	vector<QSize> res;
	auto model = arrinst[0].model;
	size_t cnt = model->preview;
	for (auto i = 0u; i < cnt; ++i)
	{
		int w, h;
		Toupcam_get_Resolution(htoupcam, i, &w, &h);
		res.emplace_back(QSize{ w, h });
	}
	return move(res);
}

size_t ToupWrapper::stillResolution() const
{
	return m_still;
}

QSize ToupWrapper::stillSize()
{
	return stillSizes().at(m_still);
}

vector<QSize> ToupWrapper::stillSizes()
{
	vector<QSize> res;
	auto model = arrinst[0].model;
	size_t cnt = model->still;
	for (auto i = 0u; i < cnt; ++i)
	{
		int w, h;
		Toupcam_get_StillResolution(htoupcam, i, &w, &h);
		res.emplace_back(QSize{ w, h });
	}
	return move(res);
}

QRect ToupWrapper::roi() const
{
	unsigned int x, y, w, h;
	Toupcam_get_Roi(htoupcam, &x, &y, &w, &h);
	return QRect{ (int)x, (int)y, (int)w, (int)h };
}

void ToupWrapper::setRoi(const QRect& roi)
{
	Toupcam_put_Roi(htoupcam, (size_t)roi.x(), (size_t)roi.y(), (size_t)roi.width(), (size_t)roi.height());
}

unsigned int ToupWrapper::rawFormat() const
{
	unsigned int fourcc, bitdepth;
	Toupcam_get_RawFormat(htoupcam, &fourcc, &bitdepth);
	return fourcc;
}

unsigned int ToupWrapper::option(unsigned int optId) const
{
	unsigned int value;
	Toupcam_get_Option(htoupcam, optId, &value);
	return value;
}

void ToupWrapper::setOption(unsigned int optId, unsigned int value)
{
	Toupcam_put_Option(htoupcam, optId, value);
}

bool ToupWrapper::realTime() const
{
	BOOL rt;
	Toupcam_get_RealTime(htoupcam, &rt);
	return rt;
}

void ToupWrapper::setRealTime(bool enable)
{
	Toupcam_put_RealTime(htoupcam, enable);
}

bool ToupWrapper::autoExposure() const
{
	BOOL ae;
	Toupcam_get_AutoExpoEnable(htoupcam, &ae);
	return ae;
}

void ToupWrapper::setAutoExposure(bool enable)
{
	Toupcam_put_AutoExpoEnable(htoupcam, enable);
}

unsigned short ToupWrapper::autoExposureTarget() const
{
	unsigned short target;
	Toupcam_get_AutoExpoTarget(htoupcam, &target);
	return target;
}

void ToupWrapper::setAutoExposureTarget(unsigned short target)
{
	Toupcam_put_AutoExpoTarget(htoupcam, target);
}

void ToupWrapper::setMaxAutoExposureTime(unsigned int time)
{
	unsigned short temp;
	Toupcam_get_ExpoAGainRange(htoupcam, nullptr, &temp, nullptr);
	Toupcam_put_MaxAutoExpoTimeAGain(htoupcam, time, temp);
}

void ToupWrapper::setMaxAutoExposureAGain(unsigned int gain)
{
	unsigned int temp;
	Toupcam_get_ExpTimeRange(htoupcam, nullptr, &temp, nullptr);
	Toupcam_put_MaxAutoExpoTimeAGain(htoupcam, temp, gain);
}

unsigned int ToupWrapper::exposureTime() const
{
	unsigned int time;
	Toupcam_get_ExpoTime(htoupcam, &time);
	return time;
}

void ToupWrapper::setExposureTime(unsigned int time)
{
	Toupcam_put_ExpoTime(htoupcam, time);
}

pair<size_t, size_t> ToupWrapper::exposureTimeRange() const
{
	unsigned int min, max;
	Toupcam_get_ExpTimeRange(htoupcam, &min, &max, nullptr);
	return pair < size_t, size_t > {min, max};
}

unsigned short ToupWrapper::exposureAGain() const
{
	unsigned short gain;
	Toupcam_get_ExpoAGain(htoupcam, &gain);
	return gain;
}

void ToupWrapper::setExposureAGain(unsigned short gain)
{
	Toupcam_put_ExpoAGain(htoupcam, gain);
}

pair<size_t, size_t> ToupWrapper::exposureAGainRange() const
{
	unsigned short min, max;
	Toupcam_get_ExpoAGainRange(htoupcam, &min, &max, nullptr);
	return pair < size_t, size_t > {min, max};
}

int ToupWrapper::hue() const
{
	int hue;
	Toupcam_get_Hue(htoupcam, &hue);
	return hue;
}

void ToupWrapper::setHue(int h)
{
	Toupcam_put_Hue(htoupcam, h);
}

int ToupWrapper::saturation() const
{
	int sat;
	Toupcam_get_Saturation(htoupcam, &sat);
	return sat;
}

void ToupWrapper::setSaturation(int sat)
{
	Toupcam_put_Saturation(htoupcam, sat);
}

int ToupWrapper::brightness() const
{
	int bright;
	Toupcam_get_Brightness(htoupcam, &bright);
	return bright;
}

void ToupWrapper::setBrightness(int bright)
{
	Toupcam_put_Brightness(htoupcam, bright);
}

int ToupWrapper::contrast() const
{
	int cont;
	Toupcam_get_Contrast(htoupcam, &cont);
	return cont;
}

void ToupWrapper::setContrast(int cont)
{
	Toupcam_put_Contrast(htoupcam, cont);
}

int ToupWrapper::gamma() const
{
	int g;
	Toupcam_get_Gamma(htoupcam, &g);
	return g;
}

void ToupWrapper::setGamma(int gamma)
{
	Toupcam_put_Gamma(htoupcam, gamma);
}

bool ToupWrapper::chrome() const
{
	BOOL chr;
	Toupcam_get_Chrome(htoupcam, &chr);
	return chr;
}

void ToupWrapper::setChrome(bool enable)
{
	Toupcam_put_Chrome(htoupcam, enable);
}

bool ToupWrapper::hFlip() const
{
	BOOL flip;
	Toupcam_get_HFlip(htoupcam, &flip);
	return flip;
}

void ToupWrapper::setHFlip(bool flip)
{
	Toupcam_put_HFlip(htoupcam, flip);
}

bool ToupWrapper::vFlip() const
{
	BOOL flip;
	Toupcam_get_VFlip(htoupcam, &flip);
	return flip;
}

void ToupWrapper::setVFlip(bool flip)
{
	Toupcam_put_VFlip(htoupcam, flip);
}

unsigned short ToupWrapper::speed() const
{
	unsigned short spd;
	Toupcam_get_Speed(htoupcam, &spd);
	return spd;
}

void ToupWrapper::setSpeed(unsigned short spd)
{
	Toupcam_put_Speed(htoupcam, spd);
}

int ToupWrapper::frequency() const
{
	int hz;
	Toupcam_get_HZ(htoupcam, &hz);
	return hz;
}

void ToupWrapper::setFrequency(int hz)
{
	Toupcam_put_HZ(htoupcam, hz);
}

bool ToupWrapper::isBinMode() const
{
	BOOL mode;
	Toupcam_get_Mode(htoupcam, &mode);
	return mode;
}

void ToupWrapper::toggleMode()
{
	bool bin = isBinMode();
	Toupcam_put_Mode(htoupcam, !bin);
}

unique_ptr<int> ToupWrapper::whiteBalanceGain() const
{
	unique_ptr<int> gain{ new int[3] };
	Toupcam_get_WhiteBalanceGain(htoupcam, gain.get());
	return gain;
}

void ToupWrapper::setWhiteBalanceGain(unique_ptr<int> gain)
{
	Toupcam_put_WhiteBalanceGain(htoupcam, gain.get());
}

int ToupWrapper::rGain() const
{
	return whiteBalanceGain().get()[0];
}

void ToupWrapper::setRGain(int r)
{
	auto now = whiteBalanceGain();
	now.get()[0] = r;
	setWhiteBalanceGain(move(now));
}

int ToupWrapper::gGain() const
{
	return whiteBalanceGain().get()[1];
}

void ToupWrapper::setGGain(int g)
{
	auto val = whiteBalanceGain();
	val.get()[1] = g;
	setWhiteBalanceGain(move(val));
}

int ToupWrapper::bGain() const
{
	return whiteBalanceGain().get()[2];
}

void ToupWrapper::setBGain(int b)
{
	auto val = whiteBalanceGain();
	val.get()[2] = b;
	setWhiteBalanceGain(move(val));
}

int ToupWrapper::whiteBalanceTemperature() const
{
	int temp;
	Toupcam_get_TempTint(htoupcam, &temp, nullptr);
	return temp;
}

void ToupWrapper::setWhiteBalanceTemperature(int temp)
{
	Toupcam_put_TempTint(htoupcam, temp, whiteBalanceTint());
}

int ToupWrapper::whiteBalanceTint() const
{
	int tint;
	Toupcam_get_TempTint(htoupcam, nullptr, &tint);
	return tint;
}

void ToupWrapper::setWhiteBalanceTint(int tint)
{
	Toupcam_put_TempTint(htoupcam, whiteBalanceTemperature(), tint);
}

QRect ToupWrapper::whiteBalanceRect() const
{
	RECT rect;
	Toupcam_get_AWBAuxRect(htoupcam, &rect);
	return QRect{ QPoint{ rect.top, rect.left }, QPoint{ rect.bottom, rect.right } };
}

void ToupWrapper::setWhiteBalanceRect(const QRect& rect)
{
	RECT r;
	r.top = rect.top();
	r.left = rect.left();
	r.bottom = rect.bottom();
	r.right = rect.right();
	Toupcam_put_AWBAuxRect(htoupcam, &r);
}

QRect ToupWrapper::autoExposureRect() const
{
	RECT rect;
	Toupcam_get_AEAuxRect(htoupcam, &rect);
	return QRect{ QPoint{ rect.top, rect.left }, QPoint{ rect.bottom, rect.right } };
}

void ToupWrapper::setAutoExposureRect(const QRect& rect)
{
	RECT r;
	r.top = rect.top();
	r.left = rect.left();
	r.bottom = rect.bottom();
	r.right = rect.right();
	Toupcam_put_AEAuxRect(htoupcam, &r);
}

bool ToupWrapper::isMono() const
{
	auto hr = Toupcam_get_MonoMode(htoupcam);
	return hr == S_OK;
}

string ToupWrapper::serialNumber() const
{
	unique_ptr<char> sn{ new char[32] };
	Toupcam_get_SerialNumber(htoupcam, sn.get());
	string res = sn.get();
	return res;
}

string ToupWrapper::fwVersion() const
{
	unique_ptr<char> sn{ new char[16] };
	Toupcam_get_FwVersion(htoupcam, sn.get());
	string res = sn.get();
	return res;
}

string ToupWrapper::hwVersion() const
{
	unique_ptr<char> sn{ new char[16] };
	Toupcam_get_HwVersion(htoupcam, sn.get());
	string res = sn.get();
	return res;
}

string ToupWrapper::productionDate() const
{
	unique_ptr<char> sn{ new char[10] };
	Toupcam_get_ProductionDate(htoupcam, sn.get());
	string res = sn.get();
	return res;
}

void ToupWrapper::awbOnePush()
{
	Toupcam_AwbOnePush(htoupcam, nullptr, nullptr);
}

void ToupWrapper::autoEqualize()
{
	Toupcam_LevelRangeAuto(htoupcam);
}