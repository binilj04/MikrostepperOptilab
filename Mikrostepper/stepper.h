#ifndef STEPPER_H
#define STEPPER_H

#include <QObject>

class Stepper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double x READ x NOTIFY xChanged)
	Q_PROPERTY(double y READ y NOTIFY yChanged)
	Q_PROPERTY(double z READ z NOTIFY zChanged)
	Q_PROPERTY(int bufferFree READ bufferFree NOTIFY bufferFreeChanged)
	Q_PROPERTY(bool limit0 READ limit0 NOTIFY limit0Changed)
	Q_PROPERTY(bool limit1 READ limit0 NOTIFY limit1Changed)
	Q_PROPERTY(bool limit2 READ limit0 NOTIFY limit2Changed)
	Q_PROPERTY(bool limit3 READ limit0 NOTIFY limit3Changed)
	Q_PROPERTY(bool limit4 READ limit0 NOTIFY limit4Changed)
	Q_PROPERTY(bool limit5 READ limit0 NOTIFY limit5Changed)
	Q_PROPERTY(bool limit6 READ limit0 NOTIFY limit6Changed)
    Q_PROPERTY(bool limit7 READ limit0 NOTIFY limit7Changed)
public:
	Stepper(QObject *parent = 0);
	~Stepper();

    virtual bool isAvailable() = 0;

	virtual double x() = 0;
	virtual double y() = 0;
	virtual double z() = 0;

    virtual int bufferFree() = 0;

	virtual bool limitAt(int id) { Q_UNUSED(id); return false; }

	bool limit0() { return limitAt(0); }
	bool limit1() { return limitAt(1); }
	bool limit2() { return limitAt(2); }
	bool limit3() { return limitAt(3); }
	bool limit4() { return limitAt(4); }
	bool limit5() { return limitAt(5); }
	bool limit6() { return limitAt(6); }
	bool limit7() { return limitAt(7); }

public slots:
    virtual int bufferSize() = 0;
    virtual void stop() = 0;

	virtual void moveX(double) = 0;
	virtual void moveY(double) = 0;
	virtual void moveZ(double) = 0;

	virtual void moveTo(const QPointF&) = 0;
	virtual void moveToZ(const double&) = 0;

    void setSpeed(double);

    virtual void setZeroX() = 0;
    virtual void setZeroY() = 0;
    virtual void setZeroZ() = 0;

signals:
	void xChanged(double x);
	void yChanged(double y);
	void zChanged(double z);
	void xyChanged(const QPointF& xy);

	void bufferFreeChanged(int buffer);
    void bufferFull();
	
	void limit0Changed(bool limit);
	void limit1Changed(bool limit);
	void limit2Changed(bool limit);
	void limit3Changed(bool limit);
	void limit4Changed(bool limit);
	void limit5Changed(bool limit);
	void limit6Changed(bool limit);
	void limit7Changed(bool limit);

protected:
    double m_speed;
};

class MockStepper : public Stepper
{
	Q_OBJECT
	double m_z, m_ztarget;
	int m_bufferFree, m_bufferSize;
    std::bitset<8> m_limit;

	QPointF m_target, m_pos;
	QTimer* eventDriver;
    bool m_available = false;
public:
	MockStepper(QObject* parent = 0);
	~MockStepper();

    bool isAvailable() { return m_available; }

	double x() { return m_pos.x(); }
	double y() { return m_pos.y(); }
	double z() { return m_z; }
    int bufferFree() { return m_bufferFree; }

	bool limitAt(int id) override { return m_limit[id]; }

public slots:
    int bufferSize() { return m_bufferSize; }
    void stop();

	void moveX(double);
	void moveY(double);
	void moveZ(double);

	void moveTo(const QPointF& npos);
	void moveToZ(const double&);

	void updateStatus();

    void setZeroX();
    void setZeroY();
    void setZeroZ();

private:
	void setTarget(const QPointF&);
	void setTarget(double x, double y);
    void setBufferFree(int);
};

class CEventHandler;
class CNCStepper : public Stepper
{
	Q_OBJECT
	double m_x, m_y, m_z;
	long m_output, m_bufferFree, m_bufferSize, m_jog;
	std::bitset<8> m_limit;
	bool m_isActive = false;
	QTimer* statusUpdater;
	CEventHandler* cncAPI;
	size_t _cnt = 0;
public:
	CNCStepper(QObject* parent = 0);
	~CNCStepper();

	bool isAvailable() { return m_isActive; }

	double x() { return m_x; }
	double y() { return m_y; }
	double z() { return m_z; }
	int bufferFree() { return m_bufferFree; }

	bool limitAt(int id) override { return m_limit[id]; }

public slots:
	int bufferSize() { return m_bufferSize; }
	void stop();

	void moveX(double);
	void moveY(double);
	void moveZ(double);

	void moveTo(const QPointF& npos);
	void moveToZ(const double&);

	void setZeroX();
	void setZeroY();
	void setZeroZ();

	void updateStatus();

	void powerStat(bool stat);

private:
	bool init();
	void deinit();

	void internalCount();
};
#endif // STEPPER_H
