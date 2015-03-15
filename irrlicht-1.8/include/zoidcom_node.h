/****************************************
* zoidcom_node.h
* node object
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDNODE_H_
#define _ZOIDNODE_H_

/** \file zoidcom_node.h
*/

#include "zoidcom.h"

class ZCom_Node_Private;
class ZCom_Control;
class ZCom_BitStream;
class ZCom_NodeEventInterceptor;
class ZCom_NodeReplicationInterceptor;
class ZCom_Replicator;

/*
** enums and typedefs
*/

/** \anchor repflags
    \name Data replication flags (how to replicate)
*/
//@{
/// no flags
#define ZCOM_REPFLAG_NONE             0
/// Replicate unreliably.
#define ZCOM_REPFLAG_UNRELIABLE       (1L << 0)
/// Replicate reliable, always sending the most recent version of the data.
#define ZCOM_REPFLAG_MOSTRECENT       (1L << 1)
/** @brief Data changes seldomly.

    This data changes relatively seldom, so transmission is optimized upon that assumption.
    All rarely-changed data is put in a special group. If none of this data needs to be updated,
    there is zero overhead for having this data registered, but if only one of the rarely-changed
    data items needs to be updated, all of the other items have to at least send control
    data stating that they don't need to be updated, which is what normal data is doing all the
    time.
*/
#define ZCOM_REPFLAG_RARELYCHANGED    (1L << 2)
/// This data shall be replicated only once, implicates ZCOM_REPFLAG_RARELYCHANGED.
/// Can't be used together with ZCOM_REPFLAG_STARTCLEAN!
#define ZCOM_REPFLAG_ONLYONCE         (1L << 3)
/// This data will generate intercept events when an interceptor is assigned
#define ZCOM_REPFLAG_INTERCEPT        (1L << 4)
/** @brief Allow reference to ZCom_ReplicatorSetup object.

    Use this when you have set up the ZCom_ReplicatorSetup object yourself and
    it is static or persists longer than all ZCom_Nodes and ZCom_Controls. If
    set, ZCom_ReplicatorSetup::Duplicate() will just return a pointer to itself
    instead of duplicating, which saves memory. This is only relevant if
    ZCom_Node::unregisterNode() is used, which might need to make internal
    copies of the replicators and their respective setups.
*/
#define ZCOM_REPFLAG_SETUPPERSISTS    (1L << 5)
/** @brief Automatically delete ZCom_ReplicatorSetup object when replicator gets deleted.

    Use this if you want ZCom_Node to automatically delete a dynamically allocated
    replicator setup on destruction. Do NOT use it if the replicator setup is used in
    more than one replicator.

    @attention Never use together with \ref ZCOM_REPFLAG_SETUPPERSISTS.
*/
#define ZCOM_REPFLAG_SETUPAUTODELETE  (1L << 6)
/** @brief Tell replicator to start clean, i.e. don't mark data dirty.

    By default, a new replicator marks it's data as dirty. That means, it has
    to be replicated. With this flag set the replicator will start off clean,
    not replicating any data until a change in the data triggers replication.

    This can be used to optimize bandwidth when creating lots of nodes with
    replicators during operation. When an object starts off with the same
    values on the server as on the client, there is no need to replicate that
    data until it is modified on the server.

    @attention Can't be used together with ZCOM_REPFLAG_ONLYONCE.
*/
#define ZCOM_REPFLAG_STARTCLEAN       (1L << 7)
//@}

/**
 * @name Data & event replication rules (when to replicate)
 * @anchor reprules
 * @note For event sending, replication rules are only applied to the node from which events are sent. That means that events are
 *       not forwarded on the authority automatically if ZCOM_REPRULE_AUTH_2_ALL is used for sending an event from a proxy or
 *       owner node.
 */
//@{

/// no rule (no replication)
#define ZCOM_REPRULE_NONE             0
/// replicate data from authority -> proxy
#define ZCOM_REPRULE_AUTH_2_PROXY     (1L << 0)
/// replicate data from authority -> owner
#define ZCOM_REPRULE_AUTH_2_OWNER     (1L << 1)
/// replicate to all
#define ZCOM_REPRULE_AUTH_2_ALL       (ZCOM_REPRULE_AUTH_2_PROXY|ZCOM_REPRULE_AUTH_2_OWNER)
/// replicate data from owner -> authority
#define ZCOM_REPRULE_OWNER_2_AUTH     (1L << 2)
//@}

/** \name Size constants for bitstream embedded data */
/// @{

/// Size of file transfer ID.
#define ZCOM_FTRANS_ID_BITS            32
/// Size of file length information
#define ZCOM_FTRANS_SIZE_BITS          32
/// Size of file chunk ID
#define ZCOM_FTRANS_CHUNK_BITS         16
/// @}

/** @brief ZCom_Node represents one networkable object
    \nosubgrouping

    Each ZCom_Node represents one object in the network. There are several types of them:
      - Unique nodes are unique to each ZCom_Control, like controlling classes. They are identified
        solely by their \ref ZCom_ClassID.
      - Tag nodes are nodes which tend to exist over a long period of time and can be linked together
        in each ZCom_Control by a specified tag. Register two tagnodes of the same ZCom_ClassID in two
        ZCom_Controls each and let those ZCom_Controls connect, then these two tagnodes will link up, too.
      - Dynamic nodes are nodes which get created and deleted in an uncontrolled manner. If the server
        creates a dynamic node, it sends a request to all connections on the same \ref Zoidlevel, which
        then in turn have to create the counterpart to that node.

    Once nodes are linked up with their counterparts, they can send events back and forth as much as have
    their data synchronized and interpolated.

    ZCom_Nodes appear in three roles (\ref eZCom_NodeRole):
      - \ref eZCom_RoleAuthority is the master node
      - \ref eZCom_RoleProxy is the proxy node that will normally sync up to the authority's state
      - \ref eZCom_RoleOwner is a special proxy. The authority can make any proxy to an owner and define
             special replication rules for it.
*/

class ZCOM_API ZCom_Node
{
protected:
  ZCom_Node_Private *m_priv;

public:
  ZCom_Node( void );
  ~ZCom_Node( void );

  /********************/
  /* node setup       */

  /** \name Node registration

     Register your node with the system, the needed \ref ZCom_ClassID is the result of a call to
     ZCom_Control::ZCom_registerClass().

     @attention Don't use the ZCom_ClassIDs in your network communication yourself, client and server
                may use completely different IDs for the same class. They are translated by Zoidcom
                automatically but obviously Zoidcom is unable to do that if the IDs are embedded in
                your custom data.
  */

  //@{

  /** @brief Register a unique node.
      @param _classid This node's class.
      @param _role The role the node should have here.
      @param _control The ZCom_Control you want the node to register with.
      @returns 'true' if everything went fine.
  */
  bool
    registerNodeUnique( ZCom_ClassID _classid, eZCom_NodeRole _role, ZCom_Control *_control );

  /** @brief Register a tagnode, initially or on request.
      @param _classid This node's class.
      @param _tag The tag you designated for this node.
      @param _role The role the node should have here.
      @param _control The ZCom_Control you want the node to register with.
      @returns 'true' if everything went fine.

      If, for some reason, the server registers a tagnode and cannot find it on the client, the callback
      ZCom_Control::ZCom_cbNodeRequest_Tag() will be called on client. From within this callback, the corresponding
      tagnode has to be created and registered with registerNodeByTag().
  */
  bool
    registerNodeByTag( ZCom_ClassID _classid, zU32 _tag, eZCom_NodeRole _role, ZCom_Control *_control );

  /** @brief Register an authoritative dynamic node.
      @param _classid This node's class.
      @param _control The ZCom_Control you want the node to register with.
      @returns 'true' if everything went fine.

      The node's role will be \ref eZCom_RoleProxy if the node is created inside ZCom_Control::ZCom_cbNodeRequest_Dynamic(),
      \ref eZCom_RoleAuthority otherwise.
  */
  bool
    registerNodeDynamic( ZCom_ClassID _classid, ZCom_Control *_control );

  /** @brief Register a requested dynamic node.
      @param _classid This node's class.
      @param _control The ZCom_Control you want the node to register with.
      @returns 'true' if everything went fine.

      The callback ZCom_Control::ZCom_cbNodeRequest_Dynamic() has been called and now a dynamic node
      is expected to be created on the client. Once the new node is created and setup, it has to be
      registered with this call from within the callback.

      The node's role will be \ref eZCom_RoleProxy if the node is created inside ZCom_Control::ZCom_cbNodeRequest_Dynamic(),
      \ref eZCom_RoleAuthority otherwise.

      @deprecated Use registerNodeDynamic() instead.
  */

  bool
    registerRequestedNode( ZCom_ClassID _classid, ZCom_Control *_control );

  /** @brief Unregister the node from ZCom_Control.
      @returns 'true' if no errors occurred.

      This will free all links of this node into the zoidcom library, you can delete it, copy it, or
      put it into an object pool for later use.

      All node options, setups, replicators etc are lost. Actually, you can just as well delete the node and
      create a new one. Using unregisterNode() will just spare you one memory allocation.

      Calls disconnectAll().
  */
  bool
    unregisterNode();

  /** @brief Unlink nodes from all peers.

      Use this if the node is going to be deleted soon, the earlier this is called, the earlier peers will get
      notified about the removal and can react accordingly. You can also just delete this instead, if you don't
      need it any longer.
  */
  void
    disconnectAll();

  //@}

  /** \name Node parameters
  */

  //@{

  /** @brief Set relative object priority. (Allowed on: Authority, Owner, Proxy)
      @param _prio The new priority.

      Setting object priorities influences how often Zoidcom will send updates for this object to peers and
      how soon events will be sent.
  */
  void
    setUpdatePriority( zU16 _prio );

  /** @brief Set default relevance of this node to all connections. (Allowed on: Authority)
      @param _default_relevance The relevance factor from 0.0 (don't replicate) - 1.0 (full relevance)

      The default relevance can be overriden with @ref setConnectionSpecificRelevance.

      @attention This should be called before registering the node, otherwise it will only apply
                 to new connections.
  */
  void
    setDefaultRelevance( zFloat _default_relevance );

  /** @brief Set current relevance of this node to a specific connection. (Allowed on: Authority)
      @param _conn The connection the new relevance is meant for.
      @param _rel The relevance factor from 0.0 - 1.0.

      Set current relevance for this connection ( 0.0 - 1.0 ), which is multiplied with the update-priority.
      This will influence the frequency, with which the node is updated for the specified connection.

      See also: getRelevantConnectionCount(), getRelevantConnections()
  */
  void
    setConnectionSpecificRelevance( ZCom_ConnID _conn, zFloat _rel );

  /** @brief Find out how many connections are relevant for this node. (Allowed on: Authority, Owner, Proxy)
      @returns Amount of connections currently linked to this node.
  */
  zU32
    getRelevantConnectionCount() const;

  /** @brief Find out which connections are relevant for this node. (Allowed on: Authority, Owner, Proxy)
      @param _conns A ptr to an array of ZCom_ConnIDs.
      @param _max Size of that array.
      @param _count Ptr to an int which will contain the number of valid IDs in the array afterwards.

      This gives you an array with IDs of all connections which are interested in updates of this node.

      @returns
        - 1 if everything went ok
        - 0 if there was an error ( like the array being NULL )
        - -1 the array was too small to hold all connections

      See also: getRelevantConnectionCount(), setConnectionSpecificRelevance()
  */
  zS32
    getRelevantConnections( ZCom_ConnID *_conns, zU32 _max, zU32 *_count ) const;

  /** @brief Force replication order of nodes by making this dependent on _othernode. _othernode is replicated first. (Allowed on: Authority)
      @param _othernode The node that shall be replicated before this node.
      @param _opt Whether to add or remove the dependency.
      
      This method should be called on authority nodes only.

      This will force _othernode to be replicated before this node whenever both nodes need to be replicated 
      to a connection. It will show no effect if one of the nodes can't be replicated (because it is private or 
      the relevance is 0) or if one of the nodes is replicated while the other is not.

      Don't make cyclic dependencies. Don't add the same dependency multiple times. (The debug version of Zoidcom will
      check for latter case).
  */
  void
    dependsOn( ZCom_Node *_othernode, eZCom_DependencyOpt _opt = eZCom_AddDependency );

  /** @brief Add node to Zoidlevel. (Allowed on: Authority)
      @param _level The level.

      This method should be called on authority nodes only.

      You can manually switch connections to different Zoidlevels, only nodes which
      have applied for a specific level will be synced while the connection is in this
      level. All nodes are in ZoidLevel 1 by default.
  */
  void
    applyForZoidLevel( zU8 _level );

  /** @brief Remove node from Zoidlevel. (Allowed on: Authority)
      @param _level The level.

      This method should be called on authority nodes only.

      See also: applyForZoidlevel().
  */
  void
    removeFromZoidLevel( zU8 _level );

  /** @brief Get amount of Zoidlevels this node is registered in.
      @returns Amount of Zoidlevels.

      Only relevant on authority nodes.
  */
  zU32
    getZoidLevelCount( ) const;

  /** @brief Get one of the nodes Zoidlevels.
      @param _index Must be in range 0 <= _index < getZoidLevelCount()
      @returns One of the node's levels. 0 = invalid.

      Use this in conjunction with getZoidLevelCount to enumerate the node's levels.
  */
  zU8
    getZoidLevel( zU32 _index ) const;

  /** @brief Force this node to successfully sync during Zoidlevel transition. (Allowed on: Authority)
      @param _enabled 'true' if node must sync, 'false' otherwise.
      @param _order sync order relative to other mustsync nodes.

      When a connection starts the transition to a Zoidlevel, the first thing it does is replicate
      and connect all nodes in the new Zoidlevel, which have the mustsync flag set. They are processed
      in an order which is the result of sorting all mustsync nodes by the _order param. If >1 nodes
      have the same order, their sync process gets started simultaneously.

      If any of the mustsync nodes reports failure, the whole Zoidlevel transition fails for the
      connection in question.

      This is only relevant for nodes in existence _before_ the connection changes to the node's
      Zoidlevel. Nodes created when a connection is already in the respective Zoidlevel will be
      handled like normal, mustsync=false nodes.

      All other nodes ( which don't have MustSync=true ) are synced after all MustSync-nodes have synced.

      <b>UseCase:</b>\n\n
                Client joins server. There is a ResourceControl class with it's affiliated node. This node has
                MustSync=true with a unique _order=1, so it will be synced before anything else. The node will
                communicate with it's newly connected peer about the files present on the client's system.
                The client is missing a file.

                - Case 1(File download enabled on server): The ResourceControl will transmit the missing file, and
                                                        give the OK to Zoidcom. The node has synced successfully
                                                        and the following nodes will now attempt to sync.

                - Case 2(File download disabled on server): The ResourceControl will tell Zoidcom that the Node failed
                                                        to sync, the Zoidlevel transition is canceled, an error
                                                        message is sent to the client. The client falls back to it's
                                                        previous zoidlevel.

      See also: setSyncResult(), setSyncResultAutoSuccess()
  */
  void
    setMustSync( bool _enabled, zU16 _order );

  /** @brief Report sync result to Zoidcom. (Allowed on: Authority)
      @param _conn The connection which is waiting for a result.
      @param _success 'true' to report success, 'false' to report failure.
      @param _errormsg Pass additional failure information in this bitstream. The bitstream will be taken over
                       by Zoidcom, so there is no need to delete it manually.

      This must be called when mustsync is true, when either the node synced successfully on the connection or not.
      If the sync failed for the connection, you can provide a ZCom_BitStream which will be sent to the client, and given
      to the callback ZCom_Control::ZCom_cbZoidResult().

      @attention If you have a mustsync node, and forget to call this method for this node, the Zoidmode transition will last
                 forever!

      See also: setMustSync(), setSyncResultAutoSuccess()
  */
  void
    setSyncResult( ZCom_ConnID _conn, bool _success, ZCom_BitStream *_errormsg );

  /** @brief Tell node to automatically report all sync requests as successful. (Allowed on: Authority)
      @param _enabled True to enable this behavior.

      Enable this to automatically set the sync result to true as soon as the request to sync is generated,
      the node won't receive the request anymore. This only applies to mustsync nodes.

      This method should be called on authority nodes only.

      See also: setSyncResult(), setMustSync()
  */
  void
    setSyncResultAutoSuccess( bool _enabled );

  /** @brief Give owner-status to proxy nodes on clients. (Allowed on: Authority)
      @param _id The connection on which this node's proxy shall receive owner status (eZCom_RoleOwner)
      @param _enabled 'true' to set owner status, 'false' to remove it.

      This permits the client on the other side of the connection a
      higher authority over the node. Normally, all nodes created on a client
      are eZCom_RoleProxy. It can advance to an eZCom_RoleOwner if the server
      permits that (with this method). This advancement won't do anything special
      on it's own, instead, the application is responsible to take some action here.
      You can define data to be replicated only to eZCom_RoleOwner nodes. Also,
      when sending events you can specify if only the owner or all proxies
      should receive the event. Furthermore, the application can filter events
      based on the role of their sender.

      Example scenario: A spaceship. The server of course has the authority over it,
      and there are 10 proxies (which replicate what the server object is doing).
      Now you declare one proxy to become the owner of that ship (in fact, one
      object may have several owners, and one client can own several objects, it
      totally depends on the application what it means to be an owner). This owner
      (which is a client) now can send events to the server node which are not
      dropped because it accepts these specific events from an owner. In our scenario,
      these events are control commands to steer the ship and fire. In turn,
      the server would replicate some additional data (as set by the application),
      like the current ammo left for each weapon and other special infos no other
      players need to or should know about.

      You can also use the owner status to delegate AI calculations to clients, for
      example.
  */
  void
    setOwner( ZCom_ConnID _id, bool _enabled );

  /** @brief Make node private. (Allowed on: Authority)
      @param _enabled 'true' to make private, 'false' to make public. (default=false)

      If a node is private, it will only get replicated to those connections, for which the node's
      role is \ref eZCom_RoleOwner. See also: \ref setOwner()
  */
  void
    setPrivate( bool _enabled );

  /**
   * Set data that will be attached to each announcement of this node to a client.
   *
   * The data needs to be set only on server side. It will be used only if the
   * node's classname (registered with ZCom_Control::ZCom_registerClass())
   * has been registered with the @ref ZCOM_CLASSFLAG_ANNOUNCEDATA flag.
   * When the flag is set, such data MUST be available.
   * The client needs to be able to read exactly the amount of data which has
   * been set here, as no size information will be transmitted.
   *
   * The bitstream goes into ownership of Zoidcom, it may not be deleted or
   * used otherwise after giving it to the node.
   *
   * More information about this topic can be found in the documentation to the flag.
   */
  void
    setAnnounceData( ZCom_BitStream *_data );

  //@}

  /********************/
  /* data replication */

  /** \name Data replication setup
        \anchor datarepsetup
      @brief Setup the data replication rules. They have to be exactly the same for all nodes of the same class on all systems.
  */
  //@{

  /** @brief Enclose all data replication setup functions with this and endReplicationSetup().
      @param _replicators_max If set to >0, Zoidcom will preallocate memory for the replicators.
             If you leave it at 0 or you register more replicators than hinted here, reallocations
             have to be performed and maybe more memory than necessary will end up being used.
  */
  bool
    beginReplicationSetup( zU16 _replicators_max );

  /** @brief Set intercept ID for all forthcoming replication registrations which have ZCOM_REPFLAG_INTERCEPT set
      @param _id The Id which will get assigned.

      The intercept ID is passed to the info structure available in the replication interceptor callback, enabling
      the application to distinguish between several data items.
  */
  void
    setInterceptID( ZCom_InterceptID _id );

  /** @brief Add an int to the data replication.
      @param _ptr Pointer to the data source.
      @param _bits The amount of relevant bits. Zoidcom will replicate no more than the relevant bits across the net link.
      @param _sign Store the sign bit as extra information, you don't need this if _bits is set to 32 on a 32 bit value (the sign is included
                   already in that case).
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.

			@attention Make sure that only pointers to zS32 or larger data types are passed here, since Zoidcom writes sizeof(zS32) bytes
			           on incoming updates!

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.
   */
  void
    addReplicationInt( zS32 *_ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1 );

  /** @brief Add a bool to the data replication.
      @param _ptr Pointer to the data source.
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.

   */
  void
    addReplicationBool( bool *_ptr, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1 );

  /** @brief Add a float to the data replication.
      @param _ptr Pointer to the data source.
      @param _mantissa_bits The amount of mantissa bits Zoidcom will replicate.
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.

      _mantissa_bits states how big your mantissa should be. Default for floats in C++ is 23. Additional 9 bits will be replicated
      for exponent and sign bit. The smaller the mantissa and the bigger the value of the float, the lower the precision will be.

      <b> Example: </b> original value(mantissa size) => replicated value
      - 10.723(10)   => 10.719
      - 20.7236(10)  => 20.719
      - 20.7236(8)   => 20.688
      - 20.7236(6)   => 20.5
      - 20.7236(4)   => 20.0
      - 100.723(10)  => 100.688
      - 1000.723(10) => 1000.500
      - 10000.723(10)=> 10000
      - 10000.723(6) => 9984

      In some cases, it may be better to make your own conversion. Take your float,
      create an approximation value for it and store it in an int, and register the
      int instead. Every time the float changes, the approx. value in the int has to
      be recomputed, or at least every time before you tell Zoidcom to process the
      updates. On the other side, you register that int, too. And every time after
      Zoidcom has processed the incoming updates, convert it back.
  */
  void
    addReplicationFloat(zFloat *_ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);

  /** @brief Add a string to the data replication.
      @param _str Pointer to the data source.
      @param _maxlen The maximum length the string will have. Zoidcom only replicates the actual length, but it is needed for other reasons.
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.

   */
  void
    addReplicationString( char *_str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);

  /** @brief Add a string to the data replication.
  @param _str Pointer to the data source.
  @param _maxlen The maximum length the string will have. Zoidcom only replicates the actual length, but it is needed for other reasons.
  @param _flags Combination of \ref repflags "Replication Flags", ORed together.
  @param _rules Combination of \ref reprules "Replication Rules", ORed together.
  @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
  @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.

  The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
  like the client's connection, node priority and node relevance.

  */
  void
    addReplicationStringW( wchar_t *_str, zU16 _maxlen, zU8 _flags, zU8 _rules, zS16 _mindelay = -1, zS16 _maxdelay = -1);

  // - same as above, but received values are interpolated every time ProcessInput is called
  // - the new parameter _ipolfac must be between 0 and 1, the higher it gets, the more influence the incoming data has
  // - the *_dst parameters will be used as interpolation target if given ( may be NULL ):
  //    incoming network updates are written to _dst
  //    the value in _ptr is interpolated against _dst
  //    you can change the contents of _dst at will and _ptr will be interpolated against the new value

  /** @brief Add an int to the data replication and ask Zoidcom to interpolate it.
      @param _ptr Pointer to the data source.
      @param _bits The amount of relevant bits. Zoidcom will replicate no more than the relevant bits across the netlink.
      @param _sign Store the sign bit as extra information, you don't need this if _bits is set to 32 on a 32 bit value (the sign is included
                   already in that case).
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _treshold The error margin. If the difference between the received and the current value is bigger than _treshold,
                       Zoidcom will just set current = received without interpolation.
      @param _dst If set, Zoidcom will store incoming updates in _dst and interpolate against _dst. If not set, Zoidcom will use internal
                  memory. Read the detailed description to find out more.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.
      @param _ipolfac Interpolation factor, the higher it becomes, the higher the influence of the incoming data over the local data.
                      Set to 0 to ignore incoming data, or 1 to just write incoming data to _ptr.

			@attention Make sure that only pointers to ints or larger data types are passed here, since Zoidcom writes sizeof(int) bytes
                 on incoming updates!

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.

      Setting the _dst parameter orders Zoidcom to store incoming updates in application provided memory. Now it is possible to provide
      custom interpolation by setting the _ipolfac to 0 and interpolate manually between _dst and _ptr. Or let Zoidcom handle interpolation
      with _ipolfac > 0, but the application can also write to _dst itself thus forcing Zoidcom to interpolate against clientside generated
      values. (E.g. You could process your physics locally as means of dead reckoning and store the current status is _dst. Zoidcom will then
      interpolate against the locally computed value, and also against incoming values from the server, which overwrite _dst).

      See also: addReplicationInt()
  */
  void
    addInterpolationInt( zS32 *_ptr, zU8 _bits, bool _sign, zU8 _flags, zU8 _rules, zS32 _treshold, zS32 *_dst = NULL,
    zS16 _mindelay = -1, zS16 _maxdelay = -1, zFloat _ipolfac = 0.4f );

  /** @brief Add a float to the data replication and ask Zoidcom to interpolate it.
      @param _ptr Pointer to the data source.
      @param _mantissa_bits The amount of mantissa bits Zoidcom will replicate.
      @param _flags Combination of \ref repflags "Replication Flags", ORed together.
      @param _rules Combination of \ref reprules "Replication Rules", ORed together.
      @param _treshold The error margin. If the difference between the received and the current value is bigger than _treshold,
                       Zoidcom will just set current = received without interpolation.
      @param _dst If set, Zoidcom will store incoming updates in _dst and interpolate against _dst. If not set, Zoidcom will use internal
                  memory. Read the detailed description to find out more.
      @param _mindelay Measured in msecs and defines how much time <b>has to pass</b> between two consecutive updates of this data.
      @param _maxdelay Measured in msecs and defines how much time <b>may pass</b> between two consecutive updates of this data.
      @param _ipolfac Interpolation factor, the higher it becomes, the higher the influence of the incoming data over the local data.
                      Set to 0 to ignore incoming data, or 1 to just write incoming data to _ptr.

      The update frequency bounds as defined by _mindelay and _maxdelay are not guaranteed. The resolution highly depends on factors
      like the client's connection, node priority and node relevance.

      Setting the _dst parameter orders Zoidcom to store incoming updates in application provided memory. Now it is possible to provide
      custom interpolation by setting the _ipolfac to 0 and interpolate manually between _dst and _ptr. Or let Zoidcom handle interpolation
      with _ipolfac > 0, but the application can also write to _dst itself thus forcing Zoidcom to interpolate against clientside generated
      values. (E.g. You could process your physics locally as means of dead reckoning and store the current status is _dst. Zoidcom will then
      interpolate against the locally computed value, and also against incoming values from the server, which overwrite _dst).

      See also: addReplicationFloat() for more details on float replication.
  */
  void
    addInterpolationFloat( zFloat *_ptr, zU8 _mantissa_bits, zU8 _flags, zU8 _rules, zFloat _treshold, zFloat *_dst = NULL,
    zS16 _mindelay = -1, zS16 _maxdelay = -1, zFloat _ipolfac = 0.4f );

  /** @brief Add replicator object.
      @param _rep Fully setup and initialized replicator.
      @param _autodelete If true, the replicator will be deleted when the node gets deleted.

      With this method you can register your own replicator to the node. When the node gets deleted,
      the replicator will get deleted if _autodelete is set.

      The replicator setups however, won't get deleted. (Replicator setups are the configuration object
      passed to the replicator's constructor.

      If you want to save memory, you'd better use this method for all your replication needs. Only this method
      allows you to use one (static) ZCom_ReplicatorSetup for several replicator objects, while the other
      replication methods allocate one ZCom_ReplicatorSetup object for each replication item.

      Read the manual for more in depth information on this.
  */
  void
    addReplicator(ZCom_Replicator *_rep, bool _autodelete);

  /** @brief End replication setup.

      This finalizes the replication setup. Make sure that nodes of the same ZCom_ClassID have the same replication setup.
      Important for that are especially:
        - order of replication elements (addReplicationXXX...)
        - the amount of bits replicated
        - the _flags
  */
  bool
    endReplicationSetup( void );

  /** @brief Set replication interceptor for this node.
      @param _interceptor Pointer to interceptor object.

      Refer to ZCom_NodeReplicationInterceptor documentation for further information.
  */
  void
    setReplicationInterceptor(ZCom_NodeReplicationInterceptor *_interceptor);
  //@}

  /*********************/
  /* event replication */

  /** \name Event replication and file transfers
  */
  //@{

  /** @brief Send an event to the peers selected with _rules: \ref reprules "Replication Rules"
      @param _mode \ref eZCom_SendMode
      @param _rules \ref reprules "Replication Rules"
      @param _data ZCom_BitStream with your event data. It will be deleted eventually.
      @returns 'true' on success.
  */
  bool
    sendEvent( eZCom_SendMode _mode, zU8 _rules, ZCom_BitStream *_data );

  /** @brief Send an event to a connected node on a specific connection only
            (needed for initializing and syncing, see \ref eZCom_Event)
      @param _mode \ref eZCom_SendMode
      @param _destconn Connection of the destination node.
      @param _data ZCom_BitStream with your event data. It will be deleted eventually.
      @returns 'true' on success.
  */
  bool
    sendEventDirect( eZCom_SendMode _mode, ZCom_BitStream *_data, ZCom_ConnID _destconn );

  /** @brief Send an event to connected nodes on a group of connections.
      @param _mode \ref eZCom_SendMode
      @param _destgroup Valid group ID from ZCom_Control::ZCom_getGroupManager()
      @param _data ZCom_BitStream with your event data. It will be deleted eventually.
      @returns 'true' on success.
  */
  bool
    sendEventToGroup( eZCom_SendMode _mode, ZCom_BitStream *_data, ZCom_GroupID _destgroup );

  /** @brief Toggle connect/remove notification.
      @param _oninit Generate event if a new proxy node connected.
      @param _onremove Generate event if a proxy node is removed.
  */
  void
    setEventNotification( bool _oninit, bool _onremove );

  /** @brief Check for new events.
      @returns 'true' if an event is waiting.
  */
  bool
    checkEventWaiting() const;

  /** @brief Get the next event.
      @param _type Pass a ptr to eZCom_Event here, the event's type will be stored there. May be NULL.
      @param _remote_role Pass a ptr to eZCom_NodeRole here, the sender's role will be stored there. May be NULL.
      @param _connid Pass a ptr to ZCom_ConnID here, the sender's connection ID will be stored here. May be NULL.
      @param _estimated_time_sent Pass a ptr to zFloat here, the estimated sending time will be stored here. May be NULL.
      @returns Bitstream containing the event data. Can be NULL.

     The event will be deleted automatically afterwards along with the ZCom_BitStream.
     The contained data for special events is detailed \ref eZCom_Event "here".

     The _estimated_time_sent parameter gives an msec value trying to estimate the time the event has left the remote
     ZCom_Control. Execute "ZoidCom::getTime() - _sent_time" to find out the event's travel time in msecs.
  */
  ZCom_BitStream*
    getNextEvent( eZCom_Event* _type, eZCom_NodeRole* _remote_role, ZCom_ConnID* _connid, zU32* _estimated_time_sent = NULL );

  /** @brief Send a file to a specific connection.
     @param _data is optional. use it to pass additional information about the file to the receiver.
     @param _path is the full path to the local file which is to be sent.
     @param _pathtosend is the path the peer will receive for that file, if NULL, _path will be used.
     @param _destconn Destination connection.
     @param _aggressivenes can be anything >0.0 - 1.0. If it is 1.0, it will always use up the complete
                           rest of the stream for sending the file. That means, when this node is asked
                           to fill the stream, no other node after this in the priority queue can fill
                           in it's data. Otherwise it will fill in (_aggresivenes * bytes left in stream)
                           bytes.
     @returns A FileTransferID you can use to check the status of the transfer. Equals ZCom_InvalidID on error.

     The destination will receive the request in the form of an eEvent_File_Incoming and can reply with acceptFile().
  */
  ZCom_FileTransID
    sendFile( const char *_path, const char *_pathtosend, ZCom_ConnID _destconn, ZCom_BitStream *_data, zFloat _aggressivenes );

  /** @brief Accept or deny an incoming file.
      @param _src_id Source connection.
      @param _ftrans_id Transfer ID as contained in the eEvent_File_Incoming event.
      @param _path Store location. If NULL, the file will be saved in the location that came with the request.
      @param _accept Set to 'true' to accept the transfer, 'false' otherwise.
  */
  void
    acceptFile( ZCom_ConnID _src_id, ZCom_FileTransID _ftrans_id, const char *_path, bool _accept );

  /** @brief Get data for a file transfer.
      @param _conn_id Connection of the transfer.
      @param _ftrans_id Id of that transfer.
      @returns Reference to a filled ZCom_FileTransInfo structure. If transfer is invalid, the resulting
               struct will have ZCom_Invalid_ID in the id field. All other fields are invalid then.
  */
  const ZCom_FileTransInfo&
    getFileInfo( ZCom_ConnID _conn_id, ZCom_FileTransID _ftrans_id ) const;

  /** @brief Set event interceptor for this node.
      @param _interceptor Pointer to interceptor object.

      Refer to ZCom_NodeEventInterceptor documentation for further information.
  */
  void
    setEventInterceptor(ZCom_NodeEventInterceptor *_interceptor);
  //@}

  /*********************/
  /*     user data     */

  /** \name User data
      @{
  */

  /** @brief Set user data pointer per relevant connection.
      @param _conn The connection the userdata is associated to.
      @param _data Pointer to the data.
      @returns true if the connection is relevant to the node and the data has been attached.

      After the connection has been closed, the attached data cannot be retrieved anymore.
  */
  bool
    setUserData( ZCom_ConnID _conn, void *_data );

  /** @brief Retrieve user data.
      @param _conn The connection ID the data was attached to.
      @returns Pointer to the previously stored data.
  */
  void*
    getUserData( ZCom_ConnID _conn ) const;

  /** @brief Set global user data pointer.
      @param _data Pointer to the data.

      In constrast to the other setUserData() method, this one stores the pointer without
      association to a connection.
  */
  void
    setUserData( void *_data );
  /** @brief Retrieve global user data pointer.
      @returns Previously stored data pointer.
  */
  void*
    getUserData() const;

  // @}

  /** \name Misc.
      @{
  */


  /** @brief Get time in msecs since last received update packet.
      @returns Time in msecs since last received update packet or -1 if no update occured, yet.
  */
  zS32
    getUpdateDelta() const;

  /** @brief Get packet rate of source connection.
      @returns The current packet send rate of the data source connection.

      This returns the current packet rate from the data source connection.
      For eZCom_RoleProxy nodes, the send rate of the connection with the eZCom_RoleAuthority node
      is returned, and for authority nodes, the send rate of the connection with the eZCom_RoleOwner node
      is returned.
  */
  zU32
    getCurrentUpdateRate() const;

  /** @brief Get estimated time in msecs until next possible update arrives.
	  @returns Estimated time in msecs or -1 if the information cannot be retrieved.

	  This tries to estimate the time until the next packet arrives from the data source connection.
	  This packet won't necessarily contain an update for this node, but it <b>could</b>.
  */
  zS32
    getEstimatedTimeUntilPossibleUpdate() const;

  /** @brief Get the ZCom_Control this node is registered with.
      @returns A ZCom_Control pointer, or NULL if node is not registered.
  */
  ZCom_Control*
    getControl() const;

  /** @brief Get role of this node.
      @returns The node's current role.
  */
  eZCom_NodeRole
    getRole() const;

  /** @brief Find out if node is in private mode.
      @returns bool 'true' if node is private.

      See also: setPrivate()
  */
  bool
    isPrivate() const;

  /** @brief Get node's class ID.
      @returns ZCom_ClassID. The class ID this node represents.
  */
  ZCom_ClassID
    getClassID() const;

  /** @brief Retrieve the node's network ID.
      @returns ZCom_NodeID

      Once the node is registered to a ZCom_Control and linked up to other nodes it owns a network ID.
      This ID is equal to all ZCom_Nodes in all connected ZCom_Controls representing the same object.
      If the node's role is eZCom_RoleAuthority, the network ID is available as soon as the node is
      registered, if the role is proxy or owner, the ID will be available as soon as the node is linked
      up to a authority node, so after the node has received eEvent_Init or eEvent_SyncRequest.
  */
  ZCom_NodeID
    getNetworkID() const;

  /// @}
  };

#endif
