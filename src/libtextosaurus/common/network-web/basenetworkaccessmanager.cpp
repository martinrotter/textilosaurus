// This file is distributed under GNU GPLv3 license. For full license text, see <project-git-repository-root-folder>/LICENSE.md.

#include "common/network-web/basenetworkaccessmanager.h"

#include "common/miscellaneous/textfactory.h"
#include "saurus/miscellaneous/application.h"

#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>

BaseNetworkAccessManager::BaseNetworkAccessManager(QObject* parent)
  : QNetworkAccessManager(parent) {
  connect(this, &BaseNetworkAccessManager::sslErrors, this, &BaseNetworkAccessManager::onSslErrors);
  loadSettings();
}

void BaseNetworkAccessManager::loadSettings() {
  QNetworkProxy new_proxy;
  const QNetworkProxy::ProxyType selected_proxy_type = static_cast<QNetworkProxy::ProxyType>(qApp->settings()->value(GROUP(Proxy),
                                                                                                                     Proxy::Type,
                                                                                                                     QNetworkProxy::
                                                                                                                     ProxyType::NoProxy).
                                                                                             toInt());

  if (selected_proxy_type == QNetworkProxy::NoProxy) {
    // No extra setting is needed, set new proxy and exit this method.
    setProxy(QNetworkProxy::NoProxy);
  }
  else if (selected_proxy_type == QNetworkProxy::DefaultProxy) {
    setProxy(QNetworkProxy::applicationProxy());
  }
  else {
    const Settings* settings = qApp->settings();

    // Custom proxy is selected, set it up.
    new_proxy.setType(selected_proxy_type);
    new_proxy.setHostName(settings->value(GROUP(Proxy), Proxy::Host).toString());
    new_proxy.setPort(quint16(settings->value(GROUP(Proxy), SETTING(Proxy::Port)).toInt()));
    new_proxy.setUser(settings->value(GROUP(Proxy), Proxy::Username).toString());
    new_proxy.setPassword(settings->value(GROUP(Proxy), Proxy::Password).toString());
    setProxy(new_proxy);
  }

  qDebug().noquote() << QSL("Settings of BaseNetworkAccessManager loaded.");
}

void BaseNetworkAccessManager::onSslErrors(QNetworkReply* reply, const QList<QSslError>& error) {
  qWarning().noquote().nospace() << QSL("Ignoring SSL errors for '")
                                 << reply->url()
                                 << QSL("': '")
                                 << reply->errorString()
                                 << QSL("' (code ")
                                 << reply->error()
                                 << QSL(").");

  reply->ignoreSslErrors(error);
}

QNetworkReply* BaseNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                       const QNetworkRequest& request,
                                                       QIODevice* outgoingData) {
  QNetworkRequest new_request = request;

  // This rapidly speeds up loading of web sites.
  // NOTE: https://en.wikipedia.org/wiki/HTTP_pipelining
  new_request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

  // Setup custom user-agent.
  new_request.setRawHeader(HTTP_HEADERS_USER_AGENT, QString(APP_USERAGENT).toLocal8Bit());
  return QNetworkAccessManager::createRequest(op, new_request, outgoingData);
}
