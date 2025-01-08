#include "schemehandler.h"
#include <QUrl>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>

SchemeHandler::SchemeHandler(QObject* parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

WebEngineView::WebEngineView(SchemeHandler* handler, QWidget* parent) 
    : QWebEngineView(parent), handler(handler) {} 
    
void WebEngineView::closeEvent(QCloseEvent* event) { 
    //qDebug() << "Window close event triggered"; 
    handler->closeWindow(); 
    QWebEngineView::closeEvent(event);
}

void SchemeHandler::requestStarted(QWebEngineUrlRequestJob* request) {

  //      qDebug() << "requestStarted called with URL:" << request->requestUrl();
        QUrl url = request->requestUrl();
        int rc = -1;
        QStringList urlparam = url.query().split('&');
        for (const auto& param : urlparam) {
        QStringList pair = param.split('=');

        if (pair.size() != 2 || pair[0] != QLatin1String("code"))
            continue;

        auto qc = pair[1];
        m_code = qc.toStdString();
        rc = 0;
        foundCode = true;
        qApp->exit(rc);
        break;
    }   
}

void SchemeHandler::handleUrl(const QUrl& url) {
    //qDebug() << "Handling URL:" << url;
    int rc = -1;
    QStringList urlparam = url.query().split('&');
    for (const auto& param : urlparam) {
        QStringList pair = param.split('=');

        if (pair.size() != 2 || pair[0] != QLatin1String("code"))
            continue;

        auto qc = pair[1];
        m_code = qc.toStdString();
        rc = 0;
        foundCode = true;
        qApp->exit(rc);
        break;
    }   
}

std::string SchemeHandler::code() const {
    return m_code;
}

bool SchemeHandler::hasFoundCode() const { 
    return foundCode;
}

void SchemeHandler::closeWindow() { 
    //qDebug() << "Window closed"; 
    windowClosed = true; 
    qApp->exit(0);
}