SET(tctar_SOURCES
  tctar.cpp
)

SET(tcuntar_SOURCES
  tcuntar.cpp
)

SET(TC_BUILD_TYPE debug)
SET(LIB_TC
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/tc/libtc_impl.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/posix/libtc_impl_posix.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/nfs4/libtc_impl_nfs4.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/util/libtc_util.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/MainNFSD/libMainServices.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/Protocols/NFS/libnfsproto.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/Protocols/NFS/libnfs4callbacks.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/Protocols/XDR/libnfs_mnt_xdr.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/SAL/libsal.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/cache_inode/libcache_inode.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/idmapper/libidmap.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/RPCAL/librpcal.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libsupport.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/Protocols/NFS/libnfsproto.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libstring_utils.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libhash.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libhashtable.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libavltree.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libuid2grp.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/support/libtc_helper.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/cidr/libcidr.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/FSAL/FSAL_PSEUDO/libfsalpseudo.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/NodeList/libNodeList.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/config_parsing/libconfig_parsing.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/libntirpc/src/libntirpc.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/os/libgos.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/util/libpath_utils.a
  ${CMAKE_CURRENT_LIST_DIR}/../../../tc_client/${TC_BUILD_TYPE}/log/liblog.a
)

SET(TC_ADDITIONAL_LIBS
  -lwbclient -ljemalloc -lcap -lblkid
  -luuid -ldl -lkrb5 -lk5crypto -lstdc++
  -lcom_err -lgssapi_krb5 -lnfsidmap -lpthread
)

INCLUDE_DIRECTORIES(../../../tc_client/include)
ADD_EXECUTABLE(tctar ${tctar_SOURCES})
TARGET_LINK_LIBRARIES(tctar  ${LIB_TC} ${TC_ADDITIONAL_LIBS} ${ADDITIONAL_LIBS} archive)
ADD_EXECUTABLE(tcuntar ${tcuntar_SOURCES})
TARGET_LINK_LIBRARIES(tcuntar  ${LIB_TC} ${TC_ADDITIONAL_LIBS} ${ADDITIONAL_LIBS} archive)
