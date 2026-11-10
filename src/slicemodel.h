#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>

class SliceModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList slices READ slices NOTIFY updated)
public:
    explicit SliceModel(QObject* parent=nullptr);
    QVariantList slices() const {return m_slices;}
    Q_INVOKABLE void refresh();
signals:
    void updated();
private:
    QTimer m_timer;
    QVariantList m_slices;
};
