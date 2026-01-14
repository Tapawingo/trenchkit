#include "ItchAuth.h"

ItchAuth::ItchAuth(QObject *parent)
    : QObject(parent)
{
}

void ItchAuth::requestApiKey() {
    emit apiKeyRequired();
}
