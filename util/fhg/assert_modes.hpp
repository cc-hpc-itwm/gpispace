#ifndef FHG_ASSERT_MODES_HPP
#define FHG_ASSERT_MODES_HPP

#define FHG_ASSERT_DISABLED  0 //! \note ignore asserts
#define FHG_ASSERT_ENABLED   1 //! \note enable asserts
#define FHG_ASSERT_LEGACY    2 //! \note fall back to assert()
#define FHG_ASSERT_EXCEPTION 3 //! \note throw fhg::assertion_failed
#define FHG_ASSERT_LOG       4 //! \note just log
#define FHG_ASSERT_LOG_ABORT 5 //! \note log and abort

#endif
