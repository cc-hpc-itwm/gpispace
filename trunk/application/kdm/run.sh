/p/hpc/sdpa/ap/git/SDPA/trunk/we/trunk/build/tests/test_layer   \
 --cfg      /p/hpc/sdpa/ap/git/KDM_VM/Kirchhoff_Model.xml       \
 --mod-path /p/hpc/sdpa/ap/git/KDM_VM/build.gcc                 \
 --load     /p/hpc/sdpa/ap/git/KDM_VM/libexec/libfvm-pc.so      \
 --load     /p/hpc/sdpa/ap/git/KDM_VM/build.gcc/libsinc.so      \
                                                         2>&1 | tee out.txt
