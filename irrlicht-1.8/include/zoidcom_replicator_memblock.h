/****************************************
* zoidcom_replicator_memblock.h
* memory chunk replicator
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Jörg Rüppel. See documentation for copyright and licensing details.
*****************************************/


#ifndef _ZOIDREPLICATORMEMBLOCK_H_
#define _ZOIDREPLICATORMEMBLOCK_H_

/** \file zoidcom_replicator_memblock.h
*/

#include "zoidcom.h"

/**
 * @brief A basic replicator for memory chunks.
 *
 * @attention This replicator always transfers the whole memory chunk, so the size shouldn't exceed
 *            that of a normal UDP packet.
 */
class ZCom_Replicate_Memblock : public ZCom_ReplicatorBasic
{
private:
  char*     m_ptr;
  char*     m_cmp;
  zU16      m_size;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCOM_API ZCom_Replicate_Memblock(void *_data, zU16 _size, ZCom_ReplicatorSetup *_setup) : ZCom_ReplicatorBasic(_setup)
  {
    m_ptr = (char*) _data;
    m_cmp = new char[_size];
    if (!m_cmp)
      return;
    m_size = _size;
	memcpy(m_cmp, m_ptr, m_size);
    m_flags |= ZCOM_REPLICATOR_INITIALIZED;
  }

  /// constructor, builds the replicatorsetup automatically
  ZCOM_API ZCom_Replicate_Memblock(void *_data, zU16 _size, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0, zS16 _mindelay = -1, zS16 _maxdelay = -1) : ZCom_ReplicatorBasic(NULL)
  {
    // make sure m_setup will be deleted when the replicator is deleted
    _flags |= ZCOM_REPFLAG_SETUPAUTODELETE;

    // create the setup object
    m_setup = new ZCom_ReplicatorSetup(_flags, _rules, _intercept_id, _mindelay, _maxdelay);

    // out of mem
    if (!m_setup)
      return;

    m_ptr = (char*) _data;
    m_cmp = new char[_size];
    if (!m_cmp)
      return;
    m_size = _size;
	memcpy(m_cmp, m_ptr, m_size);
    m_flags |= ZCOM_REPLICATOR_INITIALIZED;
  }

  ZCOM_API ~ZCom_Replicate_Memblock()
  {
    if (m_cmp) delete []m_cmp;
  }

  ZCOM_API bool checkState() {
    // compare
    bool s = (memcmp(m_ptr, m_cmp, m_size) != 0);
    // if changed, update the comparison
    if (s) memcpy(m_cmp, m_ptr, m_size);
    // return result
    return s;
  }

  ZCOM_API void packData(ZCom_BitStream *_stream) {
    // add data to stream
    _stream->addBuffer(m_ptr, m_size);
  }
  ZCOM_API void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) {
    // shall we store
    if (_store)
      _stream->getBuffer(m_ptr, m_size);
    // or just skip the data in the stream?
    else
      _stream->skipBuffer(m_size);
  }

  ZCOM_API void* peekData() {
    // check if peekData is really called from within an interceptor
    assert(getPeekStream());
    // allocate memory as peekbuffer
    char *buf = new char[m_size];
    if (!buf)
      return NULL;

    // read data into the buffer
    getPeekStream()->getBuffer(buf, m_size);
    // let zoidcom store the pointer to the buffer so we can delete it later
    peekDataStore(buf);
    // return the buffer so the caller can look at the data
    return buf;
  }

  ZCOM_API void clearPeekData() {
    // get the data we allocated above
    char *buf = (char*) peekDataRetrieve();
    // and delete it
    if (buf) delete []buf;
  };

  ZCOM_API void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {};
};

#endif
