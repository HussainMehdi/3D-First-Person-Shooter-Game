/****************************************
* zoidcom_replicator_bool.h
* boolean value replicator
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORBOOL_H_
#define _ZOIDREPLICATORBOOL_H_

/** \file zoidcom_replicator_bool.h
*/

#include "zoidcom.h"

/** @brief Replicator for bool*
*/
class ZCOM_API ZCom_Replicate_Boolp : public ZCom_ReplicatorBasic
{
private:
  bool*     m_ptr;
  bool      m_cmp;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCom_Replicate_Boolp(bool *_data, ZCom_ReplicatorSetup *_setup) : ZCom_ReplicatorBasic(_setup) { m_ptr = _data; m_flags |= ZCOM_REPLICATOR_INITIALIZED; }
  /// constructor, builds the replicatorsetup automatically
  ZCom_Replicate_Boolp(bool *_data, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  bool checkState(){bool s = (m_cmp != *m_ptr); m_cmp = *m_ptr; return s;}
  void packData(ZCom_BitStream *_stream) { _stream->addBool(*m_ptr);}
  void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) { if (_store) *m_ptr = _stream->getBool(); else _stream->getBool();}
  void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};
  void* peekData() { assert(getPeekStream()); return (void*) getPeekStream()->getBool(); }
  void clearPeekData() {};
};

/*
 *  (bool) Replicator -----------------------------------------------------------
*/

/** @brief Replicator for bool
*/
class ZCOM_API ZCom_Replicate_Bool : public ZCom_ReplicatorBasic
{
private:
  bool      m_data;
  bool      m_cmp;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCom_Replicate_Bool(bool _data, ZCom_ReplicatorSetup *_setup) : ZCom_ReplicatorBasic(_setup) {m_data = _data; m_flags |= ZCOM_REPLICATOR_INITIALIZED; }
  /// constructor, builds the replicatorsetup automatically
  ZCom_Replicate_Bool(bool _data, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0, zS16 _mindelay = -1, zS16 _maxdelay = -1);
  bool checkState(){bool s = (m_cmp != m_data); m_cmp = m_data; return s;}
  void packData(ZCom_BitStream *_stream) { _stream->addBool(m_data);}
  void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) { if (_store) m_data = _stream->getBool(); else _stream->getBool();}
  void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};
  void* peekData() { assert(getPeekStream()); return (void*) getPeekStream()->getBool(); }
  void clearPeekData() {};

  /// casting operator for accessing the class as the value it contains
  operator bool() { return m_data; }
  /// assignment operator for accessing the class as the value it contains
  bool& operator=(bool _data) {m_data = _data; return m_data;}
  /// get value
  bool getValue() { return m_data; }
  /// set value
  void setValue(bool _val) { m_data = _val; }
};

#endif
