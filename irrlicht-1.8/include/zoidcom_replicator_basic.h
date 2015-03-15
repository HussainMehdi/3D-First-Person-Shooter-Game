/****************************************
* zoidcom_replicator_basic.h
* replicator base definition
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORBASIC_H_
#define _ZOIDREPLICATORBASIC_H_

/** \file zoidcom_replicator_basic.h
*/

#include "zoidcom.h"

/** @brief Interface for standard data replicators.
  * @see @ref SyncCustom
  */
class ZCOM_API ZCom_ReplicatorBasic : public ZCom_Replicator
{
public:
  ZCom_ReplicatorBasic(ZCom_ReplicatorSetup *_setup);

  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /*               update checking              */
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /** \name Update checking.
  */
  //@{

  /** @brief Find out if data has changed since last time this has been called.
      @returns 'true' if an update is needed.

      This won't get called for all connections individually. Instead, Zoidcom calls
      it once per ZCom_processOutput() and if 'true' is returned, this replicator
      will get marked dirty internally for all interested connections.
  */
  virtual bool checkState() = 0;
  //@}

  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /*               update sending               */
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /** \name Data packing.
  */
  //@{

  /** @brief Pack current data into the stream.
      @param _stream A stream object you have to feed.

      This method is a request to write the replicated data into the
      supplied stream. Make sure the pack and unpackData() methods
      write exactly as many bits as they read from the stream, otherwise
      the rest of it becomes unusable.

      It is also possible to
  */
  virtual void packData(ZCom_BitStream *_stream) = 0;

  /** @brief Unpack data from the stream and store it locally.
      @param _stream A stream object you have to extract the data from.
      @param _store If false, only advance the bitstream but do not apply/store the new data.
              This is needed if an interceptor decided to not apply the update.
      @param _estimated_time_sent gives an msec value trying to estimate the time the data has
              left the remote ZCom_Control. Execute ZoidCom::getTime() -
              _estimated_time_sent to find out the data's travel time in msecs.

      This method is a request to read replicated data from the
      supplied stream and store it locally. Make sure the pack and
      unpackData() methods write exactly as many bits as they read from
      the stream, otherwise the rest of it becomes unusable.
  */
  virtual void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) = 0;

  //@}

  // for internal use only
  void setModified() { m_flags |= ZCOM_REPLICATOR_MODIFIED; }
};

#endif
