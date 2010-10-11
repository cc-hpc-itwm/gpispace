#ifndef COSTUMEREVENT_H
#define COSTUMEREVENT_H

#include <QEvent>

template <typename Typ>
class DataCostumerEvent : public QEvent
{
public:
	/*! @brief Constructor
			@param cmd A commando that will be assigned to an VolPoolWnd object
			@param _ptr Pointer to the data that will be assigned to an VolPoolWnd object */
  DataCostumerEvent(  QEvent::Type _type, Typ _ptr ) : QEvent( _type ) { ptr = _ptr;}

  DataCostumerEvent( QEvent::Type _type, Typ _ptr, int _nLength ) : QEvent( _type ) { ptr = _ptr; m_nLength = _nLength; }

	inline const Typ getData() const { return ptr; }

	inline int getLength() const { return m_nLength; }

private:
	/*! @brief Pointer to the data that will be assigned to an VolPoolWnd object*/
	Typ ptr;

	int m_nLength;
};



#endif
