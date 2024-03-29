#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class window; }
QT_END_NAMESPACE

class window : public QWidget
{
    Q_OBJECT

public:
    window(QWidget *parent = nullptr);
    ~window();

private slots:
    void on_exitButton_clicked();

    void on_connectButton_clicked();

    void on_ReadBtn_clicked();

    void on_WriteBtn_clicked();

    void on_SellBtn_clicked();

    void on_LoadBtn_clicked();

private:
    Ui::window *ui;
};
#endif // WINDOW_H
