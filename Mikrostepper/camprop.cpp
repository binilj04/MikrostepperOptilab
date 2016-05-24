#include "stdafx.h"
#include "camprop.h"

#include <functional>
using namespace std;

#include "appsettings.h"

CamProp::CamProp(QObject *parent)
	: QObject(parent), lastParam{ 0 }
{
}

CamProp::~CamProp()
{
	AppSettings a;
	a.saveSettings("CameraLastParams", lastParam);
}

void CamProp::saveParametersA()
{
	saveParameter(0);
}

void CamProp::loadParametersA()
{
	loadParameter(0);
}

void CamProp::saveParametersB()
{
	saveParameter(1);
}

void CamProp::loadParametersB()
{
	loadParameter(1);
}

void CamProp::saveParametersC()
{
	saveParameter(2);
}

void CamProp::loadParametersC()
{
	loadParameter(2);
}

void CamProp::saveParametersD()
{
	saveParameter(3);
}

void CamProp::loadParametersD()
{
	loadParameter(3);
}

void CamProp::reloadParams()
{
	emit hueChanged(hue());
	emit saturationChanged(saturation());
	emit brightnessChanged(brightness());
	emit contrastChanged(contrast());
	emit gammaChanged(gamma());
	emit autoexposureChanged(autoexposure());
	emit aeGainChanged(aeGain());
	emit exposureTimeChanged(exposureTime());
	emit aeTargetChanged(aeTarget());
	emit rGainChanged(rGain());
	emit gGainChanged(gGain());
	emit bGainChanged(bGain());
	emit whiteBalanceTemperatureChanged(whiteBalanceTemperature());
	emit whiteBalanceTintChanged(whiteBalanceTint());
	emit frameRateChanged(frameRate());
	emit isColorChanged(isColor());
	emit isHFlipChanged(isHFlip());
	emit isVFlipChanged(isVFlip());
	emit isBinChanged(isBin());
	emit whiteBalanceBoxChanged(whiteBalanceBox());
}

int CamProp::getCurrentParameterTeam()
{
	AppSettings a;
	return a.readInt("CameraLastParams");
}

class SettingsGenerator
{
	QString id;
public:
	SettingsGenerator(int id)
		: id{ QString::number(id) }
	{	}

	struct SettingsObj
	{
		QString s;
		AppSettings app;
		SettingsObj(const QString& setting) : s{ setting }, app{ } { }
		SettingsObj(const SettingsObj& o) : s{ o.s }, app{ } { }
		SettingsObj(SettingsObj&& o) : s{ std::move(o.s) }, app{} { }

		void operator<<(const QVariant& value)
		{
			app.saveSettings(s, value);
		}
	};

	SettingsObj operator<<(const QString& name)
	{
		return SettingsObj{ gen(name) };
	}

	QString gen(const QString& name)
	{
		return name + "_" + id;
	}
};

void CamProp::saveParameter(int id)
{
	SettingsGenerator sg{ id };
	sg << "hue" << hue();
	sg << "saturation" << saturation();
	sg << "brightness" << brightness();
	sg << "contrast" << contrast();
	sg << "gamma" << gamma();
	sg << "autoexposure" << autoexposure();
	sg << "aeGain" << aeGain();
	sg << "exposureTime" << exposureTime();
	sg << "aeTarget" << aeTarget();
	sg << "rGain" << rGain();
	sg << "gGain" << gGain();
	sg << "bGain" << bGain();
	sg << "whiteBalanceTemperature" << whiteBalanceTemperature();
	sg << "whiteBalanceTint" << whiteBalanceTint();
	sg << "frameRate" << frameRate();
	sg << "isColor" << isColor();
	sg << "isHFlip" << isHFlip();
	sg << "isVFlip" << isVFlip();
	sg << "isBin" << isBin();
	sg << "whiteBalanceBox" << whiteBalanceBox();
}

void CamProp::loadParameter(int id)
{
	SettingsGenerator sg{ id };
	AppSettings a;
	setHue(a.readDouble(sg.gen("hue"), 0.0));
	setSaturation(a.readDouble(sg.gen("saturation"), 128.0));
	setBrightness(a.readDouble(sg.gen("brightness"), 0.0));
	setGamma(a.readDouble(sg.gen("gamma"), 0.0));
	setAutoexposure(a.readBool(sg.gen("autoexposure"), true));
	setAeGain(a.readDouble(sg.gen("aeGain"), 100.0));
	setExposureTime(a.readDouble(sg.gen("exposureTime"), 67000.0));
	setAeTarget(a.readDouble(sg.gen("aeTarget"), 120));
	setRGain(a.readDouble(sg.gen("rGain"), 1.0));
	setGGain(a.readDouble(sg.gen("gGain"), 1.0));
	setBGain(a.readDouble(sg.gen("bGain"), 1.0));
	setWhiteBalanceTemperature(a.readDouble(sg.gen("whiteBalanceTemperature"), 6503.0));
	setWhiteBalanceTint(a.readDouble(sg.gen("whiteBalanceTint"), 1000.0));
	setFrameRate(a.readInt(sg.gen("frameRate"), 0));
	setColorMode(a.readBool(sg.gen("isColor"), true));
	setHFlip(a.readBool(sg.gen("isHFlip"), false));
	setVFlip(a.readBool(sg.gen("isVFlip"), false));
	setSamplingMode(a.readBool(sg.gen("isBin"), true));
	setWhiteBalanceBox(a.readRect(sg.gen("whiteBalanceBox"), QRectF{ 50.0, 50.0, 50.0, 50.0 }).toRect());
	lastParam = id;
}

void CamProp::setLastParam(int id)
{
	lastParam = id;
}

NullCamProp::NullCamProp(QObject* parent)
	: CamProp{ parent }
{

}

double NullCamProp::hue() const
{
	return 0;
}

void NullCamProp::setHue(double) 
{

}

double NullCamProp::saturation() const
{
	return 0;
}

void NullCamProp::setSaturation(double val)
{
}

double NullCamProp::brightness() const
{
	return 0;
}

void NullCamProp::setBrightness(double val)
{
}

double NullCamProp::contrast() const
{
	return 0;
}

void NullCamProp::setContrast(double val)
{
}

double NullCamProp::gamma() const
{
	return 0;
}

void NullCamProp::setGamma(double val)
{
}

bool NullCamProp::autoexposure() const
{
	return false;
}

void NullCamProp::setAutoexposure(bool val)
{
}

double NullCamProp::aeGain() const
{
	return 0;
}

void NullCamProp::setAeGain(double val)
{
}

double NullCamProp::exposureTime() const
{
	return 0;
}

void NullCamProp::setExposureTime(double val)
{
}

double NullCamProp::aeTarget() const
{
	return 0;
}

void NullCamProp::setAeTarget(double val)
{
}

double NullCamProp::maxExposureTime()
{
	return 0;
}

double NullCamProp::rGain() const
{
	return 0;
}

void NullCamProp::setRGain(double val)
{
}

double NullCamProp::gGain() const
{
	return 0;
}

void NullCamProp::setGGain(double val)
{
}

double NullCamProp::bGain() const
{
	return 0;
}

void NullCamProp::setBGain(double val)
{
}

double NullCamProp::whiteBalanceTemperature() const
{
	return 0;
}

void NullCamProp::setWhiteBalanceTemperature(double val)
{
}

double NullCamProp::whiteBalanceTint() const
{
	return 0;
}

void NullCamProp::setWhiteBalanceTint(double val)
{
}

int NullCamProp::frameRate() const
{
	return 0;
}

void NullCamProp::setFrameRate(int fr)
{
}

bool NullCamProp::isColor() const
{
	return true;
}

void NullCamProp::setColorMode(bool mode)
{
}

bool NullCamProp::isHFlip() const
{
	return true;
}

void NullCamProp::setHFlip(bool flip)
{
}

bool NullCamProp::isVFlip() const
{
	return true;
}

void NullCamProp::setVFlip(bool flip)
{
}

bool NullCamProp::isBin() const
{
	return true;
}

void NullCamProp::setSamplingMode(bool mode)
{
}

QRect NullCamProp::whiteBalanceBox() const
{
	return QRect{};
}

void NullCamProp::setWhiteBalanceBox(const QRect& r)
{
}

void NullCamProp::oneShotWB()
{
}

void NullCamProp::loadDefaultParameters()
{
}

CamProp::CameraType NullCamProp::cameraType() const
{
	return Mock;
}

double NullCamProp::controlMin(const QString& control) const
{
	return 0;
}

double NullCamProp::controlMax(const QString& control) const
{
	return 100;
}

bool NullCamProp::controlAvailable(const QString& control) const
{
	return false;
}