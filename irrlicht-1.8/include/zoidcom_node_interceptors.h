/****************************************
* zoidcom_node_interceptors.h
* interceptor interface
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDNODEINTERCEPT_H_
#define _ZOIDNODEINTERCEPT_H_

#include "zoidcom.h"

// forward
class ZCom_Replicator;

/** \file zoidcom_node_interceptors.h
*/

/** @brief Interface for intercepting node events.
    
    An application must derive all methods provided in this interface and register the instance
    of this class with the node in question. 

    Returning 'false' in one of these callbacks does not influence the behaviour of the node in
    any way. The only difference is that the event, for which 'false' is returned, won't be
    available through the normal event interface(ZCom_Node::getNextEvent()) anymore. So if you
    prefer to handle node events through callbacks instead of polling, register an event interceptor 
    and return 'false' in all of the callbacks. If you return 'true', the events will stack
    up in the node's eventqueue until the node is deleted or the events are fetched with 
    ZCom_Node::getNextEvent().
*/
class ZCom_NodeEventInterceptor
{
public:
	ZCOM_API virtual ~ZCom_NodeEventInterceptor() {}
	
  /** @brief An application generated event has been received.
      @param _node The node which received the event.
      @param _from The sender's connection ID.
      @param _remoterole Role of the sending node.
      @param _data The actual event data.
      @param _estimated_time_sent gives an msec value trying to estimate the time the event has left the remote 
                                  ZCom_Control. Execute "ZoidCom::getTime() - _estimated_time_sent" to find out the event's 
                                  travel time in msecs.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.
 */
  ZCOM_API virtual bool 
    recUserEvent(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_BitStream &_data, zU32 _estimated_time_sent) = 0;

  /** @brief A remote node has linked up with our node.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the newly connected node.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      This event is only received if it has been enabled with ZCom_Node::setEventNotification(). For further
      information refer to eZCom_EventInit.
  */
  ZCOM_API virtual bool 
    recInit(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  /** @brief A remote node has linked up with our node and ZCom_Node::setMustSync() is enabled.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the newly connected node.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventSyncRequest.
  */
  ZCOM_API virtual bool 
    recSyncRequest(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  /** @brief A remote node has unlinked from our node.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the removed node.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventRemoved.
  */
  ZCOM_API virtual bool 
    recRemoved(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) = 0;

  /** @brief A remote node wants to send a file.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the sending node.
      @param _fid ID of the filetransfer.
      @param _request Additional data submitted by the sending node.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventFile_Incoming.
  */
  ZCOM_API virtual bool 
    recFileIncoming(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid, ZCom_BitStream &_request) = 0;

  /** @brief The node received a chunk of filedata.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the sending node.
      @param _fid ID of the filetransfer.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventFile_Data.
  */
  ZCOM_API virtual bool 
    recFileData(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;

  /** @brief The node received a filetransfer abortion notification.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the sending node.
      @param _fid ID of the filetransfer.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventFile_Aborted.
  */
  ZCOM_API virtual bool 
    recFileAborted(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;

  /** @brief The node received a filetransfer completion notification.
      @param _node The node which received the event.
      @param _from The originator's connection ID.
      @param _remoterole Role of the sending node.
      @param _fid ID of the filetransfer.
      @returns The callback should return 'true' if it wants to forward the event into the node's incoming
               event queue. If 'false' is returned the event will be discarded immediately.

      For further information refer to eZCom_EventFile_Complete.
  */
  ZCOM_API virtual bool 
    recFileComplete(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) = 0;

  /* callback block for copy & pasting into your derived class
    bool recUserEvent(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_BitStream &_data, zU32 _estimated_time_sent) {return true;}
    bool recInit(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) {return true;}
    bool recSyncRequest(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) {return true;}
    bool recRemoved(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole) {return true;}
    bool recFileIncoming(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid, ZCom_BitStream &_request) {return true;}
    bool recFileData(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) {return true;}
    bool recFileAborted(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) {return true;}
    bool recFileComplete(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remoterole, ZCom_FileTransID _fid) {return true;}
  */
};

/** @brief Interface for intercepting node replications.
    
    An application must derive all methods provided in this interface and register the instance
    of this class with the node in question.
*/
class ZCom_NodeReplicationInterceptor
{
public:
	ZCOM_API virtual ~ZCom_NodeReplicationInterceptor() {}
	
  /** @brief The node is about to be replicated to a remote control.
      @param _node The node in question.
      @param _to   Connection ID of destination.
      @param _remote_role Role of destination node.
  */
  ZCOM_API virtual void 
    outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  /** @brief The node is about to be removed from a remote control.
      @param _node The node in question.
      @param _to   Connection ID of destination.
      @param _remote_role Role of destination node.

      This won't get called if the node is deleted already.
  */
  ZCOM_API virtual void 
    outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  /** @brief The node is about to be evaluated for sending data updates.
      @param _node The node in question.
      @param _to   Connection ID of destination.
      @param _remote_role Role of destination node.
      @returns Should return 'false' to prevent sending updates now, 'true' otherwise.
  */
  ZCOM_API virtual bool 
    outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) = 0;

  /** @brief The node is about to include a specific replication item in an update.
      @param _node The node in question.
      @param _to   Connection ID of destination.
      @param _remote_role Role of destination node.
      @param _replicator Pointer to the source replicator.
      @returns Callback should return 'false' to prevent inclusion in the update.

      This callback only gets called if the replication item really is about to be updated 
      now. That means, if the value did not change or is not allowed to update due to 
      mindelay constraints, this callback won't get called. You can change the value of
      the original data in this callback, and the changed data will get sent. 
      
      The intercept id of the replicator can be retrieved with
      @code
      _replicator->getSetup()->getInterceptID();
      @endcode
  */
  ZCOM_API virtual bool 
    outPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator) = 0;

  /** @brief The node has included it's update into the current packet.
      @param _node The node in question.
      @param _to   Connection ID of destination.
      @param _remote_role Role of destination node.
      @param _rep_bits Amount of payload bits in replication update.
      @param _event_bits Amount of payload bits in event data.
      @param _meta_bits Amount of bits of control data for the complete node update.

      The total amount of bits used is _rep_bits + _event_bits + _meta_bits.
  */
  ZCOM_API virtual void 
    outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) = 0;

  /** @brief Incoming data is about to be written into the replication data fields.
      @param _node The node in question.
      @param _from Connection ID of source.
      @param _remote_role Role of remote sender.
      @returns Should return 'false' to prevent applying the replication data updates.
               Events will be received nevertheless.
  */
  ZCOM_API virtual bool 
    inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role) = 0;

  /** @brief Incoming data is about to be written into a specific replication data field.
      @param _node The node in question.
      @param _from Connection ID of source.
      @param _remote_role Role of remote sender.
      @param _replicator Pointer to the target replicator.
      @param _estimated_time_sent gives an msec value trying to estimate the time the data has 
                                  left the remote ZCom_Control. Execute "ZoidCom::getTime() - 
                                  _estimated_time_sent" to find out the data's travel time in msecs.
      @returns Should return 'false' to prevent applying the replication data update.
      
      The intercept id of the replicator can be retrieved with
      @code
      _replicator->getSetup()->getInterceptID();
      @endcode
  */
  ZCOM_API virtual bool 
    inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent) = 0;

  /** @brief Incoming data has updated the node.
      @param _node The node in question.
      @param _from Connection ID of source.
      @param _remote_role Role of remote sender.
      @param _rep_bits Amount of replication data bits received.
      @param _event_bits Amount of event bits received.
      @param _meta_bits Amount of bits of control data for the complete node update.

      The total amount of bits used is _rep_bits + _event_bits + _meta_bits.
  */
  ZCOM_API virtual void 
    inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) = 0;

  /* callback block for copy & pasting into your derived class
    void outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {}
    void outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {}
    bool outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) { return true; }
    bool outPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator) { return true; }
    void outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {}
    bool inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role) { return true; }
    bool inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent) { return true; }
    void inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {}
  */
};

#endif
