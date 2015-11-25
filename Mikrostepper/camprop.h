#ifndef CAMPROP_H
#define CAMPROP_H

#include <QObject>

class CamProp : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double hue READ hue WRITE setHue NOTIFY hueChanged)
	Q_PROPERTY(double saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
	Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
	Q_PROPERTY(double contrast READ contrast  WRITE setContrast  NOTIFY contrastChanged)
	Q_PROPERTY(double gamma READ gamma  WRITE setGamma  NOTIFY gammaChanged)
	Q_PROPERTY(bool autoexposure READ autoexposure WRITE setAutoexposure NOTIFY autoexposureChanged)
	Q_PROPERTY(double aeGain READ aeGain WRITE setAeGain NOTIFY aeGainChanged)
	Q_PROPERTY(double exposureTime READ exposureTime WRITE setExposureTime NOTIFY exposureTimeChanged)
	Q_PROPERTY(double aeTarget READ aeTarget WRITE setAeTarget NOTIFY aeTargetChanged)
	Q_PROPERTY(double rGain READ rGain WRITE setRGain NOTIFY rGainChanged)
	Q_PROPERTY(double gGain READ gGain WRITE setGGain NOTIFY gGainChanged)
	Q_PROPERTY(double bGain READ bGain WRITE setBGain NOTIFY bGainChanged)
	Q_PROPERTY(double whiteBalanceTemperature READ whiteBalanceTemperature WRITE setWhiteBalanceTemperature NOTIFY whiteBalanceTemperatureChanged)
	Q_PROPERTY(double whiteBalanceTint READ whiteBalanceTint WRITE setWhiteBalanceTint NOTIFY whiteBalanceTintChanged)
	Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate NOTIFY frameRateChanged)
	Q_PROPERTY(bool isColor READ isColor WRITE setColorMode NOTIFY isColorChanged)
	Q_PROPERTY(bool isHFlip READ isHFlip WRITE setHFlip NOTIFY isHFlipChanged)
	Q_PROPERTY(bool isVFlip READ isVFlip WRITE setVFlip NOTIFY isVFlipChanged)
	Q_PROPERTY(bool isBin READ isBin WRITE setSamplingMode NOTIFY isBinChanged)
	Q_PROPERTY(QRect whiteBalanceBox READ whiteBalanceBox WRITE setWhiteBalanceBox NOTIFY whiteBalanceBoxChanged)
public:
	CamProp(QObject *parent);
	virtual ~CamProp();

	virtual double hue() const = 0;
	virtual void setHue(double) = 0;
	virtual double saturation() const = 0;
	virtual void setSaturation(double val) = 0;
	virtual double brightness() const = 0;
	virtual void setBrightness(double) = 0;
	virtual double contrast() const = 0;
	virtual void setContrast(double val) = 0;
	virtual double gamma() const = 0;
	virtual void setGamma(double val) = 0;

	virtual bool autoexposure() const = 0;
	virtual void setAutoexposure(bool) = 0;
	virtual double aeGain() const = 0;
	virtual void setAeGain(double val) = 0;
	virtual double exposureTime() const = 0;
	virtual void setExposureTime(double) = 0;
	virtual double aeTarget() const = 0;
	virtual void setAeTarget(double) = 0;
	virtual double maxExposureTime() = 0;

	virtual double rGain() const = 0;
	virtual void setRGain(double val) = 0;
	virtual double gGain() const = 0;
	virtual void setGGain(double val) = 0;
	virtual double bGain() const = 0;
	virtual void setBGain(double val) = 0;	
	
	virtual double whiteBalanceTemperature() const = 0;
	virtual void setWhiteBalanceTemperature(double) = 0;
	virtual double whiteBalanceTint() const = 0;
	virtual void setWhiteBalanceTint(double) = 0;

	virtual int frameRate() const = 0;
	virtual void setFrameRate(int) = 0;

	virtual bool isColor() const = 0;
	virtual void setColorMode(bool) = 0;
	virtual bool isHFlip() const = 0;
	virtual void setHFlip(bool) = 0;
	virtual bool isVFlip() const = 0;
	virtual void setVFlip(bool) = 0;
	virtual bool isBin() const = 0;
	virtual void setSamplingMode(bool) = 0;

	virtual QRect whiteBalanceBox() const = 0;
	virtual void setWhiteBalanceBox(const QRect&) = 0;

	enum CameraType {
		Mock, DS, Toup
	};
	Q_ENUMS(CameraType)

signals:
	void hueChanged(double);
	void saturationChanged(double);
	void brightnessChanged(double);
	void contrastChanged(double);
	void gammaChanged(double);
	void autoexposureChanged(bool);
	void aeGainChanged(double);
	void exposureTimeChanged(double);
	void aeTargetChanged(double);
	void rGainChanged(double);
	void gGainChanged(double);
	void bGainChanged(double);
	void whiteBalanceTemperatureChanged(double);
	void whiteBalanceTintChanged(double);
	void frameRateChanged(int);
	void isColorChanged(bool);
	void isHFlipChanged(bool);
	void isVFlipChanged(bool);
	void isBinChanged(bool);
	void whiteBalanceBoxChanged(QRect);

public slots:
	virtual void oneShotWB() = 0;

	virtual void saveParametersA();
	virtual void loadParametersA();
	virtual void saveParametersB();
	virtual void loadParametersB();
	virtual void saveParametersC();
	virtual void loadParametersC();
	virtual void saveParametersD();
	virtual void loadParametersD();

	virtual void loadDefaultParameters() = 0;

	virtual void reloadParams();

	virtual int getCurrentParameterTeam();

	virtual CameraType cameraType() const = 0;

	virtual double controlMin(const QString& control) const = 0;
	virtual double controlMax(const QString& control) const = 0;
	virtual bool controlAvailable(const QString& control) const = 0;

private:
	void saveParameter(int id);
	void loadParameter(int id);

	int lastParam;
};

class NullCamProp : public CamProp
{
	Q_OBJECT
public:
	NullCamProp(QObject* camera);

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
	
	CameraType cameraType() const;

	double controlMin(const QString& control) const;
	double controlMax(const QString& control) const;
	bool controlAvailable(const QString& control) const;

public slots:
	void oneShotWB();

	void loadDefaultParameters();
};

#endif // CAMPROP_H
