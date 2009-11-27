TEMPLATE            = app
CONFIG	            +=qt thread warn_on release 

BOOSTLIBS      =  \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_date_time-gcc43-mt-1_38.a \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_system-gcc43-mt-1_38.a \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_filesystem-gcc43-mt-1_38.a \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_program_options-gcc43-mt-1_38.a \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_serialization-gcc43-mt-1_38.a \
                  /p/hpc/soft/boost/1.38/gcc/lib/libboost_thread-gcc43-mt-1_38.a


INCLUDEPATH   += -I /p/hpc/soft/fhglog/1.2.0/suse11/gcc/include
INCLUDEPATH   += -I /p/hpc/soft/boost/1.38/gcc/include/boost-1_38
INCLUDEPATH   += -I /p/hpc/sdpa/ap/SDPA/sdpa/trunk
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

		


TARGET				        = ./prg
