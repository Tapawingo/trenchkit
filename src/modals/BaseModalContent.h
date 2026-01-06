#ifndef BASEMODALCONTENT_H
#define BASEMODALCONTENT_H

#include <QWidget>
#include <QSize>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;

class BaseModalContent : public QWidget {
    Q_OBJECT

public:
    enum Result {
        Rejected = 0,
        Accepted = 1,
        Custom = 2
    };

    explicit BaseModalContent(QWidget *parent = nullptr);
    ~BaseModalContent() override = default;

    Result result() const { return m_result; }

    void setTitle(const QString &title);
    void setHeaderVisible(bool visible);

    void setPreferredSize(const QSize &size) { m_preferredSize = size; }
    QSize preferredSize() const { return m_preferredSize; }

public slots:
    virtual void accept();
    virtual void reject();

signals:
    void accepted();
    void rejected();
    void finished(int result);

protected:
    void showEvent(QShowEvent *event) override;

    QVBoxLayout* bodyLayout() { return m_bodyLayout; }
    QHBoxLayout* footerLayout() { return m_footerLayout; }
    QWidget* headerWidget() { return m_headerWidget; }
    QWidget* bodyWidget() { return m_bodyWidget; }
    QWidget* footerWidget() { return m_footerWidget; }

    void setResult(Result result) { m_result = result; }

private:
    void setupLayout();

    Result m_result = Rejected;
    QLabel *m_titleLabel;
    QWidget *m_headerWidget;
    QWidget *m_bodyWidget;
    QWidget *m_footerWidget;
    QVBoxLayout *m_bodyLayout;
    QHBoxLayout *m_footerLayout;
    QSize m_preferredSize = QSize(500, 400);
};

#endif // BASEMODALCONTENT_H
