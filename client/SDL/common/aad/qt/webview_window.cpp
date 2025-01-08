#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QDebug>
#include <iostream>
#include <vector>
#include <string>
#include <freerdp/log.h>
#include <freerdp/build-config.h>
#include "schemehandler.h"

#define TAG CLIENT_TAG("sdl.webview")


bool webview_impl_run(const std::string& title, const std::string& url, std::string& code) {
    int argc = 1;
    std::vector<char> wtitle(title.begin(), title.end());
    wtitle.push_back('\0');
    char* argv[] = { wtitle.data(), nullptr };

    const auto vendor = QLatin1String(FREERDP_VENDOR_STRING);
    const auto product = QLatin1String(FREERDP_PRODUCT_STRING);
    QWebEngineUrlScheme::registerScheme(QWebEngineUrlScheme("ms-appx-web"));

    QCoreApplication::setOrganizationName(vendor);
    QCoreApplication::setApplicationName(product);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);
    //qDebug() << QString::fromStdString(url);

    SchemeHandler handler;
    QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("ms-appx-web", &handler);

    WebEngineView webview(&handler);
    QObject::connect(&webview, &QWebEngineView::urlChanged, [&handler](const QUrl &url) { 
      //qDebug() << "URL changed to:" << url;
      if (url.scheme() == "https" || url.scheme() == "ms-appx-web") { 
        handler.handleUrl(url); 
      }
    });
    webview.load(QUrl(QString::fromStdString(url)));
    webview.show();

    while (!handler.hasFoundCode() && !handler.windowClosed) { 
      app.processEvents(); 
    }

    auto val = handler.code();
    if (val.empty())
        return false;
    code = val;

    return !code.empty();
}

int main(int argc, char *argv[]) { 
    if (argc != 3) { 
        std::cerr << "Usage: " << argv[0] << " <title> <url>" << std::endl; 
        return 1;
    }

    std::string code;
    std::string title = argv[1]; 
    std::string url = argv[2];

    bool rc = webview_impl_run(title, url, code);

    if (!rc || code.empty())
		  return false;

    std::cout << code.data() << std::endl;    
}
