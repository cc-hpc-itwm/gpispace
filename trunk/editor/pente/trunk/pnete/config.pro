TEMPLATE            = app
CONFIG	            +=qt thread warn_on release

BOOSTLIBS      =  \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_date_time-gcc43-mt-1_43.a \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_system-gcc43-mt-1_43.a \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_filesystem-gcc43-mt-1_43.a \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_program_options-gcc43-mt-1_43.a \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_serialization-gcc43-mt-1_43.a \
                  /p/hpc/soft/boost/1.43/gcc/lib/libboost_thread-gcc43-mt-1_43.a


INCLUDEPATH   += -I /p/hpc/soft/fhglog/1.2.0/suse11/gcc/include
INCLUDEPATH   += -I /p/hpc/soft/boost/1.43/gcc/include/boost-1_43
INCLUDEPATH   += -I /p/hpc/sdpa/ap/git/SDPA/trunk/sdpa/trunk
LIBS          += /p/hpc/soft/fhglog/1.2.0/suse11/gcc/lib/libfhglog.a
LIBS          += $$BOOSTLIBS

HEADERS	            = ./sdpaWnd.h \


SOURCES	              = ./main.cpp \
                        ./sdpaWnd.cpp \



unix:LIBS             +=



#QMAKE_CXXFLAGS          += -O3 -xW
#QMAKE_CFLAGS            += -O3 -xW
#unix:OBJECTS_DIR      = objs
#unix:MOC_DIR          = moc




TARGET				        = ./sdpa-gui
