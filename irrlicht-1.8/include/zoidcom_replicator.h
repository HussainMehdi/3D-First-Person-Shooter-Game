/****************************************
* zoidcom_replicator.h
* replicator base definition
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATOR_H_
#define _ZOIDREPLICATOR_H_

/** \file zoidcom_replicator.h
*/

#include "zoidcom.h"

// forward decls
class ZCom_Node;
class ZCom_Node_Private;


/** \name Replicator flags.

    Stored in m_flags in ZCom_Replicator.
*/
//@{
/// the replicator holding this flag wants to get it's Process() method called regularly
#define ZCOM_REPLICATOR_CALLPROCESS     (1L << 0)
/// the replicator was allocated by zoidcom - used and handled internally
#define ZCOM_REPLICATOR_ZCOMOWNAGE      (1L << 1)
/// if replicator is registered to a node and this flag is not set, an error occurs
#define ZCOM_REPLICATOR_INITIALIZED     (1L << 2)
/// the replicator is an instance of ZCom_ReplicatorBasic
#define ZCOM_REPLICATOR_BASIC           (1L << 3)
/// the replicator is an instance of ZCom_ReplicatorAdvanced
#define ZCOM_REPLICATOR_ADVANCED        (1L << 4)
/// the replicator has replicated at least once, used in conjunction with ZCOM_REPFLAG_STARTCLEAN
#define ZCOM_REPLICATOR_MODIFIED        (1L << 5)
/// freely available for replicator implementations
#define ZCOM_REPLICATOR_USER1           (1L << 6)
/// freely available for replicator implementations
#define ZCOM_REPLICATOR_USER2           (1L << 7)
//@}

/** @brief Base class for replicator parameters.

    This class provides the base setup information for Zoidcom's data replication
    features. Replicator implementations can derive from it to add additional info, like
    e.g. extrapolation error treshold or whatever else is needed.

    Only use this class to store sharable setup information. Don't store any data specific
    to single replication items, which has to be stored inside the ZCom_Replicator itself instead.
*/
class ZCOM_API ZCom_ReplicatorSetup
{
friend class ZCom_Node_Private;
protected:
  /// Holds replication flags (ZCOM_REPFLAG_*).
  /// Never ever modify these during operation.
  zU8            m_flags;
  /// Holds replication rules (ZCOM_REPRULE_*)
  /// Never ever modify these during operation.
  zU8            m_rules;
  /// interceptor id, 0 = default
  zU8            m_intercept_id;
  /** @brief Minimum delay between updates.

      It is not allowed to change these value from -1 to >= 0 once a replicator
      is actively using this setup. All other changes are allowed.
  */
  zS16           m_mindelay;
  /** @brief Maximum delay between updates.

      It is not allowed to change these value from -1 to >= 0 once a replicator
      is actively using this setup. All other changes are allowed.
  */
  zS16           m_maxdelay;

public:
  /// constructor
  ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules);
  /// constructor
  ZCom_ReplicatorSetup(zU8 _flags, zU8 _rules, zU8 _intercept_id,
                                zS16 _min_delay, zS16 _max_delay);
  /// destructor
  virtual ~ZCom_ReplicatorSetup();

  /** @brief Create a duplication of this object.
      @returns Pointer to the setup object holding the duplicated parameters.

      If \ref ZCOM_REPFLAG_SETUPPERSISTS is set in \ref m_flags, Duplicate() has
      to simply return a pointer to itself instead of allocating a new object.
  */
  virtual ZCom_ReplicatorSetup* Duplicate();

  /** @brief Get the interceptor id.
      @returns Interceptor id.
  */
  inline zU8 getInterceptID() const { return m_intercept_id; }

  /** @brief Set the intercept id.
      @param _id New intercept id.

      The interceptor id can be used to distinguish different replicators inside an interceptor
      callback.
  */
  inline void setInterceptID(zU8 _id) { m_intercept_id = _id; }

  /** @brief Get minimum delay.
      @returns Minimum delay.
  */
  inline zS16 getMinDelay() const { return m_mindelay; }


  /** @brief Set minimum delay.
      @param _dly New minimum delay.

      @attention It is not allowed to change these value from -1 to >= 0 once a replicator
                 is actively using this setup. All other changes are allowed.
  */
  inline void setMinDelay(zS16 _dly) { m_mindelay = _dly; }

  /** @brief Get maximum delay.
      @returns Maximum delay.
  */
  inline zS16 getMaxDelay() const { return m_maxdelay; }

  /** @brief Set maximum delay.
      @param _dly New maximum delay.

      @attention It is not allowed to change these value from -1 to >= 0 once a replicator
                 is actively using this setup. All other changes are allowed.
  */
  inline void setMaxDelay(zS16 _dly) { m_maxdelay = _dly; }

  /** @brief Get the rules(ZCOM_REPRULE_*).
      @returns Replication rules.
  */
  inline zU8 getRules() const { return m_rules; }

  /** @brief Get the flags(ZCOM_REPFLAG_*).
      @returns Replication flags.
  */
  inline zU8 getFlags() const { return m_flags; }

  /// @brief Overloaded memory operator ensuring that always Zoidcom's new gets called.
  void* operator new(size_t _size);
  /// @brief Overloaded memory operator ensuring that always Zoidcom's delete gets called.
  void  operator delete(void *_p);
};

/** @brief The replicator interface.

    This class provides the interface all replicators have to implement. Each replication
    item registered with a node equals one instance of a specialization of this class. It
    holds all data specific to one single replication item.

    Combined with a specialization of the ZCom_ReplicatorSetup class it is possible to create
    totally custom replicators, with additional parameters and complex update methods.

    @see @ref SyncCustom
*/
class ZCOM_API ZCom_Replicator
{
friend class ZCom_Node_Private;
private:
  zU16                  m_id;
protected:
  /// Additional replicator flags. ZCom_Replicator() c'tor will set this to 0. (ZCOM_REPLICATOR_*)
  zU8                   m_flags;
  /** @brief pointing to an instance of the setup class - all replication parameters
      are stored here */
  ZCom_ReplicatorSetup* m_setup;
public:
  /** \name Construction / Deconstruction.
  */
  //@{
  /** @brief constructor
  */
  ZCom_Replicator(ZCom_ReplicatorSetup *_setup);

  /** @brief Destructor.
  */
  virtual ~ZCom_Replicator();

  /// @brief Overloaded memory operator ensuring that always Zoidcom's new gets called.
  /// @attention Don't overload this unless you are 100% sure what you are doing.
  void* operator new(size_t _size);
  /// @brief Overloaded memory operator ensuring that always Zoidcom's delete gets called.
  /// @attention Don't overload this unless you are 100% sure what you are doing.
  void  operator delete(void *_p);
  //@}


public:

  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /*                  peeking                   */
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /**
   * @anchor iceptpeek
   * \name Interceptor peek support.
      The methods getPeekData() and clearPeekData() need to be implemented when the replicator
      is supposed to be intercepted. Using these methods, the interceptor callback can ask the
      replicator to <i>peek</i> into the stream, without altering it's read position. Inspecting
      the data provided by the peekData() method, the callback can then decide if the update should
      be let through or not.

      A interceptor trying to see what is in the stream only needs to call the peekData()
      method. Everything else is handled by the replicator itself.

      A replicator that should support peeking has to implement peekData(), using getPeekStream()
      to get the currently processed bitstream. If it needs to allocate memory to hold the
      data, it has to make use of peekDataStore() in peekData(), and peekDataRetrieve() in
      clearPeekData().

      @code
      class MyReplicator : public ZCom_Replicator {
      void * peekData() {
        int size = getPeekStream()->getStringSize();
        char *string = new char[size];
        getPeekStream()->getString(string, size);
        // store it so it can be deleted later
        peekDataStore(buf);
        return (void*) buf;
      }

      void  clearPeekData() {
        char *str = peekDataRetrieve();
        if (str) delete []str;
      }
      @endcode

      Zoidcom automatically notices if data has been stored and calls clearPeekData() after
      the interceptor has completed.

      If the stream contains very simple data for which no memory allocation is needed, this
      will do:
      @code
      void *MyReplicator::peekData() {
        return (void*) getPeekStream()->getInt(32);
      }
      void MyReplicator::clearPeekData() {}
      @endcode

      More information can be found in the documentation of each peek method.
  */
  ///@{

  /** @brief Unpack data from the stream, but don't update the local data.
      @returns void* Pointer to peeked data.

      Interceptors need to call this if they want to know what the update contains without
      really applying that update. The replicator's peekData() implementation should extract
      the data from the stream and return it as void*. In case peekData() needs memory to
      store the peeked data (if the data is a string for example), it can be allocated
      normally with new or malloc. The resulting pointer should be stored by calling
      peekDataStore() prior to returning it. When peekDataStore() has been used to store
      the data pointer, Zoidcom will call clearPeekData() on the replicator as soon as
      the interceptor, which initiated the call to peekData(), returned. Alternatively,
      it is also possible to just return the pointer to the allocated memory, and make
      sure that the interceptors free the memory themselves, afterwards.

      The bitstream to peek from must be acquired with getPeekStream().
      It will be restored to it's previous read position after peekData() has returned.

      @attention This may only be called from inside ZCom_NodeReplicationInterceptor::inPreUpdateItem().
  */
  virtual void* peekData() = 0;

  /** @brief If peekData() has allocated memory, clear it here.

      This has to be implemented in order to clear up any memory allocated by peekData(). Use
      peekDataRetrieve() to acquire the pointer stored by peekDataStore() earlier. It is not
      necessary to call peekDataStore(NULL) to clear the pointer as Zoidcom will do this
      automatically.

      This method will get called right after ZCom_NodeReplicationInterceptor::inPreUpdateItem()
      returned but only if peekDataStore() has been used inside the interceptor.

      This method will also get called from inside peekDataStore() if it holds a pointer from
      a previous call to peekDataStore().
  */
  virtual void clearPeekData() = 0;

protected:
  /** @brief Get stream currently processed for peeking the data.
      @returns Pointer to current ZCom_BitStream.

      As you might have noticed, peekData() does not have a ZCom_BitStream parameter. That's the case
      because peekData() must be called from inside ZCom_NodeReplicationInterceptor::inPreUpdateItem(),
      and this interceptor callback has no access to the currently processed stream. Use this method
      to get a pointer to the currently processed stream instead.

      This will only return a valid result when called from inside the above mentioned interceptor
      callback.
  */
  ZCom_BitStream* getPeekStream() const;

  /** @brief Store pointer to allocated peekbuffer, so it can be deleted again.
      @param _ptr Pointer to the allocated memory.

      The pointer will get stored in a global Thread Local Storage, so that multiple ZCom_Controls
      can safely operate simultaneously in different threads. When a replicator's peekData() makes
      use of this method, expect to get a call to clearPeekData() sometime soon, which is supposed
      to free the allocated memory again.

      When you call peekDataStore() more than once with a pointer != NULL, clearPeekData() will
      get called automatically.

      Thread Local Storage means, there is one variable for each thread of the program. Replicators
      could as well declare a member variable used for that purpose instead, but that would waste a
      lot of memory when peeking interceptors are not used.
  */
  void peekDataStore(void *_ptr);

  /** @brief Retrieve the peekbuffer pointer currently stored.
      @returns Pointer previously stored with peekDataStore().
  */
  void* peekDataRetrieve();
  //@}
public:
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /*                processing                  */
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  /** @brief Do any kind of processing.
      @param _localrole States the role of the local node.
      @param _simulation_time_passed Time given to ZCom_Control::ZCom_processReplicators()

      This is intended for replicators that need to perform constant processing
      like interpolators and extrapolators. Called once everytime ZCom_Control::ZCom_processReplicators()
      is called.

      This method will only get called when the replicator has the \ref ZCOM_REPLICATOR_CALLPROCESS
      flag set.
  */
  virtual void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) = 0;

  // if this returns false, Process() won't get called.
  inline bool callProcess() { return (m_flags & ZCOM_REPLICATOR_CALLPROCESS) ? true : false ; }
public:
  ZCom_ReplicatorSetup *getSetup() const { return m_setup; }
  zU8 getFlags() const { return m_flags; }
  zU16 getId() const { return m_id; }
};



#endif


