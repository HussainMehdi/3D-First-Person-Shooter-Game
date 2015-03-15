/****************************************
* zoidcom_replicator_string.h
* string replicators
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORSTRING_H_
#define _ZOIDREPLICATORSTRING_H_

/** \file zoidcom_replicator_string.h
*/

#include "zoidcom.h"

/** @brief Parameter class for string replicators.
*/
class ZCOM_API ZCom_RSetupString : public ZCom_ReplicatorSetup
{
public:
  /// constructor
  ZCom_RSetupString(zU16 _maxlen, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
                             zS16 _mindelay = -1, zS16 _maxdelay = -1)
           : ZCom_ReplicatorSetup(_flags, _rules, _intercept_id,
                                  _mindelay, _maxdelay)
  { maxlen = _maxlen; }


  virtual ZCom_ReplicatorSetup* Duplicate();

public:
  /// Maximum length of the string. A compare buffer of that size will
  /// be allocated by the replicator.
  zU16       maxlen;
};

/** @brief Replicator for C-strings.
*/
class ZCOM_API ZCom_Replicate_Stringp : public ZCom_ReplicatorBasic
{
private:
  /// Pointer to the string.
  char      *m_ptr;
  /// Pointer to internal buffer for comparison.
  char      *m_cmp;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCom_Replicate_Stringp(char *_data, ZCom_RSetupString *_setup);
  /// constructor, builds the replicatorsetup automatically
  ZCom_Replicate_Stringp(char *_data, zU16 _maxlen, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
                                  zS16 _mindelay = -1, zS16 _maxdelay = -1);
  ~ZCom_Replicate_Stringp();
  bool checkState();
  void packData(ZCom_BitStream *_stream);
  void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) ;
  void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};

  void* peekData();
  void  clearPeekData();
};

/** @brief Replicator for C-strings.
*/
class ZCOM_API ZCom_Replicate_StringWp : public ZCom_ReplicatorBasic
{
private:
  /// Pointer to the string.
  wchar_t      *m_ptr;
  /// Pointer to internal buffer for comparison.
  wchar_t      *m_cmp;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCom_Replicate_StringWp(wchar_t *_data, ZCom_RSetupString *_setup);
  /// constructor, builds the replicatorsetup automatically
  ZCom_Replicate_StringWp(wchar_t *_data, zU16 _maxlen, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
    zS16 _mindelay = -1, zS16 _maxdelay = -1);
  ~ZCom_Replicate_StringWp();
  bool checkState();
  void packData(ZCom_BitStream *_stream);
  void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) ;
  void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};

  void* peekData();
  void  clearPeekData();
};

#endif
