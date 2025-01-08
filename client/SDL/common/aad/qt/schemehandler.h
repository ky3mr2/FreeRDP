#ifndef SCHEMEHANDLER_H
#define SCHEMEHANDLER_H

#include <QObject>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineView>

class SchemeHandler : public QWebEngineUrlSchemeHandler {
    Q_OBJECT

public:
    explicit SchemeHandler(QObject* parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob* request) override;
    void handleUrl(const QUrl& url);
    bool hasFoundCode() const;
    void closeWindow();
    bool windowClosed{false};
    [[nodiscard]] std::string code() const;    

private:
    std::string m_code{};
    bool foundCode{false};
};

class WebEngineView : public QWebEngineView { 
  Q_OBJECT 

public: 
    WebEngineView(SchemeHandler* handler, QWidget* parent = nullptr);

protected: 
    void closeEvent(QCloseEvent* event) override;
    
private: 
    SchemeHandler* handler;

};
#endif // SCHEMEHANDLER_H
