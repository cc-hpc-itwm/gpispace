#include "sdpaWnd.h"
#include "CostumerEvent.h"

SdpaWnd::SdpaWnd( QWidget *parent ) : QWidget(parent)
{
  m_parallelActivities = 0;
  m_nMinHeight = 20;
  m_nMinWidth = 150;
  m_nNbColumn = 4;

  m_ColorCreate = QColor( 155, 155, 255 );
  m_ColorRun = QColor( 255, 255, 0 );
  m_ColorOk = QColor( 0, 255, 0 );
  m_ColorFailed = QColor( 255, 0, 0 );;

  m_pScrollArea = new QScrollArea();
  m_pScrollArea->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  m_pResetB = new QPushButton( "Reset" );

  m_pWidget = new QWidget();
  m_pWidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
  m_pWidget->setMinimumHeight( 200 );

  m_pLastGL = new QGridLayout();
  m_pLastGL->setRowMinimumHeight( 0, m_nMinHeight );

  m_pGridL = new QGridLayout();
  m_pGridL->setRowMinimumHeight( -1, m_nMinHeight );

  for( int i = 0; i< m_nNbColumn; i++)
  {
    m_pLastGL->setColumnMinimumWidth( i, m_nMinWidth );
    m_pGridL->setColumnMinimumWidth( i, m_nMinWidth );
  }

  m_pParActLabel = (new QLabel ("# parallel activities: ") );
  m_pLastGL->addWidget( m_pParActLabel, 0, m_nNbColumn );
  m_pLastGL->setColumnStretch( m_nNbColumn, 1 );

  m_pGridL->setVerticalSpacing ( 0 );
  m_pGridL->setMargin ( 0 );

  m_pWidget->setLayout( m_pGridL );
  m_pScrollArea->setWidget( m_pWidget );

  // legend
  //  QLabel *lCreate = new QLabel();
  //  lCreate->setFixedSize( 20, 20 );
  //  QPixmap *p = new QPixmap(20, 20);
  //  p->fill( m_ColorCreate );
  //  lCreate->setPixmap( *p );
  QPixmap *p = 0;

  QLabel *lRun = new QLabel();
  lRun->setFixedSize( 20, 20 );
  p = new QPixmap(20, 20);
  p->fill( m_ColorRun );
  lRun->setPixmap( *p );

  QLabel *lOk = new QLabel();
  lOk->setFixedSize( 20, 20 );
  p = new QPixmap(20, 20);
  p->fill( m_ColorOk );
  lOk->setPixmap( *p );

  QLabel *lFailed = new QLabel();
  lFailed->setFixedSize( 20, 20 );
  p = new QPixmap(20, 20);
  p->fill( m_ColorFailed );
  lFailed->setPixmap( *p );

  m_cbAutoFollow  = new QCheckBox("auto follow");
  m_cbAutoFollow->setChecked(true);

  QGridLayout *g = new QGridLayout();
  g->addWidget( m_pScrollArea, 0, 0, 6, 1 );
  g->addWidget( m_pResetB, 0, 1, 1, 2 );
  g->addWidget( m_cbAutoFollow, 1, 1 );
  //  g->addWidget( lCreate, 1, 1 );
  //  g->addWidget( new QLabel( "create" ), 1, 2 );
  g->addWidget( lRun, 2, 1 );
  g->addWidget( new QLabel( "run" ), 2, 2 );
  g->addWidget( lOk, 3, 1 );
  g->addWidget( new QLabel( "success" ), 3, 2 );
  g->addWidget( lFailed, 4, 1 );
  g->addWidget( new QLabel( "failed" ), 4, 2 );
  g->addLayout( m_pLastGL, 6, 0 );
  g->setRowStretch( 5, 1 );
  setLayout( g );

  connect( this, SIGNAL( numParallelActivitiesChanged(int) ), this, SLOT( updateParallelActivities(int))  );

  connect( m_pResetB, SIGNAL( clicked() ), this, SLOT( resetSlot() ) );
  m_nCounter = 0;
  m_nFirstID = -1;
  setMinimumSize( 800, 600 );

  emit numParallelActivitiesChanged (0);
}

void SdpaWnd::updateParallelActivities (int activities)
{
  m_pParActLabel->setText ( QString ("# parallel activities: ") + QString::number(activities));
}

bool SdpaWnd::event( QEvent * e )
{
  DataCostumerEvent<WndUpdateParameter> * dce = NULL;


  switch(e->type())
  {
    case 1001:
    {
      dce = (DataCostumerEvent<WndUpdateParameter> *)e;

      WndUpdateParameter param = dce->getData();
      int pos(0);
      QColor c;

      switch( param.state )
      {
        case STATE_CREATE:
        {
          pos = 1;
          c = m_ColorCreate;
          break;
        }
        case STATE_RUN:
        {
          pos = 2;
          c = m_ColorRun;
		  m_parallelActivities++;
          break;
        }
        case STATE_OK:
        {
          pos = 3;
          c = m_ColorOk;
		  m_parallelActivities--;
          break;
        }
        case STATE_FAILED:
        {
          pos = 3;
          c = m_ColorFailed;
		  m_parallelActivities--;
          break;
        }
	    default:
		  return QWidget::event(e);
      }
      if (m_parallelActivities < 0)
	     m_parallelActivities = 0;

      if( m_nFirstID == -1 )
	  m_nFirstID = param.id;
      if( param.id < m_nFirstID ) break;

      QPushButton *l = new QPushButton(param.info);
      l->setFixedSize( m_nMinWidth, m_nMinHeight );
      l->setDefault( false );
      l->setFocusPolicy( Qt::NoFocus );
      l->setPalette( c );

      m_pGridL->addWidget(l, param.id-m_nFirstID, pos );

      QLayoutItem *item = m_pGridL->itemAtPosition( param.id-m_nFirstID, 0 );

      if( item == NULL)
      {
        QPushButton *p = new QPushButton( param.name );
        p->setFixedSize( m_nMinWidth, m_nMinHeight );
        p->setDefault( false );
        p->setFocusPolicy( Qt::NoFocus );
        m_pGridL->addWidget( p, param.id-m_nFirstID, 0 );
      }

//      // update Last grid
//      // delete all
//      for( int j = 0; j < m_nNbColumn; j++ )
//      {
//        QLayoutItem *item = m_pLastGL->itemAtPosition( 0, j );
//        if( item )
//        {
//          QWidget *ptr = item->widget();
//          if( ptr )
//          {
//            m_pLastGL->removeWidget( ptr );
//            delete ptr;
//          }
//          m_pLastGL->removeItem( item );
//        }
//      }

//      for( int j = 0; j < m_nNbColumn; j++ )
//      {
//        QLayoutItem *item = m_pGridL->itemAtPosition( param.id-m_nFirstID, j );
//        if( item )
//        {
//          QPushButton *ptr = (QPushButton*)item->widget();
//          if( ptr )
//          {
//            QPushButton *t = new QPushButton(ptr->text());
//            t->setFixedSize( m_nMinWidth, m_nMinHeight );
//            t->setDefault( false );
//            t->setFocusPolicy( Qt::NoFocus );
//            t->setPalette( ptr->palette() );
//            m_pLastGL->addWidget( t, 0, j );
//          }
//        }
//      }

      m_nCounter = MAX( m_nCounter, param.id-m_nFirstID+1);
      m_pWidget->setFixedSize( m_nNbColumn*m_nMinWidth, m_nCounter*m_nMinHeight );

	  if (m_cbAutoFollow->isChecked())
	  {
		m_pScrollArea->verticalScrollBar()->setValue (m_pScrollArea->verticalScrollBar()->maximum());
	  }
	  emit numParallelActivitiesChanged (m_parallelActivities);
      break;
    }
	default:
	  break;
  }
  return QWidget::event(e);
}

void SdpaWnd::resetSlot()
{
  for( int i = 0; i < m_nCounter; i++ )
    for( int j = 0; j < m_nNbColumn; j++ )
  {
    QLayoutItem *item = m_pGridL->itemAtPosition( i,j );
    if( item )
    {
      QWidget *ptr = item->widget();
      if( ptr )
      {
        m_pGridL->removeWidget( ptr );
        delete ptr;
      }
      m_pGridL->removeItem( item );
    }
  }

  // delete all in last grid
//  for( int j = 0; j < m_nNbColumn; j++ )
//  {
//      QLayoutItem *item = m_pLastGL->itemAtPosition( 0, j );
//      if( item )
//      {
//          QWidget *ptr = item->widget();
//          if( ptr )
//          {
//	      m_pLastGL->removeWidget( ptr );
//	      delete ptr;
//          }
//          m_pLastGL->removeItem( item );
//      }
//  }
  m_pGridL->update();
  m_nFirstID = -1;
  m_nCounter = 0;
  m_pWidget->setFixedSize( m_nNbColumn*m_nMinWidth, m_nCounter*m_nMinHeight );
  emit numParallelActivitiesChanged (0);
}
