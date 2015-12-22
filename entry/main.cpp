//Project
#include "cgt/cgt.h"
#include "cgt/emulatecgt.h"
#include "scenemodel/scenemodel.h"
#include "logger.h"

//STL

//Qt
#include <QCoreApplication>


//Переопределение вывода отладочных сообщений
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QByteArray message = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        LOG(DEBUG) << message.constData();
        break;
    case QtInfoMsg:
        LOG(INFO) << message.constData();
        break;
    case QtWarningMsg:
        LOG(WARNING) << message.constData();
        break;
    case QtCriticalMsg:
        LOG(ERROR) << message.constData();
        break;
    case QtFatalMsg:
        LOG(FATAL) << message.constData();
        abort();
    }
}

INITIALIZE_EASYLOGGINGPP
void initLogger()
{
    QDir makeLogDir;
    makeLogDir.mkdir("logs");
    el::Configurations conf;
    conf.setGlobally(el::ConfigurationType::Filename, "logs/%datetime.log");
    //conf.setGlobally(el::ConfigurationType::Format, "%datetime{%h:%m:%s.%z}:%levshort: %msg");
    conf.setGlobally(el::ConfigurationType::Format, "%msg");
    el::Logger *defaultLogger = el::Loggers::getLogger("default");
    defaultLogger->configure(conf);
    el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
    el::Loggers::addFlag(el::LoggingFlag::DisablePerformanceTrackingCheckpointComparison);
    el::Loggers::addFlag(el::LoggingFlag::DisableVModules);
    el::Loggers::addFlag(el::LoggingFlag::DisableVModulesExtensions);
    qInstallMessageHandler(myMessageOutput);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    initLogger();

    const QString fileName = "CodeGen.dll";
    const QString pathForPackage = "Elements/CNET";
    const QString fullPathCodeGen = pathForPackage + QDir::separator() + fileName;

    //ru Загружаем CodeGen в память
    if (!QFile::exists(fullPathCodeGen)) {
        qCritical(qUtf8Printable(QString("%1 library not found!").arg(fileName)));
    }
    QLibrary libCodeGen(fullPathCodeGen);
    if (libCodeGen.load())
        qInfo(qUtf8Printable(QString("%1 library successfully loaded.").arg(fileName)));
    else
        qCritical(qUtf8Printable(QString("%1 library not found!").arg(fileName)));

    //ru Определение функций кодогенератора
    buildPrepareProc = reinterpret_cast<t_buildPrepareProc>(libCodeGen.resolve("buildPrepareProc"));
    buildProcessProc = reinterpret_cast<t_buildProcessProc>(libCodeGen.resolve("buildProcessProc"));
    checkVersionProc = reinterpret_cast<t_checkVersionProc>(libCodeGen.resolve("CheckVersionProc"));

    QFile file("test.json");
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    SceneModel model;
    model.deserialize(doc);

    EmulateCgt::setSceneModel(&model);

    TBuildProcessRec rec(EmulateCgt::getCgt(), model.getIdRootContainer());
    buildProcessProc(rec);

    return a.exec();
}
