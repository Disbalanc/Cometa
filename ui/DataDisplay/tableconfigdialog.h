// tableconfigdialog.h
#pragma once
#include <QDialog>
#include <QTreeWidget>
#include <QLineEdit>
#include <QDialogButtonBox>

class TableConfigDialog : public QDialog {
    Q_OBJECT
public:
    TableConfigDialog(const QVector<QPair<QString, QString>>& tablesInfo, QWidget* parent = nullptr);
    QList<QPair<QString, QString>> getSelectedFields() const;

private:
    void setupUI(const QVector<QPair<QString, QString>>& tablesInfo);
    QTreeWidget* treeWidget;
    QLineEdit* aliasEdit;
    QDialogButtonBox* buttonBox;
};
