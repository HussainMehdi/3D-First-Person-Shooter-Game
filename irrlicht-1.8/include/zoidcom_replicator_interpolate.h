/****************************************
* zoidcom_replicator_interpolate.h
* interpolating replicators
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORINTERPOLATE_H_
#define _ZOIDREPLICATORINTERPOLATE_H_

/** \file zoidcom_replicator_interpolate.h
*/

#include "zoidcom.h"
#include "zoidcom_replicator_value.h"

/****************************************************
*            (interpolator setup class)
*****************************************************/

/** @brief Parameter class for interpolating replicators.
*/
template <typename T>
class ZCOM_API ZCom_RSetupInterpolate : public ZCom_RSetupNumeric
{
public:
  /// constructor
  ZCOM_TAPI ZCom_RSetupInterpolate(zU8 _rbits, zU8 _flags, zU8 _rules, T _ipoltreshold,
                                  zU8 _intercept_id = 0, zS16 _mindelay=-1, zS16 _maxdelay=-1,
                                  zFloat _ipolfac=0.4f)
  : ZCom_RSetupNumeric(_rbits, _flags, _rules, _intercept_id,
                                _mindelay, _maxdelay)
  {
    ipol_factor = _ipolfac;
    ipol_treshold = _ipoltreshold;
  }

  ZCOM_TAPI virtual ZCom_ReplicatorSetup* Duplicate();

  /** @brief Interpolation factor.

      A value of 1.0 always overwrites the existing data with the incoming
      update. A value of 0.0 ignores incoming updates and leaves the local
      value as is.
  */
  zFloat ipol_factor;

  /** @brief Interpolation treshold.

      If difference between incoming data and local data is greater than the
      treshold, no interpolation occurs. Instead, the local value will be
      replaced with the incoming value.
  */
  T      ipol_treshold;
};

/****************************************************
* (interpolator template)
*****************************************************/

/** @brief Interpolator template replicating numeric data and interpolating it between updates.
*/
template <typename T, int SIZE>
class ZCOM_API ZCom_Interpolate : public ZCom_ReplicatorBasic
{
  typedef typename ZCom_TypeHelper<T>::value_type TYPE;
private:
  /// application value
  ZCom_ReplicatorValue<T, SIZE> m_value;
  TYPE                          m_received_val[SIZE];
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCOM_TAPI ZCom_Interpolate(TYPE *_data, ZCom_RSetupInterpolate<TYPE> *_setup);
  /// constructor, builds the replicatorsetup automatically
  ZCOM_TAPI ZCom_Interpolate(TYPE *_data, zU8 _bits, zU8 _flags, zU8 _rules, TYPE _treshold, TYPE *_dst=NULL, zU8 _intercept_id = 0, zS16 _mindelay=-1, zS16 _maxdelay=-1, zFloat _ipolfac=0.4f);

  ZCOM_TAPI ~ZCom_Interpolate();
  ZCOM_TAPI bool checkState();
  ZCOM_TAPI void packData(ZCom_BitStream *_stream);
  ZCOM_TAPI void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent);
  ZCOM_TAPI void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed);

  /** @brief Peeking implementation.
      @returns T* array of size getSize().
  */
  ZCOM_TAPI void* peekData();
  ZCOM_TAPI void  clearPeekData();

  /** @brief Get value from array.
      @param _idx Array index.
      @returns Value at array index.
  */
  ZCOM_TAPI TYPE  getValue(zU8 _idx) { assert(_idx < SIZE); return m_value.getData()[_idx]; }

  /** @brief Set value in array.
      @param _idx Array index.
      @param _val New value.
  */
  ZCOM_TAPI void  setValue(zU8 _idx, TYPE _val) { assert(_idx < SIZE); m_value.getData()[_idx] = _val; }

  /** @brief Update complete array.
      @param _val Ptr to array of new values.
   */
  ZCOM_TAPI void  setValue(TYPE* _val) { m_value.setData(_val); }

  /** @brief Get size of array.
      @returns Size of array.
  */
  ZCOM_TAPI zU8   getSize() const { return SIZE; }

  /** @brief Get last received value.
      @returns The value last received from the network.

      This is the value currently used as interpolation target.
  */
  ZCOM_TAPI TYPE  getRecVal(zU8 _idx) const { assert(_idx < SIZE); return m_received_val[_idx]; }

  /** @brief Get a pointer to the received value.
      @returns Pointer to the variable always holding the last received value.

      This can be used to manipulate the received value.
  */
  ZCOM_TAPI TYPE* getRecValPtr(zU8 _idx) { assert(_idx < SIZE); return &m_received_val[_idx]; }

  /** @brief Overwrite the received value.
      @param _idx Index in array.
      @param _val The new value.
  */
  ZCOM_TAPI void setRecVal(zU8 _idx, TYPE _val) { assert(_idx < SIZE); m_received_val[_idx] = _val; }
};

#endif
