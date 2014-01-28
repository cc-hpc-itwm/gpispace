#ifndef UFBMIG_MSG_TYPES_HPP
#define UFBMIG_MSG_TYPES_HPP 1

namespace client { namespace command {
    // sent by the GUI to us
    enum code
      {
        INITIALIZE             = 0,
        MIGRATE                = 1,
        MIGRATE_WITH_SALT_MASK = 2,
        SALT_MASK              = 3,
        ABORT                  = 4,
        FINALIZE               = 5,

        NUM_COMMANDS
      };
  }
}

namespace server { namespace command {
    // we might send those
    enum code
      {
        WAITING_FOR_INITIALIZE = 0,

        INITIALIZING = 1,
        INITIALIZE_SUCCESS = 2,
        INITIALIZE_FAILURE = 3,

        MIGRATING = 4,
        MIGRATE_SUCCESS = 5,
        MIGRATE_FAILURE = 6,

        FINALIZING = 7,
        FINALIZE_SUCCESS = 8,
        FINALIZE_FAILURE = 9,

        PROCESSING_SALT_MASK = 10,
        PROCESS_SALT_MASK_SUCCESS = 11,
        PROCESS_SALT_MASK_FAILURE = 12,

        ABORT_ACCEPTED = 13,
        ABORT_REFUSED = 14,

        MIGRATE_META_DATA = 15,
        MIGRATE_DATA = 16,

        PROGRESS = 1000,
        LOGOUTPUT = 1001,
      };
  }
}

#endif
