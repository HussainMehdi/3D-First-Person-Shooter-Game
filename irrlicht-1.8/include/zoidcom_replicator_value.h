/****************************************
* zoidcom_replicator_value.h
* common storage class for replicators
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORVALUE_H_
#define _ZOIDREPLICATORVALUE_H_

/** \file zoidcom_replicator_value.h
*/

#include "zoidcom.h"

/**
 * @brief Holds replication data.
 *
 */
template <typename T, int SIZE> class ZCOM_API ZCom_ReplicatorValue {
  T     m_data[SIZE];
  bool  m_changed;
  public:
    ZCom_ReplicatorValue() : m_changed(true) {}
    ZCom_ReplicatorValue& operator=(ZCom_ReplicatorValue&);

    bool hasChanged();
    bool getChanged() const;
    void setChanged();
    ZCOM_TAPI T*   getData();
    void setData(T* _data);
    void setData(T _data, zU32 _idx);
    void updateData(T* _data);
};

/**
 * @brief Holds a pointer to replication data.
 */
template <typename T, int SIZE> class ZCOM_API ZCom_ReplicatorValue<T*, SIZE> {
  T*    m_data;
  T     m_cmp[SIZE];
  public:
    ZCom_ReplicatorValue();
    ZCom_ReplicatorValue(T* _data);
    ZCom_ReplicatorValue& operator=(ZCom_ReplicatorValue&);
    bool hasChanged();
    bool getChanged() const;
    void setChanged();
    ZCOM_TAPI T*   getData();
    void setData(T* _data);
    void setData(T _data, zU32 _idx);
    void updateData(T* _data);
};

#endif
