/****************************************
* zoidcom_replicator_numeric.h
* pointer-to-numerics replicators
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORNUMERIC_H_
#define _ZOIDREPLICATORNUMERIC_H_

/** \file zoidcom_replicator_numeric.h
*/

#include "zoidcom.h"
#include "zoidcom_replicator_value.h"
#include "zoidcom_typehelper.h"

/****************************************************
*         (numeric replicator setup class)
*****************************************************/

/** @brief Parameter class for numeric replicators.
*/
class ZCOM_API ZCom_RSetupNumeric : public ZCom_ReplicatorSetup
{
public:
  /// constructor
  ZCOM_TAPI ZCom_RSetupNumeric(zU8 _rbits, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
                              zS16 _mindelay = -1, zS16 _maxdelay = -1)
                              : ZCom_ReplicatorSetup(_flags, _rules, _intercept_id,
                                                     _mindelay, _maxdelay)
                    { m_relevant_bits = _rbits; }

  ZCOM_TAPI virtual ZCom_ReplicatorSetup* Duplicate();

  /// Get relevant bits.
  ZCOM_TAPI inline zU8 getRelevantBits() const { return m_relevant_bits; }

  /** @brief Set relevant bits.
      @param _bits New amount of relevant bits.

      @attention You should not change this value for an active replicator unless
                 you are 100% certain that all peer replicators do that change, too,
                 <b>before</b> they receive their next update.
  */
  ZCOM_TAPI inline void setRelevantBits(zU8 _bits) { m_relevant_bits = _bits; }
protected:
  /// The amount of relevant bits for integer types or
  /// replicated mantissa bits for float values.
  zU8       m_relevant_bits;
};


/**
 * @brief Replicator template replicating numeric data (single value or vector).
 * @param T Type of data to replicate. Supported are zU32, zS32, zFloat, zU32*, zS32* and zFloat*
 * @param SIZE Size of array to replicate. 1 for single values. Supported size is up to 12.
*/
template <typename T, int SIZE>
class ZCOM_API ZCom_Replicate_Numeric : public ZCom_ReplicatorBasic
{
  // make T a non-pointer
  typedef typename ZCom_TypeHelper<T>::value_type TYPE;
protected:
  ZCom_ReplicatorValue<T, SIZE> m_value;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCOM_TAPI ZCom_Replicate_Numeric(TYPE *_data, ZCom_RSetupNumeric *_setup);
  /// constructor, builds the replicatorsetup automatically
  ZCOM_TAPI ZCom_Replicate_Numeric(TYPE *_data, zU8 _rbits, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  ZCOM_TAPI ~ZCom_Replicate_Numeric();
  ZCOM_TAPI bool checkState();
  ZCOM_TAPI void packData(ZCom_BitStream* _stream);
  ZCOM_TAPI void unpackData(ZCom_BitStream* _stream, bool _store, zU32 _estimated_time_sent);
  ZCOM_TAPI void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};

  /** @brief Peeking implementation.
      @returns T* array of size getSize().
  */
  ZCOM_TAPI void* peekData();
  ZCOM_TAPI void  clearPeekData();

  /** @brief Get size of array.
      @returns Size of array.
  */
  ZCOM_TAPI zU8   getSize() const { return SIZE; }

  /** @brief Get value from array.
      @param _idx Array index.
      @returns Value at array index.
  */
  ZCOM_TAPI TYPE  getValue(zU8 _idx) { assert(_idx < SIZE); return m_value.getData()[_idx]; }

  /** @brief Get pointer to internal value array.
      @returns Pointer to array. Do not change the contents directly, as this won't be detected.
  */
  ZCOM_TAPI TYPE* getValue() { return m_value.getData(); }

  /** @brief Set single value in array.
      @param _idx Array index.
      @param _val New value.
  */
  ZCOM_TAPI void  setValue(zU8 _idx, TYPE _val) { assert(_idx < SIZE); m_value.getData()[_idx] = _val; }

  /** @brief Set single value in array.
      @param _val Pointer to beginning of array containing new values. Array must have the same size as template SIZE.
   */
  ZCOM_TAPI void  setValue(TYPE* _val) { m_value.setData(_val); }

};


#endif
