/****************************************
* zoidcom_conngroup.h
* connection group manager
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDCONNGROUP_H_
#define _ZOIDCONNGROUP_H_

/** \file zoidcom_conngroup.h
*/

#include "zoidcom.h"

/*
** forwards
*/
class ZCom_ConnGroupManager_Private;

/// This GroupID is always valid and points to a group containing all connections within a ZCom_Control
#define ZCOM_CONNGROUP_ALL 1

/** @brief A manager for connection groups.

    Each ZCom_Control instance holds it's own group manager. Each group from this manager
    is valid to the control instance itself and to all ZCom_Node objects registered to it.
    
    Note that groups are not synchronised currently. Groups created on one host can be
    used on this host only.

    @ref ZCOM_CONNGROUP_ALL is a special group id always representing a group which contains all
    connections of a current ZCom_Control.
*/
class ZCOM_API ZCom_ConnGroupManager
{
protected:
  ZCom_ConnGroupManager_Private *m_priv;
  /************************************/
public:
  /** @brief Constructor.
  */
  ZCom_ConnGroupManager();
  /** @brief Destructor.
  */
  ~ZCom_ConnGroupManager();

  /** @brief Clean everything up.
  */
  void
    Clear();

  /** @brief Create a new group.
      @param _size_hint How much space should be reserved? Exceeding this value later on will
                        result in reallocations.
      @returns A new group ID.
  */
  ZCom_GroupID
    createGroup(zU32 _size_hint);

  /** @brief Destroy a group.
      @param _gid ID of group to destroy.
  */
  void
    destroyGroup(ZCom_GroupID _gid);

  /** @brief Check if group exists.
      @param _gid ID of group to check.
      @returns 'true' if group exists.
  */
  bool
    checkGroupExists(ZCom_GroupID _gid);

  /** @brief Add connection to a group.
      @param _gid ID of group.
      @param _cid ID of connection.
      \return 'true' on success.
  */
  bool
    addConnection(ZCom_GroupID _gid, ZCom_ConnID _cid);

  /** @brief Remove connection from a group.
      @param _gid ID of group.
      @param _cid ID of connection.
      @returns 'true' on success.
  */
  bool
    removeConnection(ZCom_GroupID _gid, ZCom_ConnID _cid);

  /** @brief Get first connection from a group.
      @param _gid ID of group.
      @param _iterator will be initialised with call to this method
      @returns ID of connection or ZCom_Invalid_ID if empty
  */
  ZCom_ConnID
    getFirstConnection(ZCom_GroupID _gid, zU32 &_iterator);
    
  /** @brief Get next connection from a group.
      @param _gid ID of group.
      @param _iterator iterator used in previous call to getFirstConnection()
      @returns ID of connection or ZCom_Invalid_ID if no more connection IDs are available
  */
  ZCom_ConnID
    getNextConnection(ZCom_GroupID _gid, zU32 &_iterator);
  
  /** @brief Get size of a group.
      @param _gid ID of group.
      @returns Number of elements in group.
  */
  zU32
    getGroupSize(ZCom_GroupID _gid);
};

#endif
