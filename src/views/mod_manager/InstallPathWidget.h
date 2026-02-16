#ifndef INSTALLPATHWIDGET_H
#define INSTALLPATHWIDGET_H

#include <QWidget>
#include <QString>
#include <QFutureWatcher>
#include <QEvent>

class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QTimer;

class InstallPathWidget : public QWidget {
    Q_OBJECT

public:
    explicit InstallPathWidget(QWidget *parent = nullptr);
    ~InstallPathWidget() override = default;

    QString installPath() const;
    void setInstallPath(const QString &path);
    bool isValidPath() const;
    void startAutoDetection();

protected:
    void changeEvent(QEvent *event) override;

signals:
    void pathChanged(const QString &path);
    void validPathSelected(const QString &path);
    void detectionFinished(const QString &path);

private slots:
    void onBrowseClicked();
    void onPathEdited(const QString &text);
    void onDetectionComplete();
    void updateLoadingAnimation();

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();
    void validatePath(const QString &path);
    bool checkFoxholeInstallation(const QString &path) const;
    void setLoadingState(bool loading);

    QLabel *m_titleLabel;
    QLineEdit *m_pathLineEdit;
    QPushButton *m_browseButton;
    QLabel *m_statusLabel;
    QVBoxLayout *m_layout;

    QString m_currentPath;
    bool m_isValid = false;

    QFutureWatcher<QString> *m_detectionWatcher;
    QTimer *m_loadingTimer;
    int m_loadingDots = 0;
};

#endif // INSTALLPATHWIDGET_H
