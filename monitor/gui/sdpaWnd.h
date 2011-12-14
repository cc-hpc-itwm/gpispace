#ifndef SDPA_WND_H
#define SDPA_WND_H

#include <QCustomEvent>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QWidget>
#include <QLabel>
#include <QColor>

//#include "UniversalQt.h"
//#include "Universal.h"
//#include "Datastructs.h"
enum STATE_WND
{
  STATE_RUN = 1,
  STATE_FAILED = 2,
  STATE_OK = 3,
  STATE_CREATE = 4,
  STATE_MAX = 0xffffffff
};

typedef struct
{
  int id;
    enum { max_len = 1024 };
  char name[max_len];
    char info[max_len];
  STATE_WND state;
}WndUpdateParameter;




class SdpaWnd : public QWidget
{
 Q_OBJECT
  public:
    SdpaWnd(QWidget *parent = 0);
    virtual ~SdpaWnd(){};

  public slots:
    void resetSlot();
	void updateParallelActivities(int);
  signals:
    void numParallelActivitiesChanged(int);

  private:
    int calculateParallelActivities();

    bool event( QEvent * e );
    QPushButton *m_pResetB;
    QGridLayout *m_pGridL;
    QGridLayout *m_pLastGL;
    QScrollArea *m_pScrollArea;
    QCheckBox *m_cbAutoFollow;
    QCheckBox *m_cbEnable;
    QWidget *m_pWidget;
    QLabel *m_pParActLabel;
    QLabel *m_pActLabel;
    int m_nFirstID, m_nCounter;
    int m_nMinHeight, m_nMinWidth;
    int m_nNbColumn;
    QColor m_ColorCreate, m_ColorRun, m_ColorOk, m_ColorFailed;
};

#endif
