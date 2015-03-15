/****************************************
* zoidcom_replicator_advanced.h
* advanced replicator definition
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDREPLICATORADVANCED_H_
#define _ZOIDREPLICATORADVANCED_H_

/** \file zoidcom_replicator_advanced.h
*/

#include "zoidcom.h"

class ZCom_Node;

/**
 * @brief Interface for advanced replicators which need high level of control.
 *
 * As opposed to basic replicators (ZCom_ReplicatorBasic) which are aided by Zoidcom a lot,
 * the advanced replicator interface allows replicators to fully control when data has to
 * be sent, to whom, in which manner and how often. This results in extremely high flexibility
 * and on the other hand requires more work to get it working.
 *
 * One restriction remains, though. The replicator cannot surpass the owning node's priority.
 * It is only able to send when the owning node is allowed to send.
 *
 * @note Unlike the basic replicators, advanced replicators have to perform timing on their own.
 *       That means, the min- and maxdelay parameters in ZCom_ReplicatorSetup are not enforced
 *       by Zoidcom for advanced replicators. Use getLastUpdateTime(), or the _lastupdate parameter
 *       to onPreSendData() to handle this.
 *
 * @see @ref SyncCustom
 */
class ZCOM_API ZCom_ReplicatorAdvanced : public ZCom_Replicator
{
protected:
  ZCom_Node* m_node;
public:
  ZCom_ReplicatorAdvanced(ZCom_ReplicatorSetup *_setup);

  /**
   * @brief Get the node in which this replicator is registered.
   * @return A pointer to the node.
   */
  ZCom_Node* getNode() const;

  /**
   * @brief Get pointer to time of last update to given connection.
   * @param _cid Id of connection.
   * @return Pointer(!!) to time in msecs.
   *
   * This method should be used for implementing min- and maxdelay enforcement.
   *
   * If the result is non-NULL, writing to this pointer is encouraged, as advanced
   * replicators have to manage timing on their own.
   *
   * @attention This is only available (i.e. non-NULL) if at least one replicator 
   *            in the node has a mindelay or maxdelay activated.
   *            If this is not the case, the method returns NULL.
   *
   * Code example:
   * @code
   * zU32* lastupdate = getLastUpdateTime(id_of_connection_which_is_currently_processed);
   * if (lastupdate) {
   *   if (ZoidCom::getTime() - *lastupdate < m_setup->getMinDelay())
   *     // don't send data if last update to this happened too recent
   *     return;
   *   else
   *     // store the current time because an update gets sent now
   *     *lastupdate = ZoidCom::getTime();
   * }
   * @endcode
   */
  zU32* getLastUpdateTime(ZCom_ConnID _cid);

  /**
   * @brief Sends an event to peer replicators.
   * @param _mode \ref eZCom_SendMode. eZCom_ReliableOrdered is not supported in this context.
   * @param _stream The event data.
   * @param _reference_id If send mode is eZCom_UnreliableNotify, this id will be given back to the application
   *                      through onDataLost() or onDataAcked().
   *
   * This event will be received by all replicators which normally receive data from this replicator, too.
   * If this replicator is owned by an authority node and the replicator is flagged ZCOM_REPFLAG_AUTH_2_PROXY,
   * all replicators owned by a proxy of this replicators node will receive this event.
   *
   * In order to handle the event, overload ZCom_ReplicatorAdvanced::onDataReceived().
   *
   * Replicator events are sent reliable unordered or unreliable.
   *
   * @attention Make sure that onDataReceived() reads the same amount of bits sendEvent() has written,
   *            otherwise the stream can't be extracted any further.
   *
   * @attention Please note that calling sendData() won't ensure the data is sent immediately. The data
   *            will be sent as soon as the owning node's priority allows the node to send. If you want
   *            to generate data which is sent immediately, wait for the onPreSendData() callback.
   */        
  void sendData(eZCom_SendMode _mode, ZCom_BitStream *_stream, zU32 _reference_id = 0);

  /**
   * @brief Same as sendData(), but with single destination.
   * @param _mode \ref eZCom_SendMode. eZCom_ReliableOrdered is not supported in this context.
   * @param _dest Connection ID of destination.
   * @param _stream The event data.
   * @param _reference_id If send mode is eZCom_UnreliableNotify, this id will be given back to the application
   *                      through onDataLost() or onDataAcked().
   *
   * In order to handle the event, overload ZCom_ReplicatorAdvanced::onDataReceived().
   *
   * Replicator events are sent reliable unordered or unreliable.
   *
   * @attention Make sure that onDataReceived() reads the same amount of bits sendEvent() has written,
   *            otherwise the stream can't be extracted any further.
   *
   * @attention Please note that calling sendData() won't ensure the data is sent immediately. The data
   *            will be sent as soon as the owning node's priority allows the node to send. If you want
   *            to generate data which is sent immediately, wait for the onPreSendData() callback.
   */
  void sendDataDirect(eZCom_SendMode _mode, ZCom_ConnID _dest, ZCom_BitStream *_stream, zU32 _reference_id = 0);

  /**
   * @brief Zoidcom is about to transmit data to the given client.
   * @param _cid Connection ID of destination connection.
   * @param _remoterole Role of remote node that will receive the data.
   * @param _lastupdate Time of last sent update to this connection. The replicator implementation 
   *                    is responsible for updating that value when it sends data. This is only 
   *                    available (i.e. non-NULL) if at least one replicator in the node has a mindelay 
   *                    or maxdelay activated, otherwise this parameter is NULL.
   *
   * Sending data from inside this callback via sendData() or sendDataDirect() will result in immediate 
   * transmission of sent data to the client.
   *
   * Example code for _lastupdate usage:
   * @code
   * if (_lastupdate) {
   *   // don't update
   *   if (ZoidCom::getTime() - *_lastupdate < m_setup->getMinDelay())
   *     return;
   *
   *   // after update was sent, store the current time
   *   *_lastupdate = ZoidCom::getTime();
   * }
   * @endcode
   */
  virtual void onPreSendData(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, zU32 *_lastupdate) = 0;

  /**
   * @brief Data has been received from the replicator of a remote node.
   * @param _cid Connection ID of source.
   * @param _remoterole Role of remote node whose replicator has sent the data.
   * @param _stream The data that has been sent.
   * @param _store This is false if a node replication interceptor denied to accept the data. This method
   *               still has been called because it is necessary to forward the bitstream, otherwise
   *               the rest of the data in the packet becomes unusable.
   * @param _estimated_time_sent Estimated time in msecs for when the data was originally sent. The time
   *                             is comparable to the time returned by ZoidCom::getTime().
   *
   * @attention Make sure that this method forwards the bitstream by the exact amount of bytes originally sent
   *            by sendData() or sendDataDirect().
   * @attention The m_node/getNode() might return NULL when the parent node got deleted but data is received after that,
   *            so check for getNode() == NULL before calling anything on the node. This is only true for this callback,
   *            other callbacks are safe.
   *
   */
  virtual void onDataReceived(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, ZCom_BitStream &_stream, bool _store,
                                       zU32 _estimated_time_sent) = 0;

  virtual void onDataAcked(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data) = 0;
  virtual void onDataLost(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data) = 0;

  /**
   * @brief The given connection has received a packet.
   * @param _cid Connection ID of connection that received a packet.
   *
   * This will only get called for connections whose node is linked to this replicator's owning node.
   * It is used by ZCom_MovementReplicator to synchronize the extrapolation. 
   */
  virtual void onPacketReceived(ZCom_ConnID _cid) = 0;

  /**
   * @brief A node has become relevant on the specified connection and thus to this replicator, too.
   * @param _cid Id of the connection with the node.
   * @param _remoterole Role of the remote node.
   */
  virtual void onConnectionAdded(ZCom_ConnID _cid, eZCom_NodeRole _remoterole) = 0;

  /**
   * @brief The node on this connection is no longer relevant.
   * @param _cid Id of connection.
   * @param _remoterole Role of remote node.
   */
  virtual void onConnectionRemoved(ZCom_ConnID _cid, eZCom_NodeRole _remoterole) = 0;

  /**
   * @brief The role of the local node which owns this replicator has changed.
   * @param _oldrole Old role of the node.
   * @param _newrole New role of the node.
   */
  virtual void onLocalRoleChanged(eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole) = 0;

  /**
   * @brief The role of a node on a remote connection has changed it's role.
   * @param _cid Id of connection.
   * @param _oldrole Old role of remote node.
   * @param _newrole New role of remote node.
   */
  virtual void onRemoteRoleChanged(ZCom_ConnID _cid, eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole) = 0;

  /**
   * @brief Set the node in which this replicator is registered.
   * @param _node The node pointer.
   *
   * This will get called automatically by ZCom_Node::addReplicator(). There is normally no reason
   * to call this method.
   */
  void setNode(ZCom_Node *_node);
};

#endif
