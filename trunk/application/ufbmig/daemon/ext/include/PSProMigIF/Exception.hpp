/*  @brief Exception object for PSPro-Mig
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 18, 2010
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

template<typename T>
class Exception
{
  public:

    Exception(void)
    {

    }

    /*! @brief Constructor
     *  @param _value The exception value
     */
    Exception(const T& _value) :
      mValue(_value)
    {

    }

    /*! @brief Destructor
     */
    virtual ~Exception(void)
    {

    }

    /*! @return Returns the exception value
     */
    inline const T& what(void) const
    {
      return mValue;
    }

  private:

    /*! @brief Exception value
     */
    T mValue;
};

#endif
