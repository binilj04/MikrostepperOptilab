#include "stdafx.h"
#include "optilabviewer.h"

#include <thread>
#include <future>

OptilabViewer::OptilabViewer(Camera *parent)
    : QObject(parent), m_camera(parent), en_recording(WaitForFile)
{
    //Set working directory:
    QString tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).at(0) + "/Optilab/";
    QDir workdir(tempPath);
    if (workdir.exists())
        QDir::setCurrent(tempPath);
    else {
        workdir.mkpath(workdir.path());
        QDir::setCurrent(workdir.path());
    }
    auto list = workdir.entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    for (auto file : list)
		workdir.remove(file);
	connect(&m_camera->recorder, &BaseRecorder::timestatus, this, &OptilabViewer::recordingTime);

	connect(m_camera, &Camera::captureReady, 
		[this](const QString& fn) { emit captureReady(QUrl::fromLocalFile(fn)); });

	m_timer.setTimerType(Qt::PreciseTimer);
	connect(&m_timer, &QTimer::timeout, this, &OptilabViewer::preciseTimerTriggered);
}

OptilabViewer::~OptilabViewer()
{
}

QSize OptilabViewer::calculateAspectRatio(int screenWidth, int screenHeight) const {
    QSize screenSize{ screenWidth, screenHeight };
    auto sourceSize = m_camera->size();
    double aspectRatio = 1.0 * sourceSize.width() / sourceSize.height();
    double screenRatio = 1.0 * screenSize.width() / screenSize.height();
    int frameWidth, frameHeight;
    int hOffset, vOffset;
    hOffset = vOffset = 0;
    if (aspectRatio > screenRatio) {
        frameWidth = screenSize.width();
        frameHeight = 1.0 * screenSize.width() / aspectRatio;
        vOffset = (screenSize.height() - frameHeight) / 2;
    }
    else {
        frameWidth = 1.0 * screenSize.height() * aspectRatio;
        frameHeight = screenSize.height();
        hOffset = (screenSize.width() - frameWidth) / 2;
    }
    return QSize{ hOffset, vOffset };
}

QUrl OptilabViewer::captureToTemp(const QString& imgName) {
    QString appPath = QDir::currentPath();
    QString img = appPath + "/" + imgName;
    if (QFile::exists(img))
        QFile::remove(img);
    m_camera->capture(0, img); // 0 = Highest, 2 = Lowest
    return QUrl::fromLocalFile(img);
}

QUrl OptilabViewer::saveToTemp(const QString& imgName) {
	QString appPath = QDir::currentPath();
	QString img = appPath + "/" + imgName;
	if (QFile::exists(img))
		QFile::remove(img);
	m_camera->saveBuffer(img);
	return QUrl::fromLocalFile(img);
}

void OptilabViewer::copyFromTemp(const QString &imgName, const QUrl &fullPath) {
    if (QFile::exists(fullPath.toLocalFile()))
        QFile::remove(fullPath.toLocalFile());
    QFile::copy(imgName, fullPath.toLocalFile());
}

void OptilabViewer::copyToFolder(const QUrl &image, const QUrl &folder) {
    QString img = image.toLocalFile();
    QString path = folder.toLocalFile();
    QFileInfo imfile(img);
    path += "/" + imfile.fileName();
	//qDebug() << path;
	if (QFile::exists(path))
		bool suc = QFile::remove(path);
    QFile::copy(img, path);
}

void OptilabViewer::scaleImage(const QString& image, int w, int h, Qt::AspectRatioMode ar, Qt::TransformationMode md) 
{
	QImage im{ image };
	if (im.isNull()) return;
	auto img = im.scaled(w, h, ar, md);
	QFile::remove(image);
	img.save(image);
}

QSize OptilabViewer::imageSize(const QString& image)
{
	QImage im{ image };
	return im.size();
}

void OptilabViewer::addCommand(Command cmd) {
    commandPool.push(cmd);
}

void OptilabViewer::nextCommand() {
    if (!commandPool.empty()) {
        auto cmd = commandPool.front();
        cmd();
        commandPool.pop();
        emit remainingCommand(commandPool.size());
    }
}

void OptilabViewer::addCaptureCommand(const QString &imgName) {
    addCommand([=]() { saveToTemp(imgName); nextCommand(); });
}

void OptilabViewer::addWaitCommand(int msecond) {
    addCommand([=]() { QTimer::singleShot(msecond, this, SLOT(nextCommand())); });
}

using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::milliseconds;

void OptilabViewer::addCaptureWaitCommand(const QString &imgName, int msecond){
    addCommand([=]() {
		auto start = Clock::now();
        emit imageSaved(saveToTemp(imgName).toLocalFile());
		auto end = Clock::now();
		auto elapsed = std::chrono::duration_cast<Ms>(end - start);
		auto dur = (msecond - elapsed.count() <= 1) ? 1 : msecond - elapsed.count();
        QTimer::singleShot(dur, this, SLOT(nextCommand()));
    });
}

void OptilabViewer::flushCommands() {
    while (!commandPool.empty())
        commandPool.pop();
}

QStringList OptilabViewer::startSerialCapture(int interval, int fcount) {
    QStringList namelist;
    for (int i = 0; i < fcount; ++i) {
        QString fn = QString("IMG_%1.png").arg(i, 4, 10, QChar('0'));
        addCaptureWaitCommand(fn, interval);
        namelist.append(fn);
    }
    nextCommand();
    return namelist;
}

QStringList OptilabViewer::startSerialCaptureAsync(int interval, int fcount) {
	QStringList namelist;
	for (int i = 0; i < fcount; ++i) {
		QString fn = QString("IMG_%1.png").arg(i, 4, 10, QChar('0'));
		namelist.append(fn);
	}
	auto sc = [=](int itr, int fc) {
			auto msecond = Ms(itr);
		for (int i = 0; i < fc; ++i) {
			auto start = Clock::now();
			emit imageSaved(saveToTemp(namelist.at(i)).toLocalFile());
			auto end = Clock::now();
			auto elapsed = std::chrono::duration_cast<Ms>(end - start);
			auto dur = msecond - elapsed;
			if (dur.count() < 1) dur = Ms(1);
			std::this_thread::sleep_for(dur);
		}
	};
	std::async(std::launch::async, sc, interval, fcount);
	return namelist;
}

void OptilabViewer::captureAsync(const QUrl& file)
{
	std::async(std::launch::async, [this](const QUrl& f){ capture(f); }, file);
}

void OptilabViewer::capture(const QUrl& file)
{
	if (QFile::exists(file.toLocalFile()))
		QFile::remove(file.toLocalFile());
	m_camera->saveBuffer(file.toLocalFile());
	emit imageSaved(file.toLocalFile());
}

void OptilabViewer::initRecorder(const QUrl &video) {
    m_camera->recorder.initRecorder(video.toLocalFile());
	m_camera->recorder.start();
    en_recording = Recording;
    emit recordingStatusChanged();
}

void OptilabViewer::pauseRecording() {
	m_camera->recorder.pause();
    en_recording = Paused;
    emit recordingStatusChanged();
}

void OptilabViewer::resumeRecording() {
	m_camera->recorder.start();
    en_recording = Recording;
    emit recordingStatusChanged();
}

void OptilabViewer::stopRecording() {
	m_camera->recorder.stop();
    en_recording = WaitForFile;
    emit recordingStatusChanged();
}

QUrl OptilabViewer::fromLocalFile(const QString& localfile)
{
	return QUrl::fromLocalFile(localfile);
}

bool OptilabViewer::scRunning() const
{
	return m_timer.isActive();
}

void OptilabViewer::startPreciseTimer(int duration)
{
	m_timer.start(duration);
	emit preciseTimerTriggered();
	emit scRunningChanged(true);
}

void OptilabViewer::stopPreciseTimer()
{
	m_timer.stop();
	emit scRunningChanged(false);
}

QString OptilabViewer::currentPath()
{
	return QDir::currentPath();
}

QString OptilabViewer::homePath()
{
	return QDir::homePath();
}

QString OptilabViewer::tempPath()
{
	return QDir::tempPath();
}

QString OptilabViewer::rootPath()
{
	return QDir::rootPath();
}