/****************************************
* zoidcom_control.h
* network control class
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDCONTROL_H_
#define _ZOIDCONTROL_H_

#include "zoidcom.h"

/** \file zoidcom_control.h
*/

/*
** forwards
*/

class ZCom_Control_Private;
class ZCom_ConnGroupManager;
class ZCom_BitStream;
class ZCom_Address;
class ZCom_Node;

/*
** enums and typedefs
*/

/**
 * @{
 * @name Class Flags
 * @anchor classflag
 * Flags used with ZCom_Control::ZCom_registerClass().
 */

/**
 * @brief Indicate the use of additional data when replicating a node of this
 *        class.
 *
 * Each node of this class has a custom bitstream that is sent along with
 * the announcement to each client. This bitstream will be received by
 * ZCom_Control::ZCom_cbNodeRequest_Dynamic() or
 * ZCom_Control::ZCom_cbNodeRequest_Tag(). Use it to pass additional data
 * to clients. Data they need for creating the announced object.
 *
 * When this flag is set on a class, the server expects the data to be
 * available on a node when it tries to replicate it. Set the data with
 * ZCom_Node::setAnnounceData().
 *
 * The flag must be set on all systems, since existence of this data is not
 * explicitely embedded into the network stream, but implicated only be the
 * use of this flag.
 *
 * You can set the data for each target client indivually by implementing
 * ZCom_NodeReplicationInterceptor::outPreReplicateNode(), which will
 * get called directly before Zoidcom replicates the node in question.
 * Inside this interceptor callback, ZCom_Node::setAnnounceData() can
 * be called to change the announce data right before Zoidcom uses it.
 */
#define ZCOM_CLASSFLAG_ANNOUNCEDATA   (1L << 0)

/// @}

/* ===================================== */

/** @brief Network host.

    This class represents one host in the network. It manages ZCom_Nodes, connections, bandwidth
    distribution, etc. For localhost games, two ZCom_Controls should be present in one process,
    communicating through an eZCom_SocketLocal type socket.
*/
class ZCOM_API ZCom_Control
{
protected:

  ZCom_Control_Private *m_priv;

  /************************************/

public:
  ZCom_Control( void );

  /** @brief Destructor.

      It is advised to call ZCom_disconnectAll() before deleting a running ZCom_Control.
      That way it is possible to send a disconnect reason to the connected peers. This
      destructor will call ZCom_disconnectAll(), too, but without sending disconnect reason
      information.
  */
  virtual ~ZCom_Control( void );

  /** @brief Free all resources.

      Called by the destructor. If you want to free resources before the destructor gets called
      use this.

      This is if you for example need to make sure all replicators are deleted at a certain point.
      Although you deleted all nodes already, Zoidcom keeps them internally until connected peers
      have been notified about it, and due to that, the replicators will get deleted late, too.

      @attention Doing anything else besided deleting this object after calling Shutdown() is not
                 supported and results in undefined behaviour.
  */
  void Shutdown( void );

  /** @brief Set name shown in logoutput.
      @param _name string containing the name you want to use as debug name for this instance.

      If _name is NULL, the debug name will be the memory adress of this instance. (default)
   */

  void
    ZCom_setDebugName( const char *_name );

  /** @brief Create the local sockets.
      @param _useudp  True to create UDP socket
      @param _udpport The port the UDP socket will be bound to, or 0 to let OS decide.
      @param _localport The port the internal socket will be bound to, or 0 if socket
                        won't be used.
      @param _control_id_size This will be forwarded to the sockets. If it differs
                              from another ZCom_Control's (using the same socket),
                              the function fails. _control_id_size is measured in bits.
                              See also: \ref socketsharing
      @returns 'true' on success, 'false' otherwise

      This method initializes the sockets that will be used by this ZCom_Control.
      ZCom_Controls can communicate via UDP if located on different machines or in
      different processes, and they can communicate via internal sockets if they are
      in the same process. (i.e. Two ZCom_Control instances created in one application).
  */

  bool
    ZCom_initSockets( bool _useudp, zU16 _udpport, zU16 _localport, zU8 _control_id_size = 0);

  /** @brief Set how control reacts to discover requests.
      @param _opt The new behavior to apply.
      @param _discoverport UDP port to listen for discover requests on. Can be the same as the
                           control's UDP port initialised by ZCom_initSockets(). The port
                           parameter is only used for eZCom_DiscoverEnable.
      @returns 'true' on success, 'false' otherwise

      Use this to start or stop listening to discover events initiated by
      ZCom_Control::ZCom_Discover(). If \ref eZCom_DiscoverDisableAndKeep is used, the control
      will keep the UDP port open, but won't answer to incoming requests anymore. This is useful
      if you want to temporarily disable listening to broadcasts. Calling \ref eZCom_DiscoverEnable
      again with the same port as before will revert that change. Only one port can be used for
      listening, so if you select another port the first one will be closed automatically.

      When an incoming discover request is received, the callback ZCom_cbDiscoverRequest() is
      called.

      Default is no broadcast listening.
  */

  bool
    ZCom_setDiscoverListener(  eZCom_DiscoverOpt _opt, zU16 _discoverport );

  /** @brief Set this ZCom_Control's control ID.
      @param _id The ID. The Maximum value is (2^controlidsize}-1, which is set on a per
                 socket basis by ZCom_initSockets.

      Control ID's are used to identify several ZCom_Controls occupying the same UDP socket(i.e. the
      same local UDP port). Each packet is preceeded with a variable sized ID which tells the system
      which ZCom_Control is addressed by a packet.

      => ZCom_Controls which want to communicate with each other need the same ID size, which is set
         on a per socket basis, see zoidcom_socket.h.
      => This is another layer above the TCP/UDP port concept, making it easy to make do with just one
         single real UDP port.
   */

  void
    ZCom_setControlID( zU8 _id );

  /** @brief Limit outbound traffic in total and per connection, set to 0 to disable limit.
      @param _total_bps The maximum amount of bytes per second to be sent by this ZCom_Control.
      @param _perconn_bps The maximum amount of bytes per second for each single connection.

      Traffic will be dynamically distributed among all connections when a total limit is set,
      according to the number of connections and the bandwidth quality each of these connections
      requested. The maximum won't be obeyed exactly, most of the time the outgoing bandwidth will
      be nearly the maximum or a little bit over it.
  */
  void
    ZCom_setUpstreamLimit( zU32 _total_bps, zU32 _perconn_bps );

  /** @brief Register a new class.
      @param _name The name of this class, which must be unique to this ZCom_Control.
      @param _class_flags Additional options for this class. Or them together. See @ref classflag "Class flags".
      @return A ZCom_ClassID >0 on success or 0 on error.

     Register a class with this control. The returned ID is needed later for node setup. If you want
     to use the same class with another ZCom_Control instance, you have to register it there, too.
     (And of course use the other ID afterwards, given by the other control!)
  */
  ZCom_ClassID
    ZCom_registerClass( const char *_name , zU32 _class_flags = 0);

  /** @brief Retrieve class ID of registered class.
      @param _name The name of the class.
      \return A valid ZCom_ClassID >0 or 0 if class not found.

      This is currently O(n) with n = # of registered classes.
  */
  ZCom_ClassID
    ZCom_getClassID( const char *_name ) const;

  /** @brief Retrieve class name of registered class.
      @param _id The id of the class.
      @return The name of the class or NULL. Don't delete the returned string.
    */
  const char*
    ZCom_getClassName( ZCom_ClassID _cid ) const;

  /** @brief Process incoming data.
      @param _block If set to eZCom_Block, this method will block until data arrives.
             eZCom_NoBlock will make the method return immediately if no new data is present.

      Must be called regularly, checks for new input data and handles it.
  */
  void
    ZCom_processInput( eZCom_BlockMode _block = eZCom_NoBlock );


  /**
   * Process replicators.
   * @param _simulation_time_passed Time in milliseconds of how much
   *        simulation time passed since this method has been called
   *        last.
   *
   * This calls the Process() method of all replicators that need it. If not called
   * regularly, no inter- or extrapolation will happen.
   *
   * Currently only ZCom_Replicate_Movement makes use of the _simulation_time_passed parameter.
   *
   * @attention The time parameter should really be <b>simulation time</b> and <b>not real time</b>.
   *            Example: <ul>
   *                <li>the physics update rate is 50Hz = one update every 20 ms</li>
   *                <li>velocity of an object is 1/update => 50 units/second</li>
   *                <li>if for some reason the game only manages 20 updates in one second
   *                    (multitasking comes to mind), then the simulation time in that second
   *                    was not 1000 msecs, but only 20 updates * 20 msecs simulation time each = 400 msecs simulation time</li>
   *                <li>the object then actually moved only 20 units, but Zoidcom thinks it should have
   *                    moved 50</li>
   *                <li>which gives very strange effects in the ZCom_Replicate_Movement replicator
   *                    and all others which depend on proper timing</li></ul>
   *
   *             So the parameter should tell the time that has really passed ingame, not the time
   *             that passed in the system clock.
   */
  void
    ZCom_processReplicators( zU32 _simulation_time_passed );

  /** @brief Prepare and send updates.

      Must be called regularly.
  */
  void
    ZCom_processOutput();

  /** @brief Create a new ZCom_BitStream object.
      \return Pointer to a new ZCom_BitStream object. The object is allocated in the shared
              library's memory space, so you better not delete it yourself, rather use the provided
              ZCom_deleteBitStream() method instead.

      \deprecated Use 'new ZCom_BitStream' instead.
  */
  static ZCom_BitStream*
    ZCom_createBitStream();

  /** @brief Delete a ZCom_BitStream object.
      @param _bs Pointer to a ZCom_BitStream object. The object was allocated in the shared library's
                 memory space, so you better not delete it yourself, rather use this method instead.

      \deprecated Use 'delete bs' instead.
  */
  static void
    ZCom_deleteBitStream(ZCom_BitStream* _bs);

  /************************/
  /*   connection stuff   */

  /* The connection available after a successful connect is
     just a bare-boned low-level connection. It is capable of sending
     and receiving normal, self-made ZCom_BitStream objects.
  */

  /** \name Connection handling.
  */
  //@{

  /** @brief Initiate connection to other ZCom_Control.
      @param _target Address of target ZCom_Control
      @param _request You can pass additional data along the connection request. The other side may
                      accept or reject the connection according to this data. Pass NULL if you don't
                      need this. The ZCom_BitStream object should not be used any longer, Zoidcom will
                      work with it and delete it eventually.
      @returns A ZCom_ConnID != ZCom_Invalid_ID if the connection process was initiated successfully.
               Otherwise ZCom_Invalid_ID is returned.

      We will be classified as client on this connection (since we requested it).
  */
  ZCom_ConnID
    ZCom_Connect( const ZCom_Address &_target, ZCom_BitStream *_request );

  /** @brief Request the quality of incoming traffic.
      @param _id ZCom_ConnID of the connection.
      @param _pps Packets/second we would like to receive.
      @param _bpp Bytes/packet we would like to receive.

      The remote peer will try to accommodate this request to it's best ability, taking into account the
      remote upstream limit.
  */
  void
    ZCom_requestDownstreamLimit( ZCom_ConnID _id, zU16 _pps, zU16 _bpp );

  /** @brief Find other ZCom_Controls by sending a broadcast into the LAN.
      @param _address Where to look: Relevant are only control id and port.
      @param _request Additional data you want to send along with the request.
                      Can be NULL. Zoidcom will delete the bitstream eventually.
      @returns true If broadcast could be sent.

      The call is nonblocking. Each valid answer from the network will generate
      a call to ZCom_cbDiscovered(). ZCom_processInput() has to be called in order
      to generate the aforementioned callback. Only ZCom_Controls listening on the
      correct port and owning the correct control ID will answer. The to-be-discovered
      ZCom_Control needs to activate discover listening with ZCom_Control::setDiscoverListener(),
      and needs to have the callback ZCom_cbDiscoverRequest() implemented. It also
      needs to have it's ZCom_processInput() called regularly to generate
      the aforementioned callback.
  */
  bool
    ZCom_Discover( const ZCom_Address &_address , ZCom_BitStream *_request);

  /** @brief Send disconnect request.
      @param _id Id of the connection you want to disconnect.
      @param _reason Additional data to send alongside this request, or NULL.
      @returns true if no problems occured.

      \note You have to call ZCom_processInput()/output() until ZCom_cbConnectionClosed() is called.
            Otherwise it is not ensured that the disconnect request and reason are properly received
            by connected peers.
  */
  bool
    ZCom_Disconnect( const ZCom_ConnID  _id, ZCom_BitStream *_reason );

  /** @brief Request disconnect for all connections.
      @param _reason Additional data to send alongside this request, or NULL.

      \note You have to call ZCom_processInput()/output() until ZCom_cbConnectionClosed() is called.
            Otherwise it is not ensured that the disconnect request and reason are properly received
            by connected peers.
  */
  void
    ZCom_disconnectAll( ZCom_BitStream *_reason );

  /** @brief Returns this control's connection group manager.
  */
  ZCom_ConnGroupManager&
    ZCom_getGroupManager();

  /** @brief Simple way to send some data.
      @param _id The connection's ID.
      @param _stream BitStream allocated by ZCom_createBitStream().
      @param _mode Send reliable or unreliable.
      @returns 'true' if no error occured.

      Streams are held in a FIFO queue and transmitted with the next outgoing packet,
      i.e. this will hold back all regular(zoidmode) data packets until the FIFO is empty again.
      => Use of this data transmission method is not recommended, especially if the connection is
         in Zoidmode.

      Zoidcom will take over the stream and delete it eventually, so it has to be allocated with
      ZCom_createBitStream(), furthermore it should not be used or accessed in any way after that.

      Also note that Zoidcom is currently not splitting the datastream up into several smaller ones.
      If the stream holds 100k, a 100k sized UDP packet will be sent.

      The destination will receive the data with the ZCom_cbDataReceived() callback.
  */
  bool
    ZCom_sendData( const ZCom_ConnID _id, ZCom_BitStream *_stream, eZCom_SendMode _mode = eZCom_ReliableOrdered );

  /** @brief Send data to a group of connections.
      @param _gid Group ID acquired by group manager. (see ZCom_getGroupManager())
      @param _stream The data stream.
      @param _mode Sendmode.

      Hint: Use @ref ZCOM_CONNGROUP_ALL as group id to send data to all current connections.

      For more verbose information look at ZCom_sendData().
  */
  bool
    ZCom_sendDataToGroup( const ZCom_GroupID _gid, ZCom_BitStream *_stream, eZCom_SendMode _mode );

  /** @brief Send raw UDP datagram to some address.
      @param _dest Destination address.
      @param _data Pointer to block of data to send. Zoidcom will only read from it. No deletion will happen.
      @param _size Size of datablock.
      @returns true on success.

      This sends a packet of raw data to the specified address, through the UDP socket initialized with
      ZCom_initSockets().

      If the address is not resolved to IP yet, the function will block and try to resolve the IP (with 
      timeout of 2 seconds). That means the function might block for two seconds, if it gets an unresolved 
      address. You can resolve the hostname manually (and asychronously) by calling ZCom_Address::resolveHostname() 
      before passing the address to this function. Or specify an IP directly instead of hostname.
  */
  bool
    ZCom_sendDataRaw( ZCom_Address& _dest, void* _data, zU32 _size);

  /** @brief Bring connection into Zoidmode.
      @param _id The connection's ID.
      @param _level The Zoidlevel you want the connection to enter (1-127) or 0 to leave Zoidmode.
      @returns 'true' if the migration process could be started.

      After a connection has been successfully set up, you can upgrade it to a fully integrated
      and automated Zoidcom connection. When this process finishes, the callback method
      ZCom_cbZoidResult() will be called.

      Furthermore it is possible to have different levels in Zoidmode. You can create ZCom_Nodes
      and tell them only to be active in specific levels. This is essentially a filter for ZCom_Nodes.
      A connection on a level only receives ZCom_Nodes registered at the same level.

      One connection can be in only one Zoidlevel, but one ZCom_Node can be registered for more
      than one.

      This works on client and server. When called from server, the ZCom_cbZoidRequest() callback
      won't be called.
  */

  bool
    ZCom_requestZoidMode( const ZCom_ConnID _id, zU8 _level );

  //@}

  /* ----- */

  /** @brief Retrieve node by unique net id.
      @param _nid The ID.
      @returns Pointer to a ZCom_Node object, or NULL if not found.
  */
  ZCom_Node*
    ZCom_getNode( ZCom_NodeID _nid ) const;

  /** @brief Get remoteaddress of connection.
      @param _id The connection's ID.
      @returns Pointer to a const ZCom_Address* object, or NULL on error.

      Don't write to the ZCom_Address structure.
  */
  const ZCom_Address*
    ZCom_getPeer( ZCom_ConnID _id ) const;

  /** @brief Get connection statistics.
      @param _id The connection's ID.
      @returns Reference to a ZCom_ConnStats object. If the connection was not found,
               the ZCom_ConnStats structure will be zeroed out.
  */
  const ZCom_ConnStats&
    ZCom_getConnectionStats( ZCom_ConnID _id ) const;

  /** @brief Attach userdata to a connection.
      @param _id The connection's ID.
      @param _data Pointer to your data.

      Your data will never be changed or deleted by Zoidcom.
  */
  void
    ZCom_setUserData( ZCom_ConnID _id, void *_data );

  /** @brief Retrieve userdata from a connection.
      @param _id The connection's ID.
      @returns void* pointer to your data or NULL if either connection was not found or the connection
               had no data attached.
  */
  void*
    ZCom_getUserData( ZCom_ConnID _id ) const;

  /**
   * Get Zoidcom's time.
   * @returns Current time in milliseconds.
   *
   * Timestamps are in ms and are also used by ZCom_ConnStats.
   */
  static zU32
    ZCom_getCurrentTime();

  /**
   * @name Debug aids.
  */
  //@{

  /**
   * Set the amount of lag that should be simulated for this connection.
   * @param _id Connection for which the lag should be enabled. 0 for all connections.
   * @param _lagmsec Amount of lag in milliseconds.
   * @note The lag will be applied to outgoing packets only. If bidirectional lag is desired,
   *       it has to be enabled on the remote ZCom_Control as well.
   */
  void
    ZCom_simulateLag( ZCom_ConnID _id, zU32 _lagmsec );

  /**
   * Set the amount of packets that should get dropped.
   * @param _id Connection for which the packetloss should be enabled. 0 for all connections.
   * @param _amount Chance of loss between 0-1.
   * @note The loss will be applied to outgoing packets only. If bidirectional loss is desired,
   *       it has to be enabled on the remote ZCom_Control as well.
   */
  void
    ZCom_simulateLoss( ZCom_ConnID _id, zFloat _amount );

  //@}

  /******************************************************/
  /*                     callbacks                      */
  /*   scroll down for callbacks in copy&paste format   */

  /** \name Callbacks
   * @remarks Maybe you wonder why it is necessary to implement all the callbacks in a derived class although some of
   * them are not used. It is possible, and in fact was like that in older versions of Zoidcom, that the empty callback
   * bodies were defined in %ZCom_Control and thus it was not necessary to implement them all in order to get a compilable
   * class. This had one big drawback: If a callback method was misspelled in the derived class or a parameter was wrongly
   * defined, the compiler would happily compile the derived class. Using the misspelled method signature to create a new
   * method, instead of overwriting the callback from the parent class, the program stops working as intended, without any
   * compiler warning or error at all. Forcing the application to provide all callbacks by making them pure virtual in
   * ZCom_Control makes the compiler throw errors whenever one of the methods is missing. Not providing empty implementations
   * in %ZCom_Control aids in early error detection and helps avoiding time eating debug sessions caused by simple typos -
   * for the price of slightly larger and uglier class definitions.
   */

  //@{

  /** @brief Callback: connection process finished. (appears on: Client)
      @param _id The connection's ID
      @param _result The result of the connection process.
      @param _reply What the destination gave as _reply parameter to ZCom_cbConnectionRequest().

      Overload this to get notified about finished connection processes you have initiated.
  */
  virtual void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) = 0;

  /** @brief Callback: incoming connection. (appears on: Server)
      @param _id The connection's ID
      @param _request What the requester has given as _request parameter to ZCom_Connect()
      @param _reply Data you want to transmit to the requester additionally to the yes/no reply.
      @returns The callback should return 'true' if you want to accept the connection, 'false' to deny it.
  */
  virtual bool ZCom_cbConnectionRequest( ZCom_ConnID  _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ) = 0;

  /** @brief Callback: a granted, incoming connection has been fully set up. (appears on: Server)
      @param _id The connection's ID.
  */
  virtual void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) = 0;

  /** @brief Callback: connection has been closed and is about to be deleted (appears on: Server, Client)
      @param _id The connection's ID.
      @param _reason Reason code. If reason is \ref eZCom_ClosedDisconnect, then _reasondata might contain more info.
      @param _reasondata What the disconnecter gave as _reason parameter to ZCom_Disconnect() or ZCom_disconnectAll()
  */
  virtual void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata ) = 0;

  /** @brief Callback: a client requests to enter a specified ZoidLevel. (appears on: Server)
      @param _id The connection's ID.
      @param _requested_level Level the client wants to enter.
      @param _reason Fill this with additional data you want to transmit to the client. _reason is
                     only transmitted when 'false' it returned.
      @returns The method should return 'true' if you want to accept the request and 'false' otherwise.

      Both client(requester) and server(this) will get their ZCom_cbZoidResult() callbacks called, including the data
      you filled into the _reason bitstream.
  */

  virtual bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason ) = 0;

  /** @brief Callback: Zoidlevel migration process finished (appears on: Server, Client)
      @param _id The connection's ID.
      @param _result Result of the migration process.
      @param _new_level New Zoidlevel of the connection (on failure, the previous Zoidlevel is passed here)
      @param _reason Bitstream containing additional data passed by the server in ZCom_cbZoidRequest() or, if the
                     server accepted the new Zoidlevel, but the migration process failed while syncing a ZCom_Node,
                     _reason contains the data passed by that node in ZCom_Node::setSyncResult().

      Called when a connection changed zoidmode or failed to do so.
      Called both, on server and client. The client is the one, who requested the Zoidlevel.
  */
  virtual void ZCom_cbZoidResult( ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason ) = 0;

  /** @brief Callback: server requests us to create a new node for a new dynamic object/node. (appears on: Client)
      @param _id The connection's ID.
      @param _requested_class Class ID of the requested node.
      @param _announcedata  Data that was set by the server with ZCom_Node::setAnnounceData(). May be NULL.
                            If non NULL, the callback must advance the stream's read position by the exact
                            size of the data given to setAnnounceData().
			@param _role Role the newly created node will possess.
      @param _net_id The network ID of the requested node. Not needed directly here but may be useful. It can also be
                     retrieved with ZCom_Node::getNetworkID()

       Called whenever a server requests us to create a new node for a new object.
       The application MUST create a node for this class, which is identical to other nodes of the
       same class on the server. That means: It must have exactly the same replication data setup. Create the
       node, register the variables for replication, and register the node with ZCom_Node::registerRequestedNode().
       If you fail to do so, Zoidcom will be unable to extract incoming data streams any further and disconnect.
       The _net_id parameter tells us the id which is later been returned by ZCom_Node::getNetworkID().
  */

  virtual void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata,
                                           eZCom_NodeRole _role, ZCom_NodeID _net_id ) = 0;

  /** @brief Callback: server requests us to create a new local node for a remote tagnode (appears on: Client)
      @param _id The connection's ID.
      @param _requested_class Class ID of the requested node.
      @param _announcedata  Data that was set by the server with ZCom_Node::setAnnounceData(). May be NULL.
                            If non NULL, the callback must advance the stream's read position by the exact
                            size of the data given to setAnnounceData().
			@param _role Role the newly created node will possess.
      @param _tag The tag that was specified when the node was created on server.

      Called whenever the server registered a tagnode which could not be found on the client, yet. You have to create
      that node here and register it with ZCom_Node::registerNodeByTag(). If you don't, Zoidcom will disconnect.
  */

  virtual void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata,
                                       eZCom_NodeRole _role, zU32 _tag ) = 0;

  /** @brief Callback: direct data has been received. (appears on: Server, Client)
      @param _id The connection's ID.
      @param _data The data that has been received.

      Called when data has been received ( sent by ZCom_sendData() )
  */
  virtual void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) = 0;

  /** @brief Callback: another ZCom_Control has sent a discover request. (appears on: Server, Client)
      @param _addr The address where the request came from.
      @param _request Additional application provided data to the request.
      @param _reply Additional data the application wants to transmit to the discoverer.
      @returns The method should return 'true' if you want to reply, 'false' otherwise.

      For more information see ZCom_Discover().
  */

  virtual bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply) = 0;

  /** @brief Callback: another ZCom_Control responded to our ZCom_Discover(). (appears on: Server, Client)
      @param _addr The address where the discovered ZCom_Control is reachable.
      @param _reply Data that ZCom_cbDiscoverRequest() has written to _reply.

      For more information see ZCom_Discover().
  */
  virtual void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply ) = 0;
//@}

/* ****** callbacks for copy&pasting into your derived classes ******

  void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) {}
  bool ZCom_cbConnectionRequest( ZCom_ConnID  _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ){return false;}
  void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) {}
  void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata ) {}
  bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason ) {return false;}
  void ZCom_cbZoidResult( ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason ) {}
  void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, ZCom_NodeID _net_id ) {}
  void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, zU32 _tag ) {}
  void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) {}
  bool ZCom_cbDiscoverRequest( const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply ) {return false;}
  void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
*/
};


#endif
