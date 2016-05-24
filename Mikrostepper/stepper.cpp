#include "stdafx.h"
#include "stepper.h"
#include "appsettings.h"
using namespace std;

//Stepper
Stepper::Stepper(QObject *parent)
    : QObject(parent)
{
}

Stepper::~Stepper()
{
}

void Stepper::setSpeed(double spd) {
    m_speed = spd;
}

// MockStepper
MockStepper::MockStepper(QObject* parent)
    : Stepper(parent), m_z(0), m_bufferSize(14),
      m_bufferFree(14), m_ztarget(0)
{
    m_limit = std::bitset<8>(true);
    eventDriver = new QTimer(this);
    eventDriver->setInterval(1);
    connect(eventDriver, &QTimer::timeout, this, &MockStepper::updateStatus);
    eventDriver->start();
    m_available = true;
}

MockStepper::~MockStepper() {
}

void MockStepper::setBufferFree(int val) {
    if (m_bufferFree != val) {
        m_bufferFree = val;
        emit bufferFreeChanged(val);
    }
}

void MockStepper::updateStatus() {
    auto incr = [](const double &now, const double &target) -> double {
        double res = now;
        if (target > now)
            return res + .001;
        return res - .001;
    };
    if (m_target != m_pos) {
        if (m_target.x() != m_pos.x()) {
            auto npos = incr(m_pos.x(), m_target.x());
            m_pos.setX(npos);
            emit xChanged(npos);
        }
        if (m_target.y() != m_pos.y()) {
            auto npos = incr(m_pos.y(), m_target.y());
            m_pos.setY(npos);
            emit yChanged(npos);
        }
        emit xyChanged(m_pos);

        if (fabs(m_pos.x() - m_target.x()) <= .01 &&
                fabs(m_pos.y() - m_target.y()) <= .01) {
            stop();
            if (m_bufferFree == m_bufferSize)
                emit bufferFull();
        }
    }
    if (m_ztarget != m_z) {
        auto npos = incr(m_z, m_ztarget);
        m_z = npos;
        emit zChanged(npos);
    }
}

void MockStepper::setTarget(const QPointF& ntarget) {
    eventDriver->stop();
    m_target = ntarget;
    setBufferFree(bufferFree() - 1);
    eventDriver->start();
}

void MockStepper::setTarget(double x, double y) {
    double xx, yy;
    xx = (x < 0) ? m_pos.x() : x;
    yy = (y < 0) ? m_pos.y() : y;
    setTarget(QPointF(xx, yy));
}

void MockStepper::stop() {
    eventDriver->stop();
    m_target = m_pos;
    setBufferFree(bufferSize());
    emit xChanged(m_pos.x());
    emit yChanged(m_pos.y());
    emit xyChanged(m_pos);
}

void MockStepper::moveX(double dist) {
    if (bufferFree() == bufferSize()) {
        setTarget(m_pos.x() + dist, -1);
    }
}

void MockStepper::moveY(double dist) {
    if (bufferFree() == bufferSize()) {
        setTarget(-1, m_pos.y() + dist);
    }
}

void MockStepper::moveZ(double dist) {
    if (bufferFree() == bufferSize()) {
        m_ztarget = m_z + dist;
    }
}

void MockStepper::moveTo(const QPointF& npos) {
    if (bufferFree() == bufferSize()) {
        setTarget(npos);
    }
}

void MockStepper::moveToZ(const double& npos) {

}

void MockStepper::setZeroX() {
    m_pos.setX(0.0);
    updateStatus();
}

void MockStepper::setZeroY() {
    m_pos.setY(0.0);
    updateStatus();
}

void MockStepper::setZeroZ() {
    m_z = 0.0;
    updateStatus();
}

#include "api_wrapper.h"

CNCStepper::CNCStepper(QObject* parent)
	: Stepper(parent), m_limit{0}
{
	statusUpdater = new QTimer(this);
	connect(statusUpdater, &QTimer::timeout, this, &CNCStepper::updateStatus);
	statusUpdater->start(1);
	m_isActive = init();
	m_limit[7] = false;
	powerStat(false);
}

CNCStepper::~CNCStepper()
{
	deinit();
}

void __cdecl ControllerEventCallBack() { }

bool CNCStepper::init() {
	cncAPI = new CEventHandler;
	cncAPI->pEvtCallBack = ControllerEventCallBack;
	HRESULT hr = cncAPI->mObject.CreateInstance(__uuidof(CNC));
	if (cncAPI->mObject == 0) {
		delete cncAPI;
		cncAPI = NULL;
		return false;
	}
	cncAPI->ConnectSink();

	long result;
	ICoord* pCoord = 0;

	hr = CoInitialize(NULL);

	result = cncAPI->mObject->Initialize();
	if (result == 0) {
		return false;
	}

	connect(this, &CNCStepper::limit7Changed, this, &CNCStepper::powerStat);

	ControllerEventCallBack();

	m_bufferSize = cncAPI->mObject->GetBufferSize();
	return true;
}

void CNCStepper::deinit() {
	cncAPI->DisconnectSink();
	cncAPI->mObject->Dispose();
	cncAPI->mObject->Release();
	cncAPI->mObject = 0;
}

void CNCStepper::powerStat(bool stat) {
	if (!stat) {
		stop();
		m_isActive = false;
	}
	else
		m_isActive = true;
}

void CNCStepper::updateStatus() {
	ICoord *coord = 0;
	coord = cncAPI->mObject->GetPosition();
	double _x = coord->GetX();
	double _y = coord->GetY();
	double _z = coord->GetZ();
	if (m_x != _x) {
		m_x = _x;
		emit xChanged(m_x);
	}
	if (m_y != _y) {
		m_y = _y;
		emit yChanged(m_y);
	}
	emit xyChanged(QPointF(m_x, m_y));
	if (m_z != _z) {
		m_z = _z;
		emit zChanged(m_z);
	}
	coord->Release();
	coord = NULL;

	long _bufferFree = cncAPI->mObject->GetBufferFree();
	if (m_bufferFree != _bufferFree) {
		m_bufferFree = _bufferFree;
		emit bufferFreeChanged(m_bufferFree);
		if (m_bufferFree == m_bufferSize) {
			emit bufferFull();
		}
	}

	m_output = cncAPI->mObject->GetOutput();
	m_jog = cncAPI->mObject->GetJog();

	long lim = cncAPI->mObject->GetLimit();
	std::bitset<8> limit = lim;

	for (int n = 0; n < 8; ++n) {
		if (m_limit[n] != limit[n]) {
			m_limit[n] = limit[n];
			if (n == 0)
				emit limit0Changed(m_limit[0]);
			else if (n == 1)
				emit limit1Changed(m_limit[1]);
			else if (n == 2)
				emit limit2Changed(m_limit[2]);
			else if (n == 3)
				emit limit3Changed(m_limit[3]);
			else if (n == 4)
				emit limit4Changed(m_limit[4]);
			else if (n == 5)
				emit limit5Changed(m_limit[5]);
			else if (n == 6)
				emit limit6Changed(m_limit[6]);
			else if (n == 7)
				emit limit7Changed(m_limit[7]);
		}
	}
}

void CNCStepper::setZeroX() {
	ICoord* coord = 0;
	coord = cncAPI->mObject->GetPosition();
	coord->X = 0.0;
	cncAPI->mObject->SendSetPos(coord);
	coord->Release();
	coord = nullptr;

	//internalCount();
}

void CNCStepper::setZeroY() {
	ICoord* coord = 0;
	coord = cncAPI->mObject->GetPosition();
	coord->Y = 0.0;
	cncAPI->mObject->SendSetPos(coord);
	coord->Release();
	coord = nullptr;

	//internalCount();
}

void CNCStepper::setZeroZ() {
	ICoord* coord = 0;
	coord = cncAPI->mObject->GetPosition();
	coord->Z = 0.0;
	cncAPI->mObject->SendSetPos(coord);
	coord->Release();
	coord = nullptr;

	//internalCount();
}

void CNCStepper::stop() {
	cncAPI->mObject->SendStop();

	//internalCount();
}

void CNCStepper::moveX(double dist) {
	if (!m_isActive) return;
	cncAPI->mObject->SendMoveDeltaAxis(AxisEnum_X, dist, m_speed, UnitsEnum_Millimeters);

	//internalCount();
}

void CNCStepper::moveY(double dist) {
	if (!m_isActive) return;
	cncAPI->mObject->SendMoveDeltaAxis(AxisEnum_Y, dist, m_speed, UnitsEnum_Millimeters);

	//internalCount();
}

void CNCStepper::moveZ(double dist) {
	if (!m_isActive) return;
	cncAPI->mObject->SendMoveDeltaAxis(AxisEnum_Z, dist, m_speed, UnitsEnum_Millimeters);

	//internalCount();
}

void CNCStepper::moveTo(const QPointF& npos) {
	if (!m_isActive) return;
	ICoord* coord = 0;
	coord = cncAPI->mObject->GetPosition();
	coord->X = npos.x();
	coord->Y = npos.y();
	cncAPI->mObject->SendMovePos(coord, m_speed);
	coord->Release();
	coord = nullptr;

	//internalCount();
}

void CNCStepper::moveToZ(const double& nz) {
	if (!m_isActive) return;
	ICoord* coord = 0;
	coord = cncAPI->mObject->GetPosition();
	coord->Z = nz;
	cncAPI->mObject->SendMovePos(coord, m_speed);
	coord->Release();
	coord = nullptr;

	//internalCount();
}

void CNCStepper::internalCount()
{
	//auto valid = cncAPI->mObject->GetLicenseValid();
	//if (!valid)
	//{
		++_cnt;
		if (_cnt > 24)
		{
			//auto now = chrono::system_clock::now();
			deinit();
			init();
			//auto duration = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - now);
			//cout << "Re-init time: " << duration.count() << endl;
			_cnt = 0;
		}
	//}
}