// tableconfigdialog.cpp
#include "tableconfigdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QRegularExpression>
#include <QGroupBox>
#include <QFormLayout>

TableConfigDialog::TableConfigDialog(const QVector<QPair<QString, QString>>& tablesInfo, QWidget* parent)
    : QDialog(parent)
{
    setupUI(tablesInfo);
}

void TableConfigDialog::setupUI(const QVector<QPair<QString, QString>>& tablesInfo)
{
    auto* layout = new QVBoxLayout(this);
    treeWidget = new QTreeWidget(this);
    treeWidget->setHeaderHidden(true);
    treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    // Парсинг информации о таблицах с улучшенным извлечением полей
    for (const auto& table : tablesInfo) {
        QTreeWidgetItem* tableItem = new QTreeWidgetItem(treeWidget);
        tableItem->setText(0, table.first);
        tableItem->setData(0, Qt::UserRole, table.first);

        // Используем регулярное выражение для извлечения имен полей
        QRegularExpression re("\\b(\\w+)\\s+[\\w\\(\\)]+,");
        QRegularExpressionMatchIterator i = re.globalMatch(table.second);

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString fieldName = match.captured(1);

            QTreeWidgetItem* fieldItem = new QTreeWidgetItem(tableItem);
            fieldItem->setText(0, fieldName);
            fieldItem->setData(0, Qt::UserRole, table.first + "." + fieldName);
            fieldItem->setFlags(fieldItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
            fieldItem->setCheckState(0, Qt::Unchecked);
        }
    }
    treeWidget->expandAll();

    // Группа для редактирования псевдонимов
    QGroupBox* aliasGroup = new QGroupBox("Редактирование псевдонимов", this);
    QFormLayout* aliasLayout = new QFormLayout(aliasGroup);
    aliasEdit = new QLineEdit(this);
    aliasLayout->addRow("Псевдоним:", aliasEdit);

    // Кнопки
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(treeWidget);
    layout->addWidget(aliasGroup);
    layout->addWidget(buttonBox);

    // Обновляем псевдоним при выборе элемента
    connect(treeWidget, &QTreeWidget::itemSelectionChanged, [this]() {
        auto items = treeWidget->selectedItems();
        if (items.size() == 1 && items.first()->childCount() == 0) {
            aliasEdit->setEnabled(true);
            aliasEdit->setText(items.first()->text(0));
        } else {
            aliasEdit->clear();
            aliasEdit->setEnabled(false);
        }
    });

    // Сохраняем псевдоним при редактировании
    connect(aliasEdit, &QLineEdit::editingFinished, [this]() {
        auto items = treeWidget->selectedItems();
        if (items.size() == 1) {
            items.first()->setText(0, aliasEdit->text());
        }
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QList<QPair<QString, QString>> TableConfigDialog::getSelectedFields() const
{
    QList<QPair<QString, QString>> result;
    QTreeWidgetItemIterator it(treeWidget);

    while (*it) {
        if ((*it)->childCount() == 0 && (*it)->checkState(0) == Qt::Checked) {
            QString fullName = (*it)->data(0, Qt::UserRole).toString();
            QString alias = (*it)->text(0); // Используем отредактированное имя
            result.append({fullName, alias});
        }
        ++it;
    }
    return result;
}
