#ifndef ITCHAUTH_H
#define ITCHAUTH_H

#include <QObject>
#include <QString>

class ItchAuth final : public QObject {
    Q_OBJECT

public:
    explicit ItchAuth(QObject *parent = nullptr);
    ~ItchAuth() override = default;

public slots:
    void requestApiKey();

signals:
    void apiKeyRequired();
    void apiKeyEntered(QString apiKey);
    void authenticationCancelled();
};

#endif // ITCHAUTH_H
